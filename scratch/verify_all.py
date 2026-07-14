#!/usr/bin/env python3
import os
import sys
import json
import time
import shutil
import socket
import urllib.request
import urllib.error
import subprocess
import threading
from concurrent.futures import ThreadPoolExecutor

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
PORT = 8089
SERVER_URL = f"http://localhost:{PORT}"
PRICE_CACHE_FILE = os.path.join(PROJECT_ROOT, "assets/price_cache.json")
UNRESOLVED_FILE = os.path.join(PROJECT_ROOT, "unresolved_swaps.json")
RESOLVED_FILE = os.path.join(PROJECT_ROOT, "resolved_swaps.json")
POOLS_FILE = os.path.join(PROJECT_ROOT, "nonukes_pools.json")

def find_free_port():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('', 0))
    port = s.getsockname()[1]
    s.close()
    return port

class ServerContext:
    def __init__(self, port, env=None):
        self.port = port
        self.proc = None
        self.env = env if env else os.environ.copy()
        self.env["PORT"] = str(port)

    def start(self):
        server_script = os.path.join(PROJECT_ROOT, "scripts/dashboard_server.py")
        self.proc = subprocess.Popen(
            [sys.executable, server_script],
            cwd=PROJECT_ROOT,
            env=self.env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        # Wait until port is open
        start_time = time.time()
        while time.time() - start_time < 5.0:
            try:
                with socket.create_connection(("localhost", self.port), timeout=0.5):
                    return True
            except (socket.timeout, ConnectionRefusedError):
                time.sleep(0.1)
        return False

    def stop(self):
        if self.proc:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=2.0)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()
            self.proc = None

def get_api(url, path):
    full_url = f"{url}{path}"
    req = urllib.request.Request(full_url)
    try:
        with urllib.request.urlopen(req, timeout=2.0) as response:
            return response.status, response.read().decode('utf-8')
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode('utf-8')
    except Exception as e:
        return 500, str(e)

