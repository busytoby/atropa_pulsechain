import http.server
import socketserver
import os
import sys
import json
import subprocess
import struct

PORT = 3000
DIRECTORY = "frontend"

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def do_POST(self):
        if self.path == "/api/vote":
            # Read POST body
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            vote_info = json.loads(post_data.decode('utf-8'))
            
            # Save vote transaction details to our ledger file
            ledger_path = "assets/bear_evolution_ledger.json"
            os.makedirs(os.path.dirname(ledger_path), exist_ok=True)
            
            history = []
            if os.path.exists(ledger_path):
                try:
                    with open(ledger_path, 'r') as f:
                        history = json.load(f)
                except:
                    pass
                    
            history.insert(0, vote_info)
            with open(ledger_path, 'w') as f:
                json.dump(history, f, indent=2)

            # Send response
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            response = {"status": "success", "message": "Vote committed to ACAB ledger."}
            self.wfile.write(json.dumps(response).encode('utf-8'))
            
        elif self.path == "/api/evolve":
            print("[SERVER] Evolution triggered. Running predictive genome architect...")
            
            # 1. Execute the predictive bear architect compiler script
            try:
                subprocess.run([sys.executable, "scripts/predictive_bear_architect.py"], check=True)
            except Exception as e:
                print(f"[SERVER ERROR] Failed to run genome compiler: {e}")
                
            # 2. Read the resulting 12-byte compiled DNA structure
            dna_path = "assets/bear_genome.dna"
            fur_r, fur_g, fur_b = 120, 120, 120
            fur_len, scale = 150, 120
            twitch, sickness = 10, 0
            
            if os.path.exists(dna_path):
                try:
                    with open(dna_path, 'rb') as f:
                        dna_bytes = f.read(12)
                        if len(dna_bytes) == 12:
                            unpacked = struct.unpack('<BBBBBBBBBBBB', dna_bytes)
                            fur_r, fur_g, fur_b = unpacked[0], unpacked[1], unpacked[2]
                            sickness, scale, fur_len = unpacked[6], unpacked[7], unpacked[8]
                            twitch = unpacked[11]
                except Exception as e:
                    print(f"[SERVER ERROR] Failed to read DNA file: {e}")

            # Send back the compiled parameters
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            
            response = {
                "fur_r": fur_r,
                "fur_g": fur_g,
                "fur_b": fur_b,
                "fur_len": fur_len,
                "scale": scale,
                "twitch_intensity": twitch,
                "sickness_intensity": sickness,
                "phenotype": "crimson" if fur_r > 150 else ("gray" if fur_r <= 110 else "brown"),
                "id": "0x" + os.urandom(4).hex() + "ace8..."
            }
            self.wfile.write(json.dumps(response).encode('utf-8'))
        else:
            self.send_error(404, "Endpoint not found")

def start_server():
    # Ensure frontend directory exists
    if not os.path.exists(DIRECTORY):
        print(f"Error: Directory '{DIRECTORY}' not found.")
        sys.exit(1)
        
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"=== Bionika BrainZ Dashboard Server ===")
        print(f"   - Serving directory: '{DIRECTORY}'")
        print(f"   - Local Address: http://localhost:{PORT}/bear_or_not/")
        print("   - Press Ctrl+C to terminate.")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down dashboard server...")

if __name__ == '__main__':
    start_server()
