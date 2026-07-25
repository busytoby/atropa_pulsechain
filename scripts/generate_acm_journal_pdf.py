import os
import re
import datetime
from reportlab.lib.pagesizes import letter
from reportlab.lib.units import inch
from reportlab.platypus import BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, PageBreak, FrameBreak
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_JUSTIFY, TA_CENTER
from reportlab.pdfgen import canvas

def get_file_date(filepath):
    filename = os.path.basename(filepath)
    # Check for date in filename: YYYY_MM_DD or YYYY-MM-DD
    match = re.search(r'(\d{4})[_-](\d{2})[_-](\d{2})', filename)
    if match:
        try:
            return datetime.datetime(int(match.group(1)), int(match.group(2)), int(match.group(3)))
        except ValueError:
            pass
            
    # Try reading the first few lines of the file for dates
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
        
        # Header text
        self.drawString(0.5 * inch, 6.5 * inch, "PROCEEDINGS OF THE ACM, VOL. 18, 1961")
        self.drawRightString(8.5 * inch, 6.5 * inch, "LORE COMPENDIUM")
        self.setLineWidth(0.5)
        self.line(0.5 * inch, 6.4 * inch, 8.5 * inch, 6.4 * inch)
        
        # Footer text
        self.line(0.5 * inch, 0.6 * inch, 8.5 * inch, 0.6 * inch)
        page_text = f"Page {self._pageNumber} of {page_count}"
        self.drawCentredString(4.5 * inch, 0.4 * inch, page_text)
        self.restoreState()

def clean_markdown_text(text):
    import html
    # Strip markdown headers
    text = re.sub(r'^#+\s*', '', text, flags=re.MULTILINE)
    # Strip links formatting
    text = re.sub(r'\[([^\]]+)\]\([^)]+\)', r'\1', text)
    # Strip formatting chars
    text = text.replace('**', '').replace('*', '').replace('`', '').replace('_', '')
    # Escape HTML special characters
    text = html.escape(text)
    return text.strip()

def build_pdf():
    lore_dir = "lore"
    files = []
    for f in os.listdir(lore_dir):
        if f.endswith(".md"):
            path = os.path.join(lore_dir, f)
            date = get_file_date(path)
            files.append((date, path))
            
    # Sort chronologically
    files.sort(key=lambda x: x[0])
    
    # 9x7 inch custom page size
    page_width = 9.0 * inch
    page_height = 7.0 * inch
    margin = 0.5 * inch
    spacing = 0.4 * inch
    col_width = (page_width - 2 * margin - spacing) / 2.0
    col_height = page_height - 2 * margin - 0.5 * inch
    
    doc = BaseDocTemplate(
        "lore_compendium.pdf",
        pagesize=(page_width, page_height),
        leftMargin=margin,
        rightMargin=margin,
        topMargin=margin,
        bottomMargin=margin
    )
    
    frame_left = Frame(margin, margin + 0.2 * inch, col_width, col_height, id='col1')
    frame_right = Frame(margin + col_width + spacing, margin + 0.2 * inch, col_width, col_height, id='col2')
    
    template = PageTemplate(id='two_column', frames=[frame_left, frame_right])
    doc.addPageTemplates([template])
    
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
        alignment=TA_JUSTIFY,
        spaceAfter=6
    )
    
    story = []
    
    story.append(Paragraph("<b>COMPENDIUM OF LORE AND HISTORICAL TRANSCRIPTS</b>", title_style))
    story.append(Paragraph("Compiled Chronologically under ACM 1961 Standards", ParagraphStyle('Sub', parent=title_style, fontName='Times-Italic', fontSize=9)))
    story.append(Spacer(1, 0.2 * inch))
    
    for date, path in files:
        title = os.path.basename(path).replace(".md", "").replace("_", " ").title()
        story.append(Paragraph(f"<b>ARTICLE: {title}</b>", ParagraphStyle('ArtTitle', parent=title_style, alignment=TA_JUSTIFY, fontSize=9.5, spaceBefore=8)))
        story.append(Paragraph(f"<i>Published: {date.strftime('%B %d, %Y')}</i>", ParagraphStyle('ArtDate', parent=body_style, fontName='Times-Italic', fontSize=7.5)))
        
        with open(path, 'r', encoding='utf-8') as file_in:
            content = file_in.read()
            paragraphs = content.split('\n\n')
            for p in paragraphs:
                cleaned = clean_markdown_text(p)
                if cleaned:
                    story.append(Paragraph(cleaned, body_style))
        story.append(Spacer(1, 0.1 * inch))
        
    doc.build(story, canvasmaker=NumberedCanvas)
    print("PDF build complete: lore_compendium.pdf")

if __name__ == "__main__":
    build_pdf()
