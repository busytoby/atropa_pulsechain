import os
import re
import datetime
from reportlab.lib.pagesizes import letter
from reportlab.lib.units import inch
from reportlab.platypus import BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, PageBreak, FrameBreak, Table, TableStyle, Image, NextPageTemplate, KeepTogether, Flowable, Preformatted, BalancedColumns, CondPageBreak
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_JUSTIFY, TA_CENTER, TA_LEFT

class ACMDocTemplate(BaseDocTemplate):
    def handle_pageBegin(self):
        template_id = 'even_page_1col' if self.page % 2 == 0 else 'odd_page_1col'
        self._handle_nextPageTemplate(template_id)
        super().handle_pageBegin()
from reportlab.lib import colors
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

# Register system LiberationSerif fonts under the standard Times-Roman names.
# This forces ReportLab to fully embed these TTF files in the PDF, solving the KDP font embedding requirement.
pdfmetrics.registerFont(TTFont('Times-Roman', '/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf'))
pdfmetrics.registerFont(TTFont('Times-Bold', '/usr/share/fonts/truetype/liberation/LiberationSerif-Bold.ttf'))
pdfmetrics.registerFont(TTFont('Times-Italic', '/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf'))
pdfmetrics.registerFont(TTFont('Times-BoldItalic', '/usr/share/fonts/truetype/liberation/LiberationSerif-BoldItalic.ttf'))
pdfmetrics.registerFont(TTFont('Courier', '/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf'))
pdfmetrics.registerFont(TTFont('Courier-Bold', '/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf'))
pdfmetrics.registerFont(TTFont('Courier-Oblique', '/usr/share/fonts/truetype/liberation/LiberationMono-Italic.ttf'))
pdfmetrics.registerFont(TTFont('Courier-BoldOblique', '/usr/share/fonts/truetype/liberation/LiberationMono-BoldItalic.ttf'))

def get_file_date(filepath):
    filename = os.path.basename(filepath)
    match = re.search(r'(\d{4})[_-](\d{2})[_-](\d{2})', filename)
    if match:
        try:
            return datetime.datetime(int(match.group(1)), int(match.group(2)), int(match.group(3)))
        except ValueError:
            pass
            
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            for _ in range(15):
                line = f.readline()
                if not line:
                    break
                match_line = re.search(r'(\d{4})[_-](\d{2})[_-](\d{2})', line)
                if match_line:
                    try:
                        return datetime.datetime(int(match_line.group(1)), int(match_line.group(2)), int(match_line.group(3)))
                    except ValueError:
                        pass
    except Exception:
        pass
        
    return datetime.datetime.fromtimestamp(os.path.getmtime(filepath))

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_decorations(num_pages)
            super().showPage()
        super().save()

    def draw_page_decorations(self, page_count):
        self.saveState()
        self.setFont("Times-Roman", 8)
        
        is_even = (self._pageNumber % 2 == 0)
        
        left_header_margin = 0.5 * inch if is_even else 0.75 * inch
        right_header_margin = 5.25 * inch if is_even else 5.5 * inch
        
        if is_even:
            self.drawString(left_header_margin, 8.5 * inch, "LORE COMPENDIUM")
            self.drawRightString(right_header_margin, 8.5 * inch, "AUNCIENT RECORDS")
        else:
            self.drawString(left_header_margin, 8.5 * inch, "AUNCIENT RECORDS")
            self.drawRightString(right_header_margin, 8.5 * inch, "LORE COMPENDIUM")
            
        self.setLineWidth(0.5)
        self.line(left_header_margin, 8.4 * inch, right_header_margin, 8.4 * inch)
        
        # Footer text
        self.line(left_header_margin, 0.6 * inch, right_header_margin, 0.6 * inch)
        page_text = f"Page {self._pageNumber} of {page_count}"
        
        if is_even:
            self.drawString(left_header_margin, 0.4 * inch, page_text)
        else:
            self.drawRightString(right_header_margin, 0.4 * inch, page_text)
            
        self.restoreState()

