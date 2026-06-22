## Challenge Summary

**Overall risk assessment**: HIGH

## Challenges

### [High] Challenge 1: Price Cache Type Confusion Crash
- **Assumption challenged**: The server assumes `price_cache.json` either contains a valid JSON dictionary or fails parsing (raising an exception caught by `except Exception`).
- **Attack scenario**: If `price_cache.json` is overwritten with valid JSON representing a list (`[]`) or a null value (`null`), `json.load()` successfully returns `[]` or `None`. Since no exception is raised, `prices` holds a non-dict type. When `/api/nonukes/pool_details` is queried for a pool paired with a base asset (e.g., USDC or WPLS), the `get_symbol` helper function attempts to query `prices.get(addr, {})`. This raises `AttributeError: 'list' object has no attribute 'get'` or `'NoneType' object has no attribute 'get'`, crashing the thread and returning an HTTP 500 error.
- **Blast radius**: Disables the pool details pane for any pool paired with base assets, resulting in a broken dashboard page.
- **Mitigation**: Add type guards in `/api/nonukes/pool_details` and `/api/data` to ensure `prices` is always a dictionary, e.g.:
  ```python
  if not isinstance(prices, dict):
      prices = {}
  ```

### [Medium] Challenge 2: Unlocked Read/Write Concurrency Collision on Treasury Metadata
- **Assumption challenged**: Wrapping `/api/ignore` write operations with `IGNORE_LOCK` is sufficient for complete concurrency safety.
- **Attack scenario**: While `/api/ignore` updates metadata under `IGNORE_LOCK`, the GET `/api/data` endpoint reads `treasury_tokens_*.json` files concurrently *without* acquiring `IGNORE_LOCK`. If a GET request occurs while the ignore endpoint is truncating and rewriting a metadata file, the GET reader will read an empty or partial file, throwing `json.JSONDecodeError`. This error is caught, causing the server to return incomplete metadata to the frontend.
- **Blast radius**: Transient frontend display inconsistencies where treasury tokens randomly disappear or fail to load.
- **Mitigation**: Wrap the `treasury_tokens_*.json` read block in `/api/data` with `with IGNORE_LOCK:` to ensure reads and writes are mutually exclusive.

## Stress Test Results

- **Price cache is `null`** → Server does not crash but `/api/nonukes/pool_details` requests fail with HTTP 500 → `AttributeError: 'NoneType' object has no attribute 'get'` → **FAIL**
- **Price cache is `[]`** → Server does not crash but `/api/nonukes/pool_details` requests fail with HTTP 500 → `AttributeError: 'list' object has no attribute 'get'` → **FAIL**
- **Price cache price value is `null`** → Server gracefully returns `0.0` for prices and continues → Graceful fallback → **PASS**
- **Price cache price value is malformed** → Server gracefully returns `0.0` for prices and continues → Graceful fallback → **PASS**
- **Concurrent updates to `/api/ignore`** → The write operations themselves are serialized via `IGNORE_LOCK`, preventing file corruption. However, concurrent reads on `/api/data` suffer from race conditions → Intermittent empty data returned to `/api/data` → **PARTIAL PASS**
- **Server offline / online dynamic UI recovery** → Frontend catches fetch error, renders empty table, and recovers automatically via `setInterval` poll once server goes back online → UI updates table data dynamically and correctly → **PASS**

## Unchallenged Areas

- **V3 PulseX pair contracts queries** — The server queries PulseX V1/V2 pairs but does not mock web3 network connectivity, so blockchain calls are out of scope for offline unit testing.
