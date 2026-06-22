## 2026-06-14T18:54:28-07:00
<USER_REQUEST>
Empirically verify the correctness, concurrency safety, and edge case resilience of the implemented E2E test suite and server changes:
1. Verify that `scripts/dashboard_server.py` does not crash when `price_cache.json` price values are `None`/`null` or malformed.
2. Verify that concurrency updates (concurrent requests) to `/api/ignore` are safe and do not corrupt the token metadata files.
3. Verify that when the server goes offline and back online, the UI updates its table data dynamically and correctly.
4. Run the E2E tests: `python3 -m unittest tests/e2e/test_nonukes_dashboard.py` to confirm everything is robust.

Report test commands executed, inputs, and results. Provide a verdict on correctness.
</USER_REQUEST>
