## 2026-06-15T01:41:46Z

Implement bug fixes, security hardening, and test alignment for the NoNukes Token Family Dashboard (Milestones 2 & 3).

## Objective
Apply the following specific changes to `scripts/dashboard_server.py` and `frontend/nonukes_dashboard/app.js` based on the reviewers' findings:

1. **Fix Directory Traversal Vulnerability**:
   - In `scripts/dashboard_server.py` (around lines 561-578), ensure that the static file-serving fallback resolves path names using `os.path.abspath` and checks that the resolved absolute path starts with the server's root directory (or project root directory: `/home/mariarahel/src/tsfi2/atropa_pulsechain`).
   - For example:
     ```python
     clean_path = self.path.split('?')[0].lstrip('/')
     abs_root = os.path.abspath('.')
     abs_clean = os.path.abspath(clean_path)
     if clean_path and abs_clean.startswith(abs_root) and os.path.exists(abs_clean) and os.path.isfile(abs_clean):
     ```

2. **Fix Legacy Test Failures in API Endpoints**:
   - In `/api/nonukes/pool_details` endpoint:
     - Return the swaps array under the key `"swaps"` in addition to `"swap_history"` to support both old and new contracts.
     - Return `[0.0, 0.0, 0.0]` for the `"price_trends"` array if parsing `price_cache.json` fails/is corrupt. Use a try/except around the JSON load of `price_cache.json` and track if it failed (e.g. `price_cache_corrupt = True`). If corrupt, override `price_trends = [0.0, 0.0, 0.0]`.
     - Return HTTP 400 (Bad Request) instead of HTTP 404 if the `address` parameter is missing, malformed, or if `address_lower` is not found in the pools config database. This will align with `test_08_invalid_pool_addresses_400` and `test_19_security_path_traversal_scrubbing` which expect HTTP 400.

3. **XSS Mitigation in Frontend SPA**:
   - In `frontend/nonukes_dashboard/app.js`, escape dynamic data like `p.symbol` and `p.address` when rendering them inside innerHTML templates. For example, implement a helper:
     ```javascript
     function escapeHtml(str) {
         if (!str) return '';
         return str.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;").replace(/'/g, "&#039;");
     }
     ```
     Use it on all user/config-supplied text fields (like symbol, name, address, etc.) that are rendered into the DOM.

## Verification
- Run the legacy test suite:
  `python3 -m unittest tests/e2e/test_nonukes_dashboard.py`
- Run the new SPA test suite:
  `python3 -m unittest tests/e2e/test_nonukes_dashboard_spa.py`
- Verify that both test suites pass 100% without errors.
