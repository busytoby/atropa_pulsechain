#!/usr/bin/env python3
import os
import sys
import time
import json
import socket
import subprocess
import threading
import urllib.request
import urllib.error
import shutil
import tempfile
import random

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SERVER_SCRIPT = os.path.join(PROJECT_ROOT, "scripts/dashboard_server.py")

class ServerInstance:
    def __init__(self, cwd_dir, env_overrides=None):
        self.cwd_dir = cwd_dir
        self.port = self.find_free_port()
        self.url = f"http://localhost:{self.port}"
        self.proc = None
        self.env_overrides = env_overrides or {}

    @staticmethod
    def find_free_port():
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('', 0))
        port = s.getsockname()[1]
        s.close()
        return port

    def start(self):
        env = os.environ.copy()
        env["PORT"] = str(self.port)
        for k, v in self.env_overrides.items():
            env[k] = v
        self.proc = subprocess.Popen(
            [sys.executable, SERVER_SCRIPT],
            cwd=self.cwd_dir,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        # Wait for port to open
        start_time = time.time()
        while time.time() - start_time < 5.0:
            try:
                with socket.create_connection(("localhost", self.port), timeout=0.5):
                    return True
            except (socket.timeout, ConnectionRefusedError):
                time.sleep(0.1)
        raise RuntimeError("Server failed to start")

    def stop(self):
        if self.proc:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=2.0)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()

def query_endpoint(url, path):
    full_url = f"{url}{path}"
    try:
        req = urllib.request.Request(full_url)
        with urllib.request.urlopen(req, timeout=3.0) as response:
            return response.status, response.read().decode('utf-8')
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode('utf-8')
    except Exception as e:
        return 500, str(e)

def test_price_cache_malformed():
    print("\n--- Test 1.1: Malformed Price Cache ---")
    with tempfile.TemporaryDirectory() as tmpdir:
        # Create nonukes_pools.json to avoid crash or empty pool check
        shutil.copy(os.path.join(PROJECT_ROOT, "nonukes_pools.json"), os.path.join(tmpdir, "nonukes_pools.json"))
        # Create malformed price_cache.json
        with open(os.path.join(tmpdir, "price_cache.json"), "w") as f:
            f.write("{malformed json here: invalid syntax}")

        server = ServerInstance(tmpdir)
        server.start()
        try:
            status, res = query_endpoint(server.url, "/api/nonukes/pools")
            print(f"API /api/nonukes/pools response status: {status}")
            assert status == 200, f"Expected 200, got {status}"
            data = json.loads(res)
            assert data["success"] == True

            # Query details of one of the pools
            pool_addr = list(json.load(open(os.path.join(tmpdir, "nonukes_pools.json"))).keys())[0]
            status, res = query_endpoint(server.url, f"/api/nonukes/pool_details?address={pool_addr}")
            print(f"API /api/nonukes/pool_details response status: {status}")
            assert status == 200
            data = json.loads(res)
            assert data["success"] == True
            print("Successfully handled malformed price cache without crashing.")
        finally:
            server.stop()

def test_price_cache_nulls():
    print("\n--- Test 1.2: Null / Malformed Values inside Price Cache ---")
    with tempfile.TemporaryDirectory() as tmpdir:
        shutil.copy(os.path.join(PROJECT_ROOT, "nonukes_pools.json"), os.path.join(tmpdir, "nonukes_pools.json"))
        
        # Create price_cache.json with null and malformed float values
        price_cache = {
            "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62": None,
            "0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6": {
                "price": None,
                "symbol": "Atropa"
            },
            "0xd6c31ba0754c4383a41c0e9df042c62b5e918f6d": {
                "price": "not-a-number",
                "symbol": "TeddyBear"
            }
        }
        with open(os.path.join(tmpdir, "price_cache.json"), "w") as f:
            json.dump(price_cache, f)

        server = ServerInstance(tmpdir)
        server.start()
        try:
            status, res = query_endpoint(server.url, "/api/nonukes/pools")
            print(f"API /api/nonukes/pools response status: {status}")
            assert status == 200
            data = json.loads(res)
            assert data["success"] == True

            pool_addr = list(json.load(open(os.path.join(tmpdir, "nonukes_pools.json"))).keys())[0]
            status, res = query_endpoint(server.url, f"/api/nonukes/pool_details?address={pool_addr}")
            print(f"API /api/nonukes/pool_details response status: {status}")
            assert status == 200
            data = json.loads(res)
            assert data["success"] == True
            print("Successfully handled None/null and malformed values inside price cache without crashing.")
        finally:
            server.stop()

