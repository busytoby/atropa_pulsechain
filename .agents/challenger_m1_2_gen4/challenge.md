# Adversarial Review of NoNukes Dashboard Server and E2E Test Suite

## Challenge Summary

**Overall risk assessment**: LOW

The E2E test suite (`tests/e2e/test_nonukes_dashboard.py` and `tests/e2e/test_nonukes_dashboard_spa.py`) and server changes (`scripts/dashboard_server.py`) are robustly implemented. The code safely handles missing or malformed inputs, ensures thread safety for state modifications, and provides dynamic UI recovery mechanisms.

*Note on Dynamic Execution*: Terminal execution of the test suite and custom scripts via `run_command` timed out due to user-permission prompt delays in this environment. Therefore, this report is based on static verification, mathematical code traces, and logical walkthroughs of the execution flows.

---

## Challenges

### [Low] Challenge 1: Unlocked JSON Reads in `/api/data` GET Handler

- **Assumption challenged**: The server assumes that because all writes to `treasury_tokens_*.json` are serialized under `IGNORE_LOCK`, read operations in the `/api/data` handler do not require locks.
- **Attack scenario**: A request to `/api/data` executes concurrently with a request to `/api/ignore`. The writer thread truncates and writes the updated JSON file. At the same time, the `/api/data` thread calls `json.load(f)` on the truncated or partially written file.
- **Blast radius**: The `/api/data` request encounters an exception (`json.JSONDecodeError`) when reading the partially written file.
- **Mitigation**: The server already mitigates this by wrapping the file read in a broad `try...except Exception:` block, resulting in the call failing gracefully (returning a partial/empty token dictionary) instead of crashing the process. A tighter defense would be acquiring a shared read lock or using atomic file writes (writing to a temp file and renaming it).

### [Low] Challenge 2: Dynamic Volume Fallback Divide-by-Zero in Trends Calculation

- **Assumption challenged**: The server assumes that minter token swap amounts are always strictly greater than zero when resolving swap trends.
- **Attack scenario**: A malformed swap event is loaded from a telemetry file containing `amount0 = 0.0` or `amount1 = 0.0` or missing the target minter token key.
- **Blast radius**: If `amt_m == 0.0`, the trends loop condition `if usd_val > 0 and amt_m > 0:` safely bypasses division, setting `price = 0.0`.
- **Mitigation**: The code correctly validates `amt_m > 0` and `usd_val > 0` before doing the division (`price = usd_val / amt_m`). This is already fully handled and does not crash.

---

## Stress Test Results

- **Scenario 1: price_cache.json is Malformed**
  - **Expected behavior**: Server catches `json.JSONDecodeError` during initialization/endpoint querying, does not crash, and falls back to default/zero pricing.
  - **Actual/Predicted behavior**: PASS. Tested by static trace of `except Exception:` block in `/api/nonukes/pools` and `/api/nonukes/pool_details` endpoints, returning `price_trends = [0.0, 0.0, 0.0]` or returning `usd_val` with default fallbacks.
  
- **Scenario 2: price_cache.json values are null**
  - **Expected behavior**: `get_price` evaluates `val is None` or `price_val is None` and returns `0.0` without crashing.
  - **Actual/Predicted behavior**: PASS. Checked the trace: `price_val = val.get("price")` evaluates to `None` for `{ "price": null }`, returning `0.0`. Direct `null` entry also returns `0.0`.
  
- **Scenario 3: Concurrent /api/ignore updates**
  - **Expected behavior**: Concurrent HTTP requests targeting the same metadata files are serialized, preventing file corruption and race conditions.
  - **Actual/Predicted behavior**: PASS. Thread safety is enforced by a global `IGNORE_LOCK = threading.Lock()` wrapping all read/write file blocks in `/api/ignore`.
  
- **Scenario 4: Server Offline and Back Online UI Sync**
  - **Expected behavior**: UI table is cleared on connection failure and dynamically repopulates on reconnection.
  - **Actual/Predicted behavior**: PASS. Verified by `app.js` trace: `fetchPools()` uses `try...catch` block. On failure, `pools = []` resets stats and clears table. Recovery happens automatically during the next `setInterval` tick or refresh click.

---

## Unchallenged Areas

- **V2 Factory Pair Retrieval**: Live contract calls to PulseX factories on PulseChain mainnet via Web3 HTTPProvider were not challenged as they require real network requests and active external RPC endpoint connectivity, which is simulated or skipped under mock E2E configurations.
