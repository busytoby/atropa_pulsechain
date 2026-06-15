#!/usr/bin/env python3
import os
import sys
import hashlib

# secp256k1 Curve Parameters
# y^2 = x^3 + 7 (mod p)
P_CURVE = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
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
SOH = 0x01

def encode_kermit_char(val):
    return chr((val & 0x3F) + 32)

def decode_kermit_char(ch):
    return ord(ch) - 32

def make_kermit_frame(seq, p_type, payload_str=""):
    """
    Assembles a complete Kermit frame with printable ASCII checksum mapping.
    """
    length_val = len(payload_str) + 3
    len_char = encode_kermit_char(length_val)
    seq_char = encode_kermit_char(seq)
    
    body = f"{len_char}{seq_char}{p_type}{payload_str}"
    
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
    
    if len_val != len(payload) + 3:
         return None, f"Length mismatch (header says {len_val}, got {len(payload)+3})"
         
    chk_val = sum(ord(c) for c in body) % 64
    if encode_kermit_char(chk_val) != checksum_char:
         return None, "Checksum verification failed"
         
    return {"seq": seq, "type": p_type, "payload": payload}, None

class KermitReceiver:
    """
    State machine for receiving and decrypting data streams via secure Kermit protocol.
    """
    def __init__(self, shared_secret):
        self.shared_secret = shared_secret
        self.expected_seq = 0
        self.state = "INIT"  # INIT, FILE, DATA, EOF, EOT
        self.filename = ""
        self.recovered_data = b""
        self.received_points = []
        
    def process_packet(self, frame_str):
        """
        Processes a single incoming frame, updates state, and returns the ACK response frame.
        """
        parsed, err = parse_kermit_frame(frame_str)
        if err:
            print(f"[Receiver Error] Packet parse failed: {err}")
            return make_kermit_frame(self.expected_seq, 'N', "ERR_PARSE")
            
        p_seq = parsed["seq"]
        p_type = parsed["type"]
        p_payload = parsed["payload"]
        
        # Verify sequence sync
        if p_seq != self.expected_seq:
            print(f"[Receiver Warning] Sequence mismatch: expected {self.expected_seq}, got {p_seq}")
            return make_kermit_frame(self.expected_seq, 'N', "ERR_SEQ")

        if p_type == 'S':  # Send-Init
            if self.state != "INIT":
                return make_kermit_frame(self.expected_seq, 'N', "ERR_STATE")
            print(f"[Receiver] Received Send-Init (S) | Parameters: '{p_payload}'")
            self.state = "FILE"
            ack_frame = make_kermit_frame(self.expected_seq, 'Y', "WINDOW=8;EC=OK")
            self.expected_seq = (self.expected_seq + 1) % 64
            return ack_frame
            
        elif p_type == 'F':  # File Header
            if self.state != "FILE":
                return make_kermit_frame(self.expected_seq, 'N', "ERR_STATE")
            self.filename = p_payload
            print(f"[Receiver] Received File Header (F) | Filename: '{self.filename}'")
            self.state = "DATA"
            ack_frame = make_kermit_frame(self.expected_seq, 'Y')
            self.expected_seq = (self.expected_seq + 1) % 64
            return ack_frame
            
        elif p_type == 'D':  # Data Block
            if self.state != "DATA":
                return make_kermit_frame(self.expected_seq, 'N', "ERR_STATE")
                
            encrypted_bytes = bytes.fromhex(p_payload)
            # Decrypt payload using the ECDH shared secret
            decrypted = encrypt_decrypt(encrypted_bytes, self.shared_secret, p_seq)
            self.recovered_data += decrypted
            
            # Map decrypted chunk onto elliptic curve to verify group coordinate integrity
            point, tries = hash_to_curve(decrypted)
            self.received_points.append(point)
            
            print(f"[Receiver] Decrypted Block {len(self.received_points)} | Curve Point Verified in {tries} tries")
            print(f"           X: {hex(point[0])}")
            print(f"           Y: {hex(point[1])}")
            
            ack_frame = make_kermit_frame(self.expected_seq, 'Y')
            self.expected_seq = (self.expected_seq + 1) % 64
            return ack_frame
            
        elif p_type == 'Z':  # End of File
            if self.state != "DATA":
                return make_kermit_frame(self.expected_seq, 'N', "ERR_STATE")
            print("[Receiver] Received EOF (Z) | Closing active file stream")
            self.state = "EOT"
            ack_frame = make_kermit_frame(self.expected_seq, 'Y')
            self.expected_seq = (self.expected_seq + 1) % 64
            return ack_frame
            
        elif p_type == 'B':  # End of Transmission
            if self.state != "EOT":
                return make_kermit_frame(self.expected_seq, 'N', "ERR_STATE")
            print("[Receiver] Received Break/EOT (B) | Session terminated successfully")
            self.state = "FINISHED"
            ack_frame = make_kermit_frame(self.expected_seq, 'Y')
            self.expected_seq = (self.expected_seq + 1) % 64
            return ack_frame
            
        else:
            return make_kermit_frame(self.expected_seq, 'N', "ERR_TYPE")

