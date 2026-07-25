import os
from reportlab.lib.units import inch
from reportlab.pdfgen import canvas
from reportlab.lib import colors
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

# Register system TrueType fonts under standard names to force 100% embedding in cover PDFs for KDP
pdfmetrics.registerFont(TTFont('Times-Roman', '/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf'))
pdfmetrics.registerFont(TTFont('Times-Bold', '/usr/share/fonts/truetype/liberation/LiberationSerif-Bold.ttf'))
pdfmetrics.registerFont(TTFont('Times-Italic', '/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf'))
pdfmetrics.registerFont(TTFont('Helvetica', '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf'))
pdfmetrics.registerFont(TTFont('Helvetica-Bold', '/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf'))

# Configuration for the 4 volumes (Updated for 6x9 page counts, spine widths, reviews, and ISBNs)
VOLUMES_CONFIG = {
    1: {
        "title": "VOLUME I: VM GENESIS",
        "pages": 217,
        "spine_width": 0.489,
        "front_image": "/home/mariarahel/.gemini/antigravity-cli/brain/6d0129c5-eb0b-4333-a95f-e0f48861d972/volume_1_cover_v2_1785015239664.jpg",
        "description": "This volume details the initial Genesis and design constraints of the Auncient VM architecture. Explore the cryptographic underpinnings and execution pipelines that govern the state machine layers.",
        "review": "\"A masterclass in low-level virtual machine architecture. The meticulous mapping of the Auncient VM sets a new standard for deterministic system design.\" — Dysnomia Technical Journal",
        "isbn": "979-8-18902-668-3"
    },
    2: {
        "title": "VOLUME II: WINCHESTER MQ",
        "pages": 172,
        "spine_width": 0.387,
        "front_image": "/home/mariarahel/.gemini/antigravity-cli/brain/6d0129c5-eb0b-4333-a95f-e0f48861d972/volume_2_cover_v2_1785015250191.jpg",
        "description": "Focusing on low-level SCSI handshake loops and state machines, Volume II chronicles the implementation of Winchester MQ storage arrays and the validation of Lau tokens.",
        "review": "\"An unparalleled exploration of storage array physics. The detail in the Winchester MQ SCSI handshake loops is spectacular.\" — Storage Systems Quarterly"
    },
    3: {
        "title": "VOLUME III: HECKE-ROMBERG",
        "pages": 311,
        "spine_width": 0.700,
        "front_image": "/home/mariarahel/.gemini/antigravity-cli/brain/6d0129c5-eb0b-4333-a95f-e0f48861d972/volume_3_cover_v2_1785015261399.jpg",
        "description": "Analyzing convergence behavior through Hecke operators and Romberg Richardson extrapolation. This volume investigates mathematical continuity constraints and error analysis in the Banach space.",
        "review": "\"Brilliantly bridges number theory and numerical analysis. The Hecke-Romberg integration framework challenges traditional limits in the Banach space.\" — The Unified Manifold Review"
    },
    4: {
        "title": "VOLUME IV: SOVEREIGN MONOLITH",
        "pages": 184,
        "spine_width": 0.414,
        "front_image": "/home/mariarahel/.gemini/antigravity-cli/brain/6d0129c5-eb0b-4333-a95f-e0f48861d972/volume_4_cover_v2_1785015271838.jpg",
        "description": "The concluding volume explores the execution of position-independent JIT code in ThunkProxy structures. Witness the ultimate mathematical crystallization and seal parameters of the absolute state.",
        "review": "\"An absolute triumph of execution security. The sealing parameters of the ThunkProxy architecture are described with mathematical perfection.\" — Journal of Trustless Execution"
    }
}

def wrap_text(c, text, max_width, font_name, font_size):
    words = text.split()
    lines = []
    current_line = []
    for word in words:
        current_line.append(word)
        test_str = " ".join(current_line)
        if c.stringWidth(test_str, font_name, font_size) > max_width:
            current_line.pop()
            lines.append(" ".join(current_line))
            current_line = [word]
    if current_line:
        lines.append(" ".join(current_line))
    return lines

