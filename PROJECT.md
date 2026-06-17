# Project: NoNukes Token Family Dashboard

## Architecture
The NoNukes dashboard is a responsive, premium single-page application (SPA) integrated with a Python-based backend API server.
1. **Frontend Dashboard SPA**: Consists of HTML, CSS, and modern JavaScript files served statically. It provides:
   - Dynamic listing/table of all 480 pools from `nonukes_pools.json`.
   - Filters to highlight pools associated with the 7 child NoNukes tokens.
   - Real-time details panel displaying reserves and telemetry charts.
2. **Backend API Server**: Extends the existing `scripts/dashboard_server.py` to:
   - Load `nonukes_pools.json` on startup.
   - Serve a pool list API: `/api/nonukes/pools` containing pool addresses, symbols, names, target groups, reserve depths, and swap counts.
   - Serve a telemetry API: `/api/nonukes/telemetry` or integrated endpoints returning swap history, volume, and prices.
   - Fetch real-time price and swap telemetry by cross-referencing `price_cache.json` and `resolved_swaps.json`.
3. **E2E Testing Track**: Independent requirement-driven test suite validating UI loading, API connectivity, interactive table data correctness, and modal details.

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| 1 | E2E Test Suite Creation | Create comprehensive opaque-box test suite (Tiers 1-4) verifying frontend loading, table rendering of all 480 pools, detail view functionality, and dynamic API usage. Publish `TEST_READY.md`. | none | PLANNED |
| 2 | Backend API Integration | Update `scripts/dashboard_server.py` to serve `/api/nonukes/pools` and other required telemetry endpoints, loading from `nonukes_pools.json` and cross-referencing with `price_cache.json` and `resolved_swaps.json`. | none | PLANNED |
| 3 | Frontend Dashboard SPA | Create frontend files under `frontend/nonukes_dashboard` (index.html, style.css, app.js) implementing a premium dark-mode aesthetic with HSL gradients and glassmorphism. | M2 | PLANNED |
| 4 | Final Integration & Audit | Validate all frontend features against E2E test suite (Tiers 1-4). Conduct Tier 5 white-box coverage hardening, generate adversarial tests, and verify using the Forensic Integrity Auditor. | M1, M2, M3 | PLANNED |

## Interface Contracts
### Frontend SPA ↔ Backend API
- **Endpoint**: `/api/nonukes/pools`
  - Method: GET
  - Response format: JSON object containing:
    - `success`: boolean
    - `pools`: array of objects, each with:
      - `address`: string (pool contract address)
      - `symbol`: string (pool symbol)
      - `name`: string (pool name)
      - `target_group`: string ("NoNukes" or child token group)
      - `version`: string ("V1" or "V2")
      - `reserves`: object with `token0` and `token1` balances
      - `swap_count`: integer (total swaps in `resolved_swaps.json` for this pool)
      - `volume_usd`: float (total USD volume in `resolved_swaps.json` for this pool)
- **Endpoint**: `/api/nonukes/pool_details?address=<addr>`
  - Method: GET
  - Response format: JSON object containing real-time price trends, reserve balances, and swap history for the specified pool.

## Code Layout
- `frontend/nonukes_dashboard/index.html` - Dashboard HTML entry point
- `frontend/nonukes_dashboard/style.css` - Dashboard styles (dark-mode glassmorphic theme)
- `frontend/nonukes_dashboard/app.js` - Dashboard interactive application logic
- `scripts/dashboard_server.py` - Extended Python backend server
- `tests/e2e/test_nonukes_dashboard.py` - E2E test suite verifying the dashboard requirements
