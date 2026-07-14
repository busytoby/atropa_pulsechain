import sys
import os
import struct

# Nodes & Relations lookup tables
NODE_NAMES = ["WinchesterMQ", "Dysnomia VM", "ZMM Executor", "LAU Ledger", "EVM Bridge"]
RELATION_NAMES = ["routes", "executes", "proves", "inherits"]

def serialize_kg_to_dna(triples, out_path):
    """
    Serializes a list of knowledge graph triples (head, relation, tail, weight)
    into a biological-style .dna binary codon sequence.
    Each codon is exactly 4 bytes.
    """
    binary_data = bytearray()
    
    for h, r, t, w in triples:
        # Resolve string names or keep indices
        h_idx = NODE_NAMES.index(h) if isinstance(h, str) else int(h)
        r_idx = RELATION_NAMES.index(r) if isinstance(r, str) else int(r)
        t_idx = NODE_NAMES.index(t) if isinstance(t, str) else int(t)
        weight = int(w) & 0xFF
        
        # Pack into 4 bytes (Big-Endian)
        codon = struct.pack("=BBBB", h_idx, r_idx, t_idx, weight)
        binary_data.extend(codon)
        
    with open(out_path, "wb") as f:
        f.write(binary_data)
    print(f"✓ Serialized {len(triples)} KG triples to DNA file: {out_path} ({len(binary_data)} bytes)")

def deserialize_dna_to_kg(dna_path):
    """
    Deserializes a binary .dna codon file back into a structured knowledge graph list.
    """
    triples = []
    if not os.path.exists(dna_path):
        print(f"[Error] DNA file not found at {dna_path}")
        return triples
        
    with open(dna_path, "rb") as f:
        binary_data = f.read()
        
    num_codons = len(binary_data) // 4
    for c in range(num_codons):
        h_idx, r_idx, t_idx, weight = struct.unpack("=BBBB", binary_data[c*4 : (c+1)*4])
        
        # Resolve names with safety fallback
        h_name = NODE_NAMES[h_idx % len(NODE_NAMES)]
        r_name = RELATION_NAMES[r_idx % len(RELATION_NAMES)]
        t_name = NODE_NAMES[t_idx % len(NODE_NAMES)]
        
        triples.append((h_name, r_name, t_name, weight))
        
    return triples

def main():
    print("=== Knowledge Graph DNA & RDBMS Quadtree Serialization Utility ===")
    
    # 1. Define sample knowledge graph triples
    sample_kg = [
        ("WinchesterMQ", "routes", "Dysnomia VM", 204),
        ("Dysnomia VM", "executes", "ZMM Executor", 170),
        ("ZMM Executor", "proves", "LAU Ledger", 255),
        ("LAU Ledger", "inherits", "EVM Bridge", 187)
    ]
    
    dna_file = "assets/bear_genome.dna"
    
    # Serialize to DNA
    serialize_kg_to_dna(sample_kg, dna_file)
    
    # Deserialize from DNA
    decoded_kg = deserialize_dna_to_kg(dna_file)
    print("\nDecoded Knowledge Graph from DNA:")
    for idx, (h, r, t, w) in enumerate(decoded_kg, 1):
        print(f"  {idx}. ({h}) --[{r} (weight: {w})]--> ({t})")
        
    # 2. Simulate RDBMS row serialization to quadtree DAT files (.dat.bin)
    # Following Rule 13, all quadtree media assets must end in .dat.bin
    print("\nSimulating RDBMS table rows serialization to quadtree DAT (.dat.bin)...")
    rdbms_rows = [
        {"row_id": 1, "key": 0x1122, "val": 0xaabb, "x": -0.5, "y": 0.5},
        {"row_id": 2, "key": 0x3344, "val": 0xccdd, "x": 0.5, "y": -0.5}
    ]
    
    quadtree_file = "assets/table_rdbms.dat.bin"
    # Structure definition: row_id (I), key (Q), val (Q), x (f), y (f)
    with open(quadtree_file, "wb") as f:
        for row in rdbms_rows:
            f.write(struct.pack("=IQQff", row["row_id"], row["key"], row["val"], row["x"], row["y"]))
            
    print(f"✓ Serialized RDBMS table rows to quadtree media file: {quadtree_file} ({len(rdbms_rows) * 28} bytes)")

if __name__ == '__main__':
    main()
