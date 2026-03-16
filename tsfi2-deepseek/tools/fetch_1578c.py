import subprocess
import re
import sys

def get_cf_problem(contest_id, problem_index):
    url = f"https://codeforces.com/problemset/problem/{contest_id}/{problem_index}"
    result = subprocess.run(['curl', '-s', '-L', '-A', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36', url], capture_output=True, text=True)
    if result.returncode != 0:
        print("Failed to fetch")
        return
    
    html = result.stdout
    
    title_match = re.search(r'<div class="title">(.*?)</div>', html)
    title = title_match.group(1) if title_match else "Unknown"
    
    # Very basic regex to get problem statement divs
    statement_match = re.search(r'<div class="problem-statement">(.*?)</div><script', html, re.DOTALL)
    
    if not statement_match:
        print("Could not parse problem statement.")
        # Just dump the text from the page body if possible
        import xml.etree.ElementTree as ET
        # Not ideal.
    else:
        statement = statement_match.group(1)
        statement = re.sub(r'<[^>]+>', ' ', statement)
        statement = re.sub(r'\$\$\$(.*?)\$\$\$', r'`\1`', statement)
        statement = re.sub(r'
\s*
', '

', statement)
        
        with open(f"problem_{contest_id}_{problem_index}.md", "w") as f:
            f.write(f"# {title}

{statement}")
        print(f"Saved problem_{contest_id}_{problem_index}.md")

if __name__ == "__main__":
    get_cf_problem(1578, "C")