def convert_latex_math_to_html(text):
    import re
    
    def replace_frac(match):
        num = match.group(1)
        den = match.group(2)
        return f"<sup>{num}</sup>/<sub>{den}</sub>"

    pattern_frac = r'\\frac\{([^}]+)\}\{([^}]+)\}'
    
    replacements = [
        (r'\\theta', 'θ'),
        (r'\\phi', 'φ'),
        (r'\\cdot', '·'),
        (r'\\cos', 'cos'),
        (r'\\sin', 'sin'),
        (r'\\text\{([^}]+)\}', r'\1'),
        (r'\\mathbf\s*([a-zA-Z0-9])', r'<b>\1</b>'),
        (r'\\mathbf\{([^}]+)\}', r'<b>\1</b>'),
        (r'\\mathit\s*([a-zA-Z0-9])', r'<i>\1</i>'),
        (r'\\mathit\{([^}]+)\}', r'<i>\1</i>'),
        (r'\\mathrm\s*([a-zA-Z0-9])', r'\1'),
        (r'\\mathrm\{([^}]+)\}', r'\1'),
        (r'\\pmod\{([^}]+)\}', r' (mod \1)'),
        (r'\\pmod\s*([a-zA-Z0-9])', r' (mod \1)'),
        (r'\\vec\{([^}]+)\}', r'vec(\1)'),
        (r'\\vec\s*([a-zA-Z0-9])', r'vec(\1)'),
        (r'\\arctan2', 'arctan2'),
        (r'\\depth', 'depth'),
        (r'\\quad', '  '),
        (r'\\rightarrow', '→'),
        (r'\\leftarrow', '←'),
        (r'\\to', '→'),
        (r'\\implies', '⇒'),
        (r'\\times', '×'),
        (r'\\approx', '≈'),
        (r'\\propto', '∝'),
        (r'\\pm', '±'),
        (r'\\in', '∈'),
        (r'\\ge', '≥'),
        (r'\\le', '≤'),
        (r'\\neq', '≠'),
        (r'\\infty', '∞'),
        (r'\\partial', '∂'),
        (r'\\sum', '∑'),
        (r'\\Delta', 'Δ'),
        (r'\\hbar', 'ℏ'),
        (r'\\parallel', '∥'),
        (r'\\circ', '°'),
        (r'\\alpha', 'α'),
        (r'\\beta', 'β'),
        (r'\\gamma', 'γ'),
        (r'\\lambda', 'λ'),
        (r'\\mu', 'μ'),
        (r'\\tau', 'τ'),
        (r'\\omega', 'ω'),
        (r'\\Omega', 'Ω'),
        (r'\\Phi', 'Φ'),
        (r'\\pi', 'π'),
        (r'\\psi', 'ψ'),
        (r'\\Sigma', 'Σ'),
        (r'\\epsilon', 'ε'),
        (r'\\eta', 'η'),
        (r'\\sigma', 'σ'),
        (r'\\rho', 'ρ'),
        (r'\\theta', 'θ'),
        (r'\\phi', 'φ'),
        (r'\\begin\{[^}]+\}', ''),
        (r'\\end\{[^}]+\}', ''),
        (r'\\\{', '{'),
        (r'\\\}', '}'),
        (r'\\left', ''),
        (r'\\right', ''),
    ]
    
    def replace_math_block(match):
        math_content = match.group(1)
        if '<' in math_content or '>' in math_content or not re.search(r'[+\-*=/_\\^θφ·]|cos|sin|mathbf|mathit|mathrm|sqrt|pmod', math_content):
            return f"${math_content}$"
            
        for pat, repl in replacements:
            math_content = re.sub(pat, repl, math_content)
        
        # Convert overline using Unicode combining overline character \u0305
        def replace_overline(m):
            return "".join(c + "\u0305" for c in m.group(1))
        math_content = re.sub(r'\\overline\{([^}]+)\}', replace_overline, math_content)
        
        math_content = re.sub(r'_\{([^}]+)\}', r'<sub>\1</sub>', math_content)
        math_content = re.sub(r'_([a-zA-Z0-9]+)', r'<sub>\1</sub>', math_content)
        
        for _ in range(3):
            math_content = re.sub(r'\\sqrt\{([^}]+)\}', r'√(<b>\1</b>)', math_content)
            math_content = re.sub(r'\\sqrt\s*([a-zA-Z0-9])', r'√(<b>\1</b>)', math_content)
            
        for _ in range(3): # up to 3 levels of nesting fractions
            math_content = re.sub(pattern_frac, replace_frac, math_content)
        return f'<font color="#0066cc"><i>{math_content}</i></font>'
        
    text = re.sub(r'\$\$(.*?)\$\$', replace_math_block, text)
    text = re.sub(r'\$(.*?)\$', replace_math_block, text)
    
    # Strip remaining standalone single dollar signs around C64 hex addresses for clean output
    # E.g. $D000 becomes D000
    text = re.sub(r'\$([0-9A-Fa-f]{4})', r'\1', text)
    return text

