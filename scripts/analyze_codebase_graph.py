import os
import re
import struct

def analyze_codebase():
    interop_h = 'scripts/libantigravity_interop.h'
    interop_c = 'scripts/libantigravity_interop.c'
    test_c = 'scripts/test_abi_dispatch.c'
    
    # 1. Extract declared functions from header
    declared = set()
    if os.path.exists(interop_h):
        with open(interop_h, 'r') as f:
            content = f.read()
            # Match standard function declarations: return_type func_name(args);
            matches = re.findall(r'\b(interop_[a-zA-Z0-9_]+)\s*\(', content)
            for m in matches:
                declared.add(m)
                
    # 2. Extract implemented functions from C source
    implemented = set()
    if os.path.exists(interop_c):
        with open(interop_c, 'r') as f:
            content = f.read()
            matches = re.findall(r'\b(interop_[a-zA-Z0-9_]+)\s*\(', content)
            for m in matches:
                if m in declared:
                    implemented.add(m)
                    
    # 3. Extract verified functions from test file
    tested = set()
    if os.path.exists(test_c):
        with open(test_c, 'r') as f:
            content = f.read()
            for func in declared:
                if func in content:
                    tested.add(func)
                    
    untested = implemented - tested
    
    print(f"Codebase Analysis Metrics:")
    print(f"  Total Declared APIs:  {len(declared)}")
    print(f"  Implemented APIs:     {len(implemented)}")
    print(f"  Tested/Verified APIs: {len(tested)}")
    print(f"  Untested/Gaps:        {len(untested)}")
    if untested:
        print("\nUntested API Functions:")
        for fn in sorted(untested):
            print(f"  - {fn}")
        print()
    
    # 4. Serialize metrics into a standard binary quadtree asset
    # Node 0 (Root)
    node0 = (0, 0, 1000, 1000, len(declared), 1, 2, 3, 4)
    # Node 1 (NW quadrant - Tested/Verified count)
    node1 = (0, 0, 500, 500, len(tested), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 2 (NE quadrant - Untested/Gaps count)
    node2 = (500, 0, 1000, 500, len(untested), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 3 (SW quadrant - Total Source Files Scanned)
    node3 = (0, 500, 50, 1000, 3, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 4 (SE quadrant - Redundancy Cycles count)
    node4 = (500, 500, 1000, 1000, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    
    nodes = [node0, node1, node2, node3, node4]
    
    out_file = 'assets/codebase_graph.dat.bin'
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    
    with open(out_file, 'wb') as f:
        for node in nodes:
            f.write(struct.pack('IIIIIIIII', *node))
            
    print(f"✓ Successfully built codebase dependency graph asset at: {out_file}")

if __name__ == '__main__':
    analyze_codebase()
