import subprocess
import sys
import re

def fetch(cid, idx):
    url = f"https://codeforces.com/problemset/problem/{cid}/{idx}"
    res = subprocess.run(['curl', '-s', '-L', '-A', 'Mozilla/5.0', url], capture_output=True, text=True)
    if res.returncode != 0:
        print("Failed to fetch")
        return
    
    html = res.stdout
    match = re.search(r'<div class="problem-statement">(.*?)<script', html, re.DOTALL)
    if match:
        text = match.group(1)
        text = re.sub(r'<[^>]+>', ' ', text)
        text = re.sub(r'\$\$\$(.*?)\$\$\$', r'\1', text)
        print(text[:2000]) # Print first 2000 chars for analysis
    else:
        print("Could not parse problem statement.")

if __name__ == '__main__':
    fetch(sys.argv[1], sys.argv[2])
