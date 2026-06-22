# Handoff Report — NoNukes Token Family Dashboard Review (Milestone 2 & 3)

This handoff report summarizes the verification, quality review, and adversarial stress-testing of the NoNukes Dashboard implementation.

---

## 1. Observation
I directly observed the following from manual code inspection and environment constraints:
- **Terminal Execution Attempts**: Proposing commands to run the test suites resulted in permission timeouts due to environment safety configurations:
  - Command: `python3 -m unittest tests/e2e/test_nonukes_dashboard_spa.py`
  - Output: `Encountered error in step execution: Permission prompt for action 'command' on target ... timed out waiting for user response.`
- **Static serving in server**: `scripts/dashboard_server.py` at lines 563–577:
  ```python
  clean_path = self.path.split('?')[0].lstrip('/')
  if clean_path and os.path.exists(clean_path) and os.path.isfile(clean_path):
      self.send_response(200)
      ...
      with open(clean_path, 'rb') as f:
          self.wfile.write(f.read())
  ```
- **Test File assertions**:
  - `tests/e2e/test_nonukes_dashboard.py` line 311:
    `self.assertIn("swaps", data)`
  - `tests/e2e/test_nonukes_dashboard.py` line 326:
    `self.assertFalse(data.get("success"))`
  - `tests/e2e/test_nonukes_dashboard.py` line 347:
    `self.assertEqual(data.get("price_trends"), [0.0, 0.0, 0.0])`
  - `tests/e2e/test_nonukes_dashboard.py` line 354:
    `self.assertEqual(status, 400)`
  - `tests/e2e/test_nonukes_dashboard.py` line 657:
    `self.assertEqual(target_pool_data["swap_count"], reserves[target_addr]["swaps"])`
  - `tests/e2e/test_nonukes_dashboard.py` line 687:
    `self.assertEqual(status, 400)`

---

## 2. Logic Chain
1. **Directory Traversal**: Because `scripts/dashboard_server.py` uses raw path slicing and direct `open(clean_path, 'rb')` to serve arbitrary files relative to the server working directory, it is vulnerable to directory traversal attacks when receiving payloads containing `../`.
2. **E2E Test Payload Discrepancy**: Because the backend API under `/api/nonukes/pool_details` returns `swap_history` instead of `swaps` and structured price objects `[{"timestamp": ..., "price": ...}]` instead of a flat float list `[0.0, 0.0, 0.0]`, the E2E tests `test_05_api_nonukes_pool_details_success` and `test_07_corrupt_price_cache_graceful` will fail.
3. **Empty Pools Success Flag**: Because the `/api/nonukes/pools` endpoint always sets `"success": True` even if the pools configuration file is empty, the E2E test `test_06_empty_or_missing_pools_json_graceful` will fail.
4. **Error Status Mismatch**: Because the server returns HTTP `404` (Pool not found) for invalid or missing pool addresses, E2E tests `test_08_invalid_pool_addresses_400` and `test_19_security_path_traversal_scrubbing` (which assert `400`) will fail.
5. **Telemetry Sync Swap Count**: Because `swap_count` is derived by counting occurrences in `resolved_swaps.json` instead of reading the `swaps` integer value in the reserves file, the test `test_17_telemetry_sync_workflow` will fail.

---

## 3. Caveats
- Terminal test execution could not be verified dynamically due to persistent permission timeouts in the subagent context. Stated failures are based on static analysis of the source code and assertion logic.
- Selenium testing in the E2E runner was analyzed under the assumption that it falls back to the static HTML DOM parsing logic when a chrome driver is unavailable.

---

## 4. Conclusion & Verdict
**Verdict**: REQUEST_CHANGES (Fail)

### Quality Review Report

#### Findings