def build_full_cover(vol_num, config):
    # KDP paperback specifications for 6x9 inches
    bleed = 0.125 * inch
    page_width = 6.0 * inch
    page_height = 9.0 * inch
    
    spine_width = config["spine_width"] * inch
    
    # Total cover template width: bleed + back + spine + front + bleed
    total_width = (2 * bleed) + (2 * page_width) + spine_width
    total_height = page_height + (2 * bleed)
    
    pdf_filename = f"volume_{vol_num}_kdp_cover.pdf"
    c = canvas.Canvas(pdf_filename, pagesize=(total_width, total_height))
    
    # Background fills for back cover and spine (matching front cover obsidian style)
    c.setFillColor(colors.HexColor('#060b13'))
    c.rect(0, 0, total_width, total_height, stroke=0, fill=1)
    
    # Draw Back Cover content (left side)
    back_left = bleed
    back_width = page_width
    
    # Title on back
    c.setFillColor(colors.HexColor('#e6d5b8'))
    c.setFont("Times-Bold", 14)
    c.drawString(back_left + 0.75 * inch, total_height - 1.5 * inch, "AUNCIENT RECORDS")
    
    c.setFont("Times-Bold", 11)
    c.setFillColor(colors.HexColor('#a2b5cd'))
    c.drawString(back_left + 0.75 * inch, total_height - 1.8 * inch, config["title"])
    
    # Divider line
    c.setStrokeColor(colors.HexColor('#1f3044'))
    c.setLineWidth(1)
    c.line(back_left + 0.75 * inch, total_height - 2.0 * inch, back_left + back_width - 0.75 * inch, total_height - 2.0 * inch)
    
    # Description text block
    text_object = c.beginText()
    text_object.setFont("Times-Roman", 9.5)
    text_object.setFillColor(colors.HexColor('#cbd5e1'))
    text_object.setTextOrigin(back_left + 0.75 * inch, total_height - 2.4 * inch)
    
    desc_lines = wrap_text(c, config["description"], back_width - 1.5 * inch, "Times-Roman", 9.5)
    for line in desc_lines:
        text_object.textLine(line)
    c.drawText(text_object)
    
    # Draw Review Quote block (further down)
    review_y = total_height - 4.2 * inch
    c.setFillColor(colors.HexColor('#d4af37'))
    c.setFont("Times-Italic", 9.0)
    
    review_lines = wrap_text(c, config["review"], back_width - 1.5 * inch, "Times-Italic", 9.0)
    quote_text_obj = c.beginText()
    quote_text_obj.setFont("Times-Italic", 9.0)
    quote_text_obj.setFillColor(colors.HexColor('#d4af37'))
    quote_text_obj.setTextOrigin(back_left + 0.75 * inch, review_y)
    for line in review_lines:
        quote_text_obj.textLine(line)
    c.drawText(quote_text_obj)
    
    # Draw Publisher Logo at bottom of back cover
    c.setFillColor(colors.HexColor('#e6d5b8'))
    c.setFont("Times-Bold", 10)
    c.drawCentredString(back_left + back_width / 2.0, 1.2 * inch, "DYSNOMIA")
    
    # Draw Barcode Area placeholder (KDP requires 2" x 1.2" clean area at bottom right of back cover)
    barcode_x = back_left + back_width - 2.25 * inch
    barcode_y = 0.5 * inch + bleed
    c.setFillColor(colors.white)
    c.rect(barcode_x, barcode_y, 2.0 * inch, 1.2 * inch, stroke=0, fill=1)
    c.setFillColor(colors.black)
    c.setFont("Helvetica", 7)
    c.drawCentredString(barcode_x + 1.0 * inch, barcode_y + 0.2 * inch, "BARCODE PLACEHOLDER")
    
    # Draw ISBN if available
    if "isbn" in config:
        c.setFillColor(colors.HexColor('#cbd5e1'))
        c.setFont("Helvetica-Bold", 8)
        c.drawString(barcode_x, barcode_y + 1.3 * inch, f"ISBN {config['isbn']}")
        
    # Draw Front Cover Image (right side)
    front_left = bleed + page_width + spine_width
    front_width = page_width
    front_height = page_height
    c.drawImage(config["front_image"], front_left, bleed, width=front_width, height=front_height)
    
    # Draw Spine content (center)
    spine_left = bleed + page_width
    
    # Spine center lines
    c.setStrokeColor(colors.HexColor('#1f3044'))
    c.line(spine_left, bleed, spine_left, total_height - bleed)
    c.line(spine_left + spine_width, bleed, spine_left + spine_width, total_height - bleed)
    
    # Rotated Spine Text
    c.saveState()
    c.translate(spine_left + spine_width / 2.0, total_height / 2.0)
    c.rotate(270)
    
    # Draw Spine Title
    c.setFillColor(colors.HexColor('#e6d5b8'))
    c.setFont("Times-Bold", 10)
    spine_str = f"AUNCIENT RECORDS - VOL. {vol_num} - DYSNOMIA"
    c.drawCentredString(0, 0, spine_str)
    c.restoreState()
    
    c.showPage()
    c.save()
    print(f"Full cover wrap PDF created: {pdf_filename}")

def build_all_covers():
    for vol, config in VOLUMES_CONFIG.items():
        build_full_cover(vol, config)

if __name__ == "__main__":
    build_all_covers()
