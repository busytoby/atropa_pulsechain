import serial
import time
import sys

def run_test():
    port = "/dev/ttyACM0"
    baud = 115200
    print(f"Connecting to {port} at {baud} (no hardware reset)...")
    
    try:
        # Open port without flow control or toggling DTR/RTS
        ser = serial.Serial(port, baud, timeout=1, dsrdtr=False, rtscts=False)
    except Exception as e:
        print(f"Error opening port: {e}")
        sys.exit(1)
        
    commands = [
        (":b", "Testing time command"),
        (":a", "Testing random number command"),
        (":c", "Testing print config command"),
        (":h HelloNative", "Testing set handle command"),
        (":x", "Testing save config command"),
        (":c", "Testing print config command again"),
        (":n", "Testing key generation command"),
        (":t", "Testing tx count command"),
        (":r", "Testing rx count command"),
        ("Ping LoRa Test", "Testing LoRa packet transmission")
    ]
    
    for cmd, desc in commands:
        print(f"\n--- {desc} ({cmd}) ---")
        ser.write((cmd + "\r").encode('utf-8'))
        ser.flush()
        
        # Read response lines for 1 second
        time.sleep(0.3)
        read_end = time.time() + 1.0
        while time.time() < read_end:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore')
                print(line, end='')
            else:
                time.sleep(0.05)
                
    ser.close()
    print("\nTesting complete.")

if __name__ == "__main__":
    run_test()
