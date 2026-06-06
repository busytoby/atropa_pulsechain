import serial
import time
import sys
import binascii
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature

# Master Private Key used for testing/authorization
PRIVATE_KEY_HEX = "89afa5a4b8137675ae864d333fa45aecec429579bacfcf05b1a336b70ff3482c"

def sign_challenge(challenge_hex):
    # Convert hex challenge to bytes
    challenge_bytes = binascii.unhexlify(challenge_hex)
    
    # Print the local SHA-256 hash
    import hashlib
    h = hashlib.sha256(challenge_bytes).digest()
    print(f"Python Hash: {binascii.hexlify(h).decode().upper()}")
    
    # Load the private key
    priv_val = int(PRIVATE_KEY_HEX, 16)
    private_key = ec.derive_private_key(priv_val, ec.SECP256K1())
    
    # Sign challenge (using SHA256)
    der_signature = private_key.sign(challenge_bytes, ec.ECDSA(hashes.SHA256()))
    
    # Decode DER signature to (r, s)
    r, s = decode_dss_signature(der_signature)
    
    # Format as 32-byte hex strings
    r_hex = f"{r:064x}"
    s_hex = f"{s:064x}"
    
    return r_hex, s_hex

def run_auth_test():
    port = "/dev/ttyACM0"
    baud = 115200
    
    print(f"Connecting to {port} at {baud}...")
    try:
        ser = serial.Serial(port, baud, timeout=1, dsrdtr=False, rtscts=False)
    except Exception as e:
        print(f"Error opening port: {e}")
        sys.exit(1)
        
    print("Resetting board to capture new challenge...")
    ser.dtr = False
    ser.rts = True
    time.sleep(0.1)
    ser.rts = False
    ser.dtr = False
    time.sleep(1.0)
    
    challenge_hex = None
    start_time = time.time()
    
    print("Listening for hardware challenge...")
    while time.time() - start_time < 5.0:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            print(f"Device: {line}")
            if "Challenge:" in line:
                challenge_hex = line.split("Challenge:")[1].strip()
                print(f"\nCaptured Challenge: {challenge_hex}")
                break
        else:
            time.sleep(0.05)
            
    if not challenge_hex:
        print("Error: Could not capture challenge from device.")
        ser.close()
        sys.exit(1)
        
    # Generate signature
    print("Signing challenge with Master Private Key...")
    r_hex, s_hex = sign_challenge(challenge_hex)
    print(f"Signature R: {r_hex}")
    print(f"Signature S: {s_hex}")
    
    # Send auth command
    auth_cmd = f":auth {r_hex} {s_hex}\r"
    print(f"Sending authorization command...")
    ser.write(auth_cmd.encode('utf-8'))
    ser.flush()
    
    # Read response
    time.sleep(0.5)
    read_end = time.time() + 2.0
    authenticated = False
    while time.time() < read_end:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            print(f"Device: {line}")
            if "AUTH SUCCESS" in line:
                authenticated = True
        else:
            time.sleep(0.05)
            
    if authenticated:
        print("\nAuthentication successful! Trying to execute a locked command (:b)...")
        ser.write(b":b\r")
        ser.flush()
        time.sleep(0.5)
        while ser.in_waiting:
            print(f"Device: {ser.readline().decode('utf-8', errors='ignore').strip()}")
            
        print("\nLocking the device back (:lock)...")
        ser.write(b":lock\r")
        ser.flush()
        time.sleep(0.5)
        while ser.in_waiting:
            print(f"Device: {ser.readline().decode('utf-8', errors='ignore').strip()}")
    else:
        print("\nAuthentication failed.")
        
    ser.close()

if __name__ == "__main__":
    run_auth_test()