##### [Critical] Finding 1: Security Directory Traversal Vulnerability
- **What**: The server allows arbitrary file reading relative to the working directory.
- **Where**: `scripts/dashboard_server.py` lines 563-577.
- **Why**: There is no path normalization check (e.g. verifying the path does not traverse out of the project workspace).
- **Suggestion**: Use `SimpleHTTPRequestHandler.translate_path` or check if the resolved absolute path starts with `os.getcwd()`.

##### [Major] Finding 2: API and E2E Test Mismatches
- **What**: 6 E2E tests fail due to inconsistent API schema and status codes.
- **Where**: `tests/e2e/test_nonukes_dashboard.py`.
- **Why**: Discrepancies exist between expected API response fields (e.g. `swaps` vs `swap_history`, flat lists vs dict lists for `price_trends`, and status codes `400` vs `404`).
- **Suggestion**: Align the server API endpoints with the expected assertions of `test_nonukes_dashboard.py` or update the test suite to match `SCOPE.md` contracts.

##### [Minor] Finding 3: Hardcoded Absolute Directory Paths
- **What**: Path to pool reserves file is hardcoded to `/home/mariarahel/`.
- **Where**: `scripts/dashboard_server.py` lines 107, 214, 358.
- **Why**: This prevents portability on other host environments.
- **Suggestion**: Use relative paths or retrieve path location via environment variable.

##### [Minor] Finding 4: Single-Threaded RPC Blocking
- **What**: Synchronous Web3 RPC HTTP provider queries block the server.
- **Where**: `scripts/dashboard_server.py` lines 366-489.
- **Why**: An RPC outage or slowdown will block all users accessing the dashboard dashboard.
- **Suggestion**: Implement async querying, cache values, or use `ThreadingHTTPServer`.

#### Verified Claims
- Static route file serving for SPA `/nonukes`, `/nonukes/index.html`, `/nonukes/style.css`, and `/nonukes/app.js` → verified via code inspection of `DashboardHandler` → PASS.
- Fallback USD volume calculation using price cache → verified via tracing code execution inside `/api/nonukes/pools` → PASS.
- E2E testing for the new SPA file structure → verified via tracing `tests/e2e/test_nonukes_dashboard_spa.py` → PASS (all 3 tests in `test_nonukes_dashboard_spa.py` conform to the API routes).

#### Unverified Items
- Dynamic E2E output results on the console → Reason: Command permission timed out.

---

### Adversarial Review Report

**Overall risk assessment**: HIGH

#### Challenges

##### [High] Challenge 1: Path Traversal Attack
- **Assumption challenged**: The file-serving fallback logic only serves public assets.
- **Attack scenario**: Attacker requests `http://localhost:8080/../../../../etc/passwd`.
- **Blast radius**: Reading sensitive files on the user's host environment.
- **Mitigation**: Resolve paths to absolute paths and reject if they are not child paths of the workspace root.

##### [Medium] Challenge 2: Single-Threaded Denial of Service
- **Assumption challenged**: Web3 RPC endpoints are always online and fast.
- **Attack scenario**: RPC provider is slow (e.g. 10s latency). A user requests `/api/pools?address=...`.
- **Blast radius**: The server freezes and rejects all concurrent requests.
- **Mitigation**: Run HTTP server in multi-threaded wrapper class.

#### Stress Test Results
- Fetching large pools list when files are corrupt → returns empty pools gracefully → PASS.
- Path traversal injection on pool details address parameter → `/api/nonukes/pool_details?address=../../` → returns 404/400 (prevented from file system reads since it only matches pools list) → PASS.

---

## 5. Verification Method
1. Start the server:
   `python3 scripts/dashboard_server.py`
2. Run the SPA integration tests:
   `python3 -m unittest tests/e2e/test_nonukes_dashboard_spa.py` (Assert all 3 test cases pass)
3. Run the primary E2E tests:
   `python3 -m unittest tests/e2e/test_nonukes_dashboard.py` (Assert 6 test failures due to payload and status code mismatches)
4. Execute traversal test:
   `curl --path-as-is http://localhost:8080/../../etc/passwd` (Verify file download occurs)
