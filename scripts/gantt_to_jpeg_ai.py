#!/usr/bin/env python3
import os
import sys
import re
import json
import math
from datetime import datetime, timedelta
from PIL import Image, ImageDraw, ImageFont

def parse_gantt(md_content):
    """
    Parses a Mermaid Gantt chart syntax from markdown content.
    Returns a dictionary with title, sections, and tasks.
    """
    gantt_blocks = []
    # Match ```mermaid ... gantt ... ``` or just gantt blocks
    matches = re.findall(r'```(?:mermaid)?\s*\n\s*gantt(.*?)\n\s*```', md_content, re.DOTALL | re.IGNORECASE)
    if not matches:
        matches = re.findall(r'gantt(.*?)$', md_content, re.DOTALL | re.IGNORECASE)
        
    for block in matches:
        title = "Auncient Gantt Chart"
        date_format = "YYYY-MM-DD"
        sections = []
        current_section = "General"
        tasks = []
        
        lines = block.split('\n')
        for line in lines:
            line = line.strip()
            if not line or line.startswith('%'):
                continue
            
            title_match = re.match(r'^title\s+(.+)$', line, re.IGNORECASE)
            if title_match:
                title = title_match.group(1).strip()
                continue
                
            df_match = re.match(r'^dateFormat\s+(.+)$', line, re.IGNORECASE)
            if df_match:
                date_format = df_match.group(1).strip()
                continue
                
            sec_match = re.match(r'^section\s+(.+)$', line, re.IGNORECASE)
            if sec_match:
                current_section = sec_match.group(1).strip()
                if current_section not in sections:
                    sections.append(current_section)
                continue
                
            task_match = re.match(r'^([^:]+):([^:]+)$', line)
            if task_match:
                task_name = task_match.group(1).strip()
                params_str = task_match.group(2).strip()
                params = [p.strip() for p in params_str.split(',')]
                
                tags = []
                start = None
                duration = None
                
                if len(params) == 3:
                    tags.append(params[0])
                    start = params[1]
                    duration = params[2]
                elif len(params) == 2:
                    if params[0] in ['active', 'done', 'crit', 'after']:
                        tags.append(params[0])
                        duration = params[1]
                    else:
                        start = params[0]
                        duration = params[1]
                elif len(params) == 1:
                    duration = params[0]
                
                tasks.append({
                    "name": task_name,
                    "section": current_section,
                    "tags": tags,
                    "start": start,
                    "duration": duration
                })
        
        if not sections and tasks:
            sections = ["General"]
            
        gantt_blocks.append({
            "title": title,
            "date_format": date_format,
            "sections": sections,
            "tasks": tasks
        })
        
    return gantt_blocks

def parse_date_or_val(val_str, date_format):
    if not val_str:
        return None
    val_str = val_str.strip()
    if re.match(r'^\d{4}-\d{2}-\d{2}$', val_str):
        try:
            return datetime.strptime(val_str, "%Y-%m-%d")
        except ValueError:
            return None
    try:
        if '.' in val_str:
            return float(val_str)
        return int(val_str)
    except ValueError:
        return val_str

def parse_duration(dur_str):
    if not dur_str:
        return 1
    dur_str = dur_str.strip()
    match = re.match(r'^(\d+)([dwhs])?$', dur_str, re.IGNORECASE)
    if match:
        val = int(match.group(1))
        unit = match.group(2)
        if unit:
            unit = unit.lower()
            if unit == 'w':
                return val * 7
            elif unit == 'h':
                return val / 24.0
        return val
    try:
        return float(dur_str)
    except ValueError:
        return 1

