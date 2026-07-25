import os
import re
import datetime

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

def clean_tex_text(text):
    # Escape standard LaTeX special characters
    text = text.replace('\\', '\\textbackslash ')
    text = text.replace('&', '\\&')
    text = text.replace('%', '\\%')
    text = text.replace('$', '\\$')
    text = text.replace('#', '\\#')
    text = text.replace('_', '\\_')
    text = text.replace('{', '\\{')
    text = text.replace('}', '\\}')
    text = text.replace('~', '\\textasciitilde ')
    text = text.replace('^', '\\textasciicircum ')
    
    # Inline markdown styles conversion
    text = re.sub(r'\*\*([^*]+)\*\*', r'\\textbf{\1}', text)
    text = re.sub(r'\*([^*]+)\*', r'\\textit{\1}', text)
    text = re.sub(r'`([^`]+)`', r'\\texttt{\1}', text)
    return text

def parse_table_to_tex(rows):
    tex_rows = []
    headers = None
    for r in rows:
        r = r.strip()
        if not r or r.startswith('|---') or r.startswith('|:---') or r.startswith('|-'):
            continue
        cells = [clean_tex_text(c.strip()) for c in r.split('|')]
        if cells and cells[0] == '':
            cells = cells[1:]
        if cells and cells[-1] == '':
            cells = cells[:-1]
            
        if not headers:
            headers = cells
        else:
            tex_rows.append(' & '.join(cells) + ' \\\\ \\hline')
            
    if not headers:
        return ""
        
    num_cols = len(headers)
    col_spec = '|' + 'l|' * num_cols
    
    header_line = ' & '.join(headers) + ' \\\\ \\hline\\hline'
    
    tex_table = []
    tex_table.append('\\begin{table}[h]')
    tex_table.append('\\centering')
    tex_table.append(f'\\begin{{tabular}}{{{col_spec}}}')
    tex_table.append('\\hline')
    tex_table.append(header_line)
    for row in tex_rows:
        tex_table.append(row)
    tex_table.append('\\end{tabular}')
    tex_table.append('\\end{table}')
    return '\n'.join(tex_table)

def convert_md_file_to_tex(filepath, date):
    title = os.path.basename(filepath).replace(".md", "").replace("_", " ").title()
    tex_content = []
    tex_content.append(f"\\section*{{ARTICLE: {title}}}")
    tex_content.append(f"\\noindent\\textit{{Published: {date.strftime('%B %d, %Y')}}}\\\\")
    tex_content.append("\\vspace{0.1in}")
    
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        # Parse table block
        if line.startswith('|'):
            table_lines = []
            while i < len(lines) and lines[i].strip().startswith('|'):
                table_lines.append(lines[i])
                i += 1
            t_tex = parse_table_to_tex(table_lines)
            if t_tex:
                tex_content.append(t_tex)
            continue
            
        # Parse images/charts
        img_match = re.search(r'!\[[^\]]*\]\(([^)]+)\)', line)
        if img_match:
            img_path = img_match.group(1).strip().split('?')[0]
            ext = os.path.splitext(img_path)[1].lower()
            if ext in ['.jpg', '.jpeg', '.png', '.gif', '.pdf']:
                tex_content.append('\\begin{figure}[h]')
                tex_content.append('\\centering')
                tex_content.append(f'\\includegraphics[width=\\linewidth]{{{img_path}}}')
                tex_content.append(f'\\caption{{{clean_tex_text(os.path.basename(img_path))}}}')
                tex_content.append('\\end{figure}')
            i += 1
            continue
            
        # Parse standard text paragraphs
        if line:
            p_text = []
            while i < len(lines) and lines[i].strip() and not lines[i].strip().startswith('|'):
                p_text.append(lines[i].strip())
                i += 1
            full_p = ' '.join(p_text)
            full_p = re.sub(r'^#+\s*', '', full_p)
            cleaned_p = clean_tex_text(full_p)
            if cleaned_p:
                tex_content.append(cleaned_p + "\n\n")
        else:
            i += 1
            
    return '\n'.join(tex_content)

def build_tex():
    lore_dir = "lore"
    files = []
    for f in os.listdir(lore_dir):
        if f.endswith(".md"):
            path = os.path.join(lore_dir, f)
            date = get_file_date(path)
            files.append((date, path))
            
    files.sort(key=lambda x: x[0])
    
    tex_document = []
    
    # Preambles for 9x7 paper book layout in 2 columns
    tex_document.append("\\documentclass[9pt,twocolumn]{article}")
    tex_document.append("\\usepackage[paperwidth=9in,paperheight=7in,margin=0.5in]{geometry}")
    tex_document.append("\\usepackage{mathptmx}") # Times-Roman font
    tex_document.append("\\usepackage{graphicx}")
    tex_document.append("\\usepackage{multicol}")
    tex_document.append("\\usepackage{fancyhdr}")
    tex_document.append("\\pagestyle{fancy}")
    tex_document.append("\\fancyhf{}")
    tex_document.append("\\fancyhead[L]{PROCEEDINGS OF THE ACM, VOL. 18, 1961}")
    tex_document.append("\\fancyhead[R]{LORE COMPENDIUM}")
    tex_document.append("\\fancyfoot[C]{\\thepage}")
    tex_document.append("\\renewcommand{\\headrulewidth}{0.5pt}")
    tex_document.append("\\renewcommand{\\footrulewidth}{0.5pt}")
    
    tex_document.append("\\begin{document}")
    
    # Document main title
    tex_document.append("\\twocolumn[")
    tex_document.append("\\begin{center}")
    tex_document.append("{\\Huge\\bf COMPENDIUM OF LORE AND HISTORICAL TRANSCRIPTS}\\\\")
    tex_document.append("\\vspace{0.1in}")
    tex_document.append("{\\large\\it Compiled Chronologically under ACM 1961 Standards}\\\\")
    tex_document.append("\\vspace{0.2in}")
    tex_document.append("\\end{center}")
    tex_document.append("]")
    
    # Add each article content
    for date, filepath in files:
        tex_document.append(convert_md_file_to_tex(filepath, date))
        tex_document.append("\\vspace{0.2in}\\hrule\\vspace{0.2in}")
        
    tex_document.append("\\end{document}")
    
    output_path = "lore_compendium.tex"
    with open(output_path, "w", encoding="utf-8") as out:
        out.write('\n'.join(tex_document))
        
    print(f"LaTeX document build complete: {output_path}")

if __name__ == "__main__":
    build_tex()
