# E2E Testing Design Strategy & Environment Probe Report
**E2E Testing Explorer (explorer_m1_2)**
*Date: 2026-06-15T01:31:00Z*

---

## 1. Executive Summary
This report presents the findings of the environment probe and workspace layout audit, and recommends a design strategy for Tiers 1-4 E2E testing of the NoNukes Dashboard.

Our probe confirmed that:
- **Chromium** is installed on the system, making headless browser execution viable.
- **Requests** is available in the Python environment, allowing direct API contract testing.
- No direct references to `selenium` or `playwright` exist in the codebase, and execution permissions timed out, indicating a risk of environment restrictions.
- To guarantee robust verification, we propose a **Dual-Driver E2E Test Strategy**:
  - **Primary**: Headless Selenium Webdriver using the local Chromium installation.
  - **Fallback**: Hybrid API Integration & Static DOM Parser (using `requests` and Python's standard `html.parser` and `json` libraries) to verify the SPA structure and server contracts without browser dependencies.

---

## 2. Environment and System Tools Audit

### 2.1. Browser Capabilities
We inspected `/home/mariarahel/src/tsfi2/atropa_pulsechain/dpkg-selections-dysnomia` to verify installed system-level browsers:
- **Chromium**: Installed (lines 63-65):
  - `chromium` (install)
  - `chromium-common` (install)
  - `chromium-sandbox` (install)
- **Firefox / Geckodriver**: Not installed.
- **Chromedriver**: Chromium-based E2E tests are supported using the installed Chromium package.

### 2.2. Python testing packages
By searching the codebase and import files, we identified:
- **Requests**: Present and extensively used in `tsfi2-deepseek/tools/` (e.g. `fetch_flow_history.py` and `probe_trpc.py`).
- **Unittest**: Part of the Python standard library, used in existing E2E tests (`tests/e2e/test_dashboard_container.py` and `run_e2e_tests.py`).
- **Selenium / Playwright / Pyppeteer**: No current imports. Because OS-level command execution timed out on permission prompts, we assume that direct pip installations might be restricted. 

---

## 3. Workspace Layout & Codebase Analysis

### 3.1. Data Layer: `nonukes_pools.json`
- **Location**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/nonukes_pools.json` (122 KB).
- **Contents**: 480 unique pool records.
- **Format**:
  ```json
  "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": {
      "target_group": "NoNukes",
      "version": "V1",
      "symbol": "$INCOGNITO",
      "name": "$INCOGNITO",
      "other_addr": "0x9b3b6b8ff7434e9ec2b6d3b032b98152ccf4d266"
  }
  ```
- **Child Token Groups**: We identified 7 child token groups in the JSON mapping: `NoNukes`, `EViL`, `REARING`, `ᠵᠡᠮ`, `ELIZABETHNESIA`, `WAR`, and `OG`.

### 3.2. Backend API: `scripts/dashboard_server.py`
- **Current Routes**:
  - `/api/data`: Returns token prices, resolved swaps, unresolved swaps, and treasury tokens.
  - `/api/nonukes`: Serves the static list of pools from `nonukes_pools.json` and reserves from a hardcoded cache file.
  - `/api/pools?address=<addr>`: Uses `web3` to query PulseChain RPC.
- **Discrepancy / Required Updates**: `PROJECT.md` contracts require the backend server to serve:
  - `/api/nonukes/pools`: Returns all 480 pools with dynamic reserves, swap counts, and USD volume.
  - `/api/nonukes/pool_details?address=<addr>`: Returns real-time price trends, reserve balances, and swap history.
  - These must be implemented in Milestone 2.

### 3.3. Frontend Dashboard SPA
- **Location**: `frontend/nonukes_dashboard.html` currently contains a draft template that fetches `/api/nonukes` and displays tables.
- **Required Layout**: According to `PROJECT.md`, the final files must be organized under `frontend/nonukes_dashboard/` as:
  - `index.html` (SPA entry point)
  - `style.css` (Glassmorphism theme)
  - `app.js` (JavaScript logic)
- **E2E Target**: E2E tests must verify both the draft HTML (`frontend/nonukes_dashboard.html`) and the future structured SPA files (`frontend/nonukes_dashboard/index.html`).

---

## 4. E2E Test Suite Design Strategy (Tiers 1-4)
The test suite will be located at `tests/e2e/test_nonukes_dashboard.py` and run via `python3 -m unittest tests/e2e/test_nonukes_dashboard.py`.

### 4.1. Dual-Driver Architecture
To ensure execution success in all environments, the test suite should implement a fallback pattern:
```python
try:
    from selenium import webdriver
    from selenium.webdriver.chrome.options import Options
    SELENIUM_AVAILABLE = True
except ImportError:
    SELENIUM_AVAILABLE = False
```
- **If Selenium is available**: Run tests in headless Chromium.
- **If Selenium is missing**: Fall back to **Hybrid API + Static DOM Validation** (inspect HTML structure, tags, JS files, and query API endpoints directly).

---

### 4.2. Feature Test Matrix (Tiers 1-4)

#### Tier 1: Feature Verification
*Objective: Verify that all standard components load and operate under normal conditions.*
1. **Frontend Loading**: Verify `index.html` serves HTTP 200, parses correctly, and references CSS and JS files.
2. **Backend API Connectivity**: Call `/api/nonukes/pools`, assert standard JSON response format, `success: True`, and verify the root array contains pool objects.
3. **Table Rendering (480 Pools)**: 
   - *Selenium*: Wait for table rows, assert that exactly 480 rows are rendered.
   - *Hybrid*: Read `nonukes_pools.json`, verify HTML contains container `#pool-list-container` and JavaScript maps pool objects.
4. **Modal/Details Trigger**: 
   - *Selenium*: Click a pool row, assert that modal `#lp-modal` gets class `.active`.
   - *Hybrid*: Assert DOM contains `#lp-modal`, close buttons, and details containers.
5. **Pool Detail API**: Call `/api/nonukes/pool_details?address=<addr>` for a valid address and verify it returns reserves and swap history contracts.

#### Tier 2: Boundary & Failure Modes
*Objective: Verify robustness under data anomalies and invalid inputs.*
6. **Invalid/Malformed Address**: Request `/api/nonukes/pool_details?address=invalid_addr` or `?address=0x000` and assert that the server returns HTTP 400 Bad Request instead of throwing a 500 error.
7. **Missing Data Files**: Run the server with `nonukes_pools.json` renamed/removed, verify `/api/nonukes/pools` returns a clean `{ "success": false, "pools": [] }` without crashing the server.
8. **Corrupted Price Cache**: Write malformed JSON into `price_cache.json`, verify server handles parsing errors gracefully.
9. **Filter Bound Queries**: input extremely long strings, empty strings, and SQL injection payloads into the search filter, verifying table displays "No matching pools" instead of breaking DOM layout.
10. **Port Collision**: Start the server on a busy port, verify it throws clean exceptions and supports dynamic port binding.

#### Tier 3: Pairwise & Combination Tests
*Objective: Verify interaction of multiple filters and system states.*
11. **Text Search + Group Filter**: Search for symbol `INCOGNITO` while the "EViL" group filter is active. Verify that only matching pools in the specific group are rendered.
12. **Server Outage + UI Reload**: Start frontend with the API server stopped, verify UI displays a clear "Failed to load pool configuration" error alert.
13. **Active Filter + Refresh**: Select a filter group and click the "Refresh Data" button. Verify the active filter persists after data reload.
14. **RPC Failure during Detail Query**: Simulate RPC connection failure when calling `/api/nonukes/pool_details`, verify modal displays "Failed fetching pool reserves" message while keeping the rest of the page active.

#### Tier 4: Real-World Scenarios
*Objective: Simulate complete end-to-end user workflows.*
15. **Scenario 1 (Full User Journey)**: 
    - Server starts, browser opens `index.html`.
    - Verify stats cards display counts matching `nonukes_pools.json`.
    - Apply filter "NoNukes", verify row count decreases.
    - Click the first pool row, modal opens, details (reserves, volume) load from API.
    - Close modal, verify layout returns to original state.
16. **Scenario 2 (Telemetry Sync)**:
    - Simulate a new swap event in `resolved_swaps.json`.
    - Click "Refresh Data" on the frontend.
    - Verify that the target pool row displays the updated swap count and volume in real-time.
17. **Scenario 3 (Filter State Restoration)**:
    - Set search query to `INCOGNITO`, select group "NoNukes", click a row to view modal.
    - Close modal, verify search and group filters are still active.
    - Click "All Pools" and verify search result count restores to the full list.
18. **Scenario 4 (RPC Rate Limiting/Transient Errors)**:
    - Intercept RPC requests to return empty data.
    - Click pool details row, verify loader spinner appears, times out cleanly, and reports error without page freeze.
19. **Scenario 5 (Security Scrubbing)**:
    - Attempt directory traversal on `/api/nonukes/pool_details?address=../../etc/passwd`.
    - Verify server returns 400 Bad Request, verifying strict address sanitization is active.

---

## 5. Implementation Roadmap
1. **Scaffolding (`tests/e2e/test_nonukes_dashboard.py`)**: Create the test class with `unittest` and support for both Selenium and Hybrid execution.
2. **Subprocess Management**: Implement context managers to safely launch `scripts/dashboard_server.py` on a random free port and shut it down cleanly after tests run.
3. **Mock Data Utilities**: Include helper functions to write temporary mock `nonukes_pools.json` and cache files to avoid mutating production data.
4. **Layout Compliance Check**: Ensure tests specifically check `frontend/nonukes_dashboard/index.html` as the main SPA entry point once implemented.
