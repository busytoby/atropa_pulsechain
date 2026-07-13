import http.server
import socketserver
import os
import sys

PORT = 3000
DIRECTORY = "frontend"

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

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
