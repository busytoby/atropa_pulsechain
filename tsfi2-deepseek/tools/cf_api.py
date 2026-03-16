import urllib.request
import json
import subprocess
import re

def get_problemset():
    """Fetches the Codeforces problemset."""
    url = "https://codeforces.com/api/problemset.problems"
    try:
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            if data['status'] == 'OK':
                return data['result']['problems']
            else:
                print(f"Error fetching problemset: {data.get('comment')}")
                return []
    except Exception as e:
        print(f"Exception fetching problemset: {e}")
        return []

def fetch_problem_statement(contest_id, index):
    """Fetches the problem statement and attempts to extract limits."""
    url = f"https://codeforces.com/problemset/problem/{contest_id}/{index}"
    result = subprocess.run(
        ['curl', '-s', '-L', '-A', 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36', url],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        return None, None
    
    html = result.stdout
    
    # Try to extract memory limit
    mem_limit = "Unknown"
    mem_match = re.search(r'memory limit per test</div>(.*?)</div>', html, re.DOTALL)
    if mem_match:
        mem_text = mem_match.group(1)
        mem_text = re.sub(r'<[^>]+>', '', mem_text).strip()
        mem_limit = mem_text

    # Extract title
    title_match = re.search(r'<div class="title">(.*?)</div>', html)
    title = title_match.group(1) if title_match else f"Problem {contest_id}{index}"

    return title, mem_limit
