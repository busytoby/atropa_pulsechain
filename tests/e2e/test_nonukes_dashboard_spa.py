#!/usr/bin/env python3
import os
import sys
import time
import json
import shutil
import tempfile
import socket
import subprocess
import unittest
import urllib.request
import urllib.parse

class TestNoNukesDashboardSPA(unittest.TestCase):
    sandbox_dir = None
    server_proc = None
    port = None
    server_url = None

    @classmethod
    def setUpClass(cls):
        # Create sandbox temp directory
        cls.sandbox_dir_obj = tempfile.TemporaryDirectory(prefix="nonukes_spa_sandbox_")
        cls.sandbox_dir = os.path.abspath(cls.sandbox_dir_obj.name)

        # Create frontend/nonukes_dashboard dir inside sandbox
        os.makedirs(os.path.join(cls.sandbox_dir, "frontend", "nonukes_dashboard"), exist_ok=True)

        # Copy data files
        os.makedirs(os.path.join(cls.sandbox_dir, "assets"), exist_ok=True)
        with open(os.path.join(cls.sandbox_dir, "assets", "nonukes_pools.json"), "w") as f:
            json.dump({
                "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": {
                    "target_group": "NoNukes",
                    "version": "V1",
                    "symbol": "$INCOGNITO",
                    "name": "$INCOGNITO",
                    "other_addr": "0x9b3b6b8ff7434e9ec2b6d3b032b98152ccf4d266"
                }
            }, f)

        with open(os.path.join(cls.sandbox_dir, "price_cache.json"), "w") as f:
            json.dump({
                "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62": {
                    "price": 0.5,
                    "symbol": "NoNukes",
                    "name": "NoNukes"
                },
                "0x9b3b6b8ff7434e9ec2b6d3b032b98152ccf4d266": {
                    "price": 1.5,
                    "symbol": "$INCOGNITO",
                    "name": "$INCOGNITO"
                }
            }, f)

        with open(os.path.join(cls.sandbox_dir, "resolved_swaps.json"), "w") as f:
            json.dump([
                {
                    "tx_hash": "0xabcdef123",
                    "pool_address": "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda",
                    "version": "PulseX V1",
                    "token0": {
                        "address": "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62",
                        "symbol": "NoNukes"
                    },
                    "token1": {
                        "address": "0x9b3b6b8ff7434e9ec2b6d3b032b98152ccf4d266",
                        "symbol": "$INCOGNITO"
                    },
                    "amount0": 100.0,
                    "amount1": 33.33,
                    "usd_value": 0.0,
                    "timestamp": 1781441813
                }
            ], f)

        src_frontend = "/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/nonukes_dashboard"
        dest_frontend = os.path.join(cls.sandbox_dir, "frontend", "nonukes_dashboard")
        shutil.copy(os.path.join(src_frontend, "index.html"), os.path.join(dest_frontend, "index.html"))
        shutil.copy(os.path.join(src_frontend, "style.css"), os.path.join(dest_frontend, "style.css"))
        shutil.copy(os.path.join(src_frontend, "app.js"), os.path.join(dest_frontend, "app.js"))

        # Start server subprocess
        cls.port = cls.find_free_port()
        cls.server_url = f"http://localhost:{cls.port}"
        
        env = os.environ.copy()
        env["PORT"] = str(cls.port)
        
        server_script = "/home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/dashboard_server.py"
        cls.server_proc = subprocess.Popen(
            [sys.executable, server_script],
            cwd=cls.sandbox_dir,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        # Poll server until active
        start_time = time.time()
        while time.time() - start_time < 5.0:
            try:
                with socket.create_connection(("localhost", cls.port), timeout=0.5):
                    break
            except (socket.timeout, ConnectionRefusedError):
                time.sleep(0.1)
        else:
            raise RuntimeError(f"Sandbox backend server failed to start on port {cls.port}")

    @classmethod
    def tearDownClass(cls):
        if cls.server_proc:
            try:
                cls.server_proc.terminate()
                cls.server_proc.wait(timeout=2.0)
            except Exception:
                try:
                    cls.server_proc.kill()
                except Exception:
                    pass
        try:
            cls.sandbox_dir_obj.cleanup()
        except Exception:
            pass

    @classmethod
    def find_free_port(cls):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('', 0))
        port = s.getsockname()[1]
        s.close()
        return port

    def get_api_response(self, path):
        url = f"{self.server_url}{path}"
        req = urllib.request.Request(url)
        try:
            with urllib.request.urlopen(req, timeout=3.0) as response:
                return response.status, response.read().decode('utf-8'), response.info()
        except urllib.error.HTTPError as e:
            return e.code, e.read().decode('utf-8'), e.info()
        except Exception as e:
            return 500, str(e), {}

    def test_static_routes(self):
        for path in ['/nonukes', '/nonukes/', '/nonukes/index.html']:
            status, content, headers = self.get_api_response(path)
            self.assertEqual(status, 200, f"Path {path} failed")
            self.assertIn("NoNukes Family", content)
            self.assertIn("text/html", headers.get_content_type())

        status, content, headers = self.get_api_response('/nonukes/style.css')
        self.assertEqual(status, 200)
        self.assertIn("glass-bg", content)
        self.assertIn("text/css", headers.get_content_type())

        status, content, headers = self.get_api_response('/nonukes/app.js')
        self.assertEqual(status, 200)
        self.assertIn("fetchPools", content)
        self.assertTrue("javascript" in headers.get_content_type() or "js" in headers.get_content_type())

    def test_api_pools_fallback_volume(self):
        status, content, headers = self.get_api_response('/api/nonukes/pools')
        self.assertEqual(status, 200)
        data = json.loads(content)
        self.assertTrue(data.get("success"))
        pools = data.get("pools", [])
        self.assertEqual(len(pools), 1)
        pool = pools[0]
        
        self.assertEqual(pool.get("volume_usd"), 50.0)
        self.assertEqual(pool.get("swap_count"), 1)

    def test_api_pool_details(self):
        status, content, headers = self.get_api_response('/api/nonukes/pool_details?address=0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda')
        self.assertEqual(status, 200)
        data = json.loads(content)
        self.assertTrue(data.get("success"))
        self.assertEqual(data.get("address"), "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda")
        
        trends = data.get("price_trends", [])
        self.assertEqual(len(trends), 1)
        self.assertAlmostEqual(trends[0]["price"], 50.0 / 33.33, places=4)

        history = data.get("swap_history", [])
        self.assertEqual(len(history), 1)
        self.assertEqual(history[0]["usd_value"], 50.0)

if __name__ == "__main__":
    unittest.main()
