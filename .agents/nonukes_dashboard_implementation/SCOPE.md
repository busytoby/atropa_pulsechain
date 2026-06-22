# Scope: NoNukes Dashboard Implementation

## Architecture
The NoNukes dashboard implementation spans three core milestones:
1. **Backend API Integration (Milestone 2)**: Extending `scripts/dashboard_server.py` to serve `/api/nonukes/pools` and `/api/nonukes/pool_details` by reading `nonukes_pools.json` and integrating with `price_cache.json` and `resolved_swaps.json`.
2. **Frontend Dashboard SPA (Milestone 3)**: Creating index.html, style.css, and app.js inside `frontend/nonukes_dashboard/` with premium styling (glassmorphism, dark-mode, HSL gradients) and interactive pool table, filters, and charts.
3. **Final Integration & Audit (Milestone 4)**: Verify implementation against E2E test track (Milestone 1 output: `TEST_READY.md`), apply white-box coverage checks, build adversarial tests (Tier 5), and obtain clean Forensic Auditor attestation.

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| 2 | Backend API Integration | Update `scripts/dashboard_server.py` to serve `/api/nonukes/pools` and `/api/nonukes/pool_details` with real-time reserves, prices, swap counts, and volumes. | none | DONE |
| 3 | Frontend Dashboard SPA | Create frontend files in `frontend/nonukes_dashboard/` integrating with Backend API, adding filters and premium styling. | M2 | DONE |
| 4 | Final Integration & Audit | Pass E2E tests (Tiers 1-4), perform Tier 5 hardening, and run Forensic Auditor. | M2, M3 | DONE |

## Interface Contracts
### Frontend SPA ↔ Backend API
- **Endpoint**: `/api/nonukes/pools`
  - Method: GET
  - Response format:
    ```json
    {
      "success": true,
      "pools": [
        {
          "address": "0x...",
          "symbol": "$INCOGNITO",
          "name": "$INCOGNITO",
          "target_group": "NoNukes",
          "version": "V1",
          "reserves": {
            "token0": 1000.0,
            "token1": 250000.0
          },
          "swap_count": 12,
          "volume_usd": 1500.50
        }
      ]
    }
    ```
- **Endpoint**: `/api/nonukes/pool_details?address=<addr>`
  - Method: GET
  - Response format:
    ```json
    {
      "success": true,
      "address": "0x...",
      "price_trends": [
        {"timestamp": 1781441813, "price": 1.25}
      ],
      "reserves": {
        "token0": 1000.0,
        "token1": 250000.0,
        "token0_symbol": "$INCOGNITO",
        "token1_symbol": "WPLS"
      },
      "swap_history": [
        {
          "tx_hash": "0x...",
          "timestamp": 1781441813,
          "amount0": 10.0,
          "amount1": 2500.0,
          "usd_value": 12.50
        }
      ]
    }
    ```

## Code Layout
- `frontend/nonukes_dashboard/index.html` - Premium UI SPA markup
- `frontend/nonukes_dashboard/style.css` - Glassmorphism, dark-mode CSS styles
- `frontend/nonukes_dashboard/app.js` - SPA JavaScript app logic
- `scripts/dashboard_server.py` - Extended Python backend server API
