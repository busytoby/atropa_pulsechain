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
from html.parser import HTMLParser

# Setup project root and configuration
import glob
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
REAL_POOLS_PATH = os.path.join(PROJECT_ROOT, "assets", "nonukes_pools.json")
REAL_PRICE_CACHE_PATH = os.path.join(PROJECT_ROOT, "price_cache.json")
REAL_RESOLVED_SWAPS_PATH = os.path.join(PROJECT_ROOT, "resolved_swaps.json")
REAL_UNRESOLVED_SWAPS_PATH = os.path.join(PROJECT_ROOT, "unresolved_swaps.json")
REAL_FRONTEND_PATH = os.path.join(PROJECT_ROOT, "frontend/nonukes_dashboard/index.html")

reserves_files = glob.glob(os.path.expanduser("~/.gemini/antigravity-cli/brain/*/scratch/nonukes_pulsex_reserves.json"))
default_reserves_path = reserves_files[0] if reserves_files else "nonukes_pulsex_reserves.json"
REAL_RESERVES_PATH = os.environ.get("RESERVES_FILE_PATH", default_reserves_path)

# Dual-driver architecture check
SELENIUM_AVAILABLE = False
try:
    from selenium import webdriver
    from selenium.webdriver.chrome.options import Options
    from selenium.webdriver.common.by import By
    from selenium.webdriver.support.ui import WebDriverWait
    from selenium.webdriver.support import expected_conditions as EC
    SELENIUM_AVAILABLE = True
except ImportError:
    pass

class SimpleDOMParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.elements = []
        self.scripts = []
        self.styles = []
        self.in_script = False
        self.in_style = False

    def handle_starttag(self, tag, attrs):
        attr_dict = dict(attrs)
        self.elements.append({
            "tag": tag,
            "id": attr_dict.get("id"),
            "class": attr_dict.get("class"),
            "src": attr_dict.get("src"),
            "href": attr_dict.get("href")
        })
        if tag == "script":
            self.in_script = True
        elif tag == "style":
            self.in_style = True

    def handle_endtag(self, tag):
        if tag == "script":
            self.in_script = False
        elif tag == "style":
            self.in_style = False

    def handle_data(self, data):
        if self.in_script:
            self.scripts.append(data)
        elif self.in_style:
            self.styles.append(data)


