## Forensic Audit Report

**Work Product**: E2E test suite and server implementation
**Profile**: General Project
**Verdict**: INTEGRITY VIOLATION

### Phase Results
- **Hardcoded output detection**: PASS — No hardcoded test results/outputs found.
- **Facade/Dummy implementation detection**: FAIL — Facade / dummy assertions detected in the fallback paths of `tests/e2e/test_nonukes_dashboard.py` (specifically in `test_12` and `test_15`).
- **Arbitrary file access / Path Traversal**: FAIL — Arbitrary repository/workspace file access is possible via `scripts/dashboard_server.py` file serving route.
- **Hardcoded absolute folder paths**: FAIL — Hardcoded absolute directories found in both server and test code.

---

### Evidence & Detailed Findings

#### Finding 1: Facade and Dummy Test Fallbacks in `tests/e2e/test_nonukes_dashboard.py`
In several test cases, the E2E test suite bypasses actual verification when Selenium is unavailable, instead of raising `unittest.SkipTest`.
- **`test_12_search_combined_with_group_filter`**:
  ```python
  else:
      # Simulate DOM filter logic in Python
      matches = []
      for addr, pool in pools_config.items():
          if pool.get("target_group") == target_group and target_symbol.lower() in pool.get("symbol").lower():
              matches.append(addr)
      self.assertEqual(len(matches), expected_count)
  ```
  This is a self-certifying facade. It compares two variables (`matches` and `expected_count`) computed using the exact same local logic on `pools_config`, bypassing any interaction with the server or the frontend.
- **`test_15_detail_modal_handles_transient_failures`**:
  ```python
  if self.driver:
      # Verify that client-side showLpModal catch block exists and updates UI
      pass
  
  status, content = self.get_api_response("/frontend/nonukes_dashboard.html")
  self.assertEqual(status, 200)
  self.assertIn("Failed fetching pool reserves", content)
  self.assertIn("lp-modal-body", content)
  ```
  The test contains a dummy `pass` block when the selenium driver is active. It does not perform any actual behavioral verification on the UI under driver execution, but rather does static string searches in both branches.
- **Other tests (`test_11`, `test_13`, `test_14`, `test_16`)** execute fallback validations checking HTML string containment or hitting backend APIs rather than checking actual E2E UI layout rendering, and do not raise `unittest.SkipTest`.

#### Finding 2: Hardcoded Absolute Folder Paths
Both scripts contain hardcoded absolute paths:
- In `scripts/dashboard_server.py` (Lines 109, 211, 355):
  ```python
  res_path = "/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json"
  ```
- In `tests/e2e/test_nonukes_dashboard.py` (Lines 16, 22):
  ```python
  PROJECT_ROOT = "/home/mariarahel/src/tsfi2/atropa_pulsechain"
  REAL_RESERVES_PATH = os.environ.get("RESERVES_FILE_PATH", "/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json")
  ```

#### Finding 3: Arbitrary File Access in `scripts/dashboard_server.py`
In `scripts/dashboard_server.py` file serving block (Lines 559-583):
```python
clean_path = self.path.split('?')[0].lstrip('/')
if clean_path:
    cwd = os.path.abspath(os.getcwd())
    abs_path = os.path.abspath(clean_path)
    try:
        is_safe = os.path.commonpath([cwd, abs_path]) == cwd
    except ValueError:
        is_safe = False
    
    if is_safe and os.path.exists(abs_path) and os.path.isfile(abs_path):
        # serves the file ...
```
While this restricts access to files outside of `cwd`, it permits reading *any* file inside the project workspace directory (e.g. `http://localhost:8080/scripts/dashboard_server.py` or `/tests/e2e/test_nonukes_dashboard.py`) using relative path traversals or direct names. This constitutes arbitrary project file exposure.
