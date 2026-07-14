import urllib.request
import json

def test_kermit_api():
    url = "http://localhost:8080/api/kermit"
    
    # 1. Valid test
    # data = b"Hello" (hex "48656c6c6f")
    # seq = 3, type = 'D'
    # sum_val = 3 + ord('D') + len(data) + sum(data)
    # sum_val = 3 + 68 + 5 + (72 + 101 + 108 + 108 + 111) = 76 + 500 = 576
    # chk = ((576 & 0x3F) + 32) = 0 + 32 = 32 (ASCII space)
    payload_valid = {
        "seq": 3,
        "type": "D",
        "data": "48656c6c6f", # "Hello"
        "checksum": 32
    }
    
    req = urllib.request.Request(
        url,
        data=json.dumps(payload_valid).encode('utf-8'),
        headers={'Content-Type': 'application/json'}
    )
    
    with urllib.request.urlopen(req) as response:
        res_data = json.loads(response.read().decode('utf-8'))
        print("[TEST] Valid packet response:", res_data)
        assert res_data['type'] == 'Y' # ACK
        assert res_data['status'] == 'ACK'

    # 2. Invalid Checksum test
    payload_invalid = {
        "seq": 3,
        "type": "D",
        "data": "48656c6c6f",
        "checksum": 99 # Wrong checksum
    }
    
    req = urllib.request.Request(
        url,
        data=json.dumps(payload_invalid).encode('utf-8'),
        headers={'Content-Type': 'application/json'}
    )
    
    with urllib.request.urlopen(req) as response:
        res_data = json.loads(response.read().decode('utf-8'))
        print("[TEST] Invalid packet response:", res_data)
        assert res_data['type'] == 'N' # NAK
        assert 'NAK' in res_data['status']

    print("[SUCCESS] Multiplayer server Kermit endpoint verification passed.")

if __name__ == "__main__":
    test_kermit_api()
