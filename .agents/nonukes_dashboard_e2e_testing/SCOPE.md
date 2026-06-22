# Scope: Milestone 1 (E2E Test Suite Creation)

## Architecture
The E2E testing track designs a comprehensive opaque-box test suite derived from requirements.
- Test runner: Python `unittest` framework executing `tests/e2e/test_nonukes_dashboard.py`.
- Targets:
  - Frontend SPA served statically or loaded locally.
  - Python Backend API server (`scripts/dashboard_server.py`) serving endpoints like `/api/nonukes/pools` and `/api/nonukes/pool_details`.
- Verification method: Simulates browser loads and user interactions using a suitable tool available in the environment (e.g. Selenium/Playwright or lightweight equivalents), or mocks the backend/frontend environment if full browser testing is not possible due to environment restrictions. Note: We must first explore the environment to confirm tool support.

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| 1.1 | Investigation & Environment Audit | Probe environment for browser testing capabilities (Playwright, Selenium, Pyppeteer, or custom web client), checking python dependencies, browser availability, and how the frontend is expected to be served/tested. | none | DONE |
| 1.2 | Test Case Design & Scaffolding | Define feature coverage (Tier 1-4) test cases, input boundaries, combinations, and application scenarios. Create test shell in `tests/e2e/test_nonukes_dashboard.py`. | 1.1 | DONE |
| 1.3 | Test Implementation | Implement all Tier 1-4 tests covering UI loading, interactive table, details modal/panel, and dynamic python server API interaction. Implement mock data setup if needed. | 1.2 | DONE |
| 1.4 | Code Review & Verification | Run the test suite against a simulated/stubbed frontend/backend to ensure tests are correct, robust, and correctly verify requirements. Verify layout is compliant. | 1.3 | DONE |
| 1.5 | Final Publication | Generate and publish `TEST_READY.md` summarizing the test runner commands, coverage, and features verified. Report status to parent. | 1.4 | DONE |

## Interface Contracts
### Frontend Dashboard SPA ↔ Backend API Server
- **Endpoint**: `/api/nonukes/pools`
  - Method: GET
  - Response: `{ "success": true, "pools": [ { "address": "...", "symbol": "...", "name": "...", "target_group": "...", "version": "...", "reserves": { "token0": "...", "token1": "..." }, "swap_count": 0, "volume_usd": 0.0 } ] }`
- **Endpoint**: `/api/nonukes/pool_details?address=<addr>`
  - Method: GET
  - Response: `{ "success": true, "address": "...", "price_trends": [...], "reserves": { "token0": "...", "token1": "..." }, "swaps": [...] }`
