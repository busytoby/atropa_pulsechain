# Handoff Report: E2E Test Suite and Backend Server Correctness Verification

**Author**: E2E Testing Challenger (challenger_m1_1_gen3)  
**Date**: 2026-06-15T01:50:30Z  
**Scope**: `tests/e2e/test_nonukes_dashboard.py`, `scripts/dashboard_server.py`

---

## 1. Observation

### Command Execution Attempts
We attempted to run the E2E test suite in the target environment:
- Command: `python3 -m unittest tests/e2e/test_nonukes_dashboard.py` from directory `/home/mariarahel/src/tsfi2/atropa_pulsechain`.
- Result: The environment timed out waiting for user permission twice (once synchronous, once backgrounded):
  > "Encountered error in step execution: Permission prompt for action 'command' on target 'python3 -m unittest tests/e2e/test_nonukes_dashboard.py' timed out waiting for user response. The user was not able to provide permission on time."

Consequently, we performed static and adversarial code analysis on the test suite and backend server, validating assumptions, security boundaries, and concurrency patterns.

### E2E Test Suite Code Analysis (`tests/e2e/test_nonukes_dashboard.py`)
1. **Alphabetical Execution Order & State Leak**: Tests are run sequentially.
   - `test_13_server_offline_error_handling` stops the server and runs assertions:
     ```python
     501:         # Temporarily stop the server
     502:         self.server_proc.terminate()
     503:         self.server_proc.wait()
     504:         
     505:         # Click refresh
     506:         refresh_btn = self.driver.find_element(By.ID, "btn-refresh")
     507:         refresh_btn.click()
     508:         time.sleep(1.0)
     509:         
     510:         # Verify empty table
     511:         rows = self.driver.find_elements(By.CSS_SELECTOR, "#pool-table tbody tr[data-address]")
     512:         self.assertEqual(len(rows), 0)
     ```
     If lines 505–512 raise an `AssertionError` or any other error, the execution aborts before reaching the server restart code block at lines 513–534.
2. **Race Condition in Port Collision Test**:
   - `test_09_busy_port_collision_handling` starts a second server process and calls `time.sleep(1.0)`. If the second process has not terminated within 1.0s, the code terminates it without asserting non-zero exit:
     ```python
     392:             ret = proc.poll()
     393:             if ret is not None:
     394:                 self.assertNotEqual(ret, 0)
     395:             else:
     396:                 proc.terminate()
     397:                 proc.wait()
     ```
     Under CPU pressure, Python startup/module loading may exceed 1.0s, letting the test pass silently without asserting collision handling.
3. **Telemetry Test Restores reserves but not swaps**:
   - `test_17_telemetry_sync_workflow` backs up and restores `nonukes_pulsex_reserves.json` but modifies `resolved_swaps.json` without restoring it in the `finally` block:
     ```python
     658:             self.write_sandbox_json("resolved_swaps.json", resolved_backup + new_swaps)
     ```
4. **Testing Coverage Gap**:
   - The test suite only runs selenium tests against `/nonukes/index.html` (the modular SPA). It completely omits the main `/` root dashboard (serving the `HTML_CONTENT` Oracle page) and the `/api/data`, `/api/pools`, and `/api/ignore` API endpoints.

### Backend Server Code Analysis (`scripts/dashboard_server.py`)
1. **Concurrency and Lost Updates**:
   - The server implements `ThreadingHTTPServer` (line 1685) but uses no mutex locks when reading or writing JSON databases:
     - `/api/ignore` reads/writes `treasury_tokens_unknown_minter.json` and other `treasury_tokens_*.json` files:
       ```python
       517:                         with open(fpath, "r") as f:
       518:                             data = json.load(f)
       519:                         if address in data:
       520:                             data[address]["ignored"] = ignored_val
       521:                             with open(fpath, "w") as f:
       522:                                 json.dump(data, f, indent=4)
       ```
2. **Non-Atomic File Updates**:
   - Updates to JSON databases use direct write modes (`"w"`). If the process terminates abnormally (SIGKILL, power failure, OOM) mid-write, the file is truncated/corrupted.
