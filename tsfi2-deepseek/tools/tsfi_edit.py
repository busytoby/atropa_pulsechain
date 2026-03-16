import sys
import os
import re

def print_usage():
    print("Usage: python3 tools/tsfi_edit.py <file_path> <mode> <arg1> [arg2]")
    print("Modes:")
    print("  write <content_file>              - Overwrites file_path with contents of content_file")
    print("  append <content_file>             - Appends contents of content_file to file_path")
    print("  replace <search_file> <rep_file>  - Replaces exact contents of search_file with rep_file in file_path")
    print("  regex <pattern_file> <rep_file>   - Replaces regex pattern in pattern_file with rep_file in file_path")

if len(sys.argv) < 4:
    print_usage()
    sys.exit(1)

file_path = sys.argv[1]
mode = sys.argv[2]

if mode == "write":
    content_file = sys.argv[3]
    with open(content_file, "r") as f:
        content = f.read()
    with open(file_path, "w") as f:
        f.write(content)
    print(f"[TSFi Edit] Successfully wrote to {file_path}")

elif mode == "append":
    content_file = sys.argv[3]
    with open(content_file, "r") as f:
        content = f.read()
    with open(file_path, "a") as f:
        f.write(content)
    print(f"[TSFi Edit] Successfully appended to {file_path}")

elif mode == "replace":
    if len(sys.argv) < 5:
        print_usage()
        sys.exit(1)
    search_file = sys.argv[3]
    replace_file = sys.argv[4]
    
    with open(search_file, "r") as f:
        search_str = f.read()
    with open(replace_file, "r") as f:
        replace_str = f.read()
        
    with open(file_path, "r") as f:
        target_content = f.read()
        
    if search_str not in target_content:
        print(f"[TSFi Edit] FRACTURE: Search string not found in {file_path}")
        sys.exit(1)
        
    new_content = target_content.replace(search_str, replace_str)
    
    with open(file_path, "w") as f:
        f.write(new_content)
    print(f"[TSFi Edit] Successfully replaced content in {file_path}")

elif mode == "regex":
    if len(sys.argv) < 5:
        print_usage()
        sys.exit(1)
    pattern_file = sys.argv[3]
    replace_file = sys.argv[4]
    
    with open(pattern_file, "r") as f:
        pattern_str = f.read().strip()
    with open(replace_file, "r") as f:
        replace_str = f.read()
        
    with open(file_path, "r") as f:
        target_content = f.read()
        
    try:
        new_content, count = re.subn(pattern_str, replace_str, target_content, flags=re.DOTALL)
        if count == 0:
            print(f"[TSFi Edit] WARNING: Regex pattern matched 0 occurrences in {file_path}")
        else:
            with open(file_path, "w") as f:
                f.write(new_content)
            print(f"[TSFi Edit] Successfully regex-replaced {count} occurrence(s) in {file_path}")
    except re.error as e:
        print(f"[TSFi Edit] FRACTURE: Invalid regex pattern: {e}")
        sys.exit(1)

else:
    print(f"Unknown mode: {mode}")
    print_usage()
    sys.exit(1)