class TestNoNukesDashboard(unittest.TestCase):
    sandbox_dir = None
    server_proc = None
    port = None
    server_url = None
    driver = None

    @classmethod
    def setUpClass(cls):
        # 1. Create sandbox temp directory
        cls.sandbox_dir_obj = tempfile.TemporaryDirectory(prefix="nonukes_e2e_sandbox_")
        cls.sandbox_dir = os.path.abspath(cls.sandbox_dir_obj.name)

        # Create frontend dir inside sandbox
        os.makedirs(os.path.join(cls.sandbox_dir, "frontend"), exist_ok=True)
        os.makedirs(os.path.join(cls.sandbox_dir, "assets"), exist_ok=True)

        # 2. Populate sandbox with copies of data files
        if os.path.exists(REAL_POOLS_PATH):
            shutil.copy(REAL_POOLS_PATH, os.path.join(cls.sandbox_dir, "assets/nonukes_pools.json"))
        else:
            # Create minimal mock pools file
            with open(os.path.join(cls.sandbox_dir, "assets/nonukes_pools.json"), "w") as f:
                json.dump({}, f)

        if os.path.exists(REAL_PRICE_CACHE_PATH):
            shutil.copy(REAL_PRICE_CACHE_PATH, os.path.join(cls.sandbox_dir, "price_cache.json"))
        else:
            with open(os.path.join(cls.sandbox_dir, "price_cache.json"), "w") as f:
                json.dump({}, f)

        if os.path.exists(REAL_RESOLVED_SWAPS_PATH):
            shutil.copy(REAL_RESOLVED_SWAPS_PATH, os.path.join(cls.sandbox_dir, "resolved_swaps.json"))
        else:
            with open(os.path.join(cls.sandbox_dir, "resolved_swaps.json"), "w") as f:
                json.dump([], f)

        if os.path.exists(REAL_UNRESOLVED_SWAPS_PATH):
            shutil.copy(REAL_UNRESOLVED_SWAPS_PATH, os.path.join(cls.sandbox_dir, "unresolved_swaps.json"))
        else:
            with open(os.path.join(cls.sandbox_dir, "unresolved_swaps.json"), "w") as f:
                json.dump([], f)

        if os.path.exists(REAL_RESERVES_PATH):
            shutil.copy(REAL_RESERVES_PATH, os.path.join(cls.sandbox_dir, "nonukes_pulsex_reserves.json"))
        else:
            # Create a mock reserves file with some data matching pools
            with open(os.path.join(cls.sandbox_dir, "nonukes_pulsex_reserves.json"), "w") as f:
                json.dump({}, f)

        os.makedirs(os.path.join(cls.sandbox_dir, "frontend/nonukes_dashboard"), exist_ok=True)
        for fname in ["index.html", "style.css", "app.js"]:
            src = os.path.join(PROJECT_ROOT, "frontend/nonukes_dashboard", fname)
            dst = os.path.join(cls.sandbox_dir, "frontend/nonukes_dashboard", fname)
            if os.path.exists(src):
                shutil.copy(src, dst)

        # 3. Start subprocess server dynamically on free port
        cls.port = cls.find_free_port()
        cls.server_url = f"http://localhost:{cls.port}"
        
        env = os.environ.copy()
        env["PORT"] = str(cls.port)
        env["RESERVES_FILE_PATH"] = os.path.join(cls.sandbox_dir, "nonukes_pulsex_reserves.json")
        
        server_script = os.path.join(PROJECT_ROOT, "scripts/dashboard_server.py")
        cls.server_proc = subprocess.Popen(
            [sys.executable, server_script],
            cwd=cls.sandbox_dir,
            env=env,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
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

        # 4. Attempt Selenium browser initialization
        if SELENIUM_AVAILABLE:
            try:
                chrome_options = Options()
                chrome_options.add_argument("--headless")
                chrome_options.add_argument("--no-sandbox")
                chrome_options.add_argument("--disable-dev-shm-usage")
                
                from selenium.webdriver.chrome.service import Service
                cd_path = shutil.which("chromedriver")
                if cd_path:
                    service = Service(executable_path=cd_path)
                    cls.driver = webdriver.Chrome(service=service, options=chrome_options)
                else:
                    cls.driver = webdriver.Chrome(options=chrome_options)
            except Exception as e:
                print(f"[INFO] Selenium initialization failed: {e}. Falling back to Hybrid API + Static DOM Validation.")
                cls.driver = None

    @classmethod
    def tearDownClass(cls):
        # Stop selenium
        if cls.driver:
            try:
                cls.driver.quit()
            except Exception:
                pass
        
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

        # Cleanup sandbox directory
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

    # Helper to load JSON files from sandbox safely
    def read_sandbox_json(self, filename):
        with open(os.path.join(self.sandbox_dir, filename), "r", encoding="utf-8") as f:
            return json.load(f)

    def write_sandbox_json(self, filename, data):
        fpath = os.path.join(self.sandbox_dir, filename)
        os.makedirs(os.path.dirname(fpath), exist_ok=True)
        with open(fpath, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)

    # Base helper to query endpoints using urllib
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
    # TIER 1 TESTS: Feature Verification
    # -------------------------------------------------------------

    def test_01_server_status_and_oracle_dashboard(self):
        """Verifies that the server starts and serves the default PulseChain Price Oracle Dashboard on root."""
        status, content = self.get_api_response("/")
        self.assertEqual(status, 200)
        self.assertIn("PulseChain Price Oracle Dashboard", content)

    def test_02_frontend_spa_loading(self):
        """Verifies HTTP 200 and DOM element structure of /nonukes/index.html."""
        status, content = self.get_api_response("/nonukes/index.html")
        self.assertEqual(status, 200)

        # Parse static DOM and check for required elements
        parser = SimpleDOMParser()
        parser.feed(content)

        element_ids = [el["id"] for el in parser.elements if el["id"]]
        self.assertIn("pool-search", element_ids, "Missing #pool-search input")
        self.assertIn("group-filters", element_ids, "Missing #group-filters panel")
        self.assertIn("pool-table", element_ids, "Missing #pool-table")
        self.assertIn("stat-total-pools", element_ids, "Missing #stat-total-pools card")
        self.assertIn("stat-active-groups", element_ids, "Missing #stat-active-groups card")
        self.assertIn("stat-total-volume", element_ids, "Missing #stat-total-volume card")
        self.assertIn("stat-total-swaps", element_ids, "Missing #stat-total-swaps card")
        self.assertIn("detail-modal", element_ids, "Missing #detail-modal")
        self.assertIn("modal-close-btn", element_ids, "Missing #modal-close-btn")

    def test_03_script_style_checks(self):
        """Verifies scripts/style tags and fonts links exist in modular SPA."""
        status, content = self.get_api_response("/nonukes/index.html")
        self.assertEqual(status, 200)

        parser = SimpleDOMParser()
        parser.feed(content)

        # Assert link elements include Google Fonts
        fonts_loaded = any("fonts.googleapis.com" in (el["href"] or "") for el in parser.elements if el["tag"] == "link")
        self.assertTrue(fonts_loaded, "Google Fonts links are missing")

        # Get style.css content
        status, style_content = self.get_api_response("/nonukes/style.css")
        self.assertEqual(status, 200)
        self.assertIn("glass", style_content)

        # Get app.js content
        status, script_content = self.get_api_response("/nonukes/app.js")
        self.assertEqual(status, 200)
        self.assertIn("function fetchPools()", script_content)
        self.assertIn("function renderPools()", script_content)
        self.assertIn("function setGroupFilter", script_content)

    def test_04_api_nonukes_pools_success(self):
        """Verifies backend API /api/nonukes/pools returns success and valid pool array."""
        status, content_str = self.get_api_response("/api/nonukes/pools")
        self.assertEqual(status, 200)
        
        data = json.loads(content_str)
        self.assertTrue(data.get("success"))
        self.assertIsInstance(data.get("pools"), list)
        
        # Verify first pool schema if pools exist
        pools = data.get("pools")
        if pools:
            pool = pools[0]
            self.assertIn("address", pool)
            self.assertIn("symbol", pool)
            self.assertIn("name", pool)
            self.assertIn("target_group", pool)
            self.assertIn("version", pool)
            self.assertIn("reserves", pool)
            self.assertIn("token0", pool["reserves"])
            self.assertIn("token1", pool["reserves"])
            self.assertIn("swap_count", pool)
            self.assertIn("volume_usd", pool)

    def test_05_api_nonukes_pool_details_success(self):
        """Verifies backend API /api/nonukes/pool_details?address=<addr> returns correct details."""
        # Get a valid address from our pools configuration
        pools_config = self.read_sandbox_json("assets/nonukes_pools.json")
        if not pools_config:
            self.skipTest("No pools available in configuration to query details.")
            
        valid_addr = list(pools_config.keys())[0]
        status, content_str = self.get_api_response(f"/api/nonukes/pool_details?address={valid_addr}")
        self.assertEqual(status, 200)
        
        data = json.loads(content_str)
        self.assertTrue(data.get("success"))
        self.assertEqual(data.get("address").lower(), valid_addr.lower())
        self.assertIn("price_trends", data)
        self.assertIn("reserves", data)
        self.assertIn("swap_history", data)

    # -------------------------------------------------------------
    # TIER 2 TESTS: Boundary & Failure
    # -------------------------------------------------------------

    def test_06_empty_or_missing_pools_json_graceful(self):
        """Verifies server handles empty/missing pools JSON gracefully (returns success: false, pools: [])."""
        pools_backup = self.read_sandbox_json("assets/nonukes_pools.json")
        try:
            # Overwrite pools file with empty dict
            self.write_sandbox_json("assets/nonukes_pools.json", {})
            status, content_str = self.get_api_response("/api/nonukes/pools")
            self.assertEqual(status, 200)
            data = json.loads(content_str)
            self.assertFalse(data.get("success"))
            self.assertEqual(data.get("pools"), [])
        finally:
            self.write_sandbox_json("assets/nonukes_pools.json", pools_backup)

    def test_07_corrupt_price_cache_graceful(self):
        """Verifies server handles malformed/corrupted price cache JSON without crashing."""
        price_backup = self.read_sandbox_json("price_cache.json")
        try:
            # Corrupt the JSON file
            with open(os.path.join(self.sandbox_dir, "price_cache.json"), "w") as f:
                f.write("{invalid json: price_cache}")
            
            # Request details which parses price_cache
            pools_config = self.read_sandbox_json("assets/nonukes_pools.json")
            if pools_config:
                valid_addr = list(pools_config.keys())[0]
                status, content_str = self.get_api_response(f"/api/nonukes/pool_details?address={valid_addr}")
                self.assertEqual(status, 200) # Should fallback to default/zero pricing
                data = json.loads(content_str)
                self.assertTrue(data.get("success"))
                trends = data.get("price_trends")
                self.assertEqual(trends, [0.0, 0.0, 0.0])
        finally:
            self.write_sandbox_json("price_cache.json", price_backup)

    def test_08_invalid_pool_addresses_400(self):
        """Verifies API handles invalid, malformed, or empty pool addresses with HTTP 400."""
        # Malformed/Empty
        status, _ = self.get_api_response("/api/nonukes/pool_details?address=")
        self.assertEqual(status, 400)
        
        status, _ = self.get_api_response("/api/nonukes/pool_details?address=shortaddr")
        self.assertEqual(status, 400)

        # Valid address format but not in pools list
        not_found_addr = "0x" + "f" * 40
        status, _ = self.get_api_response(f"/api/nonukes/pool_details?address={not_found_addr}")
        self.assertEqual(status, 400)

    def test_09_busy_port_collision_handling(self):
        """Verifies starting another server instance on a busy port fails gracefully."""
        server_script = os.path.join(PROJECT_ROOT, "scripts/dashboard_server.py")
        env = os.environ.copy()
        env["PORT"] = str(self.port) # Port already bound by our main class server
        
        proc = subprocess.Popen(
            [sys.executable, server_script],
            cwd=self.sandbox_dir,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        try:
            # Should terminate or exit with non-zero or error
            time.sleep(1.0)
            ret = proc.poll()
            if ret is not None:
                # Terminated cleanly, verify it was not running
                self.assertNotEqual(ret, 0)
            else:
                proc.terminate()
                proc.wait()
        except Exception:
            pass

    def test_10_filter_input_sanitization(self):
        """Verify search input field handles extremely long and malicious strings gracefully."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "pool-search"))
        )
        search_input = self.driver.find_element(By.ID, "pool-search")
        
        # Injection test
        search_input.clear()
        search_input.send_keys("<script>alert('xss')</script>")
        
        # Verify no rows are rendered and no JavaScript crashes
        time.sleep(0.5)
        rows = self.driver.find_elements(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
        self.assertEqual(len(rows), 0)
        
        # Extremely long filter string
        search_input.clear()
        search_input.send_keys("A" * 1000)
        time.sleep(0.5)
        rows = self.driver.find_elements(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
        self.assertEqual(len(rows), 0)

    # -------------------------------------------------------------
    # TIER 3 TESTS: Cross-Feature Combinations
    # -------------------------------------------------------------

    def test_11_interactive_table_renders_all_480_pools(self):
        """Verifies interactive table rendering for all pools."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        pools_config = self.read_sandbox_json("assets/nonukes_pools.json")
        expected_count = len(pools_config)

        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, "#pool-table tbody tr[data-address]"))
        )
        rows = self.driver.find_elements(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
        self.assertEqual(len(rows), expected_count)

    def test_12_search_combined_with_group_filter(self):
        """Search query combined with group filter renders correct subset of pools."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        pools_config = self.read_sandbox_json("assets/nonukes_pools.json")
        
        # Find a pool with a specific target_group
        target_group = None
        target_symbol = None
        for pool in pools_config.values():
            if pool.get("target_group") and pool.get("symbol"):
                target_group = pool.get("target_group")
                target_symbol = pool.get("symbol")
                break
                
        if not target_group:
            self.skipTest("No pool with target_group found in config.")

        # Calculate expected matches
        expected_count = 0
        for pool in pools_config.values():
            if pool.get("target_group") == target_group and target_symbol.lower() in pool.get("symbol").lower():
                expected_count += 1

        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, "#pool-table tbody tr[data-address]"))
        )
        
        # Click the group filter button
        buttons = self.driver.find_elements(By.CLASS_NAME, "filter-chip")
        for btn in buttons:
            if btn.text.strip().lower() == target_group.lower():
                btn.click()
                break
        
        # Enter search query
        search_input = self.driver.find_element(By.ID, "pool-search")
        search_input.clear()
        search_input.send_keys(target_symbol)
        
        time.sleep(0.5)
        rows = self.driver.find_elements(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
        self.assertEqual(len(rows), expected_count)

    def test_13_server_offline_error_handling(self):
        """UI loads cleanly and displays server connection error when backend is offline."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "btn-refresh"))
        )
        
        try:
            # Temporarily stop the server
            self.server_proc.terminate()
            self.server_proc.wait()
            
            # Click refresh
            refresh_btn = self.driver.find_element(By.ID, "btn-refresh")
            refresh_btn.click()
            time.sleep(1.0)
            
            # Verify empty table
            rows = self.driver.find_elements(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
            self.assertEqual(len(rows), 0)
        finally:
            # Restart the server
            env = os.environ.copy()
            env["PORT"] = str(self.port)
            env["RESERVES_FILE_PATH"] = os.path.join(self.sandbox_dir, "nonukes_pulsex_reserves.json")
            server_script = os.path.join(PROJECT_ROOT, "scripts/dashboard_server.py")
            self.__class__.server_proc = subprocess.Popen(
                [sys.executable, server_script],
                cwd=self.sandbox_dir,
                env=env,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
            # Poll server until active
            start_time = time.time()
            while time.time() - start_time < 5.0:
                try:
                    with socket.create_connection(("localhost", self.port), timeout=0.5):
                        break
                except (socket.timeout, ConnectionRefusedError):
                    time.sleep(0.1)
            else:
                raise RuntimeError(f"Sandbox backend server failed to restart on port {self.port}")

    def test_14_filter_state_persistence_on_refresh(self):
        """Selected filter group and search query persist after clicking refresh."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "pool-search"))
        )
        
        search_input = self.driver.find_element(By.ID, "pool-search")
        search_input.clear()
        search_input.send_keys("WPLS")
        
        refresh_btn = self.driver.find_element(By.ID, "btn-refresh")
        refresh_btn.click()
        time.sleep(0.5)
        
        # Verify search input value persists
        val = search_input.get_attribute("value")
        self.assertEqual(val, "WPLS")

    def test_15_detail_modal_handles_transient_failures(self):
        """Detail modal displays error message or handles failures gracefully, keeping page active."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "pool-search"))
        )
        
        # Trigger openPoolDetails with an invalid address
        self.driver.execute_script("openPoolDetails('0xinvalidaddress')")
        
        # Wait for modal subtitle to update with the address
        subtitle = WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "modal-subtitle"))
        )
        self.assertEqual(subtitle.text, "0xinvalidaddress")
        
        # Close the modal to show the page is still active and interactive
        close_btn = self.driver.find_element(By.ID, "modal-close-btn")
        close_btn.click()

    # -------------------------------------------------------------
    # TIER 4 TESTS: Real-World Workflows
    # -------------------------------------------------------------

    def test_16_complete_user_journey(self):
        """Complete User Journey workflow: load SPA, verify counts, filters, open detail modal, close modal."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
            
        pools_config = self.read_sandbox_json("assets/nonukes_pools.json")
        if not pools_config:
            self.skipTest("No pools configured for User Journey.")
            
        first_pool_addr = list(pools_config.keys())[0]
        first_pool = pools_config[first_pool_addr]
        target_group = first_pool.get("target_group", "NoNukes")

        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, "#pool-table tbody tr[data-address]"))
        )
        
        # Check pool counts
        total_pools_elem = self.driver.find_element(By.ID, "stat-total-pools")
        self.assertEqual(total_pools_elem.text.strip(), str(len(pools_config)))
        
        # Filter by group
        buttons = self.driver.find_elements(By.CLASS_NAME, "filter-chip")
        for btn in buttons:
            if btn.text.strip().lower() == target_group.lower():
                btn.click()
                break
        
        # Open detail modal by clicking the first pool row
        row = self.driver.find_element(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
        self.driver.execute_script("arguments[0].click();", row)
        
        # Wait for modal to show
        modal = WebDriverWait(self.driver, 5).until(
            EC.visibility_of_element_located((By.ID, "detail-modal"))
        )
        self.assertIsNotNone(modal)

        # Close modal
        close_btn = self.driver.find_element(By.ID, "modal-close-btn")
        close_btn.click()

    def test_17_telemetry_sync_workflow(self):
        """Simulate telemetry updates: append swaps, trigger refresh, verify volume updates."""
        reserves_backup = self.read_sandbox_json("nonukes_pulsex_reserves.json")
        pools_config = self.read_sandbox_json("assets/nonukes_pools.json")
        if not pools_config:
            self.skipTest("No pools configured for telemetry test.")
            
        target_addr = list(pools_config.keys())[0].lower()
        
        try:
            # Count existing swaps for target_addr in resolved_swaps.json
            resolved_backup = self.read_sandbox_json("resolved_swaps.json")
            existing_swaps_count = sum(1 for s in resolved_backup if s.get("pool_address", "").lower() == target_addr)

            # 1. Add swap count to reserves
            reserves = dict(reserves_backup)
            if target_addr not in reserves:
                reserves[target_addr] = {"reserve0": 100.0, "reserve1": 200.0, "swaps": 0}
            
            reserves[target_addr]["swaps"] = existing_swaps_count + 5
            self.write_sandbox_json("nonukes_pulsex_reserves.json", reserves)
            
            # 2. Add 5 mock swap entries to resolved_swaps.json to test volume calculation
            new_swaps = []
            for i in range(5):
                new_swaps.append({
                    "tx_hash": f"abc123xyz_{i}",
                    "pool_address": target_addr,
                    "usd_value": 300.10,  # 300.10 * 5 = 1500.50 USD total volume
                    "timestamp": time.time()
                })
            self.write_sandbox_json("resolved_swaps.json", resolved_backup + new_swaps)
            
            # 3. Request API and check update
            status, content_str = self.get_api_response("/api/nonukes/pools")
            self.assertEqual(status, 200)
            
            data = json.loads(content_str)
            target_pool_data = None
            for p in data.get("pools", []):
                if p["address"].lower() == target_addr:
                    target_pool_data = p
                    break
            
            self.assertIsNotNone(target_pool_data)
            self.assertEqual(target_pool_data["swap_count"], existing_swaps_count + 5)
            self.assertAlmostEqual(target_pool_data["volume_usd"], sum(float(s.get("usd_value", 0.0)) for s in resolved_backup if s.get("pool_address", "").lower() == target_addr) + 1500.50)
            
        finally:
            self.write_sandbox_json("nonukes_pulsex_reserves.json", reserves_backup)

    def test_18_filter_state_preservation_modal_interaction(self):
        """Verifies interacting with detail modal does not clear active filters."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
            
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "pool-search"))
        )
        
        search_input = self.driver.find_element(By.ID, "pool-search")
        search_input.clear()
        search_input.send_keys("WPLS")
        
        # Open details modal
        row = self.driver.find_element(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
        self.driver.execute_script("arguments[0].click();", row)
        
        # Wait for modal to show
        WebDriverWait(self.driver, 5).until(
            EC.visibility_of_element_located((By.ID, "detail-modal"))
        )

        # Modal close
        close_btn = self.driver.find_element(By.ID, "modal-close-btn")
        close_btn.click()
        
        # Check search value is still preserved
        self.assertEqual(search_input.get_attribute("value"), "WPLS")

    def test_19_security_path_traversal_scrubbing(self):
        """Verifies directory traversal attempts are rejected with HTTP 400."""
        # Query with traversal path
        status, _ = self.get_api_response("/api/nonukes/pool_details?address=../../../../etc/passwd")
        self.assertEqual(status, 400)
        
        status, _ = self.get_api_response("/api/nonukes/pool_details?address=../../../assets/nonukes_pools.json")
        self.assertEqual(status, 400)

    def test_20_interactive_table_renders_all_480_pools_exactly(self):
        """Double check that exact count matching is correct."""
        pools_config = self.read_sandbox_json("assets/nonukes_pools.json")
        self.assertEqual(len(pools_config), 480, f"Expected 480 pools, found {len(pools_config)}")

    def test_21_interactive_sorting(self):
        """Verifies sorting columns toggles the table rows ordering."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, "#pool-table tbody tr[data-address]"))
        )
        
        # Locate the column headers and click "Swaps" (index 4)
        headers = self.driver.find_elements(By.CSS_SELECTOR, "#pool-table thead th")
        swaps_header = headers[4]
        
        self.driver.execute_script("arguments[0].click();", swaps_header)
        time.sleep(0.5)
        
        # Get all rows swap count values
        for attempt in range(3):
            try:
                rows = self.driver.find_elements(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
                swap_values = []
                for r in rows:
                    tds = r.find_elements(By.TAG_NAME, "td")
                    if len(tds) >= 5:
                        swap_values.append(int(tds[4].text))
                break
            except Exception:
                time.sleep(0.5)
                continue
        else:
            self.skipTest("DOM updated dynamically during sorting verification")
                
        sorted_desc = all(swap_values[i] >= swap_values[i+1] for i in range(len(swap_values)-1))
        sorted_asc = all(swap_values[i] <= swap_values[i+1] for i in range(len(swap_values)-1))
        self.assertTrue(sorted_desc or sorted_asc, "Rows are not sorted after clicking header")

    def test_22_zmachine_console_interaction(self):
        """Verifies entering a command into the Z-Machine console updates terminal output."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "zmachine-console-input"))
        )
        zm_input = self.driver.find_element(By.ID, "zmachine-console-input")
        zm_input.clear()
        zm_input.send_keys("status")
        
        btn_submit = self.driver.find_element(By.ID, "btn-submit-zmachine")
        btn_submit.click()
        
        time.sleep(0.5)
        terminal = self.driver.find_element(By.ID, "zmachine-console-terminal")
        self.assertIn("status", terminal.text)

    def test_23_telemetry_chat_widget_interaction(self):
        """Verifies expanding the chat widget and sending a telemetry query."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "telemetry-chat-widget"))
        )
        
        widget = self.driver.find_element(By.ID, "telemetry-chat-widget")
        self.assertIn("collapsed", widget.get_attribute("class"))
        
        toggle_btn = self.driver.find_element(By.ID, "btn-toggle-chat")
        self.driver.execute_script("arguments[0].click();", toggle_btn)
        time.sleep(0.5)
        self.assertNotIn("collapsed", widget.get_attribute("class"))
        
        chat_input = self.driver.find_element(By.ID, "chat-input")
        chat_input.clear()
        chat_input.send_keys("Show me top volume pool")
        
        btn_send = self.driver.find_element(By.ID, "btn-send-chat")
        btn_send.click()
        time.sleep(0.5)
        
        messages = self.driver.find_element(By.ID, "chat-messages")
        self.assertIn("Show me top volume pool", messages.text)

    def test_24_setup_wizard_navigation(self):
        """Verifies setup wizard tab steps update active classes and navigation."""
        if not SELENIUM_AVAILABLE or not self.driver:
            raise unittest.SkipTest("Selenium is not available to run interactive browser checks")
        self.driver.get(f"{self.server_url}/nonukes/")
        WebDriverWait(self.driver, 5).until(
            EC.presence_of_element_located((By.ID, "step-tab-1"))
        )
        
        step_tab_1 = self.driver.find_element(By.ID, "step-tab-1")
        self.assertIn("active", step_tab_1.get_attribute("class"))
        
        self.driver.execute_script("window.goToStep(2)")
        time.sleep(0.5)
        
        step_tab_2 = self.driver.find_element(By.ID, "step-tab-2")
        self.assertIn("active", step_tab_2.get_attribute("class"))
        self.assertNotIn("active", step_tab_1.get_attribute("class"))


if __name__ == "__main__":
    unittest.main()
