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
import threading

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
REAL_POOLS_PATH = os.path.join(PROJECT_ROOT, "nonukes_pools.json")
REAL_PRICE_CACHE_PATH = os.path.join(PROJECT_ROOT, "price_cache.json")
REAL_RESOLVED_SWAPS_PATH = os.path.join(PROJECT_ROOT, "resolved_swaps.json")
REAL_UNRESOLVED_SWAPS_PATH = os.path.join(PROJECT_ROOT, "unresolved_swaps.json")

class TestNoNukesDashboardAdversarial(unittest.TestCase):
    sandbox_dir = None
    server_proc = None
    port = None
    server_url = None
    stdout_file = None
    stderr_file = None

    @classmethod
    def setUpClass(cls):
        # 1. Create sandbox temp directory
        cls.sandbox_dir_obj = tempfile.TemporaryDirectory(prefix="nonukes_adversarial_sandbox_")
        cls.sandbox_dir = os.path.abspath(cls.sandbox_dir_obj.name)

        # Create frontend dir inside sandbox
        os.makedirs(os.path.join(cls.sandbox_dir, "frontend", "nonukes_dashboard"), exist_ok=True)

        # 2. Populate sandbox with copies of data files
        cls.restore_default_files()

        # Copy frontend files
        for fname in ["index.html", "style.css", "app.js"]:
            src = os.path.join(PROJECT_ROOT, "frontend/nonukes_dashboard", fname)
            dst = os.path.join(cls.sandbox_dir, "frontend/nonukes_dashboard", fname)
            if os.path.exists(src):
                shutil.copy(src, dst)

        # 3. Start server subprocess
        cls.port = cls.find_free_port()
        cls.server_url = f"http://localhost:{cls.port}"
        
        env = os.environ.copy()
        env["PORT"] = str(cls.port)
        env["RESERVES_FILE_PATH"] = os.path.join(cls.sandbox_dir, "nonukes_pulsex_reserves.json")
        
        server_script = os.path.join(PROJECT_ROOT, "scripts/dashboard_server.py")
        
        cls.stdout_file = open(os.path.join(cls.sandbox_dir, "server_stdout.log"), "w")
        cls.stderr_file = open(os.path.join(cls.sandbox_dir, "server_stderr.log"), "w")
        
        cls.server_proc = subprocess.Popen(
            [sys.executable, server_script],
            cwd=cls.sandbox_dir,
            env=env,
            stdout=cls.stdout_file,
            stderr=cls.stderr_file
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
        # Stop server subprocess
        if cls.server_proc:
            try:
                cls.server_proc.terminate()
                cls.server_proc.wait(timeout=2.0)
            except Exception:
                try:
                    cls.server_proc.kill()
                except Exception:
                    pass
        
        if cls.stdout_file:
            cls.stdout_file.close()
        if cls.stderr_file:
            cls.stderr_file.close()

        # Cleanup sandbox directory
        try:
            cls.sandbox_dir_obj.cleanup()
        except Exception:
            pass

    @classmethod
    def restore_default_files(cls):
        # Setup clean default mock files
        pools = {
            "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": {
                "target_group": "NoNukes",
                "version": "V1",
                "symbol": "$INCOGNITO",
                "name": "$INCOGNITO",
                "other_addr": "0x9b3b6b8ff7434e9ec2b6d3b032b98152ccf4d266"
            }
        }
        prices = {
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
        }
        resolved = [
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
        ]
        reserves = {
            "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": {
                "reserve0": 1000.0,
                "reserve1": 2000.0,
                "token0": "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62",
                "token1": "0x9b3b6b8ff7434e9ec2b6d3b032b98152ccf4d266"
            }
        }

        cls.write_sandbox_json("nonukes_pools.json", pools)
        cls.write_sandbox_json("price_cache.json", prices)
        cls.write_sandbox_json("resolved_swaps.json", resolved)
        cls.write_sandbox_json("unresolved_swaps.json", [])
        cls.write_sandbox_json("nonukes_pulsex_reserves.json", reserves)

        # Clear any treasury files
        for fpath in os.listdir(cls.sandbox_dir):
            if fpath.startswith("treasury_tokens_"):
                try:
                    os.remove(os.path.join(cls.sandbox_dir, fpath))
                except Exception:
                    pass

    @classmethod
    def find_free_port(cls):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('', 0))
        port = s.getsockname()[1]
        s.close()
        return port

    @classmethod
    def write_sandbox_json(cls, filename, data):
        with open(os.path.join(cls.sandbox_dir, filename), "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)

    def setUp(self):
        # Truncate/clear stderr log before each test
        self.clear_stderr_log()

    def tearDown(self):
        # Restore clean default files
        self.restore_default_files()

    def clear_stderr_log(self):
        # Close and reopen file to truncate
        if self.stderr_file:
            self.stderr_file.close()
        
        # Flush the buffer by opening write mode
        with open(os.path.join(self.sandbox_dir, "server_stderr.log"), "w") as f:
            f.truncate(0)
            
        # Re-open for the server process to continue writing
        self.__class__.stderr_file = open(os.path.join(self.sandbox_dir, "server_stderr.log"), "a")

    def assert_no_traceback(self):
        # Flush stream
        if self.stderr_file:
            self.stderr_file.flush()
        with open(os.path.join(self.sandbox_dir, "server_stderr.log"), "r") as f:
            content = f.read()
        self.assertNotIn("Traceback (most recent call last)", content, 
                         f"Server stderr contained traceback:\n{content}")

    def get_api_response(self, path):
        url = f"{self.server_url}{path}"
        req = urllib.request.Request(url)
        try:
            with urllib.request.urlopen(req, timeout=3.0) as response:
                return response.status, response.read().decode('utf-8')
        except urllib.error.HTTPError as e:
            return e.code, e.read().decode('utf-8')
        except Exception as e:
            return 500, str(e)

    # -------------------------------------------------------------
    # ADVERSARIAL TEST CASES
    # -------------------------------------------------------------

    def test_01_api_pools_malformed_address_odd_length(self):
        """Verify /api/pools handles address with odd length without unhandled crash/traceback."""
        status, response = self.get_api_response("/api/pools?address=0x1")
        # Standard HTTP response should be 400 or 500 gracefully, but not expose traceback
        self.assertIn(status, [400, 404, 500])
        self.assert_no_traceback()

    def test_02_api_pools_malformed_address_missing_prefix(self):
        """Verify /api/pools handles address missing '0x' prefix without unhandled crash/traceback."""
        status, response = self.get_api_response("/api/pools?address=174A0ad99c60c20D9B3D94c3095BC1fb9ddEFd62")
        self.assertIn(status, [400, 404, 500])
        self.assert_no_traceback()

    def test_03_api_pools_malformed_address_non_hex_chars(self):
        """Verify /api/pools handles address with non-hex characters without unhandled crash/traceback."""
        status, response = self.get_api_response("/api/pools?address=0x174A0ad99c60c20D9B3D94c3095BC1fb9ddEFd6g")
        self.assertIn(status, [400, 404, 500])
        self.assert_no_traceback()

    def test_04_api_pool_details_extremely_long_address(self):
        """Verify /api/nonukes/pool_details handles extremely large parameter sizes without crashing."""
        long_addr = "0x" + "a" * 100000
        status, response = self.get_api_response(f"/api/nonukes/pool_details?address={long_addr}")
        self.assertEqual(status, 400)
        self.assert_no_traceback()

    def test_05_api_pools_extremely_long_address(self):
        """Verify /api/pools handles extremely large parameter sizes without unhandled crash/traceback."""
        long_addr = "0x" + "a" * 100000
        status, response = self.get_api_response(f"/api/pools?address={long_addr}")
        self.assertIn(status, [400, 404, 500])
        self.assert_no_traceback()

    def test_06_corrupt_pools_json_type_confusion(self):
        """Verify /api/nonukes/pools handles nonukes_pools.json type confusion (list instead of dict) gracefully."""
        self.write_sandbox_json("nonukes_pools.json", [])
        status, response = self.get_api_response("/api/nonukes/pools")
        self.assertIn(status, [400, 500])
        self.assert_no_traceback()

    def test_07_corrupt_reserves_json_type_confusion(self):
        """Verify /api/nonukes/pools handles reserves JSON type confusion (list instead of dict) gracefully."""
        self.write_sandbox_json("nonukes_pulsex_reserves.json", [])
        status, response = self.get_api_response("/api/nonukes/pools")
        self.assertIn(status, [400, 500])
        self.assert_no_traceback()

    def test_08_corrupt_resolved_swaps_json_null(self):
        """Verify /api/nonukes/pools handles resolved_swaps.json containing null gracefully."""
        # Note: 'null' in JSON parses to None in Python
        with open(os.path.join(self.sandbox_dir, "resolved_swaps.json"), "w") as f:
            f.write("null")
        status, response = self.get_api_response("/api/nonukes/pools")
        self.assertIn(status, [400, 500])
        self.assert_no_traceback()

    def test_09_corrupt_resolved_swaps_json_type_confusion_dict(self):
        """Verify /api/nonukes/pools handles resolved_swaps.json containing dict instead of list gracefully."""
        self.write_sandbox_json("resolved_swaps.json", {"key": "value"})
        status, response = self.get_api_response("/api/nonukes/pools")
        self.assertIn(status, [400, 500])
        self.assert_no_traceback()

    def test_10_api_ignore_corrupt_unknown_minter_json(self):
        """Verify /api/ignore handles treasury_tokens_unknown_minter.json type confusion gracefully."""
        self.write_sandbox_json("treasury_tokens_unknown_minter.json", [])
        # Trigger default minter logic by ignoring an address not registered anywhere
        status, response = self.get_api_response("/api/ignore?address=0x1234567890123456789012345678901234567890&ignored=true")
        self.assertIn(status, [400, 500])
        self.assert_no_traceback()

    def test_11_concurrent_requests_stress(self):
        """Verify that concurrent requests do not cause deadlocks or exceptions."""
        threads = []
        errors = []

        def worker():
            for _ in range(20):
                try:
                    # Alternating requests
                    status, _ = self.get_api_response("/api/data")
                    if status != 200:
                        errors.append(f"Data status: {status}")
                    
                    status, _ = self.get_api_response("/api/nonukes/pools")
                    if status != 200:
                        errors.append(f"Pools status: {status}")
                except Exception as e:
                    errors.append(str(e))

        for _ in range(5):
            t = threading.Thread(target=worker)
            threads.append(t)
            t.start()

        for t in threads:
            t.join()

        self.assertEqual(errors, [], f"Concurrent execution errors: {errors}")
        self.assert_no_traceback()

if __name__ == "__main__":
    unittest.main()
