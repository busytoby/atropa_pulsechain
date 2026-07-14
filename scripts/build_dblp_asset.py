import os
import struct

def build_dblp_graph():
    # Construct a 5-node quadtree representing DBLP authors/venues
    # Format per node: x_min, y_min, x_max, y_max, value, children[4]
    # Total size per node: 9 * 4 = 36 bytes
    
    # Node 0 (Root)
    node0 = (0, 0, 1000, 1000, 50, 1, 2, 3, 4)
    # Node 1 (NW quadrant - Venue: 'AI')
    node1 = (0, 0, 500, 500, 80, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 2 (NE quadrant - Venue: 'Systems')
    node2 = (500, 0, 1000, 500, 90, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 3 (SW quadrant - Venue: 'Databases')
    node3 = (0, 500, 50, 1000, 75, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 4 (SE quadrant - Venue: 'Theory')
    node4 = (500, 500, 1000, 1000, 60, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    
    nodes = [node0, node1, node2, node3, node4]
    
    # Place directly inside project assets directory
    out_file = 'assets/dblp_graph.dat.bin'
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    
    with open(out_file, 'wb') as f:
        for node in nodes:
            f.write(struct.pack('IIIIIIIII', *node))
            
    print(f"✓ Successfully built DBLP quadtree asset at: {out_file}")

if __name__ == '__main__':
    build_dblp_graph()
