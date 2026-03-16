import re

with open('src/tsfi_teddy_editor.c', 'r') as f:
    lines = f.readlines()

out = []
in_font_block = False

for line in lines:
    if 'stb_truetype' in line or 'tsfi_ottype' in line or 'tsfi_font' in line:
        continue
        
    out.append(line)

with open('src/tsfi_teddy_editor.c', 'w') as f:
    f.writelines(out)

