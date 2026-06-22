# Synthesis of Explorer Findings - NoNukes Dashboard E2E Testing Strategy

## 1. Consensus on Environment & Capabilities
- **System Browsers**: Chromium is installed on the system (and listed under `dpkg-selections-dysnomia` as installed). Firefox is also listed in some selections but Chromium is the primary focus.
- **Python Libraries**: Standard `unittest` framework and `urllib` / `json` are available. Python `requests` is present. Selenium and other automated browser tools may trigger environment timeouts or sandbox restrictions.
- **Dual-Driver Architecture**: To ensure robust execution in all environments, the E2E test suite must utilize a dual-driver pattern:
  - **Selenium Webdriver (Primary)**: Headless Chromium.
  - **Hybrid API + Static DOM Validation (Fallback)**: If `selenium` imports fail or browser setup fails, tests query endpoints directly via python `requests`/`urllib` and inspect the SPA HTML/JS layout structure using Python's built-in libraries.

## 2. Test Scope & Target Endpoints
- **Test File Location**: `tests/e2e/test_nonukes_dashboard.py`.
- **Target Backend Server**: `scripts/dashboard_server.py`.
- **Target Frontend Layout**:
  - Verify draft file `frontend/nonukes_dashboard.html` and the future structured SPA files (`frontend/nonukes_dashboard/index.html`).
- **Target Backend Endpoints**:
  - `/api/nonukes/pools`: Serve/verify all 480 pools with dynamic reserves, swap counts, and USD volume.
  - `/api/nonukes/pool_details?address=<addr>`: Serve/verify price trends, reserve balances, and swap history.
- **Mock Data**: Set up sandbox copies/mocks of `nonukes_pools.json`, `price_cache.json`, and `resolved_swaps.json` to prevent mutation of workspace data and guarantee hermetic tests.

## 3. Test Cases (Tiers 1-4)
- **Tier 1 (Feature Verification)**:
  1. Frontend page loads successfully (HTTP 200, checks script and link elements).
  2. Table renders exactly 480 pools from JSON.
  3. Clicking a pool row triggers detail view modal/panel (#lp-modal).
  4. Backend API `/api/nonukes/pools` returns success and pools array.
  5. Backend API `/api/nonukes/pool_details?address=<addr>` returns reserves/swaps structure.
- **Tier 2 (Boundary & Failure)**:
  6. Server handles empty or missing pools JSON gracefully (returns success: false, pools: []).
  7. Server handles malformed/corrupted JSON price cache gracefully.
  8. API handles invalid/malformed/empty pool addresses with HTTP 400.
  9. Port collision/busy port exception handling during server start.
  10. Filter input fields sanitize and handle extremely long/invalid strings gracefully.
- **Tier 3 (Cross-Feature Combinations)**:
  11. Search query combined with child token group filter renders correct subset of pools.
  12. UI loads cleanly and displays server connection error when backend is offline.
  13. Selected filter group and search query persist after clicking refresh.
  14. Detail modal displays error message if RPC fails during detail fetch, keeping the rest of the page active.
- **Tier 4 (Real-World Workflows)**:
  15. Complete User Journey: starts server, loads index.html, verifies pool counts, filters by group, opens modal, verifies detail info, closes modal.
  16. Telemetry Sync Workflow: appends new swap event to JSON, triggers refresh, verifies pool table updates swap count and volume in real-time.
  17. Filter state preservation through modal interaction.
  18. Intercept/simulate RPC rates or transient failures to verify loading spinner timeouts gracefully.
  19. Directory traversal security scrubbing on `/api/nonukes/pool_details?address=../../etc/passwd`.

## 4. Implementation Plan
- The Worker will write the test runner class `TestNoNukesDashboard` inside `tests/e2e/test_nonukes_dashboard.py`.
- Subprocess management: Test suite startup spins up the backend server on a dynamic port and tears it down on test teardown.
- Sandbox workspace isolation: Symlinks/copies data files to a temporary sandbox directory for test run isolation.
