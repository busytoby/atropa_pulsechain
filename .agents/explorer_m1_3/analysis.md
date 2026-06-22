# E2E Test Strategy & Environment Probe Report
**Author**: explorer_m1_3 (E2E Testing Explorer)
**Date**: 2026-06-15T01:45:00Z

---

## 1. Executive Summary

This report outlines findings from a comprehensive environment probe and codebase inspection of the NoNukes PulseX Ecosystem Dashboard project. 

Our investigation confirmed:
1. **System Browsers**: Headless Chromium (`chromium`) and Firefox (`firefox-esr`) are installed at the system level (`dpkg-selections-dysnomia`). No system-level web drivers (`chromedriver` or `geckodriver`) are package-installed, though they can be managed dynamically.
2. **Python Environment**: The standard testing framework is `unittest`. Python `requests` is available. No project-wide dependencies force Selenium or Playwright, but custom test files (`tests/e2e/test_nonukes_dashboard.py`) implement a dual-driver architecture: utilizing Selenium if available, and gracefully falling back to a custom `SimpleDOMParser` (based on standard `html.parser.HTMLParser`) and API validation when browser drivers are absent.
3. **Workspace Layout**: The workspace contains a single-file dashboard draft (`frontend/nonukes_dashboard.html`) and a multi-file SPA structure (`frontend/nonukes_dashboard/index.html`, `style.css`, `app.js`). The multi-file SPA implements the `#detail-modal` and price trend SVG rendering.
4. **Backend Server**: The `scripts/dashboard_server.py` has been updated by parallel/previous agent actions to route `/nonukes/` files and expose `/api/nonukes/pools` (aggregating data from `nonukes_pools.json`, reserves, and `resolved_swaps.json`) and `/api/nonukes/pool_details?address=<addr>` (returning reserve details, swap history, and price trend points).

Based on these findings, we recommend a **Dual-Driver E2E Testing Strategy** structured across Tiers 1-4 to guarantee test runner resilience and robust feature verification.

---

## 2. Environment & System Tools Audit

### 2.1. Browser Capabilities
By inspecting the system package file `/home/mariarahel/src/tsfi2/atropa_pulsechain/dpkg-selections-dysnomia`, we verified the presence of the following graphical/browser packages:
* **Chromium**: Installed via:
  * `chromium` (line 63)
  * `chromium-common` (line 64)
  * `chromium-sandbox` (line 65)
* **Firefox**: Installed via:
  * `firefox` (line 137)
  * `firefox-esr` (line 138)
* **Web Drivers**: Neither `chromedriver` nor `geckodriver` are installed as system packages. E2E test scripts must either manage webdriver installation dynamically (via package managers or third-party managers) or fallback to static HTML validation.

### 2.2. Python Testing Packages
* **Standard Test Framework**: `unittest` is the default runner.
* **Network/API Testing**: `requests` is present in the environment (imported in deepseek tools).
* **Browser Automation**: `selenium` is imported conditionally in `tests/e2e/test_nonukes_dashboard.py`. If absent, tests do not crash; they adapt dynamically.
* **HTML Parsing Fallback**: Rather than failing when Selenium is unavailable, tests parse DOM structures statically using Python's standard `html.parser.HTMLParser`.

---

## 3. Workspace Layout Inspection

### 3.1. Frontend Codebase
There are two frontend entry points:
1. **Draft Template (`frontend/nonukes_dashboard.html`)**:
   * A single-file layout containing simple structure, styling, and basic script.
   * Hits `/api/nonukes` to fetch and render list of pools.
   * **Limitations**: Does not contain any detail modal or charts.
2. **SPA Directory (`frontend/nonukes_dashboard/`)**:
   * `index.html`: Contains the core layout, stats card containers, filter chip containers, pool data table, and a dedicated `#detail-modal` overlay containing reserve displays, a swap history table, and an SVG `#chart-container`.
   * `style.css`: A comprehensive glassmorphism theme stylesheet.
   * `app.js`: Connects filters (search + chips), sorts pool tables, triggers `#detail-modal` via row click handler (`openPoolDetails`), queries `/api/nonukes/pool_details?address=<addr>`, and dynamically renders an inline SVG chart (`drawChart`) based on historical price trend points.

### 3.2. Data Files
* **Pool Database (`nonukes_pools.json`)**: Contains 480 unique pools mapping token addresses to version (V1/V2), symbol, name, and target group (`NoNukes`, `EViL`, `REARING`, `ᠵᠡᠮ`, `ELIZABETHNESIA`, `WAR`, `OG`).
* **Reserves Cache (`nonukes_pulsex_reserves.json`)**: Stored in gemini scratch space `/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json`. Contains reserve levels, total supply, and event counts (swaps, mints, burns) per pool.
* **Swap/Price Cache (`resolved_swaps.json`, `unresolved_swaps.json`, `price_cache.json`)**: Used by the backend to calculate USD volume and price metrics.