3. **Input Sanitization Failures in `/api/ignore`**:
   - No validation on `address` query parameter (lines 507–508) other than `.lower()`.
   - On the root page `/` (`HTML_CONTENT`), the address is rendered directly inside an HTML attribute:
     ```html
     1368:                                 <td class="clickable-token" onclick="showLpModal('${addr}', '${b64Symbol}', '${b64Name}', true)">
     ```
4. **XSS via Token Metadata**:
   - The root page `/` uses `innerHTML` to render token symbols and names from `/api/data` without HTML escaping:
     ```javascript
     1252:                                 <span>${item.token0.symbol} ➡️ ${item.token1.symbol}</span>
     ```
5. **External Network Requests**:
   - The `/api/pools` handler queries a real public RPC endpoint:
     ```python
     382:                 w3 = Web3(Web3.HTTPProvider("https://rpc.pulsechain.com"))
     ```

---

## 2. Logic Chain

1. **Test Pollution & Cascading Failure**:
   - `test_13` shuts down the server.
   - If an assertion fails during the offline check, the test method raises an exception immediately.
   - The method exits without running the code below it (the restart logic).
   - Subsequent tests (`test_14` to `test_20`) execute on the same class instance.
   - Because they depend on a running server to respond to urllib or selenium queries, they all fail.
   - *Conclusion*: A single failure in `test_13` will pollute the environment and cause cascading failures.

2. **State DB Corruption under Concurrency**:
   - `ThreadingHTTPServer` runs each request on a new thread.
   - `/api/ignore` performs a read-modify-write on `treasury_tokens_unknown_minter.json`.
   - Thread A truncates the file during `open(fpath, "w")` and starts writing.
   - Concurrently, Thread B tries to read the file `open(fpath, "r")`.
   - Thread B reads empty/partial data, raising `json.decoder.JSONDecodeError`.
   - The exception is caught by `except Exception: pass`, resulting in `tokens = {}`.
   - Thread B inserts its key, writes to file, and overwrites the entire database with a single entry, wiping out all previously registered tokens.
   - *Conclusion*: Concurrent calls to `/api/ignore` will lead to database data loss.

3. **Stored XSS via `/api/ignore`**:
   - An attacker makes a request to `/api/ignore?address=');alert(document.cookie);('`.
   - The server does not validate the address and saves it into `treasury_tokens_unknown_minter.json` as the key.
   - The dashboard root page fetches `/api/data` and receives the key `');alert(document.cookie);('`.
   - The JavaScript client renders the row using a template string:
     ```html
     onclick="showLpModal('');alert(document.cookie);('', ...)"
     ```
   - When a user views the dashboard and clicks the token row, the browser executes the injected JavaScript.
   - *Conclusion*: Stored XSS vulnerability exists on the root dashboard page.

4. **Stored XSS via Token Metadata**:
   - An attacker deploys a token on PulseChain with a malicious symbol, e.g., `<img src=x onerror=alert(1)>`.
   - The scraper logs swaps for this token and writes them to `resolved_swaps.json`.
   - The server returns this data via `/api/data`.
   - The root page JavaScript renders the timeline:
     ```javascript
     timeline.innerHTML = ... `<span>${item.token0.symbol} ➡️ ${item.token1.symbol}</span>`
     ```
   - The browser parses the malicious symbol as HTML, triggering the payload.
   - *Conclusion*: Stored XSS vulnerability exists via on-chain token metadata.

---

## 3. Caveats

- **Runtime Verification**: Because terminal commands timed out due to non-interactive environment security restrictions, the execution output of the E2E suite under Selenium could not be physically observed. However, the static analysis confirms the logic is sound and the flaws exist in the code base.
- **Selenium Headless Dependencies**: Under environments where Google Chrome or `chromedriver` is not configured, Selenium tests will automatically skip.

---

## 4. Conclusion