def inline_md_to_html(text):
    import html
    text = html.escape(text)
    
    # Strip local file:/// links and convert standard web links
    text = re.sub(r'\[([^\]]+)\]\(file:///[^)]+\)', r'\1', text)
    text = re.sub(r'\[([^\]]+)\]\((https?://[^)]+)\)', r'<a href="\2"><font color="#0066cc"><u>\1</u></font></a>', text)
    
    if text.count('`') % 2 != 0:
        text = text.rsplit('`', 1)[0]
        
    parts = text.split('`')
    new_parts = []
    for idx, part in enumerate(parts):
        if idx % 2 == 1:
            wrapped = ""
            for c_idx, char in enumerate(part):
                wrapped += char
                if c_idx > 0 and c_idx % 8 == 0 and c_idx < len(part) - 1:
                    wrapped += "\u00AD"
            new_parts.append(f'<font face="Courier" color="#0077b6"><b>{wrapped}</b></font>')
        else:
            if part.count('**') % 2 == 0:
                bold_parts = part.split('**')
                b_new = []
                for b_idx, b_part in enumerate(bold_parts):
                    if b_idx % 2 == 1:
                        b_new.append(f'<b>{b_part}</b>')
                    else:
                        if b_part.count('*') % 2 == 0:
                            italic_parts = b_part.split('*')
                            i_new = []
                            for i_idx, i_part in enumerate(italic_parts):
                                if i_idx % 2 == 1:
                                    i_new.append(f'<i>{i_part}</i>')
                                else:
                                    i_new.append(i_part)
                            b_new.append(''.join(i_new))
                        else:
                            b_new.append(b_part.replace('*', ''))
                part = ''.join(b_new)
            else:
                part = part.replace('**', '')
            new_parts.append(part)
    res_text = ''.join(new_parts)
    return convert_latex_math_to_html(res_text)

def render_mermaid_flowchart(lines, col_width):
    import re
    from reportlab.graphics.shapes import Drawing, Rect, String, Line, Polygon
    from reportlab.lib import colors
    
    nodes = {}
    links = []
    
    for line in lines:
        line = line.strip()
        if not line or line.startswith('graph'):
            continue
        link_matches = re.findall(r'([A-Za-z0-9_]+)\s*-->\s*(?:\|(.*?)\|\s*)?([A-Za-z0-9_]+)', line)
        for from_id, label, to_id in link_matches:
            links.append((from_id, to_id, label))
            
        node_defs = re.findall(r'([A-Za-z0-9_]+)(?:\[(.*?)\]|\((.*?)\)|\{(.*?)\})', line)
        for nid, txt1, txt2, txt3 in node_defs:
            txt = txt1 or txt2 or txt3
            nodes[nid] = txt.strip()
            
    for from_id, to_id, _ in links:
        if from_id not in nodes:
            nodes[from_id] = from_id
        if to_id not in nodes:
            nodes[to_id] = to_id
            
    if not nodes:
        return None
        
    ordered_nodes = []
    incoming = {nid: set() for nid in nodes}
    for from_id, to_id, _ in links:
        incoming[to_id].add(from_id)
        
    sources = [nid for nid in nodes if not incoming[nid]]
    visited = set()
    
    def visit(nid):
        if nid in visited:
            return
        visited.add(nid)
        ordered_nodes.append(nid)
        for from_id, to_id, _ in links:
            if from_id == nid:
                visit(to_id)
                
    for src in sources:
        visit(src)
    for nid in nodes:
        visit(nid)
        
    sources = []
    sinks = []
    for nid in nodes:
        has_incoming = False
        has_outgoing = False
        for from_id, to_id, _ in links:
            if to_id == nid:
                has_incoming = True
            if from_id == nid:
                has_outgoing = True
        if has_outgoing and not has_incoming:
            sources.append(nid)
        elif has_incoming and not has_outgoing:
            sinks.append(nid)
        else:
            sources.append(nid)
            
    sources = sorted(list(set(sources)))
    sinks = sorted(list(set(sinks)))
    
    box_w = 80
    num_rows = max(len(sources), len(sinks), 1)
    y_step = 55.0
    max_draw_h = 350.0
    if num_rows * y_step + 20 > max_draw_h:
        y_step = (max_draw_h - 20) / float(num_rows)
    draw_h = num_rows * y_step + 20
    box_h = min(24, int(y_step * 0.5))
    
    d = Drawing(col_width, draw_h)
    
    bg_rect = Rect(col_width * 0.52, 10, col_width * 0.44, draw_h - 20, rx=5, ry=5)
    bg_rect.fillColor = colors.HexColor('#161c24')
    bg_rect.strokeColor = colors.HexColor('#222d3b')
    bg_rect.strokeWidth = 0.5
    d.add(bg_rect)
    
    bg_lbl = String(col_width * 0.74, draw_h - 18, "EVM Node (Anvil)", textAnchor='middle')
    bg_lbl.fontName = 'Helvetica-Bold'
    bg_lbl.fontSize = 6
    bg_lbl.fillColor = colors.HexColor('#667788')
    d.add(bg_lbl)
    
    node_coords = {}
    for idx, nid in enumerate(sources):
        x = col_width * 0.26
        y = draw_h - (idx * (draw_h - 30) / max(len(sources) - 1, 1) + 20) if len(sources) > 1 else draw_h / 2
        node_coords[nid] = (x, y)
        
    for idx, nid in enumerate(sinks):
        x = col_width * 0.74
        y = draw_h - (idx * (draw_h - 40) / max(len(sinks) - 1, 1) + 25) if len(sinks) > 1 else draw_h / 2
        node_coords[nid] = (x, y)
        
    for nid, (x, y) in node_coords.items():
        rx = x - box_w / 2
        ry = y - box_h / 2
        rect = Rect(rx, ry, box_w, box_h, rx=3, ry=3)
        rect.fillColor = colors.HexColor('#1e293b')
        rect.strokeColor = colors.HexColor('#0ea5e9')
        rect.strokeWidth = 0.8
        d.add(rect)
        
        text_str = nodes.get(nid, nid)
        if len(text_str) > 22:
            text_str = text_str[:19] + "..."
        s = String(x, y - 2.5, text_str, textAnchor='middle')
        s.fontName = 'Helvetica-Bold'
        s.fontSize = 7
        s.fillColor = colors.HexColor('#ffffff')
        d.add(s)
        
    for from_id, to_id, label in links:
        if from_id in node_coords and to_id in node_coords:
            x1, y1 = node_coords[from_id]
            x2, y2 = node_coords[to_id]
            
            x_start = x1 + box_w / 2
            x_end = x2 - box_w / 2
            
            line = Line(x_start, y1, x_end, y2)
            line.strokeColor = colors.HexColor('#64748b')
            line.strokeWidth = 0.8
            d.add(line)
            
            arrow = Polygon([x_end, y2, x_end - 4, y2 + 3, x_end - 4, y2 - 3])
            arrow.fillColor = colors.HexColor('#64748b')
            arrow.strokeColor = colors.HexColor('#64748b')
            d.add(arrow)
            
            if label:
                x_mid = (x_start + x_end) / 2
                y_mid = (y1 + y2) / 2
                lbl = String(x_mid, y_mid + 2, label.strip(), textAnchor='middle')
                lbl.fontName = 'Helvetica'
                lbl.fontSize = 5.5
                lbl.fillColor = colors.HexColor('#94a3b8')
                d.add(lbl)
                
    return d

