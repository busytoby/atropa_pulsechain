# E2E Test Strategy & Environment Analysis: NoNukes Dashboard

## Executive Summary
This report details the system capabilities, workspace structure, and proposed E2E testing architecture for the NoNukes PulseX Ecosystem Dashboard. By structuring tests across a 4-tier model (from core functionality to real-world integration), the design ensures robust verification of UI rendering, search/filter logic, detail modals, and python server API integration under normal, boundary, and failure states.

---

## 1. Environment & Browser Capability Audit

### A. Operating System & Browser Capabilities
Our audit of the system package database (`dpkg-selections-dysnomia`) reveals that the underlying Debian-based Linux environment contains the following GUI/browser capabilities:
- **Chromium Browser**: Installed (`chromium`, `chromium-common`, `chromium-sandbox`). This is ideal for Playwright or Selenium tests targeting a WebKit/Blink engine.
- **Firefox Browser**: Installed (`firefox`, `firefox-esr`). This enables cross-browser compatibility checks via Firefox's Gecko engine.
- **XWayland & X11 Utilities**: System configurations indicate X11/XWayland server packages are present; however, E2E browser tests must run in **headless mode** to avoid visual rendering failures or displays timeouts in CLI-only CI environments.

### B. Python E2E Testing Tooling
Since direct runtime execution was limited by permission timeouts during the audit, we recommend a robust, dual-strategy approach for Python test runners:
1. **Primary Strategy (Browser-Driven)**: Utilize **Playwright (Python)** or **Selenium WebDriver**. Playwright is highly recommended due to its modern API, automatic wait states, ease of installation (`playwright install chromium`), and robust support for headless Chromium.
2. **Fallback Strategy (DOM-Parsing)**: If browser binaries cannot be spawned in the container due to sandbox permissions, E2E tests should utilize **BeautifulSoup4** (to parse structure, DOM bindings, and inline CSS/JS scripts) and the **Requests/urllib** libraries to simulate HTTP actions. This mimics the existing testing pattern in `tests/e2e/test_dashboard_container.py` and provides a zero-dependency fallback.

---

## 2. Workspace Layout & Component Inspection

### A. Static Frontend Files
- **`frontend/nonukes_dashboard.html`**:
  - The page serves as a single-page application (SPA) displaying a clean, cyberpunk-themed interface (utilizing Outfit and Share Tech Mono Google Fonts).
  - Main interactive inputs: Search box (`input#search`) and token group filters (`.group-filter`).
  - Key layout components: Stats grid showing unique pool counts, V1/V2 counts, and swap metrics, followed by a data grid listing pools.
  - Critical JS logic: `loadData()` asynchronously fetches `/api/nonukes`, caches results in `allPools`, and dynamically renders rows using vanilla JS DOM manipulation (`renderDashboard()`).
  - **Identified Gap**: Unlike the main Price Oracle Dashboard served at `/`, `frontend/nonukes_dashboard.html` does not currently contain a pool detail modal/panel. We propose adding an LP detail modal triggered by row selection, aligning it with the Price Oracle modal.

### B. JSON Data Sources
- **`nonukes_pools.json`**:
  - Contains structural metadata for all 480 ecosystem pools.
  - Keys are pool addresses. Values are objects mapping: `target_group` ("NoNukes", "EViL", "REARING", etc.), `version` ("V1" or "V2"), `symbol`, `name`, and `other_addr` (paired token address).
- **`nonukes_pulsex_reserves.json`** (saved in scratch space):
  - Contains real-time reserves metadata.
  - Fields per pool: `reserve0`, `reserve1`, `raw_reserve0`, `raw_reserve1`, `total_supply`, and event counts (`swaps`, `mints`, `burns`).