def inject_jpeg_ai_markers(jpeg_path, metadata):
    with open(jpeg_path, 'rb') as f:
        data = f.read()
        
    if data[0:2] != b'\xFF\xD8':
        raise ValueError("Invalid JPEG file SOI")
        
    prefix = b"ISO_IEC_6048_JPEG_AI\x00"
    meta_json = json.dumps(metadata).encode('utf-8')
    payload = prefix + meta_json
    length = len(payload) + 2
    
    app11_marker = b'\xFF\xEB' + length.to_bytes(2, 'big') + payload
    new_data = data[0:2] + app11_marker + data[2:]
    
    with open(jpeg_path, 'wb') as f:
        f.write(new_data)

def draw_glow_rect(draw, coords, color):
    """
    Renders a hollow rectangle with diagonal cross-hatching and electron-beam glow bloom.
    """
    x1, y1, x2, y2 = coords
    r, g, b = color
    
    # 1. Draw diagonal cross-hatching inside the bar (X1+2 to X2-2)
    h = y2 - y1
    step = 12
    hatch_color = (r // 3, g // 3, b // 3)
    for x_start in range(int(x1) - int(h), int(x2), step):
        x_prime = max(x1 + 2, x_start)
        x_prime_prime = min(x2 - 2, x_start + h)
        if x_prime < x_prime_prime:
            y_prime = y1 + (x_prime - x_start)
            y_prime_prime = y1 + (x_prime_prime - x_start)
            draw.line([(x_prime, y_prime), (x_prime_prime, y_prime_prime)], fill=hatch_color, width=1)
            
    # 2. Draw outer glow (thickest, lowest brightness)
    draw.rectangle([x1 - 3, y1 - 3, x2 + 3, y2 + 3], outline=(r // 6, g // 6, b // 6), width=7)
    # Mid glow (medium thickness, mid brightness)
    draw.rectangle([x1 - 1, y1 - 1, x2 + 1, y2 + 1], outline=(r // 2, g // 2, b // 2), width=3)
    # Core beam (thin, full brightness)
    draw.rectangle([x1, y1, x2, y2], outline=color, width=1)
    
    # Draw vector corner overshoots/crosshairs
    overshoot = 4
    # Top-Left
    draw.line([(x1 - overshoot, y1), (x1 + overshoot, y1)], fill=color, width=1)
    draw.line([(x1, y1 - overshoot), (x1, y1 + overshoot)], fill=color, width=1)
    # Top-Right
    draw.line([(x2 - overshoot, y1), (x2 + overshoot, y1)], fill=color, width=1)
    draw.line([(x2, y1 - overshoot), (x2, y1 + overshoot)], fill=color, width=1)
    # Bottom-Left
    draw.line([(x1 - overshoot, y2), (x1 + overshoot, y2)], fill=color, width=1)
    draw.line([(x1, y2 - overshoot), (x1, y2 + overshoot)], fill=color, width=1)
    # Bottom-Right
    draw.line([(x2 - overshoot, y2), (x2 + overshoot, y2)], fill=color, width=1)
    draw.line([(x2, y2 - overshoot), (x2, y2 + overshoot)], fill=color, width=1)

def render_demoscene_gantt(gantt_data, output_path):
    width = 960
    height = 640
    
    image = Image.new("RGB", (width, height), (0, 0, 0))
    draw = ImageDraw.Draw(image)
    
    # Compute 2D Plasma Background
    plasma_scale = 8
    pw = width // plasma_scale
    ph = height // plasma_scale
    
    for py in range(ph):
        y_coord = py * plasma_scale
        for px in range(pw):
            x_coord = px * plasma_scale
            
            cx = x_coord - width / 2
            cy = y_coord - height / 2
            
            v = 0.0
            v += math.sin(x_coord / 40.0)
            v += math.sin(y_coord / 30.0)
            v += math.sin((x_coord + y_coord) / 40.0)
            v += math.sin(math.sqrt(cx*cx + cy*cy) / 35.0)
            
            v = (v + 4.0) / 8.0
            v = max(0.0, min(1.0, v))
            
            if v < 0.5:
                t = v * 2.0
                r = int(30 * (1.0 - t))
                g = int(45 * t)
                b = int(60 * (1.0 - t) + 15 * t)
            else:
                t = (v - 0.5) * 2.0
                r = int(5 * t)
                g = int(45 * (1.0 - t) + 5 * t)
                b = int(15 * (1.0 - t) + 8 * t)
                
            draw.rectangle([(x_coord, y_coord), (x_coord + plasma_scale, y_coord + plasma_scale)], fill=(r, g, b))

    # CRT scanlines
    for y in range(0, height, 2):
        draw.line([(0, y), (width, y)], fill=(0, 0, 0, 80))

    # Colors
    c_white = (255, 255, 255)
    c_green = (0, 255, 80)
    c_purple = (180, 0, 255)
    
    # Glowing double frames
    draw.rectangle([(15, 15), (width - 15, height - 15)], outline=(90, 0, 127), width=3)
    draw.rectangle([(12, 12), (width - 12, height - 12)], outline=(0, 127, 40), width=1)
    
    try:
        font_large = ImageFont.truetype("DejaVuSansMono-Bold.ttf", 24)
        font_med = ImageFont.truetype("DejaVuSansMono.ttf", 15)
        font_small = ImageFont.truetype("DejaVuSansMono.ttf", 12)
    except IOError:
        font_large = ImageFont.load_default()
        font_med = ImageFont.load_default()
        font_small = ImageFont.load_default()
        
    # Title copper bar
    title_bar_y = 40
    title_bar_h = 45
    for offset in range(title_bar_h):
        frac = offset / title_bar_h
        r = int(180 * (1.0 - frac))
        g = int(255 * frac)
        b = int(255 * (1.0 - frac))
        draw.line([(15, title_bar_y + offset), (width - 15, title_bar_y + offset)], fill=(r, g, b))
        
    title_text = f"/// {gantt_data['title'].upper()} ///"
    draw.text((35, title_bar_y + 8), title_text, fill=c_white, font=font_large)
    
    # Sub-header
    draw.text((35, 18), "AUNCIENT DEMOSCENE VECTOR ENGINE V7.0 // COMPATIBLE WITH JPEG AI ISO/IEC 6048", fill=c_green, font=font_small)
    
    # Chart coordinates
    chart_x_start = 240
    chart_x_end = 925
    chart_y_start = 110
    
    tasks = gantt_data["tasks"]
    if not tasks:
        draw.text((100, 200), "NO ANNOTATED TASKS FOUND", fill=c_purple, font=font_large)
        image.save(output_path, "JPEG", quality=95)
        return
        
    parsed_tasks = []
    base_date = None
    
    for t in tasks:
        start_raw = t["start"]
        dur_raw = t["duration"]
        dur_units = parse_duration(dur_raw)
        
        if not start_raw:
            if parsed_tasks:
                start_val = parsed_tasks[-1]["end_val"]
            else:
                start_val = datetime.now() if gantt_data["date_format"] == "YYYY-MM-DD" else 0
        else:
            start_val = parse_date_or_val(start_raw, gantt_data["date_format"])
            
        if isinstance(start_val, datetime):
            if base_date is None or start_val < base_date:
                base_date = start_val
            end_val = start_val + timedelta(days=dur_units)
        else:
            if start_val is None:
                start_val = 0
            end_val = start_val + dur_units
            
        parsed_tasks.append({
            "name": t["name"],
            "section": t["section"],
            "tags": t["tags"],
            "start_val": start_val,
            "end_val": end_val,
            "dur_units": dur_units
        })
        
    min_val = None
    max_val = None
    for pt in parsed_tasks:
        s_val = pt["start_val"]
        e_val = pt["end_val"]
        if isinstance(s_val, datetime):
            pt["start_num"] = (s_val - base_date).days
            pt["end_num"] = (e_val - base_date).days
        else:
            pt["start_num"] = float(s_val)
            pt["end_num"] = float(e_val)
            
        if min_val is None or pt["start_num"] < min_val:
            min_val = pt["start_num"]
        if max_val is None or pt["end_num"] > max_val:
            max_val = pt["end_num"]
            
    if min_val == max_val:
        max_val += 1.0
        
    total_span = max_val - min_val
    
    # Timeline
    timeline_y = chart_y_start
    draw.line([(chart_x_start, timeline_y), (chart_x_end, timeline_y)], fill=c_purple, width=2)
    
    intervals = 6
    for i in range(intervals + 1):
        frac = i / intervals
        x_pos = chart_x_start + frac * (chart_x_end - chart_x_start)
        draw.line([(x_pos, timeline_y), (x_pos, height - 35)], fill=(80, 80, 110), width=1)
        
        curr_offset = min_val + frac * total_span
        if base_date:
            curr_date = base_date + timedelta(days=int(curr_offset))
            label = curr_date.strftime("%m/%d")
        else:
            label = f"{curr_offset:.1f}"
        draw.text((x_pos - 15, timeline_y - 18), label, fill=c_white, font=font_small)
        
    # Rows
    row_y = chart_y_start + 15
    row_height = 42
    
    sections = gantt_data["sections"]
    section_colors = [c_green, c_purple, c_white]
    
    for pt in parsed_tasks:
        sec_idx = sections.index(pt["section"]) % len(section_colors) if pt["section"] in sections else 0
        bar_color = section_colors[sec_idx]
        
        if "active" in pt["tags"]:
            bar_color = c_green
        elif "done" in pt["tags"]:
            bar_color = (0, 180, 60)
            
        task_label = pt["name"]
        if len(task_label) > 22:
            task_label = task_label[:19] + "..."
            
        draw.text((37, row_y + 7), task_label, fill=(0, 0, 0), font=font_med)
        draw.text((35, row_y + 5), task_label, fill=c_white, font=font_med)
        
        start_frac = (pt["start_num"] - min_val) / total_span
        end_frac = (pt["end_num"] - min_val) / total_span
        
        bx_start = chart_x_start + start_frac * (chart_x_end - chart_x_start)
        bx_end = chart_x_start + end_frac * (chart_x_end - chart_x_start)
        
        if bx_end < bx_start + 4:
            bx_end = bx_start + 4
            
        bar_y_top = row_y + 4
        bar_y_bot = row_y + row_height - 12
        
        draw_glow_rect(draw, [bx_start, bar_y_top, bx_end, bar_y_bot], bar_color)
            
        draw.text((37, row_y + 22), f"[{pt['section']}]", fill=(0, 0, 0), font=font_small)
        draw.text((35, row_y + 20), f"[{pt['section']}]", fill=bar_color, font=font_small)
        
        row_y += row_height
        if row_y > height - 70:
            break
            
    image.save(output_path, "JPEG", quality=95)
    
    # ISO/IEC 6048 metadata
    metadata = {
        "standard": "ISO/IEC 6048 JPEG AI",
        "profile": "Demoscene 4K Assembly",
        "timestamp": datetime.now().isoformat(),
        "title": gantt_data["title"],
        "sections": gantt_data["sections"],
        "task_count": len(tasks),
        "encoder": "Auncient_Gantt_6048"
    }
    inject_jpeg_ai_markers(output_path, metadata)

def main():
    if len(sys.argv) < 3:
        print("Usage: python3 gantt_to_jpeg_ai.py <input_md_file> <output_jpeg_file>")
        sys.exit(1)
        
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found.")
        sys.exit(1)
        
    with open(input_file, 'r', encoding='utf-8') as f:
        md_content = f.read()
        
    gantt_blocks = parse_gantt(md_content)
    if not gantt_blocks:
        print("Error: No Mermaid Gantt chart annotations found in the input markdown file.")
        sys.exit(1)
        
    render_demoscene_gantt(gantt_blocks[0], output_file)
    print(f"Successfully compiled Gantt chart from '{input_file}' to demoscene 4k style JPEG AI image: '{output_file}'")

if __name__ == "__main__":
    main()
