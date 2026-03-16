import os
import struct

def patch_execstack(filepath):
    print(f"=== TSFi ELF Binary Patcher ===")
    print(f"[INFO] Targeting {filepath}...")
    
    with open(filepath, 'r+b') as f:
        # Read ELF Header
        magic = f.read(4)
        if magic != b'\x7fELF':
            print("[FRACTURE] Not a valid ELF file.")
            return

        # Read architecture (1 = 32-bit, 2 = 64-bit)
        f.seek(4)
        arch = f.read(1)[0]
        if arch != 2:
            print("[FRACTURE] Only 64-bit ELF supported.")
            return

        # Read endianness (1 = LE, 2 = BE)
        f.seek(5)
        endian = f.read(1)[0]
        endian_char = '<' if endian == 1 else '>'

        # Read Program Header Offset (e_phoff is at 0x20 for 64-bit)
        f.seek(0x20)
        e_phoff = struct.unpack(endian_char + 'Q', f.read(8))[0]

        # Read Size of Program Header Entry (e_phentsize is at 0x36 for 64-bit)
        f.seek(0x36)
        e_phentsize = struct.unpack(endian_char + 'H', f.read(2))[0]

        # Read Number of Program Header Entries (e_phnum is at 0x38 for 64-bit)
        f.seek(0x38)
        e_phnum = struct.unpack(endian_char + 'H', f.read(2))[0]

        print(f"  -> Found {e_phnum} Program Headers at offset 0x{e_phoff:X}")

        # Iterate through program headers to find PT_GNU_STACK
        PT_GNU_STACK = 0x6474e551
        found = False

        for i in range(e_phnum):
            ph_offset = e_phoff + (i * e_phentsize)
            f.seek(ph_offset)
            
            # Program Header 64-bit struct: 
            # uint32 p_type, uint32 p_flags, uint64 p_offset, uint64 p_vaddr, 
            # uint64 p_paddr, uint64 p_filesz, uint64 p_memsz, uint64 p_align
            p_type, p_flags = struct.unpack(endian_char + 'II', f.read(8))
            
            if p_type == PT_GNU_STACK:
                found = True
                print(f"  -> [PASS] Located PT_GNU_STACK header at offset 0x{ph_offset:X}")
                print(f"  -> Current Flags: 0x{p_flags:X}")
                
                # Check if it has Execute permission (PF_X = 1, PF_W = 2, PF_R = 4)
                # If flags == 7 (RWX), we want to change it to 6 (RW)
                if p_flags & 1:
                    print("  -> Executable flag detected. Mathematically severing Execute permission...")
                    new_flags = p_flags & ~1 # Clear the PF_X bit
                    
                    # Write the new flags back to the binary
                    f.seek(ph_offset + 4)
                    f.write(struct.pack(endian_char + 'I', new_flags))
                    print(f"  -> [ABSOLUTE SUCCESS] Flags mutated to: 0x{new_flags:X} (RW)")
                else:
                    print("  -> Binary is already secure. No Executable stack flag found.")
                break

        if not found:
            print("[WARN] PT_GNU_STACK header not found in binary.")

import sys

if __name__ == "__main__":
    if len(sys.argv) > 1:
        target_lib = sys.argv[1]
    else:
        target_lib = "rocm_env/lib/python3.11/site-packages/torch/lib/libamdhip64.so"
        
    if os.path.exists(target_lib):
        patch_execstack(target_lib)
    else:
        print(f"[FRACTURE] Target library missing: {target_lib}")