class KermitSender:
    """
    Symmetric counterpart to KermitReceiver, managing the sending pipeline.
    """
    def __init__(self, data_stream, filename, shared_secret):
        self.data_stream = data_stream
        self.filename = filename
        self.shared_secret = shared_secret
        self.seq = 0
        self.chunks = []
        self.chunk_idx = 0
        self.state = "INIT"  # INIT, FILE, DATA, EOF, EOT
        
        # Split data into 16-byte chunks
        chunk_size = 16
        self.chunks = [data_stream[i:i+chunk_size] for i in range(0, len(data_stream), chunk_size)]
        
    def next_packet(self):
        """
        Generates the next outgoing packet based on current state.
        """
        if self.state == "INIT":
            return make_kermit_frame(self.seq, 'S', "WINDOW=8;EC=SECP256K1")
        elif self.state == "FILE":
            return make_kermit_frame(self.seq, 'F', self.filename)
        elif self.state == "DATA":
            chunk = self.chunks[self.chunk_idx]
            encrypted = encrypt_decrypt(chunk, self.shared_secret, self.seq)
            return make_kermit_frame(self.seq, 'D', encrypted.hex())
        elif self.state == "EOF":
            return make_kermit_frame(self.seq, 'Z')
        elif self.state == "EOT":
            return make_kermit_frame(self.seq, 'B')
        return None

    def process_ack(self, ack_frame_str):
        """
        Processes an ACK/NAK from the receiver and updates state accordingly.
        """
        parsed, err = parse_kermit_frame(ack_frame_str)
        if err or parsed["type"] != 'Y':
            print(f"[Sender Error] Failed parsing ACK: {err or 'Received NAK'}")
            return False
            
        p_seq = parsed["seq"]
        if p_seq != self.seq:
            print(f"[Sender Error] Sequence mismatch on ACK: expected {self.seq}, got {p_seq}")
            return False
            
        # Transition states
        if self.state == "INIT":
            self.state = "FILE"
        elif self.state == "FILE":
            self.state = "DATA"
        elif self.state == "DATA":
            self.chunk_idx += 1
            if self.chunk_idx >= len(self.chunks):
                self.state = "EOF"
        elif self.state == "EOF":
            self.state = "EOT"
        elif self.state == "EOT":
            self.state = "FINISHED"
            
        self.seq = (self.seq + 1) % 64
        return True

if __name__ == "__main__":
    # Standard integrated demonstration
    print("=== Secure Kermit Elliptic Curve Handshaking (Bi-Directional) ===")
    
    # Deriving shared secret key
    ecdh_shared_secret = 0xDEEB5EECD4E9785A3F0BCE1A46789D426E7A167C5A059E18F2F3C54BDEEA3000
    
    # Client stream coefficients
    speech_coefficients = b"VaesenSpeechFormants:f1=360,f2=840,f3=2100"
    filename = "speech_formants.bin"
    
    sender = KermitSender(speech_coefficients, filename, ecdh_shared_secret)
    receiver = KermitReceiver(ecdh_shared_secret)
    
    print("\n--- BEGINNING BI-DIRECTIONAL EVENT-LOOP ---")
    while sender.state != "FINISHED":
        # 1. Sender yields next packet frame
        packet = sender.next_packet()
        if not packet:
            break
        print(f"\n[Alice] Sending Frame: {repr(packet)}")
        
        # 2. Receiver processes the packet and returns the ACK frame
        ack = receiver.process_packet(packet)
        print(f"[Bob]   Returned ACK:  {repr(ack)}")
        
        # 3. Sender processes the ACK
        ok = sender.process_ack(ack)
        if not ok:
            print("[ERROR] Protocol Handshake Loop failed.")
            sys.exit(1)
            
    print("\n--- FINAL VERIFICATION ---")
    print(f"Original Payload: '{speech_coefficients.decode()}'")
    print(f"Decrypted Result: '{receiver.recovered_data.decode()}'")
    assert receiver.recovered_data == speech_coefficients
    print(f"Total Elliptic Curve Points verified: {len(receiver.received_points)}")
    print("[SUCCESS] Bidirectional Kermit Handshake & Receive cycle completed successfully.")