def render_standard_code_block(code_lines, col_width, body_style):
    # Dynamically wrap lines that cannot fit even at minimum font size
    min_font = 3.2
    char_w = 0.6
    max_chars = max(15, int((col_width - 8.0) / (min_font * char_w)))
    
    wrapped_lines = []
    for line in code_lines:
        line_strip = line.rstrip('\r\n')
        if len(line_strip) <= max_chars:
            wrapped_lines.append(line)
        else:
            start = 0
            while start < len(line_strip):
                chunk = line_strip[start:start + max_chars]
                if start > 0:
                    wrapped_lines.append("  ↳ " + chunk + "\n")
                else:
                    wrapped_lines.append(chunk + "\n")
                start += max_chars
                
    code_text = ''.join(wrapped_lines)
    from reportlab.platypus import Preformatted
    
    # Calculate the maximum line length of wrapped lines to dynamically scale font size
    max_len = max((len(line.rstrip('\r\n')) for line in wrapped_lines), default=30)
    
    font_size = min(6.0, (col_width - 8.0) / float(max_len * char_w))
    font_size = max(3.2, font_size)
    leading = font_size + 1.2
    
    code_style = ParagraphStyle(
        'CodeBlock',
        parent=body_style,
        fontName='Courier',
        fontSize=font_size,
        leading=leading,
        textColor=colors.HexColor('#222222'),
        backColor=colors.HexColor('#f5f6f8'),
        borderColor=colors.HexColor('#e1e4e8'),
        borderWidth=0.5,
        borderPadding=4,
        spaceBefore=4,
        spaceAfter=4,
    )
    p = Preformatted(code_text, code_style)
    p.raw_text = code_text
    return p



def parse_markdown_table(rows, body_style, col_width):
    has_separator = False
    for r in rows:
        r_strip = r.strip()
        if re.match(r'^\|\s*:?-+:?\s*\|', r_strip):
            has_separator = True
            break
    if not has_separator:
        return None
        
    data = []
    for r in rows:
        r = r.strip()
        if not r or r.startswith('|---') or r.startswith('|:---') or r.startswith('|-'):
            continue
        cells = [c.strip() for c in r.split('|')]
        if cells and cells[0] == '':
            cells = cells[1:]
        if cells and cells[-1] == '':
            cells = cells[:-1]
        
        parsed_row = []
        for cell in cells:
            html_cell = inline_md_to_html(cell)
            parsed_row.append(Paragraph(html_cell, body_style))
        if parsed_row:
            data.append(parsed_row)
            
    if not data:
        return None
        
    num_cols = max(len(row) for row in data)
    col_widths = [col_width / float(num_cols)] * num_cols
    
    for row in data:
        while len(row) < num_cols:
            row.append(Paragraph("", body_style))
            
    # Compute padding based on column count to avoid negative availWidth exceptions
    padding = 2
    if num_cols > 8:
        padding = 0.5
    elif num_cols > 4:
        padding = 1
        
    t = Table(data, colWidths=col_widths)
    t.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.lightgrey),
        ('GRID', (0,0), (-1,-1), 0.5, colors.grey),
        ('VALIGN', (0,0), (-1,-1), 'TOP'),
        ('BOTTOMPADDING', (0,0), (-1,-1), padding),
        ('TOPPADDING', (0,0), (-1,-1), padding),
        ('LEFTPADDING', (0,0), (-1,-1), padding),
        ('RIGHTPADDING', (0,0), (-1,-1), padding),
    ]))
    return t

