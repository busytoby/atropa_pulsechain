## 2026-06-15T01:34:12Z

Implement Milestone 2 (Backend API Integration) and Milestone 3 (Frontend Dashboard SPA) for the NoNukes Token Family Dashboard.

## Objective
1. **Backend Integration**:
   - Modify `scripts/dashboard_server.py` to serve:
     - Static routes for the SPA: `/nonukes`, `/nonukes/`, `/nonukes/index.html`, `/nonukes/style.css`, and `/nonukes/app.js` serving `frontend/nonukes_dashboard/index.html`, `frontend/nonukes_dashboard/style.css`, and `frontend/nonukes_dashboard/app.js` respectively.
     - REST API `/api/nonukes/pools`: Return a list of all pools from `nonukes_pools.json` enriched with reserves (loaded from `/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json`), swap counts, and volumes (usd_value) calculated from `resolved_swaps.json`.
     - REST API `/api/nonukes/pool_details?address=<addr>`: Return detailed reserves, chronological price trends (usd_value / minter_amount for each swap), and reverse-chronological swap history for the requested pool address.
   - For both APIs, ensure there is a fallback for dynamic USD valuation if `usd_value` is 0.0 in the swap: lookup the swap's token0 or token1 in `price_cache.json` and multiply the price by amount0 or amount1.

2. **Frontend Dashboard SPA**:
   - Create the directory `frontend/nonukes_dashboard/`.
   - Create `frontend/nonukes_dashboard/index.html` (dashboard page, statistics panel, filter bar, interactive sorted table, detail overlay/modal).
   - Create `frontend/nonukes_dashboard/style.css` (premium dark-mode, glassmorphism styling, HSL gradients, responsive table, modal, charts).
   - Create `frontend/nonukes_dashboard/app.js` (fetching pools data, group filters, searching, interactive sorting, modal opening, querying details, drawing SVG price trend chart).

Use the exact design and implementation details from `/home/mariarahel/src/tsfi2/atropa_pulsechain/.agents/explorer_m2_m3_design/handoff.md`.

## Verification
- Start the server: `python3 scripts/dashboard_server.py`
- Verify static frontend files load correctly under `/nonukes/`.
- Verify APIs `/api/nonukes/pools` and `/api/nonukes/pool_details` return valid JSON matching contracts.
- Verify everything runs without Python exceptions.