### C. Python Backend Server
- **`scripts/dashboard_server.py`**:
  - Uses standard `http.server.SimpleHTTPRequestHandler` running on port 8080.
  - Routes relevant to NoNukes:
    - `GET /api/nonukes`: Reads `nonukes_pools.json` and the reserves file from scratch space, combining them into a unified response: `{"success": true, "pools": {...}, "reserves": {...}}`.
    - `GET /api/pools?address=<address>`: Uses Web3 to query PulseChain nodes for active V1/V2 reserves of a specific token.
    - Static routing: The server naturally hosts `frontend/nonukes_dashboard.html` from the workspace root folder.

---

## 3. Tiers 1-4 E2E Testing Design Strategy

We propose a 4-tier testing hierarchy implemented in `tests/e2e/test_nonukes_dashboard.py` running under the standard `unittest` framework:

```
+-----------------------------------------------------------+
|               unittest (Test Runner Framework)             |
+-----------------------------+-----------------------------+
                              |
      +-----------------------+-----------------------+
      |                                               |
      v                                               v
+-----------+                                   +-----------+
| Playwright|                                   |  Mock /   |
| /Selenium | (If Headless Browsers Avail)      |  BS4 /    | (Fallback mode)
|   Suite   |                                   |  Requests |
+-----+-----+                                   +-----+-----+
      |                                               |
      +-----------------------+-----------------------+
                              |
                              v
   +-----------------------------------------------------+
   |  Tier 1: Core Smoke Paths                           |
   |  - Page Load / Elements check                       |
   |  - Render all 480 Pools                             |
   +-----------------------------------------------------+
   |  Tier 2: Edge Cases & Failure Modes                 |
   |  - Missing JSON database                            |
   |  - Web3 RPC offline (details query fails)           |
   +-----------------------------------------------------+
   |  Tier 3: Combinatorial & Multi-state                |
   |  - Search query + Group filter combination         |
   |  - Modal open/loader/close cycle                    |
   +-----------------------------------------------------+
   |  Tier 4: System Integration                         |
   |  - Server port binding & static asset serving       |
   |  - Mock Web3 RPC calls (CODE_ONLY network compliance) |
   +-----------------------------------------------------+
```

### Tier 1: Core Feature Verification (Smoke / Positive Paths)
*Goal: Validate that all critical UI components load and render correctly under normal conditions.*
- **Test 1.1: Static Asset Loading**: Verify that opening `frontend/nonukes_dashboard.html` loads all essential DOM nodes (inputs, table headers, stats container).
- **Test 1.2: Server API Validation**: Verify that `GET /api/nonukes` returns a valid JSON payload with 200 OK status containing pools and reserves.
- **Test 1.3: Table Rendering of All Pools**: Assert that the dashboard successfully processes the API response and renders exactly the count of pools present in `nonukes_pools.json` (480 rows).
- **Test 1.4: Stats Cards Aggregation**: Verify that the values inside the cards ("Total Unique Pools", "V1 Pools", "V2 Pools", "Total Pool Swaps") match the mathematical aggregates of the active data.
- **Test 1.5: Target Group Filtering**: Verify that selecting a filter button (e.g. "EViL") correctly filters the rows and updates the aggregate counts dynamically.
- **Test 1.6: Search Bar Filtering**: Input a token symbol or address and assert that only matching rows are displayed.
- **Test 1.7: Detail Modal Trigger**: Assert that clicking on a pool row displays a loading spinner and then pops up a detail overlay with accurate pool properties.

### Tier 2: Boundary Conditions & Failure Modes
*Goal: Ensure UI and server resilience under empty, missing, or corrupted data conditions.*
- **Test 2.1: Missing Database Files**: Remove/rename `nonukes_pools.json` and verify the `/api/nonukes` endpoint returns empty data structures instead of crash loops.
- **Test 2.2: Corrupted JSON Syntax**: Write malformed JSON to the pools configuration and verify that the backend logs the error and serves an empty dataset, while the frontend displays a readable error/empty state.
- **Test 2.3: Zero Reserves & Zero Swaps**: Verify that pools with zero active reserves or zero swap counts do not trigger division-by-zero errors in JavaScript or rendering logic.
- **Test 2.4: Empty Search Results**: Search for a random string and verify that the table renders a clear "No matching pools found" message.
- **Test 2.5: Sanitized Input Checks**: input script tags or SQL patterns into the search field and assert that DOM text insertion prevents script execution (XSS protection).
- **Test 2.6: Web3 RPC Provider Offline**: Simulate an offline connection to the PulseChain node for `/api/pools` and verify that the detail modal displays an "RPC Offline" message rather than hanging indefinitely.

