import json
import sys
import os

def generate_stub(benchmark):
    file_name = f"cf_{benchmark['id']}.c"
    
    stub_content = f"""// {benchmark['name']} (Codeforces {benchmark['id']})
// Rating: {benchmark['rating']} | Tags: {', '.join(benchmark['tags'])}
// Memory Limit: {benchmark['memory_limit']}
// NOTE: Using TSFi2 lau_memory allocator for profiling.

#include <stdio.h>
#include <stdlib.h>
#include "../src/lau_malloc.h"

// Override standard allocator to stress test lau_memory
#define malloc(s) lau_malloc(s)
#define free(p) lau_free(p)
#define calloc(n, s) lau_calloc(n, s)
#define realloc(p, s) lau_realloc(p, s)

void solve() {{
    // TODO: Implement solution using standard IO and dynamic structures
}}

int main() {{
    int t = 1;
    // if (scanf("%d", &t) != 1) return 0; // Uncomment if multiple test cases
    while (t--) {{
        solve();
    }}
    return 0;
}}
"""
    with open(file_name, "w") as f:
        f.write(stub_content)
        
    print(f"Generated C stub for {benchmark['id']} at {file_name}")
    print(f"To compile and test overhead: gcc -O3 -I../src {file_name} ../src/lau_malloc.c ../src/lau_memory.c -o {benchmark['id']}_test")

def main():
    if not os.path.exists("benchmark_queue.json"):
        print("benchmark_queue.json not found. Run memory_benchmark_selector.py first.")
        return
        
    with open("benchmark_queue.json", "r") as f:
        queue = json.load(f)
        
    if not queue:
        print("Queue is empty.")
        return
        
    benchmark = queue.pop(0)
    
    with open("benchmark_queue.json", "w") as f:
        json.dump(queue, f, indent=4)
        
    generate_stub(benchmark)

if __name__ == "__main__":
    main()