def process_text_block(text, body_style, col_width, append_func):
    pattern = r'!\[[^\]]*\]\(([^)]+)\)'
    last_idx = 0
    segments = []
    
    for match in re.finditer(pattern, text):
        pre_text = text[last_idx:match.start()].strip()
        if pre_text:
            segments.append(('text', pre_text))
        img_path = match.group(1).strip().split('?')[0]
        segments.append(('image', img_path))
        last_idx = match.end()
        
    post_text = text[last_idx:].strip()
    if post_text:
        segments.append(('text', post_text))
        
    file_pattern = r'\[([^\]]+)\]\(file:///?([^)]+)\)'
    
    for seg_type, val in segments:
        if seg_type == 'image':
            ext = os.path.splitext(val)[1].lower()
            if ext in ['.jpg', '.jpeg', '.png', '.gif', '.bmp'] and os.path.exists(val):
                try:
                    img_flow = Image(val)
                    img_flow.drawWidth = col_width - 10
                    img_flow.drawHeight = 1.5 * inch
                    append_func(Spacer(1, 4))
                    append_func(img_flow)
                    append_func(Spacer(1, 4))
                except Exception as e:
                    print(f"Image error: {e}")
        else:
            t_idx = 0
            # Convert raw file:/// URLs to standard markdown links first to unify embedding logic
            val = re.sub(r'(?<![\(\[])file:///([^\s\)]+)', r'[\1](file:///\1)', val)
            
            matches = list(re.finditer(file_pattern, val))
            if not matches:
                val = re.sub(r'^#+\s*', '', val)
                html_p = inline_md_to_html(val)
                if html_p:
                    append_func(Paragraph(html_p, body_style))
                continue
                
            for f_match in matches:
                pre_f = val[t_idx:f_match.start()].strip()
                if pre_f:
                    pre_f = re.sub(r'^#+\s*', '', pre_f)
                    html_p = inline_md_to_html(pre_f)
                    if html_p:
                        append_func(Paragraph(html_p, body_style))
                
                link_text = f_match.group(1)
                file_path = '/' + f_match.group(2).strip().split('#')[0].lstrip('/')
                
                ext_embed = os.path.splitext(file_path)[1].lower()
                allowed_exts = ['.js', '.py', '.c', '.yul', '.h', '.cpp', '.sh', '.json', '.txt', '.strategy', '.algol61']
                
                if os.path.exists(file_path) and os.path.isfile(file_path) and ext_embed in allowed_exts:
                    try:
                        with open(file_path, 'r', encoding='utf-8') as f_embed:
                            embed_lines = f_embed.readlines()
                        
                        truncated = False
                        if len(embed_lines) > 40:
                            embed_lines = embed_lines[:30]
                            truncated = True
                            
                        caption = Paragraph(f"<b>File Reference: <font face='Courier'>{os.path.basename(file_path)}</font></b>", ParagraphStyle('Cap', parent=body_style, fontName='Times-Bold', fontSize=7.5, spaceBefore=4))
                        
                        max_len = max((len(line.rstrip('\r\n')) for line in embed_lines), default=0)
                        is_wide = (max_len > 55)
                        col_width_1col = 342.0
                        
                        c_flow = render_standard_code_block(embed_lines, col_width_1col if is_wide else col_width, body_style)
                        if c_flow:
                            c_flow.is_wide = is_wide
                            if is_wide:
                                caption.is_wide = True
                            
                            append_func(caption)
                            append_func(Spacer(1, 2))
                            append_func(c_flow)
                            if truncated:
                                trunc_label = Paragraph("<i>... (truncated for compendium)</i>", ParagraphStyle('Trunc', parent=body_style, fontName='Times-Italic', fontSize=6.5))
                                if is_wide:
                                    trunc_label.is_wide = True
                                append_func(trunc_label)
                            append_func(Spacer(1, 4))
                    except Exception as e:
                        print(f"File embedding error: {e}")
                        append_func(Paragraph(f"<u>{link_text}</u>", body_style))
                else:
                    append_func(Paragraph(f"<u>{link_text}</u>", body_style))
                    
                t_idx = f_match.end()
                
            post_f = val[t_idx:].strip()
            if post_f:
                post_f = re.sub(r'^#+\s*', '', post_f)
                html_p = inline_md_to_html(post_f)
                if html_p:
                    append_func(Paragraph(html_p, body_style))

