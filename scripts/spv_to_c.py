import struct
import sys

def convert_spv_to_c(spv_path, output_path):
    with open(spv_path, 'rb') as f:
        data = f.read()
    
    # Align to 32-bit words
    words = []
    for i in range(0, len(data), 4):
        word_data = data[i:i+4]
        if len(word_data) < 4:
            word_data = word_data.ljust(4, b'\x00')
        word = struct.unpack('<I', word_data)[0]
        words.append(word)
    
    # Generate C code
    c_lines = [
        "#pragma once",
        "#include <stdint.h>",
        "#include <stddef.h>",
        "",
        "// Compiled SPIR-V compute shader for 19D weight traversal",
        f"const uint32_t zmm_traverse_spv[] = {{"
    ]
    
    # Format words in rows of 6
    row_words = []
    for idx, word in enumerate(words):
        row_words.append(f"0x{word:08x}")
        if len(row_words) == 6 or idx == len(words) - 1:
            comma = "," if idx < len(words) - 1 else ""
            c_lines.append("    " + ", ".join(row_words) + comma)
            row_words = []
            
    c_lines.append("};")
    c_lines.append(f"const size_t zmm_traverse_spv_size = sizeof(zmm_traverse_spv);")
    c_lines.append("")
    
    with open(output_path, 'w') as f:
        f.write("\n".join(c_lines))
    print(f"Successfully converted {spv_path} to {output_path} ({len(words)} words)")

if __name__ == '__main__':
    convert_spv_to_c('shaders/zmm_traverse.spv', 'shaders/zmm_traverse_spv.h')