1. The E2E test suite (`test_nonukes_dashboard.py`) contains a critical cleanup gap in `test_13` that causes subsequent tests to fail if `test_13` fails.
2. The backend server (`dashboard_server.py`) has critical concurrency vulnerabilities (Lost Updates / database deletion) and high security vulnerabilities (Stored DOM XSS via `/api/ignore` and unescaped token metadata).
3. The E2E test suite suffers from a large coverage gap as it completely ignores the root `/` dashboard server logic.

---

## 5. Verification Method

To verify these findings manually when command permissions are available:
1. **Cascading Failure**: Force a failure in `test_13` (e.g. assert 1 == 0 before restarting the server) and observe that all subsequent tests fail.
2. **Concurrency Data Loss**: Run a script that parallelizes 50 concurrent requests to `/api/ignore?address=0x<address>&ignored=true` with unique addresses, and check if `treasury_tokens_unknown_minter.json` ends up with fewer than 50 entries.
3. **Stored XSS**:
   - Call `curl "http://localhost:8080/api/ignore?address=');alert('XSS');('"`
   - Open a browser at `http://localhost:8080/`
   - Go to the Treasury Token Registry card, choose the active tab, and click the row for the malicious address. Verify that the alert dialog pops up.

---

# Adversarial Review Report (Critic Role)

**Overall risk assessment**: CRITICAL

## Challenges

### [Critical] Challenge 1: Concurrency Race Condition on State Files
- **Assumption challenged**: Multiple clients can safely toggle token ignored states concurrently.
- **Attack scenario**: Thread A writes a file while Thread B reads it, causing a crash or database truncation.
- **Blast radius**: Wipes the entire database of tracked/ignored treasury tokens.
- **Mitigation**: Implement a thread lock (`threading.Lock`) around all read-write operations for JSON database files, or migrate to a transactional database (e.g., SQLite).

### [High] Challenge 2: Stored DOM XSS in Dashboard
- **Assumption challenged**: The `address` query parameter passed to `/api/ignore` is always a valid hex address.
- **Attack scenario**: An attacker sends a payload that escapes the JavaScript string delimiter in the generated `onclick` attribute.
- **Blast radius**: Arbitrary JavaScript execution in the context of the user session when visiting the dashboard.
- **Mitigation**: Sanitise `address` to ensure it matches `^0x[a-fA-F0-9]{40}$`. Additionally, HTML-escape all variables rendered in Javascript template strings.

### [High] Challenge 3: HTML Injection via Token Symbols and Names
- **Assumption challenged**: On-chain token symbols and names are safe plain text.
- **Attack scenario**: A token is registered with a symbol containing HTML tags (`<script>`, `<img>`).
- **Blast radius**: Stored XSS for anyone viewing the live activity feed on the root page.
- **Mitigation**: Escape all token symbol and name fields before appending them using `innerHTML`, or use `textContent` to populate table cells.

### [Medium] Challenge 4: Test Suite Cascading Failures
- **Assumption challenged**: Tests can shut down shared state (the server) safely without affecting other tests.
- **Attack scenario**: `test_13` fails during assertion, skipping the server restart logic.
- **Blast radius**: Cascading failures for all tests that run after `test_13`.
- **Mitigation**: Wrap the shutdown/assertion sequence in `try-finally` or implement server lifecycle management in `tearDown` / `setUp` hooks.

## Stress Test Results

| Scenario | Expected Behavior | Predicted Behavior | Pass/Fail |
|---|---|---|---|
| Port Collision | Second server exits with error | Second server exits with code 1 | Pass |
| Test 13 Failure Cleanup | Subsequent tests pass | Subsequent tests fail | Fail |
| Concurrent API requests | State persists correctly | Database is wiped / truncated | Fail |
| Traversal query to `/api/nonukes/pool_details` | Rejected with HTTP 400 | Rejected with HTTP 400 | Pass |
| Traversal query to `/api/ignore` | Rejected with HTTP 400 | Accepted and saved to JSON | Fail |

## Unchallenged Areas
- Selenium browser interaction speed is not challenged as headless execution is environment-dependent.
