## Challenge Summary

**Overall risk assessment**: MEDIUM

## Challenges

### [Medium] Challenge 1: Type Confusion Crash on nonukes_pools.json
- **Assumption challenged**: The server assumes that `nonukes_pools.json` is either missing, corrupted, or parses into a dictionary.
- **Attack scenario**: If `nonukes_pools.json` is modified/overwritten with an empty list `[]` or a null value `null`, `json.load()` successfully returns `[]` or `None`. Since no exception is raised, `pools_data` holds a non-dict type. When `/api/nonukes/pools` or `/api/nonukes/pool_details` is queried, the code attempts to call `pools_data.items()`. This raises `AttributeError: 'list' object has no attribute 'items'` or `'NoneType' object has no attribute 'items'`, causing the request to crash with an HTTP 500 internal server error.
- **Blast radius**: Completely disables both the pools overview page `/api/nonukes/pools` and the pool details page `/api/nonukes/pool_details`.
- **Mitigation**: Add a type guard check `if not isinstance(pools_data, dict): pools_data = {}` immediately after the try-except blocks that load `pools_data` in `/api/nonukes/pools` and `/api/nonukes/pool_details`.

### [Medium] Challenge 2: Type Confusion Crash on Reserves JSON
- **Assumption challenged**: The server assumes `nonukes_pulsex_reserves.json` (or the reserves file at `RESERVES_FILE_PATH`) is either missing, corrupted, or parses into a dictionary.
- **Attack scenario**: If the reserves file contains an empty list `[]` or a null value `null`, `json.load()` returns `[]` or `None`. When `/api/nonukes/pools` or `/api/nonukes/pool_details` is queried, the code attempts to call `reserves_data.get(...)`. This raises `AttributeError: 'list' object has no attribute 'get'` or `'NoneType' object has no attribute 'get'`, causing the request to crash with an HTTP 500 internal server error.
- **Blast radius**: Disables the pools overview page and the pool details page.
- **Mitigation**: Add a type guard check `if not isinstance(reserves_data, dict): reserves_data = {}` immediately after the try-except blocks that load `reserves_data` in `/api/nonukes/pools` and `/api/nonukes/pool_details`.

### [Medium] Challenge 3: Type Confusion Crash on resolved_swaps.json
- **Assumption challenged**: The server assumes `resolved_swaps.json` is either missing, corrupted, or parses into a list.
- **Attack scenario**: If `resolved_swaps.json` contains a dictionary `{}` or a null value `null`, `json.load()` returns `{}` or `None`. When `/api/nonukes/pools` or `/api/nonukes/pool_details` is queried, the code attempts to iterate over the parsed structure: `for swap in resolved_swaps:`. This raises `TypeError: 'NoneType' object is not iterable` (for `None`) or `AttributeError: 'str' object has no attribute 'get'` (for `{}` since iterating yields key strings).
- **Blast radius**: Disables the pools overview page and the pool details page.
- **Mitigation**: Add a type guard check `if not isinstance(resolved_swaps, list): resolved_swaps = []` immediately after the try-except blocks that load `resolved_swaps` in `/api/nonukes/pools` and `/api/nonukes/pool_details`.

## Stress Test Results

- **Price cache is `null`** → Server gracefully handles it via type guard and fallback to default pricing → HTTP 200 success → **PASS**
- **Price cache is `[]`** → Server gracefully handles it via type guard and fallback to default pricing → HTTP 200 success → **PASS**
- **Price cache price value is `null`** → Server returns `0.0` and continues → HTTP 200 success → **PASS**
- **Price cache price value is malformed** → Server returns `0.0` and continues → HTTP 200 success → **PASS**
- **Concurrent ignore/unignore updates and data reads** → Serialized via `IGNORE_LOCK`, protecting against file corruption and race conditions → HTTP 200 success with complete metadata → **PASS**
- **Server offline / online UI recovery** → Frontend UI displays empty table on error, automatically fetches and refreshes when server recovers → Correct dynamic updates → **PASS**
- **`nonukes_pools.json` is `null`** → Server fails with `AttributeError` on `.items()` → HTTP 500 internal server error → **FAIL**
- **Reserves JSON is `null`** → Server fails with `AttributeError` on `.get()` → HTTP 500 internal server error → **FAIL**
- **`resolved_swaps.json` is `null`** → Server fails with `TypeError: 'NoneType' object is not iterable` → HTTP 500 internal server error → **FAIL**

## Unchallenged Areas

- **PulseX contract integrations via Web3** — Live contract queries on `/api/pools` require a live RPC connection. Since testing is run offline, the actual external blockchain interactions could not be mocked or tested locally.