def test_concurrency_ignore():
    print("\n--- Test 2: Concurrency updates to /api/ignore ---")
    with tempfile.TemporaryDirectory() as tmpdir:
        # Copy some token metadata files
        for fpath in glob.glob(os.path.join(PROJECT_ROOT, "treasury_tokens_*.json")):
            shutil.copy(fpath, tmpdir)

        server = ServerInstance(tmpdir)
        server.start()
        
        # We will make 100 concurrent requests to /api/ignore
        errors = []
        threads = []
        
        # Pick some addresses
        addresses = [
            "0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6",
            "0xd6c31ba0754c4383a41c0e9df042c62b5e918f6d",
            "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62",
            "0x0000000000000000000000000000000000000001",
            "0x0000000000000000000000000000000000000002"
        ]
        
        def worker(idx):
            addr = random.choice(addresses)
            ignored = random.choice(["true", "false"])
            status, res = query_endpoint(server.url, f"/api/ignore?address={addr}&ignored={ignored}")
            if status != 200:
                errors.append(f"Thread {idx} failed: status {status}, response: {res}")
            else:
                try:
                    data = json.loads(res)
                    if not data.get("success"):
                        errors.append(f"Thread {idx} got success: false")
                except Exception as e:
                    errors.append(f"Thread {idx} failed to parse JSON: {e}")

        for i in range(100):
            t = threading.Thread(target=worker, args=(i,))
            threads.append(t)
            t.start()

        for t in threads:
            t.join()

        print(f"Concurrency requests finished. Total errors encountered: {len(errors)}")
        if errors:
            for err in errors[:5]:
                print(f"Error sample: {err}")
        assert len(errors) == 0, "Errors during concurrent ignore requests!"

        # Now verify all JSON files in tmpdir are still valid JSON and not corrupted
        import glob
        for fpath in glob.glob(os.path.join(tmpdir, "treasury_tokens_*.json")):
            try:
                with open(fpath, "r") as f:
                    data = json.load(f)
                print(f"Successfully loaded {os.path.basename(fpath)} (keys: {len(data)})")
            except Exception as e:
                raise AssertionError(f"File {os.path.basename(fpath)} is corrupted: {e}")

        print("Concurrency check passed successfully! Locks are operating correctly and files are not corrupted.")
        server.stop()

def test_offline_online_recovery():
    print("\n--- Test 3: Offline/Online dynamic recovery check ---")
    # Verify that in app.js, when fetchPools catches an error, it resets pools to []
    # and renders appropriately, and recovers when the endpoint is active again.
    app_js_path = os.path.join(PROJECT_ROOT, "frontend/nonukes_dashboard/app.js")
    with open(app_js_path, "r") as f:
        app_js_content = f.read()

    # Verify that fetchPools has try-catch error handler
    assert "async function fetchPools()" in app_js_content
    assert "catch (err)" in app_js_content
    assert "pools = []" in app_js_content
    assert "renderPools()" in app_js_content
    assert "setInterval(fetchPools, 5000)" in app_js_content or "setInterval(fetchPools, 2000)" in app_js_content
    print("Static verification of app.js fetchPools recovery logic passed successfully.")

if __name__ == "__main__":
    import glob
    try:
        test_price_cache_malformed()
        test_price_cache_nulls()
        test_concurrency_ignore()
        test_offline_online_recovery()
        print("\nAll custom empirical checks passed successfully!")
    except AssertionError as e:
        print(f"\nVerification FAILED: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"\nVerification encountered unexpected error: {e}")
        sys.exit(1)