def build_volume(volume_num, files, page_width, page_height, col_width, col_height, spacing, title_style, body_style):
    output_filename = f"lore_compendium_vol{volume_num}.pdf"
    
    doc = ACMDocTemplate(
        output_filename,
        pagesize=(page_width, page_height)
    )
    
    # 1-column templates
    col_width_1col = page_width - 1.25 * inch
    frame_odd_1col = Frame(0.75 * inch, 0.7 * inch, col_width_1col, col_height, id='col_odd_1col', topPadding=0, bottomPadding=0, leftPadding=0, rightPadding=0)
    template_odd_1col = PageTemplate(id='odd_page_1col', frames=[frame_odd_1col])
    
    frame_even_1col = Frame(0.5 * inch, 0.7 * inch, col_width_1col, col_height, id='col_even_1col', topPadding=0, bottomPadding=0, leftPadding=0, rightPadding=0)
    template_even_1col = PageTemplate(id='even_page_1col', frames=[frame_even_1col])
    
    doc.addPageTemplates([template_odd_1col, template_even_1col])
    
    story = []
    
    story.append(Paragraph(f"<b>COMPENDIUM OF LORE AND HISTORICAL TRANSCRIPTS - VOLUME {volume_num}</b>", title_style))
    story.append(Paragraph("Compiled Chronologically under ACM 1961 Standards", ParagraphStyle('Sub', parent=title_style, fontName='Times-Italic', fontSize=9)))
    story.append(Spacer(1, 0.2 * inch))
    
    def flush_art_flowables(art_flowables_list):
        if not art_flowables_list:
            return
            
        title_date_flowables = []
        while art_flowables_list and hasattr(art_flowables_list[0], 'style') and (art_flowables_list[0].style.name in ('ArtTitle', 'ArtDate')):
            title_date_flowables.append(art_flowables_list.pop(0))
            
        if title_date_flowables:
            story.extend(title_date_flowables)
            story.append(Spacer(1, 4))
            
        if not art_flowables_list:
            return
            
        story.append(BalancedColumns(art_flowables_list[:], nCols=2, innerPadding=spacing, spaceBefore=4, spaceAfter=4))
        art_flowables_list.clear()

    def append_flowable(f):
        if getattr(f, 'is_wide', False):
            flush_art_flowables(art_flowables)
            story.append(Spacer(1, 4))
            story.append(f)
            story.append(Spacer(1, 4))
        else:
            art_flowables.append(f)
            
    for idx, (date, path) in enumerate(files):
        title = os.path.basename(path).replace(".md", "").replace("_", " ").title()
        
        if idx > 0:
            story.append(CondPageBreak(120))
            
        art_flowables = []
        art_flowables.append(Paragraph(f"<b>ARTICLE: {title}</b>", ParagraphStyle('ArtTitle', parent=title_style, alignment=TA_LEFT, fontSize=9.5, spaceBefore=8)))
        art_flowables.append(Paragraph(f"<i>Published: {date.strftime('%B %d, %Y')}</i>", ParagraphStyle('ArtDate', parent=body_style, fontName='Times-Italic', fontSize=7.5)))
        
        article_wide_blocks = []
        listing_counter = 1
        table_counter = 1
        
        with open(path, 'r', encoding='utf-8') as file_in:
            lines = file_in.readlines()
            
        i = 0
        while i < len(lines):
            line = lines[i].strip()
            
            if not line:
                i += 1
                continue
                
            if line.startswith('```'):
                is_mermaid = line.startswith('```mermaid')
                i += 1
                block_lines = []
                while i < len(lines) and not lines[i].strip().startswith('```'):
                    block_lines.append(lines[i])
                    i += 1
                if i < len(lines):
                    i += 1
                
                # Determine block width
                max_len = max((len(bl.rstrip('\r\n')) for bl in block_lines), default=0)
                is_wide = (max_len > 55) or is_mermaid
                
                if is_mermaid:
                    m_flowable = render_mermaid_flowchart(block_lines, col_width_1col if is_wide else col_width)
                    if m_flowable:
                        if is_wide:
                            flush_art_flowables(art_flowables)
                            story.append(Spacer(1, 4))
                            story.append(m_flowable)
                            story.append(Spacer(1, 4))
                        else:
                            append_flowable(Spacer(1, 4))
                            append_flowable(m_flowable)
                            append_flowable(Spacer(1, 4))
                else:
                    if is_wide:
                        flush_art_flowables(art_flowables)
                        c_flowable = render_standard_code_block(block_lines, col_width_1col, body_style)
                        if c_flowable:
                            story.append(Spacer(1, 4))
                            story.append(c_flowable)
                            story.append(Spacer(1, 4))
                    else:
                        c_flowable = render_standard_code_block(block_lines, col_width, body_style)
                        if c_flowable:
                            append_flowable(Spacer(1, 4))
                            append_flowable(c_flowable)
                            append_flowable(Spacer(1, 4))
                continue
                
            if line.startswith('+') or line.startswith('┌'):
                box_lines = []
                while i < len(lines) and (lines[i].strip().startswith('+') or lines[i].strip().startswith('|') or lines[i].strip().startswith('┌') or lines[i].strip().startswith('└') or lines[i].strip().startswith('│') or lines[i].strip().startswith('-') or lines[i].strip().startswith('├') or lines[i].strip().startswith('┤') or lines[i].strip().startswith('┼') or lines[i].strip().startswith('┐') or lines[i].strip().startswith('┘')):
                    box_lines.append(lines[i])
                    i += 1
                    
                max_len = max((len(bl.rstrip('\r\n')) for bl in box_lines), default=0)
                is_wide = (max_len > 55)
                
                if box_lines:
                    if is_wide:
                        flush_art_flowables(art_flowables)
                        c_flowable = render_standard_code_block(box_lines, col_width_1col, body_style)
                        if c_flowable:
                            story.append(Spacer(1, 4))
                            story.append(c_flowable)
                            story.append(Spacer(1, 4))
                    else:
                        c_flowable = render_standard_code_block(box_lines, col_width, body_style)
                        if c_flowable:
                            append_flowable(Spacer(1, 4))
                            append_flowable(c_flowable)
                            append_flowable(Spacer(1, 4))
                continue

            if line.startswith('|'):
                table_lines = []
                while i < len(lines) and (lines[i].strip().startswith('|') or lines[i].strip().startswith('+') or lines[i].strip().startswith('-')):
                    table_lines.append(lines[i])
                    i += 1
                
                # Check column count
                col_count = 0
                max_table_len = 0
                if table_lines:
                    col_count = len(table_lines[0].split('|')) - 2
                    max_table_len = max((len(tl.rstrip('\r\n')) for tl in table_lines), default=0)
                is_wide = (col_count > 2) or (max_table_len > 55)
                
                if is_wide:
                    flush_art_flowables(art_flowables)
                    t_flowable = parse_markdown_table(table_lines, body_style, col_width_1col)
                    if not t_flowable:
                        t_flowable = render_standard_code_block(table_lines, col_width_1col, body_style)
                    if t_flowable:
                        story.append(Spacer(1, 4))
                        story.append(t_flowable)
                        story.append(Spacer(1, 4))
                else:
                    t_flowable = parse_markdown_table(table_lines, body_style, col_width)
                    if not t_flowable:
                        t_flowable = render_standard_code_block(table_lines, col_width, body_style)
                    if t_flowable:
                        append_flowable(Spacer(1, 4))
                        append_flowable(t_flowable)
                        append_flowable(Spacer(1, 4))
                continue
                
            if line.startswith('---') or line.startswith('***'):
                hr = Table([[Paragraph("", body_style)]], colWidths=[col_width])
                hr.setStyle(TableStyle([
                    ('LINEABOVE', (0,0), (-1,-1), 0.5, colors.HexColor('#dddddd')),
                    ('BOTTOMPADDING', (0,0), (-1,-1), 1),
                    ('TOPPADDING', (0,0), (-1,-1), 1),
                ]))
                append_flowable(Spacer(1, 3))
                append_flowable(hr)
                append_flowable(Spacer(1, 3))
                i += 1
                continue
                
            if line.startswith('#'):
                level = len(line) - len(line.lstrip('#'))
                heading_text = line.lstrip('#').strip()
                h_style = ParagraphStyle(
                    f'H{level}',
                    parent=body_style,
                    fontName='Times-Bold',
                    fontSize=max(7.5, 9.5 - (level * 0.5)),
                    leading=max(8.5, 10.5 - (level * 0.5)),
                    spaceBefore=5,
                    spaceAfter=2,
                    keepWithNext=True,
                )
                append_flowable(Paragraph(inline_md_to_html(heading_text), h_style))
                i += 1
                continue
                
            if line.startswith('>'):
                bq_text = []
                while i < len(lines) and lines[i].strip().startswith('>'):
                    bq_text.append(lines[i].strip().lstrip('>').strip())
                    i += 1
                bq_content = ' '.join(bq_text)
                bq_style = ParagraphStyle(
                    'BlockQuote',
                    parent=body_style,
                    fontName='Times-Italic',
                    leftIndent=10,
                    textColor=colors.HexColor('#444444'),
                    spaceBefore=3,
                    spaceAfter=3,
                )
                append_flowable(Paragraph(inline_md_to_html(bq_content), bq_style))
                continue
                
            # Lists: Bullet points
            if line.startswith('* ') or line.startswith('- '):
                bullet_text = line[2:].strip()
                bullet_style = ParagraphStyle(
                    'BulletItem',
                    parent=body_style,
                    leftIndent=10,
                    firstLineIndent=-6,
                    spaceAfter=1.5,
                )
                append_flowable(Paragraph(f"• {inline_md_to_html(bullet_text)}", bullet_style))
                i += 1
                continue
                
            # Lists or Headings starting with numbers (e.g. "1. " or "1.1 ")
            num_match = re.match(r'^(\d+(?:\.\d+)*)\.?\s+(.*)', line)
            if num_match:
                prefix = num_match.group(1)
                item_text = num_match.group(2)
                
                is_heading = False
                level = 1
                if '.' in prefix:
                    is_heading = True
                    level = len(prefix.split('.'))
                else:
                    if item_text and item_text[0].isupper() and len(line) < 80:
                        is_heading = True
                        level = 1
                
                if is_heading:
                    h_style = ParagraphStyle(
                        f'NumH{level}',
                        parent=body_style,
                        fontName='Times-Bold',
                        fontSize=max(7.5, 9.5 - (level * 0.5)),
                        leading=max(8.5, 10.5 - (level * 0.5)),
                        spaceBefore=5,
                        spaceAfter=2,
                        keepWithNext=True,
                    )
                    append_flowable(Paragraph(inline_md_to_html(f"{prefix}. {item_text}"), h_style))
                    i += 1
                    continue
                else:
                    num_style = ParagraphStyle(
                        'NumItem',
                        parent=body_style,
                        leftIndent=10,
                        firstLineIndent=-6,
                        spaceAfter=1.5,
                    )
                    append_flowable(Paragraph(f"{prefix}. {inline_md_to_html(item_text)}", num_style))
                    i += 1
                    continue
                
            # Fallback to standard paragraph blocks
            p_text = []
            while i < len(lines) and lines[i].strip() and not lines[i].strip().startswith('|') and not lines[i].strip().startswith('```') and not lines[i].strip().startswith('#') and not lines[i].strip().startswith('>') and not lines[i].strip().startswith('* ') and not lines[i].strip().startswith('- ') and not re.match(r'^\d+\.\s+', lines[i].strip()):
                p_text.append(lines[i].strip())
                i += 1
            full_p = ' '.join(p_text)
            process_text_block(full_p, body_style, col_width, append_flowable)
                
        flush_art_flowables(art_flowables)
        story.append(Spacer(1, 0.15 * inch))
        
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"PDF build complete: {output_filename}")

