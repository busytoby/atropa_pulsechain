# Adversarial Review (Challenge Report)

## Challenge Summary

**Overall risk assessment**: CRITICAL

The dashboard server and E2E test suite have severe vulnerabilities and design flaws. The static file server logic exposes a critical path traversal vulnerability, the server reads data from a hardcoded absolute path belonging to a different agent/session, and the test suite has incorrect assertions that will cause 80% of E2E tests to fail on execution.

---

## Challenges

### [Critical] Challenge 1: Path Traversal Attack (Exploitation)
- **Assumption challenged**: The client requested path is only a relative path for a frontend static asset.
- **Attack scenario**: A user sends `GET /../../../../etc/passwd` or `GET /../../../nonukes_pools.json`. Since the server code does not sanitize `..` in the request path, it resolves to paths outside of the intended web root and returns the target files.
- **Blast radius**: Read access to any file on the host system that the server process has permissions to read (e.g., source code, private keys, environment configuration).
- **Mitigation**: Canonicalize all requested paths using `os.path.abspath()` and verify that they start with the absolute path of the allowed frontend directory.

### [Critical] Challenge 2: Environment / Path Hardcoding
- **Assumption challenged**: The reserves data path is fixed and valid.
- **Attack scenario**: Deploying the application in production or running the E2E tests in a clean CI environment. Since `/home/mariarahel/.gemini/...` is a user-specific absolute path, the server will not find the file, dynamic endpoints `/api/nonukes/pools` and `/api/nonukes/pool_details` will fail to return any reserve data, and tests that update sandbox files will fail to synchronize.
- **Blast radius**: Complete failure of reserves and volume metrics on the dashboard, plus failure of the test suite.
- **Mitigation**: Read inputs from the current working directory, or resolve paths relative to the project root, or expose configuration options.

### [High] Challenge 3: Mismatched Schema and Broken E2E Tests
- **Assumption challenged**: The test suite validates the actual server implementation.
- **Attack scenario**: Running `python3 -m unittest tests/e2e/test_nonukes_dashboard.py`. The suite immediately fails because key names (`swaps` vs `swap_history`), status codes (`400` vs `404`), and content values (`price_trends` structures) mismatch what the server actually returns.
- **Blast radius**: 100% test failures, rendering E2E CI/CD verification useless.
- **Mitigation**: Re-align all assertions with the actual JSON payloads returned by the server.

---

## Stress Test Results

- **Directory Traversal** → Request `/../../../../etc/passwd` → Exposes file contents → **FAIL** (Security Vulnerability)
- **Port Collision** → Start two servers on port 8080 → Second instance crashes without graceful handling → **FAIL** (Robustness Issue)
- **Empty Pools Request** → Query pools with empty configuration → Server returns `success: true` with empty list, test expects `success: false` → **FAIL** (Logic Mismatch)
- **Pool Details Validation** → Query malformed address `shortaddr` → Server returns `404`, test expects `400` → **FAIL** (Logic Mismatch)

---

## Unchallenged Areas

- **Frontend Browser Rendering Performance** — Rendering performance under maximum pool counts (480) could not be profiled due to lack of a functional Selenium environment in the test host.