def run_tests():
    results = {
        "price_cache_tests": [],
        "concurrency_tests": None,
        "offline_online_tests": None
    }
    
    # 1. Backups
    backups = {}
    files_to_backup = ["assets/price_cache.json"]
    import glob
    for fpath in glob.glob(os.path.join(PROJECT_ROOT, "treasury_tokens_*.json")):
        files_to_backup.append(os.path.basename(fpath))

    print("Backing up files...")
    for fname in files_to_backup:
        fpath = os.path.join(PROJECT_ROOT, fname)
        if os.path.exists(fpath):
            with open(fpath, "r", encoding="utf-8") as f:
                backups[fname] = f.read()
        else:
            backups[fname] = None

    port = find_free_port()
    url = f"http://localhost:{port}"
    server = ServerContext(port)
    
    try:
        print(f"Starting server on port {port}...")
        if not server.start():
            print("Failed to start server.")
            sys.exit(1)

        # -------------------------------------------------------------
        # TEST 1: Price Cache Edge Cases
        # -------------------------------------------------------------
        print("\n--- Running Price Cache Tests ---")
        
        test_cases = [
            {"name": "malformed_json", "content": "{invalid json: price_cache}"},
            {"name": "entire_null", "content": "null"},
            {"name": "entire_array", "content": "[]"},
            {"name": "price_val_null", "content": json.dumps({"0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": {"price": None}})},
            {"name": "val_null", "content": json.dumps({"0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": None})},
            {"name": "val_malformed_string", "content": json.dumps({"0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": "not-a-float"})}
        ]

        valid_pool_addr = "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda"

        for tc in test_cases:
            print(f"Testing price_cache content type: {tc['name']}")
            # Write target price cache
            with open(PRICE_CACHE_FILE, "w", encoding="utf-8") as f:
                f.write(tc["content"])
            
            # Query /api/nonukes/pools
            pools_status, pools_res = get_api(url, "/api/nonukes/pools")
            
            # Query /api/nonukes/pool_details
            details_status, details_res = get_api(url, f"/api/nonukes/pool_details?address={valid_pool_addr}")
            
            tc_res = {
                "case": tc["name"],
                "pools_status": pools_status,
                "details_status": details_status,
                "pools_crashed": (pools_status >= 500 or "AttributeError" in pools_res),
                "details_crashed": (details_status >= 500 or "AttributeError" in details_res or "Internal Server Error" in details_res)
            }
            results["price_cache_tests"].append(tc_res)
            print(f"  /api/nonukes/pools: Status {pools_status}, Crashed: {tc_res['pools_crashed']}")
            print(f"  /api/nonukes/pool_details: Status {details_status}, Crashed: {tc_res['details_crashed']}")
            if tc_res["details_crashed"]:
                print(f"    Error detail: {details_res[:200]}")

        # Restore price_cache
        if backups.get("assets/price_cache.json") is not None:
            with open(PRICE_CACHE_FILE, "w", encoding="utf-8") as f:
                f.write(backups["assets/price_cache.json"])
        else:
            if os.path.exists(PRICE_CACHE_FILE):
                os.remove(PRICE_CACHE_FILE)

        # -------------------------------------------------------------
        # TEST 2: /api/ignore Concurrency safety
        # -------------------------------------------------------------
        print("\n--- Running Concurrency Tests ---")
        
        # We will trigger concurrent ignore/unignore calls for the target pool addresses
        test_addresses = [
            "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda",
            "0x78307167683f948bbbc7923a3e690d13eebf2425",
            "0x2f71d85850ae9d06d7dbd43eb7120e09ee9ec9c6",
            "0x4242424242424242424242424242424242424242" # Unknown
        ]
        
        num_requests = 100
        concurrency_results = []
        
        def send_ignore_req(idx):
            addr = test_addresses[idx % len(test_addresses)]
            ignored = (idx % 2 == 0)
            status, res = get_api(url, f"/api/ignore?address={addr}&ignored={'true' if ignored else 'false'}")
            return idx, addr, ignored, status, res

        print(f"Sending {num_requests} concurrent requests to /api/ignore...")
        with ThreadPoolExecutor(max_workers=20) as executor:
            futures = [executor.submit(send_ignore_req, i) for i in range(num_requests)]
            for fut in futures:
                idx, addr, ignored, status, res = fut.result()
                concurrency_results.append({
                    "idx": idx,
                    "address": addr,
                    "ignored": ignored,
                    "status": status,
                    "response": res
                })
        
        # Verify JSON integrity of all metadata files
        corruption_found = False
        corrupt_details = []
        for fpath in glob.glob(os.path.join(PROJECT_ROOT, "treasury_tokens_*.json")):
            try:
                with open(fpath, "r", encoding="utf-8") as f:
                    json.load(f)
            except Exception as e:
                corruption_found = True
                corrupt_details.append(f"{os.path.basename(fpath)}: {str(e)}")

        results["concurrency_tests"] = {
            "total_requests": num_requests,
            "success_count": sum(1 for r in concurrency_results if r["status"] == 200),
            "failure_count": sum(1 for r in concurrency_results if r["status"] != 200),
            "files_corrupted": corruption_found,
            "corruption_details": corrupt_details
        }
        print(f"Concurrency results:")
        print(f"  Success: {results['concurrency_tests']['success_count']}/{num_requests}")
        print(f"  Failures: {results['concurrency_tests']['failure_count']}")
        print(f"  Files Corrupted: {corruption_found}")
        if corruption_found:
            print(f"    Details: {corrupt_details}")

        # -------------------------------------------------------------
        # TEST 3: Offline / Online dynamic updates
        # -------------------------------------------------------------
        print("\n--- Running Offline/Online Tests ---")
        
        # Step A: Query pools (online)
        status1, res1 = get_api(url, "/api/nonukes/pools")
        online_success_1 = (status1 == 200 and json.loads(res1).get("success") == True)
        
        # Step B: Turn server offline
        print("Stopping server to simulate offline state...")
        server.stop()
        
        status2, res2 = get_api(url, "/api/nonukes/pools")
        offline_success = (status2 != 200) # Connection refused / error
        
        # Step C: Turn server back online
        print("Restarting server to simulate online state...")
        server.start()
        
        status3, res3 = get_api(url, "/api/nonukes/pools")
        online_success_2 = (status3 == 200 and json.loads(res3).get("success") == True)
        
        # Verify app.js setInterval presence and error handling
        app_js_path = os.path.join(PROJECT_ROOT, "frontend/nonukes_dashboard/app.js")
        has_set_interval = False
        has_catch_block = False
        if os.path.exists(app_js_path):
            with open(app_js_path, "r", encoding="utf-8") as f:
                content = f.read()
                if "setInterval(fetchPools, 5000)" in content:
                    has_set_interval = True
                if "catch (err)" in content and "pools = []" in content:
                    has_catch_block = True

        results["offline_online_tests"] = {
            "initial_online_success": online_success_1,
            "offline_success": offline_success,
            "recovery_online_success": online_success_2,
            "app_js_setInterval_found": has_set_interval,
            "app_js_catch_block_clears_pools": has_catch_block
        }
        print(f"Offline/Online results:")
        print(f"  Initial Online OK: {online_success_1}")
        print(f"  Offline OK (no response): {offline_success}")
        print(f"  Recovery Online OK: {online_success_2}")
        print(f"  app.js setInterval found: {has_set_interval}")
        print(f"  app.js catch block handles error: {has_catch_block}")

    finally:
        print("\nStopping server and restoring backups...")
        server.stop()
        # Restoring backups
        for fname, content in backups.items():
            fpath = os.path.join(PROJECT_ROOT, fname)
            if content is not None:
                with open(fpath, "w", encoding="utf-8") as f:
                    f.write(content)
            else:
                if os.path.exists(fpath):
                    os.remove(fpath)

    # Save verification JSON results
    with open(os.path.join(PROJECT_ROOT, "scratch/verification_results.json"), "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2)
    print("Verification completed. Results written to scratch/verification_results.json")

if __name__ == "__main__":
    run_tests()