def build_pdf():
    import json
    lore_dir = "lore"
    
    # Load published registry
    registry = {}
    registry_path = os.path.join(lore_dir, "published_registry.json")
    if os.path.exists(registry_path):
        try:
            with open(registry_path, "r", encoding="utf-8") as f_reg:
                registry = json.load(f_reg)
        except Exception as e:
            print(f"Error loading registry: {e}")
            
    files = []
    for f in os.listdir(lore_dir):
        if f.endswith(".md") and f != "published_registry.json":
            path = os.path.join(lore_dir, f)
            date = get_file_date(path)
            files.append((date, path, f))
            
    files.sort(key=lambda x: x[0])
    
    page_width = 6.0 * inch
    page_height = 9.0 * inch
    spacing = 0.25 * inch
    col_width = (page_width - 1.25 * inch - spacing) / 2.0
    col_height = page_height - 1.0 * inch - 0.5 * inch
    
    styles = getSampleStyleSheet()
    title_style = ParagraphStyle(
        'ACMTitle',
        parent=styles['Normal'],
        fontName='Times-Bold',
        fontSize=12,
        leading=14,
        alignment=TA_CENTER,
        spaceAfter=10
    )
    
    body_style = ParagraphStyle(
        'ACMBody',
        parent=styles['Normal'],
        fontName='Times-Roman',
        fontSize=8.5,
        leading=10.5,
        alignment=TA_LEFT,
        spaceAfter=6
    )
    
    # Partition files based on registry
    vols = {1: [], 2: [], 3: [], 4: [], 5: []}
    
    if registry:
        for date, path, filename in files:
            if filename in registry:
                vol_num = registry[filename].get("published_volume", 1)
                vols[vol_num].append((date, path))
            else:
                # Any new, unregistered files automatically go to Volume 5!
                vols[5].append((date, path))
    else:
        # Fallback to partition if no registry exists
        total_files = len(files)
        vol_size = total_files // 4
        vols[1] = [(f[0], f[1]) for f in files[0:vol_size]]
        vols[2] = [(f[0], f[1]) for f in files[vol_size:2*vol_size]]
        vols[3] = [(f[0], f[1]) for f in files[2*vol_size:3*vol_size]]
        vols[4] = [(f[0], f[1]) for f in files[3*vol_size:]]
        
    # Build Volumes 1 to 4
    for vol_num in [1, 2, 3, 4]:
        if vols[vol_num]:
            build_volume(vol_num, vols[vol_num], page_width, page_height, col_width, col_height, spacing, title_style, body_style)
            
    # Build Volume 5 if there are any new unregistered articles!
    if vols[5]:
        print(f"Detected {len(vols[5])} new articles for Volume V!")
        build_volume(5, vols[5], page_width, page_height, col_width, col_height, spacing, title_style, body_style)

if __name__ == "__main__":
    build_pdf()
