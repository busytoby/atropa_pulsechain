import serial
import time
import sys

def run_test():
    port = "/dev/ttyACM0"
    baud = 115200
    print(f"Connecting to {port} at {baud}...")
    
    try:
        ser = serial.Serial(port, baud, timeout=2)
    except Exception as e:
        print(f"Error opening port: {e}")
        sys.exit(1)
        
    # Toggle DTR/RTS to reset the board and capture boot output
    print("Resetting board...")
    ser.dtr = False
    ser.rts = True
    time.sleep(0.1)
    ser.rts = False
    ser.dtr = False
    time.sleep(1.0)
    
    # Read initial boot logs and MathInit output
    print("--- Boot Output ---")
    end_time = time.time() + 3.0
    while time.time() < end_time:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore')
            print(line, end='')
        else:
            time.sleep(0.1)
            
    commands = [
        (":b", "Testing time command"),
        (":a", "Testing random number command"),
        (":c", "Testing print config command"),
        (":h HelloWorld", "Testing set handle command"),
        (":x", "Testing save config command"),
        (":c", "Testing print config command again (to verify handle was saved)"),
        (":n", "Testing key generation command"),
        (":t", "Testing tx count command"),
        (":r", "Testing rx count command"),
        ("Ping LoRa Test", "Testing LoRa packet transmission")
    ]
    
    for cmd, desc in commands:
        print(f"\n--- {desc} ({cmd}) ---")
        ser.write((cmd + "\r").encode('utf-8'))
        ser.flush()
        
        # Read response lines for 1.5 seconds
        time.sleep(0.5)
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
