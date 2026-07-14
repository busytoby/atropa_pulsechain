import os
import json
import struct
import urllib.request
import urllib.parse

def fetch_and_map_scholar():
    query = "knowledge graph"
    encoded_query = urllib.parse.quote(query)
    url = f"https://api.semanticscholar.org/graph/v1/paper/search?query={encoded_query}&sortBy=citationCount&limit=5&fields=title,authors,citationCount,abstract,url"
    
    print(f"Querying Semantic Scholar API: {url}\n")
    try:
        req = urllib.request.Request(
            url, 
            headers={'User-Agent': 'Mozilla/5.0'}
        )
        with urllib.request.urlopen(req, timeout=8) as response:
            data = json.loads(response.read().decode('utf-8'))
            papers = data.get('data', [])
        print(f"✓ Successfully retrieved {len(papers)} papers from Semantic Scholar.\n")
    except Exception as e:
        print(f"API Query offline or rate-limited: {e}")
        print("Using offline simulated publication database for review...\n")
        # Robust offline fallback containing actual popular publication records
        papers = [
            {
                "title": "Translating Embeddings for Modeling Multi-relational Data",
                "authors": [{"name": "Antoine Bordes"}, {"name": "Nicolas Usunier"}],
                "citationCount": 5400,
                "abstract": "We consider the problem of embedding entities and relations of multi-relational graphs into low-dimensional vector spaces. Our goal is to propose a tool that is easy to train and scales to very large databases. We introduce TransE, a method which models relationships by interpreting them as translations in the embedding space.",
                "url": "https://proceedings.neurips.cc/paper/2013/file/14859757d89025142a7b80abf3747f5a-Paper.pdf"
            },
            {
                "title": "Knowledge Graph Embedding by Translating on Hyperplanes",
                "authors": [{"name": "Zhen Wang"}, {"name": "Jianwen Zhang"}],
                "citationCount": 3800,
                "abstract": "Knowledge graph embedding maps entities and relations into a continuous vector space. TransE is a promising method. However, it has flaws in dealing with 1-to-N, N-to-1, and N-to-N relations. We propose TransH which projects entities into relation-specific hyperplanes to resolve these mappings.",
                "url": "https://www.aaai.org/ocs/index.php/AAAI/AAAI14/paper/viewFiles/8531/8546"
            },
            {
                "title": "Knowledge Graph Embedding by Relational Rotation",
                "authors": [{"name": "Zhiqing Sun"}, {"name": "Zhi-Hong Deng"}],
                "citationCount": 2100,
                "abstract": "We present RotatE, a knowledge graph embedding method which defines relations as rotations in complex vector spaces. This enables modeling of key relation patterns including symmetry, asymmetry, inversion, and composition.",
                "url": "https://openreview.net/forum?id=HkgEQnRqYQ"
            }
        ]

    # Display documents for comprehensive review
    author_ids = []
    citations = []
    print("=" * 80)
    print("                       PUBLICATION DOCUMENTS REVIEW")
    print("=" * 80)
    for idx, paper in enumerate(papers):
        title = paper.get('title', 'Unknown Title')
        authors_list = ", ".join([a.get('name', 'Unknown') for a in paper.get('authors', [])])
        citation_count = paper.get('citationCount', 0)
        abstract = paper.get('abstract', 'No abstract available.')
        paper_url = paper.get('url', 'No URL available.')
        
        print(f"[{idx + 1}] {title}")
        print(f"    Authors:   {authors_list}")
        print(f"    Citations: {citation_count}")
        print(f"    URL:       {paper_url}")
        print(f"    Abstract:  {abstract[:300]}...")
        print("-" * 80)
        
        # Build node maps from authors
        author_ids.append(100 + idx)
        citations.append(citation_count)

    if not citations:
        citations = [1000]
        author_ids = [100]

    # Calculate popularity-biased scaling factor (avg citations scaled to fit uint32)
    avg_citations = sum(citations) / len(citations)
    bias_factor = int(1500 * (1.0 + (avg_citations / 1000.0)))
    if bias_factor > 10000:
        bias_factor = 10000 # Clamp limit

    # Node 0 (Root)
    node0 = (0, 0, 1000, 1000, len(author_ids), 1, 2, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 1 (NW quadrant - bias factor scaling)
    node1 = (0, 0, 500, 500, bias_factor, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    # Node 2 (NE quadrant - preferred author count)
    node2 = (500, 0, 1000, 500, len(author_ids), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
    
    nodes = [node0, node1, node2]
    out_file = 'assets/ouroboros_metadata.dat.bin'
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    
    with open(out_file, 'wb') as f:
        for node in nodes:
            f.write(struct.pack('IIIIIIIII', *node))
    print(f"✓ Saved Ouroboros preference metadata to: {out_file}\n")

if __name__ == '__main__':
    fetch_and_map_scholar()
