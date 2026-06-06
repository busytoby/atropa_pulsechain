import serial
import time
import sys

def test_rng():
    port = "/dev/ttyACM0"
    baud = 115200
    samples_count = 20
    
    print(f"Connecting to {port} at {baud}...")
    try:
        ser = serial.Serial(port, baud, timeout=1, dsrdtr=False, rtscts=False)
    except Exception as e:
        print(f"Error opening port: {e}")
        sys.exit(1)
        
    print(f"Querying {samples_count} hardware random numbers from the device...")
    
    random_numbers = []
    
    # Flush input buffer
    ser.reset_input_buffer()
    
    for i in range(samples_count):
        # Send random command (:a)
        ser.write(b":a\r")
        ser.flush()
        
        # Read response
        time.sleep(0.1)
        response_received = False
        start_time = time.time()
        
        while time.time() - start_time < 0.5:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if "random =" in line:
                    try:
                        # Extract the number
                        parts = line.split("=")
                        val = int(parts[1].strip())
                        random_numbers.append(val)
                        print(f"Sample {i+1:02d}: {val} (Hex: {hex(val)})")
                        response_received = True
                        break
                    except ValueError:
                        pass
            time.sleep(0.02)
            
        if not response_received:
            print(f"Sample {i+1:02d}: Timeout or invalid response.")
            
    ser.close()
    
    if random_numbers:
        print("\nVerification statistics:")
        print(f"  Successfully retrieved: {len(random_numbers)} / {samples_count}")
        print(f"  Min Value: {min(random_numbers)}")
        print(f"  Max Value: {max(random_numbers)}")
        # Check uniqueness to verify it's not repeating the same sequence
        unique_count = len(set(random_numbers))
        print(f"  Unique Values: {unique_count} / {len(random_numbers)}")
        if unique_count == len(random_numbers):
            print("  RNG Quality Check: PASSED (All retrieved numbers are unique)")
        else:
            print("  RNG Quality Check: FAILED (Duplicates detected)")
    else:
        print("\nNo random numbers could be retrieved.")

if __name__ == "__main__":
    test_rng()
