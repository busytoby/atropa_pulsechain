import os
import struct
import urllib.request
import xml.etree.ElementTree as ET

def fetch_and_map_dblp():
    url = "https://dblp.org/search/pub/api?q=knowledge+graph&h=5"
    print(f"Fetching simulated publication nodes from: {url}")
    try:
        req = urllib.request.Request(
            url, 
            headers={'User-Agent': 'Mozilla/5.0'}
        )
        with urllib.request.urlopen(req, timeout=5) as response:
            xml_data = response.read()
        root = ET.fromstring(xml_data)
        hits = root.findall('.//hit')
        print(f"✓ Retrieved {len(hits)} DBLP publication results.")
    except Exception as e:
        print(f"Using offline simulated DBLP records: {e}")
        # Fallback offline simulation
        hits = []

    # Map parsed authors to graph IDs and popularity metrics (mock citations)
    author_ids = [10, 20, 30]
    citations = [120, 85, 40]
    avg_citations = sum(citations) / len(citations)
    
    # Scale bias factor based on average citation count
    # Base bias 1.5, scaled dynamically
    bias_factor = int(1500 * (1.0 + (avg_citations / 100.0)))
    
    print(f"Preferred authors mapped to graph nodes: {author_ids}")
    print(f"Citations: {citations} (avg: {avg_citations:.1f}) -> Bias factor scale: {bias_factor / 1000.0:.3f}")
    
    # Write preference metadata into a binary quadtree index (36-byte packed structures)
    # Node 0 (Root)
    node0 = (0, 0, 1000, 1000, len(author_ids), 1, 2, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 1 (NW quadrant - bias factor scaling scaled by 1000)
    node1 = (0, 0, 500, 500, bias_factor, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 2 (NE quadrant - preferred author count)
    node2 = (500, 0, 1000, 500, len(author_ids), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    
    nodes = [node0, node1, node2]
    out_file = 'assets/ouroboros_metadata.dat.bin'
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    
    with open(out_file, 'wb') as f:
        for node in nodes:
            f.write(struct.pack('IIIIIIIII', *node))
    print(f"✓ Saved Ouroboros preference metadata to: {out_file}")

if __name__ == '__main__':
    fetch_and_map_dblp()
