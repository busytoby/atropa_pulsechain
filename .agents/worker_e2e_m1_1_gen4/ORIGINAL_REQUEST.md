## 2026-06-14T18:52:24-07:00
Objective: Apply critical correctness, reliability, security, and concurrency improvements to the NoNukes Dashboard SPA E2E test suite and backend server script.

Target Files to modify:
1. `/home/mariarahel/src/tsfi2/atropa_pulsechain/tests/e2e/test_nonukes_dashboard.py`
2. `/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/nonukes_dashboard/app.js`
3. `/home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/dashboard_server.py`

Required Modifications:
- **Deadlock prevention in tests**:
  - In `setUpClass` and `test_13_server_offline_error_handling` within `/home/mariarahel/src/tsfi2/atropa_pulsechain/tests/e2e/test_nonukes_dashboard.py`, redirect stdout and stderr of the server subprocess `subprocess.Popen` to `subprocess.DEVNULL` instead of `subprocess.PIPE` to prevent OS buffer deadlock.
- **Test isolation in `test_13_server_offline_error_handling`**:
  - Wrap the stop, refresh, assert, and restart logic in a `try...finally` block in `tests/e2e/test_nonukes_dashboard.py` to guarantee that the server is restarted even if assertions fail.
- **Frontend UI state on offline error**:
  - In `/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/nonukes_dashboard/app.js`, edit `fetchPools()` so that if the fetch request fails or `data.success` is false, the `pools` array is cleared (`pools = []`), and `extractGroups()`, `renderStats()`, `renderGroupFilters()`, and `renderPools()` are called to update the DOM to clear stale elements and match the offline error state.
- **Price cache Null pointer/type error protection**:
  - In `/home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/dashboard_server.py`, around lines 160-161 and 286-287, define a helper function `get_price(prices, addr)` that returns a float. Ensure it checks if the value returned from `prices.get(addr)` is a dictionary (and has a key `"price"`), checks for `None`/`null` price values, and catches any `ValueError`/`TypeError` during the conversion, falling back to `0.0`. Update both price-resolution logic blocks to use this helper.
- **Access control & sanitize `/api/ignore` endpoint**:
  - In `/home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/dashboard_server.py`, validate that the query parameter `address` starts with `0x`, is exactly 42 characters long, and consists only of hexadecimal characters (e.g. using regex pattern `^0x[0-9a-fA-F]{40}$`). Return HTTP 400 Bad Request with a JSON error response `{"success": false, "error": "..."}` if invalid.
- **Thread-safe file operations**:
  - In `/home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/dashboard_server.py`, introduce a global threading lock (`import threading` and `IGNORE_LOCK = threading.Lock()`) and wrap all read/write file operations inside the `/api/ignore` endpoint handler (under `with IGNORE_LOCK:`) to prevent concurrent updates from causing file corruption.

Verification:
- Run the E2E test suite locally using the command `python3 -m unittest tests/e2e/test_nonukes_dashboard.py`. Verify that all tests pass, including the headless selenium checks if selenium is available, or the static DOM verification fallbacks if selenium is not.
- Document the commands and test outputs in your handoff.
