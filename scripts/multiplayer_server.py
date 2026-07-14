import http.server
import json
import urllib.parse
import os
import random
import time

# Global memory state mapping player IDs to their active Gauntlet configurations
PLAYER_REGISTRY = {}

# Coordinated Level, Spawner statistics, and Stochastic Neural Accumulator logic state
GLOBAL_STATE = {
    "level": 1,
    "mapSeed": 42.0,
    "destroyedSpawnersTotal": 0,
    "spawnersActive": 4,
    "accumulatorLatency": 50.0,  # in ms
    "accumulatorJitter": 5.0      # variance
}

class UnifiedGauntletHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Allow cross-origin requests for testing
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200, "ok")
        self.end_headers()

    def do_POST(self):
        parsed_url = urllib.parse.urlparse(self.path)
        if parsed_url.path == '/api/sync':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            
            try:
                client_state = json.loads(post_data.decode('utf-8'))
                player_id = client_state.get('id')
                now = client_state.get('time', int(time.time() * 1000))
                
                # 1. Stochastic Neural Accumulator Logic (Jitter tracking)
                if player_id and player_id in PLAYER_REGISTRY:
                    prev_state = PLAYER_REGISTRY[player_id]
                    time_diff = now - prev_state.get('lastSeen', now)
                    if 0 < time_diff < 1000:
                        # Model rolling average latency delta (ideal tick is 50ms)
                        error_offset = abs(time_diff - 50.0)
                        GLOBAL_STATE['accumulatorLatency'] = (0.9 * GLOBAL_STATE['accumulatorLatency']) + (0.1 * error_offset)
                        deviation = abs(error_offset - GLOBAL_STATE['accumulatorLatency'])
                        GLOBAL_STATE['accumulatorJitter'] = (0.9 * GLOBAL_STATE['accumulatorJitter']) + (0.1 * (deviation ** 2))

                # 2. Coordinated Advancement Level logic
                client_level = client_state.get('level', 1)
                if client_level > GLOBAL_STATE['level']:
                    GLOBAL_STATE['level'] = client_level
                    GLOBAL_STATE['mapSeed'] = random.uniform(1.0, 1000.0)
                    GLOBAL_STATE['spawnersActive'] = 4 + (client_level * 2)
                    print(f"[Auncient Server] Coordinated Level Advance triggered by {player_id}. New Level: {client_level}")

                # 3. Spawner Statistics updates
                destroyed_spawner_increment = client_state.get('spawnersDestroyedThisTick', 0)
                if destroyed_spawner_increment > 0:
                    GLOBAL_STATE['destroyedSpawnersTotal'] += destroyed_spawner_increment
                    GLOBAL_STATE['spawnersActive'] = max(0, GLOBAL_STATE['spawnersActive'] - destroyed_spawner_increment)

                if player_id:
                    # Update player registry state
                    PLAYER_REGISTRY[player_id] = {
                        'x': client_state.get('x', 100),
                        'y': client_state.get('y', 100),
                        'charType': client_state.get('charType', 'Warrior'),
                        'health': client_state.get('health', 2000),
                        'score': client_state.get('score', 0),
                        'lastSeen': now
                    }
                
                # Prune inactive peers (unseen for more than 5 seconds)
                to_delete = [pid for pid, pstate in PLAYER_REGISTRY.items() if now - pstate['lastSeen'] > 5000]
                for pid in to_delete:
                    del PLAYER_REGISTRY[pid]
                
                # Consolidate response including global coordinated properties
                sync_payload = {
                    'players': PLAYER_REGISTRY,
                    'global': GLOBAL_STATE
                }
                
                response_data = json.dumps(sync_payload)
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(response_data.encode('utf-8'))
                return
            except Exception as e:
                self.send_response(400)
                self.end_headers()
                self.wfile.write(f"Error: {str(e)}".encode('utf-8'))
                return

        super().do_POST()

    def translate_path(self, path):
        # Ensure we serve static files relative to the project root
        root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
        path = super().translate_path(path)
        rel = os.path.relpath(path, os.getcwd())
        return os.path.join(root_dir, rel)

if __name__ == '__main__':
    port = 8080
    server_address = ('', port)
    httpd = http.server.HTTPServer(server_address, UnifiedGauntletHandler)
    print(f"[Auncient Server] Unified Gauntlet Teleconference Node running on port {port}...")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nServer shutting down.")
