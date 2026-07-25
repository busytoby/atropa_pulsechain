import os
import re
import datetime
from reportlab.lib.pagesizes import letter
from reportlab.lib.units import inch
from reportlab.platypus import BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, PageBreak, FrameBreak, Table, TableStyle, Image, NextPageTemplate, KeepTogether
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_JUSTIFY, TA_CENTER, TA_LEFT
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
    ]
    
    def replace_math_block(match):
        math_content = match.group(1)
        # Avoid modifying standard C64 hex address ranges like "$D000–$DFFF"
        # and prevent matching across HTML tags
        if '<' in math_content or '>' in math_content or not re.search(r'[+\-*=/_\\^θφ·]|cos|sin', math_content):
            return f"${math_content}$"
            
        for _ in range(3): # up to 3 levels of nesting fractions
            math_content = re.sub(pattern_frac, replace_frac, math_content)
        for pat, repl in replacements:
            math_content = re.sub(pat, repl, math_content)
        math_content = re.sub(r'_\{([^}]+)\}', r'<sub>\1</sub>', math_content)
        math_content = re.sub(r'_([a-zA-Z0-9]+)', r'<sub>\1</sub>', math_content)
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
    
    if text.count('`') % 2 != 0:
        text = text.rsplit('`', 1)[0]
        
    parts = text.split('`')
    new_parts = []
    for idx, part in enumerate(parts):
        if idx % 2 == 1:
            new_parts.append(f'<font face="Courier">{part}</font>')
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


def parse_markdown_table(rows, body_style, col_width):
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

def process_text_block(text, body_style, col_width, story):
    pattern = r'!\[[^\]]*\]\(([^)]+)\)'
    last_idx = 0
    for match in re.finditer(pattern, text):
        pre_text = text[last_idx:match.start()].strip()
        if pre_text:
            pre_text = re.sub(r'^#+\s*', '', pre_text)
            html_p = inline_md_to_html(pre_text)
            if html_p:
                story.append(Paragraph(html_p, body_style))
                
        img_path = match.group(1).strip().split('?')[0]
        print(f"Found image: {img_path}")
        ext = os.path.splitext(img_path)[1].lower()
        if ext in ['.jpg', '.jpeg', '.png', '.gif', '.bmp'] and os.path.exists(img_path):
            try:
                img_flow = Image(img_path)
                img_flow.drawWidth = col_width - 10
                img_flow.drawHeight = 1.5 * inch
                story.append(Spacer(1, 4))
                story.append(img_flow)
                story.append(Spacer(1, 4))
            except Exception as e:
                print(f"Image error for {img_path}: {e}")
                
        last_idx = match.end()
        
    post_text = text[last_idx:].strip()
    if post_text:
        post_text = re.sub(r'^#+\s*', '', post_text)
        html_p = inline_md_to_html(post_text)
        if html_p:
            story.append(Paragraph(html_p, body_style))

def build_volume(volume_num, files, page_width, page_height, col_width, col_height, spacing, title_style, body_style):
    output_filename = f"lore_compendium_vol{volume_num}.pdf"
    
    doc = BaseDocTemplate(
        output_filename,
        pagesize=(page_width, page_height)
    )
    
    frame_left_odd = Frame(0.75 * inch, 0.7 * inch, col_width, col_height, id='col1_odd')
    frame_right_odd = Frame(0.75 * inch + col_width + spacing, 0.7 * inch, col_width, col_height, id='col2_odd')
    template_odd = PageTemplate(id='odd_page', frames=[frame_left_odd, frame_right_odd])
    
    frame_left_even = Frame(0.5 * inch, 0.7 * inch, col_width, col_height, id='col1_even')
    frame_right_even = Frame(0.5 * inch + col_width + spacing, 0.7 * inch, col_width, col_height, id='col2_even')
    template_even = PageTemplate(id='even_page', frames=[frame_left_even, frame_right_even])
    
    doc.addPageTemplates([template_odd, template_even])
    
    story = []
    story.append(NextPageTemplate(['even_page', 'odd_page']))
    
    story.append(Paragraph(f"<b>COMPENDIUM OF LORE AND HISTORICAL TRANSCRIPTS - VOLUME {volume_num}</b>", title_style))
    story.append(Paragraph("Compiled Chronologically under ACM 1961 Standards", ParagraphStyle('Sub', parent=title_style, fontName='Times-Italic', fontSize=9)))
    story.append(Spacer(1, 0.2 * inch))
    
    for date, path in files:
        title = os.path.basename(path).replace(".md", "").replace("_", " ").title()
        
        art_flowables = []
        art_flowables.append(Paragraph(f"<b>ARTICLE: {title}</b>", ParagraphStyle('ArtTitle', parent=title_style, alignment=TA_LEFT, fontSize=9.5, spaceBefore=8)))
        art_flowables.append(Paragraph(f"<i>Published: {date.strftime('%B %d, %Y')}</i>", ParagraphStyle('ArtDate', parent=body_style, fontName='Times-Italic', fontSize=7.5)))
        
        with open(path, 'r', encoding='utf-8') as file_in:
            lines = file_in.readlines()
            
        i = 0
        while i < len(lines):
            line = lines[i].strip()
            
            if line.startswith('|'):
                table_lines = []
                while i < len(lines) and lines[i].strip().startswith('|'):
                    table_lines.append(lines[i])
                    i += 1
                t_flowable = parse_markdown_table(table_lines, body_style, col_width)
                if t_flowable:
                    art_flowables.append(Spacer(1, 4))
                    art_flowables.append(t_flowable)
                    art_flowables.append(Spacer(1, 4))
                continue
                
            if line:
                p_text = []
                while i < len(lines) and lines[i].strip() and not lines[i].strip().startswith('|'):
                    p_text.append(lines[i].strip())
                    i += 1
                full_p = ' '.join(p_text)
                process_text_block(full_p, body_style, col_width, art_flowables)
            else:
                i += 1
                
        # Group Title, Date, and the first text paragraph/table together to prevent orphans
        if len(art_flowables) >= 3:
            together_flowables = art_flowables[:3]
            remaining_flowables = art_flowables[3:]
            story.append(KeepTogether(together_flowables))
            story.extend(remaining_flowables)
        else:
            story.extend(art_flowables)
            
        story.append(Spacer(1, 0.1 * inch))
        
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
    spacing = 0.3 * inch
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
