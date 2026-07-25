#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Auncient Lexicon Database Generator
Serializes Transfluxor word configurations into conforming binary databases.
"""

import sys
import struct
import argparse

def derive_frequencies(command_id):
    # F1 = 300.0 + ((command_id * 50) % 700)
    f1 = 300.0 + float((command_id * 50) % 700)
    f2 = f1 * 2.0
    return f1, f2

def derive_decay(name):
    if name.startswith("WMQ_"):
        return 0.4
    elif name.startswith("ABI_"):
        return 0.2
    elif name.startswith("SPK_RELEASE"):
        return 0.5
    return 0.3

def main():
    parser = argparse.ArgumentParser(description="Generate Auncient Lexicon binary dictionary files.")
    parser.add_argument("-o", "--output", required=True, help="Output target file path (must end with .dat.bin)")
    parser.add_argument("--words", nargs="+", help="Space-separated word parameters: NAME,ID,WMQ_CMD,ABI_OP", default=[])
    args = parser.parse_args()

    # Rule 13 check
    if not args.output.lower().endswith(".dat.bin"):
        print("[ERROR] Export target must have a .dat.bin extension.")
        sys.exit(1)

    packed_words = []
    for word_str in args.words:
        parts = word_str.split(",")
        if len(parts) != 4:
            print(f"[ERROR] Invalid word definition format: {word_str}. Use NAME,ID,WMQ_CMD,ABI_OP")
            sys.exit(1)
        
        name = parts[0]
        command_id = int(parts[1])
        wmq_cmd = int(parts[2], 16)
        abi_op = int(parts[3], 16)

        f1, f2 = derive_frequencies(command_id)
        decay = derive_decay(name)
        amplitude = 5.0 # Standard Wheeler Jump amplitude

        # Pack C structure: auncient_transfluxor_word_t
        # char name[32] -> 32s
        # uint32_t word_id -> I
        # 4 bytes padding -> 4x
        # double f1, f2, decay, amplitude -> d, d, d, d (actually 5 doubles in C structure: f1, f2, decay, amplitude)
        # Wait, f1, f2, decay, amplitude = 4 doubles. Let's look at C structure definition:
        # typedef struct {
        #     char name[32];
        #     uint32_t word_id;
        #     double f1;
        #     double f2;
        #     double decay;
        #     double amplitude;
        #     uint32_t wmq_cmd;
        #     uint32_t abi_op;
        # } auncient_transfluxor_word_t;
        # Wait! The C structure has: f1 (double), f2 (double), decay (double), amplitude (double) -> 4 doubles!
        # And wmq_cmd (uint32_t), abi_op (uint32_t) -> 2 uint32_ts.
        # Pack string: 32s (32 bytes) + I (4 bytes) + 4x (4 padding bytes) + d (8) + d (8) + d (8) + d (8) + I (4) + I (4) = 80 bytes.
        # Let's pack!
        name_bytes = name.encode("utf-8")[:31]
        packed = struct.pack("32s I 4x d d d d I I", name_bytes, command_id, f1, f2, decay, amplitude, wmq_cmd, abi_op)
        packed_words.append(packed)

    with open(args.output, "wb") as f:
        # Write AUNC signature and word count
        f.write(b"AUNC")
        f.write(struct.pack("I", len(packed_words)))
        for pw in packed_words:
            f.write(pw)

    print(f"[SUCCESS] Exported {len(packed_words)} words to '{args.output}'")

if __name__ == "__main__":
    main()
