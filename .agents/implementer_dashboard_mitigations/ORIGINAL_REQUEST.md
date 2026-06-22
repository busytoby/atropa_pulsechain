## 2026-06-15T01:58:53Z

Apply the following mitigations to `/home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/dashboard_server.py`:

1. **Type guard for prices**:
   Whenever `price_cache.json` is loaded (in `/api/data` around line 83, in `/api/nonukes/pools` around line 155, and in `/api/nonukes/pool_details` around line 231), add:
   `if not isinstance(prices, dict): prices = {}`
   to ensure that `prices` is guaranteed to be a dictionary, even if `price_cache.json` contains `null` or a JSON array `[]`. This prevents `AttributeError` during price/symbol lookup in the detail view.

2. **Lock reads in /api/data**:
   In the `/api/data` endpoint, wrap the file-reading of `treasury_tokens_*.json` (around lines 107-114) under the `IGNORE_LOCK`:
   ```python
   with IGNORE_LOCK:
       for fpath in glob.glob("treasury_tokens_*.json"):
           ...
   ```
   This ensures that concurrent reads and writes of the treasury token metadata files are mutually exclusive, preventing race conditions.

Verification:
- Run the verification script: `python3 scratch/verify_all.py`
- Verify that both the price cache tests and the concurrency tests now report clean passes (no crashes, no corruption, 100/100 success).
- Run the unit tests: `python3 -m unittest tests/e2e/test_nonukes_dashboard.py` to ensure everything remains fully correct.
- Include the output and logs of these commands in your handoff report.