### 3.3. Server Routing (`scripts/dashboard_server.py`)
The HTTP server has been configured to serve the SPA static routes and APIs:
* **Static Assets**: Routes `/nonukes`, `/nonukes/`, `/nonukes/index.html`, `/nonukes/style.css`, and `/nonukes/app.js` point directly to the files in `frontend/nonukes_dashboard`.
* **API Endpoints**:
  * `/api/nonukes/pools`: Parses `nonukes_pools.json`, reads dynamic reserves, reads `resolved_swaps.json` to calculate dynamic swap counts and USD volume, and returns a unified JSON list of all 480 pools.
  * `/api/nonukes/pool_details?address=<address>`: Validates pool address, loads reserves, and returns dynamic price trend logs, current reserve values, and detailed swap history.

---

## 4. Tiers 1-4 E2E Testing Design Strategy

To verify the NoNukes Dashboard under all system states, we recommend organizing the E2E tests into four distinct Tiers. Tests must implement the dual-driver architecture (Selenium/Chromium browser-automation with a robust DOM-parser fallback) to operate in restricted or headless containers.

### Tier 1: Feature Verification (Positive Paths)
* **SPA Page Load & DOM Validation**: Open `/nonukes/index.html`. Assert HTTP 200 and verify that all key containers (`#main-layout`, `#pool-search`, `#group-filters`, `#pool-table`, stats cards) are present.
* **Backend API Validation**: Directly query `/api/nonukes/pools`. Verify the structure is `success: true` and contains a list of pools with reserves, swap counts, and volume fields.
* **Table Rendering (480 Pools)**: Wait for table rows to load. Assert that exactly 480 table rows are rendered.
* **Stats Cards Verification**: Assert that "Total Unique Pools", "Active Groups", "Total Volume (USD)", and "Total Swap Count" render mathematically correct aggregates.
* **Filter Chips Rendering**: Verify that filter buttons are rendered for all groups (NoNukes, EViL, REARING, etc.) plus an "ALL" option.
* **Row Click & Detail Modal Trigger**: Simulate a click on the first pool row. Verify that the `#detail-modal` receives the `.active` class, the loader spinner is shown, and the modal title updates with token symbols.
* **Detail Modal Reserves Loading**: Verify the modal correctly lists dynamic reserve values matching the target pool.

### Tier 2: Boundary Conditions & Failure Modes
* **Invalid/Malformed Address**: Query `/api/nonukes/pool_details?address=invalid_addr` or empty strings. Verify the server returns HTTP 400 Bad Request with a clear JSON error, avoiding python server exceptions.
* **Missing JSON Database**: Rename `nonukes_pools.json` and request `/api/nonukes/pools`. Verify the API returns `success: false` and empty lists cleanly instead of throwing 500 Server Errors.
* **Corrupted JSON Files**: Write malformed JSON to `price_cache.json` or `resolved_swaps.json`. Verify the server handles JSON decode errors gracefully and falls back to default values.
* **Empty Search/Filter Result**: Input a long randomized search string (e.g. `xyz123abc`). Assert that the UI renders a clean "No pools matching current filters" message in the table body.
* **XSS Search Protection**: Input `<script>alert('xss')</script>` into the search box. Verify the input is handled as text and does not execute script logs.
* **RPC Connection Failure**: Simulate Web3 node offline status. Verify the detail modal reports "Failed fetching pool reserves" or loader timeout gracefully.

### Tier 3: Combinatorial & Multi-state Interactions
* **Combined Filters (Text + Chip)**: Set search input to a specific symbol (e.g. `INCOGNITO`) and select a filter chip (e.g. `NoNukes`). Verify that only rows matching BOTH parameters render.
* **Rapid Filter Swapping**: Rapidly click between different group filter chips. Verify the DOM renders the correct subset of pools and updates stats cards instantly without race conditions.
* **Detail Modal Lifecycle**: Open a pool's detail modal, wait for content load, click the close button, and verify the modal visibility class is removed, returning the user to the unchanged parent page.
* **Dynamic Telemetry Sync**: Update `nonukes_pulsex_reserves.json` or `resolved_swaps.json` in the background with new swap events. Trigger dynamic refresh (via "↻ Refresh" button). Assert that the specific pool row update is rendered immediately without page reload.

### Tier 4: System Integration & Real-World Scenarios
* **Subprocess Daemon Allocation**: Spin up the server daemon `scripts/dashboard_server.py` dynamically on a free port (avoiding collisions) and test standard routing.
* **Dynamic Port Collisions**: Start a second daemon instance on the same port. Assert it fails cleanly or binds to a dynamic backup port.
* **Web3 RPC Mocking (CODE_ONLY Compliance)**: Mock Web3 HTTP provider calls to avoid making outbound blockchain queries, satisfying CODE_ONLY network constraints.
* **Performance Benchmark**: Load all 480 pools. Verify that DOM rendering executes within 1 second, and no memory leaks are triggered by rapid table sorting.
* **Process Teardown**: Shutdown the subprocess server and clean up any temporary sandbox folders.

---

## 5. Verification Method

### 5.1. Test Command
To verify the E2E tests, execute the test script directly from the workspace root:
```bash
python3 -m unittest tests/e2e/test_nonukes_dashboard.py
python3 -m unittest tests/e2e/test_nonukes_dashboard_spa.py
```

### 5.2. Output Invalidation Conditions
The test results are considered invalid if:
* The backend server processes crash or fail to release socket bindings on port teardown.
* Outbound HTTP calls are made to live PulseChain RPC nodes during tests (violates CODE_ONLY networking).
* Tests fail to run in environments lacking Selenium/Chrome installation (the fallback DOM-parser must work).
