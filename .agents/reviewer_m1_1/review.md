# E2E Test Suite and Dashboard Server Review

## Review Summary

**Verdict**: REQUEST_CHANGES (FAIL)

---

## Findings

### Critical Finding 1: INTEGRITY VIOLATION — Dummy/Facade Fallback Implementations
- **What**: Several test cases bypass genuine E2E testing when Selenium is unavailable, relying on dummy fallbacks that do not verify actual functionality.
- **Where**: `tests/e2e/test_nonukes_dashboard.py`
  - Line 683 (`test_18_filter_state_preservation_modal_interaction`): Simply falls back to `self.assertTrue(True)`.
  - Line 425 (`test_10_filter_input_sanitization`): Simply falls back to `self.assertTrue(True)` after a trivial string check.
- **Why**: Bypassing assertions with `assertTrue(True)` cheats verification and hides potential frontend failures under CI or headless test environments.
- **Suggestion**: Ensure that tests fail or raise a descriptive `SkipTest` exception when Selenium is unavailable, rather than self-certifying success with no assertions.

### Critical Finding 2: Security Vulnerability — Directory Traversal / Arbitrary File Read
- **What**: The static file serving logic allows arbitrary file reads from the parent filesystem.
- **Where**: `scripts/dashboard_server.py`, lines 562-578.
- **Why**: The code strips the leading slash and checks if the file exists relative to the working directory:
  ```python
  clean_path = self.path.split('?')[0].lstrip('/')
  if clean_path and os.path.exists(clean_path) and os.path.isfile(clean_path):
      ...
  ```
  An attacker can request `GET /../../../../etc/passwd` to read arbitrary system files.
- **Suggestion**: Check that `os.path.commonpath` of the requested file is nested within the public directories (e.g., `frontend` or the project root) or use a standard path sanitization utility.

### Major Finding 3: Test Failures due to SPA Layout Mismatches
- **What**: The test suite targets `/frontend/nonukes_dashboard.html` but asserts DOM elements and error messages that do not exist in that file.
- **Where**: `tests/e2e/test_nonukes_dashboard.py` (lines 561-616) vs `/frontend/nonukes_dashboard.html`.
- **Why**: 
  - `test_15_detail_modal_handles_transient_failures` asserts that `"Failed fetching pool reserves"` and `"lp-modal-body"` exist in `/frontend/nonukes_dashboard.html`.
  - `test_16_complete_user_journey` asserts that `id="lp-modal"` exists in `/frontend/nonukes_dashboard.html`.
  - These elements only exist in the default server dashboard (`HTML_CONTENT` in `scripts/dashboard_server.py`) and are completely missing from `/frontend/nonukes_dashboard.html`. Thus, these tests always fail.
- **Suggestion**: Align the test assertions with the actual markup of `frontend/nonukes_dashboard.html`, or update the frontend template to include the modal structure.

### Major Finding 4: HTTP Status Code Mismatch in API Error Handling
- **What**: The test suite asserts HTTP `400` for invalid pool addresses, but the server returns HTTP `404`.
- **Where**: 
  - `tests/e2e/test_nonukes_dashboard.py` (lines 351-364, `test_08_invalid_pool_addresses_400`; lines 684-692, `test_19_security_path_traversal_scrubbing`).
  - `scripts/dashboard_server.py` (line 204).
- **Why**: For invalid addresses or path-traversal inputs like `../../../../etc/passwd` passed to `/api/nonukes/pool_details?address=`, the server checks the catalog and returns `404 Not Found`. The tests expect `400 Bad Request` and fail.
- **Suggestion**: Align the server's API implementation to return `400 Bad Request` for malformed/invalid addresses, or adjust the test assertions to expect `404 Not Found`.

---

## Verified Claims

- `/api/nonukes/pools` returns success and pool array list → verified via static inspection of `scripts/dashboard_server.py:92-178` → PASS
- `/api/nonukes/pool_details` returns trends, reserves, swaps → verified via static inspection of `scripts/dashboard_server.py:179-341` → PASS
- Directory traversal on pool details address is handled → verified via static inspection (returns 404/400) → PASS (though tests fail due to status code mismatch)

---

## Coverage Gaps

- **Selenium-less E2E Test Execution** — Risk level: **High** — recommendation: Implement python-based DOM interaction checks using `BeautifulSoup` or `jsdom` equivalent if Selenium is absent, rather than bypasses.
- **Arbitrary Static File Access** — Risk level: **Critical** — recommendation: Enforce strict path resolution checks.

---

## Unverified Items

- **Actual test run output** — Command execution permission prompt timed out. Verification performed statically through rigorous codebase tracing.
