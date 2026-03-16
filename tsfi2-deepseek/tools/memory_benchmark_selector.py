import sys
import json
from cf_api import get_problemset, fetch_problem_statement

def is_memory_intensive(problem):
    """Determine if a problem is likely to stress a custom memory allocator."""
    if 'rating' not in problem or problem['rating'] < 2400:
        return False
        
    target_tags = {'data structures', 'dsu', 'trees', 'graphs', 'implementation', 'strings'}
    problem_tags = set(problem.get('tags', []))
    
    return bool(target_tags.intersection(problem_tags))

def select_memory_benchmarks(limit=10):
    print("Fetching Codeforces problemset...")
    problems = get_problemset()
    if not problems:
        print("Failed to fetch problems.")
        return []

    print(f"Total problems fetched: {len(problems)}")
    
    candidates = [p for p in problems if is_memory_intensive(p)]
    # Sort by rating descending to get the hardest ones
    candidates.sort(key=lambda p: p.get('rating', 0), reverse=True)
    
    print(f"Found {len(candidates)} memory-intensive candidates (Rating >= 2400).")
    
    benchmarks = []
    
    for p in candidates[:limit]:
        cid = p['contestId']
        idx = p['index']
        rating = p.get('rating', 0)
        tags = p.get('tags', [])
        
        print(f"Probing {cid}{idx} (Rating {rating})... ", end='', flush=True)
        title, mem_limit = fetch_problem_statement(cid, idx)
        
        if title:
            print(f"Mem Limit: {mem_limit}")
            benchmarks.append({
                "id": f"{cid}{idx}",
                "contestId": cid,
                "index": idx,
                "name": title,
                "rating": rating,
                "tags": tags,
                "memory_limit": mem_limit
            })
        else:
            print("Failed.")

    return benchmarks

if __name__ == "__main__":
    count = int(sys.argv[1]) if len(sys.argv) > 1 else 10
    benchmarks = select_memory_benchmarks(count)
    
    with open('benchmark_queue.json', 'w') as f:
        json.dump(benchmarks, f, indent=4)
        
    print(f"\\nSuccessfully generated benchmark_queue.json with {len(benchmarks)} candidates.")
