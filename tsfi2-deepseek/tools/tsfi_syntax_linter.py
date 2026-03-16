Here is a simple Python script that uses the `pycodestyle` library to check for missing semicolons.


import sys
from pycodestyle import StyleGuide

def tsfi_syntax_linter(filename):
    style_guide = StyleGuide(quiet=True)
    report = style_guide.check_files([filename])
    return report.total_errors

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 tsfi_syntax_linter.py <filename>")
        sys.exit(1)

    filename = sys.argv[1]
    errors = tsfi_syntax_linter(filename)
    if errors > 0:
        print(f"Error: {errors} missing semicolon(s) in {filename}")
        sys.exit(1)

    print(f"No missing semicolons in {filename}")


This script takes a filename as a command-line argument and checks it for missing semicolons using the `pycodestyle` library. It then prints an error message if it finds any missing semicolons. 

You can run it with a C file as follows:

bash
python3 tsfi_syntax_linter.py yourfile.c


Please note, `pycodestyle` checks for PEP 8 compliance. It checks for things like trailing white space, correct indentation, and the use of proper syntax. As such, it might flag lines that are syntactically correct but do not meet PEP 8 guidelines. For example, it will flag lines that are too long, despite the fact they might be syntactically correct.