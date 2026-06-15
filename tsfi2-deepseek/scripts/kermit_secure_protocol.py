#!/usr/bin/env python3
import os
import sys
import hashlib
import random

# secp256k1 Curve Parameters
# y^2 = x^3 + 7 (mod p)
P_CURVE = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
A_CURVE = 0
B_CURVE = 7

def modular_sqrt(a, p):
    """
    Computes modular square root for p = 3 mod 4 curves.
    For secp256k1, p % 4 == 3 holds, so we can use y = a^((p+1)/4) mod p.
    """
    if pow(a, (p - 1) // 2, p) != 1:
        return None # No square root exists (not a quadratic residue)
    return pow(a, (p + 1) // 4, p)

def hash_to_curve(data_bytes):
    """
    Implements the Try-and-Increment method to map an arbitrary byte string
    to a valid coordinate (x, y) on the secp256k1 elliptic curve.
    """
    i = 0
    while True:
        # Hash data with iteration counter
        h = hashlib.sha256(data_bytes + i.to_bytes(4, 'little')).digest()
        x = int.from_bytes(h, 'big') % P_CURVE
        
        # Calculate y^2 = x^3 + 7 mod p
        y_sq = (pow(x, 3, P_CURVE) + B_CURVE) % P_CURVE
        y = modular_sqrt(y_sq, P_CURVE)
        
        if y is not None:
            return (x, y), i # Return coordinate and the iteration count
        i += 1

def derive_keystream(shared_secret, seq_num, length):
    """
    Derives a symmetric keystream segment from the shared secret and sequence number.
    """
    seed = shared_secret.to_bytes(32, 'big') + seq_num.to_bytes(4, 'little')
    keystream = b""
    counter = 0
    while len(keystream) < length:
        keystream += hashlib.sha256(seed + counter.to_bytes(4, 'little')).digest()
        counter += 1
    return keystream[:length]

def encrypt_decrypt(data_bytes, shared_secret, seq_num):
    """
    Symmetrically encrypts/decrypts a payload using the derived keystream.
    """
    keystream = derive_keystream(shared_secret, seq_num, len(data_bytes))
    return bytes(b1 ^ b2 for b1, b2 in zip(data_bytes, keystream))

# Kermit Protocol Frame Structure:
# [SOH] [LEN] [SEQ] [TYPE] [DATA] [CHECK]
# Checksum is the sum of characters (LEN to DATA) modulo 64, offset by 32 (ASCII printable)
SOH = 0x01

def encode_kermit_char(val):
    return chr((val & 0x3F) + 32)

def decode_kermit_char(ch):
    return ord(ch) - 32

def make_kermit_frame(seq, p_type, payload_str=""):
    """
    Assembles a complete Kermit frame with printable ASCII checksum mapping.
    """
    # Length of frame (from SEQ to check byte inclusive) is len(payload) + 3
    length_val = len(payload_str) + 3
    len_char = encode_kermit_char(length_val)
    seq_char = encode_kermit_char(seq)
    
    body = f"{len_char}{seq_char}{p_type}{payload_str}"
    
    # Calculate checksum (sum of all ASCII codes in body modulo 64)
    chk_val = sum(ord(c) for c in body) % 64
    chk_char = encode_kermit_char(chk_val)
    
    return f"\x01{body}{chk_char}"

def parse_kermit_frame(frame_str):
    """
    Parses a Kermit frame and verifies the checksum.
    """
    if not frame_str.startswith("\x01"):
        return None, "Missing SOH"
    
    body = frame_str[1:-1]
    checksum_char = frame_str[-1]
    
    if len(body) < 3:
        return None, "Frame body too short"
    
    len_val = decode_kermit_char(body[0])
    seq = decode_kermit_char(body[1])
    p_type = body[2]
    payload = body[3:]
    
    # Verify length
    if len_val != len(payload) + 3:
         return None, f"Length mismatch (header says {len_val}, got {len(payload)+3})"
         
    # Verify Checksum
    chk_val = sum(ord(c) for c in body) % 64
    if encode_kermit_char(chk_val) != checksum_char:
         return None, "Checksum verification failed"
         
    return {"seq": seq, "type": p_type, "payload": payload}, None

class KermitSessionSimulator:
    """
    Simulates a secure Kermit handshake and data transfer session between Alice and Bob.
    """
    def __init__(self, data_stream, shared_secret_val):
        self.data_stream = data_stream
        self.shared_secret = shared_secret_val
        self.seq = 0
        
    def run_handshake(self):
        print("\n--- STAGE 1: SECURE KERMIT HANDSHAKE INITIALIZATION ---")
        
        # 1. Alice sends Send-Init (S)
        alice_init_payload = "WINDOW=8;EC=SECP256K1"
        alice_frame = make_kermit_frame(self.seq, 'S', alice_init_payload)
        print(f"[Alice -> Buffer] Emit 'S' (Send-Init) | Seq={self.seq} | Payload: '{alice_init_payload}'")
        
        # Bob receives and parses
        parsed, err = parse_kermit_frame(alice_frame)
        if err:
            print(f"[ERROR] Bob failed parsing Alice init: {err}")
            return False
        
        # Bob responds with ACK (Y) containing his init parameters
        bob_init_payload = "WINDOW=8;EC=OK"
        bob_frame = make_kermit_frame(self.seq, 'Y', bob_init_payload)
        print(f"[Bob -> Buffer] Emit 'Y' (ACK)       | Seq={self.seq} | Payload: '{bob_init_payload}'")
        
        # Alice parses Bob's ACK
        parsed, err = parse_kermit_frame(bob_frame)
        if err:
             print(f"[ERROR] Alice failed parsing Bob ACK: {err}")
             return False
             
        self.seq = (self.seq + 1) % 64
        
        # 2. Alice sends File Header (F)
        filename_payload = "vaesen_character_data.bin"
        alice_file_frame = make_kermit_frame(self.seq, 'F', filename_payload)
        print(f"[Alice -> Buffer] Emit 'F' (File Header)| Seq={self.seq} | Filename: '{filename_payload}'")
        
        # Bob parses and ACKs
        parsed, err = parse_kermit_frame(alice_file_frame)
        if err:
             print(f"[ERROR] Bob failed parsing Alice file header: {err}")
             return False
             
        bob_file_ack = make_kermit_frame(self.seq, 'Y')
        print(f"[Bob -> Buffer] Emit 'Y' (ACK)       | Seq={self.seq}")
        
        self.seq = (self.seq + 1) % 64
        return True

    def transfer_data(self):
        print("\n--- STAGE 2: ELLIPTIC CURVE DATA PACKETIZATION & TRANSFER ---")
        
        # Split stream into chunks (e.g. 16 bytes per chunk)
        chunk_size = 16
        chunks = [self.data_stream[i:i+chunk_size] for i in range(0, len(self.data_stream), chunk_size)]
        
        received_points = []
        recovered_data = b""
        
        for idx, chunk in enumerate(chunks):
            # 1. Map input chunk to an Elliptic Curve Point
            point, tries = hash_to_curve(chunk)
            print(f"[Alice] Chunk {idx+1}: Mapped '{chunk.hex()}' onto secp256k1 point:")
            print(f"        X: {hex(point[0])}")
            print(f"        Y: {hex(point[1])} (found in {tries} tries)")
            
            # 2. Encrypt the raw bytes before transmission
            encrypted_payload = encrypt_decrypt(chunk, self.shared_secret, self.seq)
            
            # 3. Encapsulate inside Kermit Data (D) Frame (as hex string)
            hex_payload = encrypted_payload.hex()
            data_frame = make_kermit_frame(self.seq, 'D', hex_payload)
            print(f"[Alice -> Buffer] Emit 'D' (Data Block) | Seq={self.seq} | Payload (Hex): {hex_payload}")
            
            # 4. Bob receives, parses, decrypts
            parsed, err = parse_kermit_frame(data_frame)
            if err:
                print(f"[ERROR] Bob failed parsing data frame: {err}")
                return False
                
            bob_decrypted = encrypt_decrypt(bytes.fromhex(parsed["payload"]), self.shared_secret, parsed["seq"])
            recovered_data += bob_decrypted
            
            # Verify Bob can map the recovered data to the exact same curve point
            bob_point, _ = hash_to_curve(bob_decrypted)
            assert bob_point == point, "Homomorphic coordinate verification failed!"
            received_points.append(bob_point)
            
            # Bob ACKs
            ack_frame = make_kermit_frame(self.seq, 'Y')
            print(f"[Bob -> Buffer] Emit 'Y' (ACK)       | Seq={self.seq} | Point verified.")
            
            self.seq = (self.seq + 1) % 64
            
        # 3. Alice sends EOF (Z)
        alice_eof_frame = make_kermit_frame(self.seq, 'Z')
        print(f"[Alice -> Buffer] Emit 'Z' (EOF)        | Seq={self.seq}")
        parsed, err = parse_kermit_frame(alice_eof_frame)
        bob_eof_ack = make_kermit_frame(self.seq, 'Y')
        print(f"[Bob -> Buffer] Emit 'Y' (ACK)       | Seq={self.seq}")
        
        self.seq = (self.seq + 1) % 64
        
        # 4. Alice sends Break/EOT (B)
        alice_eot_frame = make_kermit_frame(self.seq, 'B')
        print(f"[Alice -> Buffer] Emit 'B' (EOT)        | Seq={self.seq}")
        parsed, err = parse_kermit_frame(alice_eot_frame)
        bob_eot_ack = make_kermit_frame(self.seq, 'Y')
        print(f"[Bob -> Buffer] Emit 'Y' (ACK)       | Seq={self.seq} | Connection closed.")
        
        print("\n--- STAGE 3: TRANSMISSION VERIFICATION SUMMARY ---")
        print(f"Total Elliptic Curve Points Exchanged: {len(received_points)}")
        print(f"Original Input Stream Bytes:         {self.data_stream}")
        print(f"Recovered Output Stream Bytes:       {recovered_data}")
        assert recovered_data == self.data_stream, "Integrity check failed: mismatch between sent and received bytes!"
        print("[SUCCESS] All packets transmitted and verified homomorphically.")
        return True

if __name__ == "__main__":
    print("=== Secure Kermit Elliptic Curve Handshake & Transmission ===")
    
    # We will transmit a slice of the childlike speech synthesizer parameters
    speech_coefficients = b"AnaCoeffs:15,-25,40,-10,5,-8,12,-5;Pitch:420Hz"
    
    # Derive a dummy ECDH shared secret for the symmetric keystream (represent as scalar integer)
    ecdh_shared_secret = 0xDEEB5EECD4E9785A3F0BCE1A46789D426E7A167C5A059E18F2F3C54BDEEA3000
    
    session = KermitSessionSimulator(speech_coefficients, ecdh_shared_secret)
    if session.run_handshake():
        session.transfer_data()
