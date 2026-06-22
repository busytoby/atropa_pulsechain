# Quality Review Report

## Review Summary

**Verdict**: REQUEST_CHANGES
**Critical Finding**: INTEGRITY VIOLATION (Facade Test Suite)

*Explanation*: The E2E test suite at `tests/e2e/test_nonukes_dashboard.py` consists of 20 test cases designed to test Tiers 1-4 of the PulseChain Price Oracle / NoNukes Dashboard. However, static analysis reveals that a significant number of these tests contain incorrect assertions, mismatched query parameters, wrong path references, and rely on hardcoded environment-specific absolute paths. The test suite cannot pass in its current state. The test suite appears to be a facade designed to look complete without implementing correct E2E assertions for the actual server code.

---

## Findings

### [Critical] Finding 1: INTEGRITY VIOLATION - Facade Test Suite
- **What**: The E2E test suite contains multiple assertions that mismatch the server implementation and will fail under any run.
- **Where**: `tests/e2e/test_nonukes_dashboard.py` (multiple test cases: 05, 06, 07, 08, 15, 16, 17, 19).
- **Why**: 
  - `test_05` asserts a key `"swaps"` in pool details, but the server returns `"swap_history"`.
  - `test_06` asserts that empty pools return `success: false`, but the server returns `success: true` with an empty pools list.
  - `test_07` asserts that corrupt price cache returns `price_trends = [0.0, 0.0, 0.0]`, but the server returns a list of dictionaries.
  - `test_08` and `test_19` assert HTTP status 400 for malformed/missing addresses, but the server returns 404.
  - `test_15` and `test_16` check `/frontend/nonukes_dashboard.html` for modal elements like `lp-modal-body` and `id="lp-modal"` which are completely absent in that file (they exist in the modular index.html or the server's inline HTML).
  - `test_17` writes to the sandbox reserves file, but the server reads from a hardcoded absolute path, meaning the update is ignored.
- **Suggestion**: Rewrite the test suite to target `/nonukes/index.html` (the correct production modular dashboard) and fix all JSON schema/HTTP status code assertions to match the actual server behavior.

### [Critical] Finding 2: Security - Path Traversal Vulnerability
- **What**: The static file server logic in the dashboard handler is vulnerable to directory traversal attacks.
- **Where**: `scripts/dashboard_server.py`, lines 562–577.
- **Why**: The server takes the requested URL path, strips leading slashes, and calls `os.path.exists()` and `open()` directly on the result relative to the working directory without any sanitization. This allows an attacker to request `/../../../../etc/passwd` or other sensitive system files and retrieve them.
- **Suggestion**: Resolve the requested file path to an absolute path using `os.path.abspath()` and verify that it starts with the absolute path of the allowed document root.

### [Critical] Finding 3: Hardcoded Absolute Paths and Portability
- **What**: The server and test suite hardcode an absolute path to a specific user's app data directory.
- **Where**:
  - `scripts/dashboard_server.py`, lines 107 and 214.
  - `tests/e2e/test_nonukes_dashboard.py`, line 22.
- **Why**: The path `/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json` is hardcoded. This prevents portability across different systems or workspaces and forces E2E tests to fail if run in isolation or under a different session.
- **Suggestion**: Use relative paths from the workspace root, or configure path inputs using environment variables (e.g., `RESERVES_FILE_PATH`).

---

## Verified Claims

- **480 Pools Configuration** → Verified via inspecting `nonukes_pools.json` (3362 lines of JSON, exactly 480 pool objects) → **PASS**
- **Server Port Collision** → The server correctly terminates or exits with non-zero when the port is busy → **PASS**

---

## Coverage Gaps

- **Modular Dashboard Testing** — The E2E tests target `/frontend/nonukes_dashboard.html` (which is an obsolete single-file version) instead of `/nonukes/index.html` (the modular version served at the root `/nonukes` path). Risk level: **HIGH**. Recommendation: Re-route tests to point to the production-ready modular path.

---

## Unverified Items

- **Selenium Browser Automation** — Automated UI interaction (e.g., button clicks, search typing) was not verified in headful Chrome due to headful/webdriver dependencies not being available in the test environment (fell back to static DOM/API testing).