### Tier 3: Combinatorial & Multi-state Interactions
*Goal: Validate the interface's behavior during complex, stateful user paths.*
- **Test 3.1: Combined Search and Filter Query**: Apply a search text AND a target group filter simultaneously, verifying that only rows satisfying BOTH constraints remain visible.
- **Test 3.2: Filter State Transition Sequences**: Simulate a user clicking through all target group filters in rapid succession and check that DOM nodes and stats counters update correctly for each step.
- **Test 3.3: Detail Modal Open/Close Lifecycle**: Verify that clicking a row shows the modal loader -> renders results -> clicking the close button (`&times;` or overlay) properly sets the modal's display state back to hidden.
- **Test 3.4: Dynamic Live Data Reload**: Simulate clicking the "↻ Refresh Data" button. Modify a reserve value in the database, trigger the refresh, and verify that the new value is updated in the UI without a full page reload.

### Tier 4: System Integration & Real-World Scenarios
*Goal: End-to-end integration testing of server subprocesses, ports, and mocked Web3 protocols.*
- **Test 4.1: Server Daemon & Port Allocation**: Launch the `dashboard_server.py` as a subprocess on a dynamically discovered free port (to avoid collisions), confirm port binding, and fetch `/frontend/nonukes_dashboard.html`.
- **Test 4.2: Web3 RPC Mocking (CODE_ONLY Compliance)**: Since external network requests are restricted under CODE_ONLY network mode, mock the `web3` client calls to return mock contract symbols, decimals, and pair coordinates, allowing full `/api/pools` integration testing.
- **Test 4.3: High-Load Performance Verification**: Measure page load times and ensure that rendering all 480 pools does not block the UI thread or cause major memory leaks.
- **Test 4.4: End-to-End Treasury Token Ignore Flow**:
  1. Load the catalog.
  2. Locate a treasury token.
  3. Send a request to `/api/ignore?address=<addr>&ignored=true`.
  4. Verify the database updates.
  5. Refresh the page and assert that the ignored token is now excluded from the live catalog.
- **Test 4.5: Clean Process Teardown**: Verify that the test runner terminates the server subprocess, frees port bindings, and cleans up any temporary price caches.

---

## 4. Proposed Test Runner Architecture

We recommend creating `tests/e2e/test_nonukes_dashboard.py` with the following structure:

```python
import os
import sys
import json
import unittest
import subprocess
import socket
from urllib.request import urlopen, Request

class NoNukesE2ETestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # 1. Locate a free port
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('', 0))
        cls.port = s.getsockname()[1]
        s.close()
        
        # 2. Start dashboard_server.py as a subprocess
        cls.server_proc = subprocess.Popen(
            [sys.executable, "scripts/dashboard_server.py"],
            env=dict(os.environ, PORT=str(cls.port)),
            stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        # Wait briefly for server startup
        cls.base_url = f"http://localhost:{cls.port}"
        
    @classmethod
    def tearDownClass(cls):
        cls.server_proc.terminate()
        cls.server_proc.wait()

    def test_tier1_api_nonukes(self):
        req = Request(f"{self.base_url}/api/nonukes")
        with urlopen(req) as resp:
            data = json.loads(resp.read().decode('utf-8'))
            self.assertTrue(data.get("success"))
            self.assertIn("pools", data)
            self.assertIn("reserves", data)

    # ... Add Playwright-based or BS4-based tests for Tiers 1-4 ...
```
