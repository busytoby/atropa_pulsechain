# Adversarial Review Report: Dashboard Server Hardening

## Challenge Summary

**Overall risk assessment**: CRITICAL

The NoNukes Token Family Dashboard server (`scripts/dashboard_server.py`) exhibits several critical architectural vulnerability vectors:
1. **Unhandled Input Crashes (HTTP 500 / traceback exposure)**: In `/api/pools`, input addresses are parsed directly with `Web3.to_checksum_address` outside the try-catch block. A malformed address (odd length, non-hex, missing prefix, or extremely long string) results in an unhandled `ValueError` or `TypeError`, leaking raw Python stack trace to stderr and returning raw HTTP 500 error pages.
2. **Type Confusion Uncaught Exceptions**: The server lacks type guards when reading and parsing JSON configuration and data files. If any of the following files contain a valid JSON format of an unexpected type (e.g., list `[]` instead of dict `{}` or a `null` literal):
   - `nonukes_pools.json`
   - `nonukes_pulsex_reserves.json` (pointed to by `RESERVES_FILE_PATH`)
   - `resolved_swaps.json`
   - `treasury_tokens_unknown_minter.json`
   The server will crash dynamically with uncaught `AttributeError` or `TypeError` tracebacks inside request handler methods like `/api/nonukes/pools`, `/api/nonukes/pool_details`, or `/api/ignore`. This makes it highly vulnerable to Denial of Service (DoS) if these database files are corrupted or manipulated.

---

## Challenges

### [High] Challenge 1: Unhandled Address Checksumming in `/api/pools`
- **Assumption challenged**: User-supplied address parameters passed to `Web3.to_checksum_address` are always valid hex-encoded, 40-character strings with a `0x` prefix.
- **Attack scenario**: Sending a GET request to `/api/pools?address=0x1` (too short), `/api/pools?address=0xinvalidaddress` (non-hex chars), or `/api/pools?address=174A0ad...` (missing prefix).
- **Blast radius**: The exception propagates outside `do_GET` causing the SimpleHTTPRequestHandler to crash the active request, log a full stack trace to console stderr, and return a raw 500 Internal Server Error page to the client.
- **Mitigation**: Wrap `Web3.to_checksum_address(address)` in a try-except block, or sanitize the address parameter using regex pattern `^0x[0-9a-fA-F]{40}$` (like done in `/api/ignore`) and return `400 Bad Request` if it fails validation.

### [Critical] Challenge 2: Type Confusion Vulnerability in `nonukes_pools.json`
- **Assumption challenged**: `nonukes_pools.json` is always parsed into a dictionary.
- **Attack scenario**: A corrupted or empty array `[]` is written to `nonukes_pools.json`. A client requests `/api/nonukes/pools`.
- **Blast radius**: `json.load()` successfully returns a list `[]`. The server executes `pools_data.items()` on a list, raising `AttributeError: 'list' object has no attribute 'items'`, causing the request handler to crash and return 500 with stack trace.
- **Mitigation**: Add a type guard immediately after parsing:
  ```python
  if not isinstance(pools_data, dict):
      pools_data = {}
  ```

### [Critical] Challenge 3: Type Confusion Vulnerability in `nonukes_pulsex_reserves.json`
- **Assumption challenged**: `nonukes_pulsex_reserves.json` is always parsed into a dictionary.
- **Attack scenario**: A corrupted or empty array `[]` is written to the reserves file. A client requests `/api/nonukes/pools` or `/api/nonukes/pool_details`.
- **Blast radius**: `reserves_data` evaluates to a list `[]`. The server calls `reserves_data.get(...)`, raising `AttributeError: 'list' object has no attribute 'get'` and throwing a 500 traceback crash.
- **Mitigation**: Add type guard:
  ```python
  if not isinstance(reserves_data, dict):
      reserves_data = {}
  ```

### [Critical] Challenge 4: Type Confusion Vulnerability in `resolved_swaps.json`
- **Assumption challenged**: `resolved_swaps.json` is always parsed into a list.
- **Attack scenario**: `resolved_swaps.json` is corrupted to contain a JSON object `{"key": "value"}` or a null value `null`.
- **Blast radius**: 
  - If `null`: `resolved_swaps` is `None`. Looping `for swap in resolved_swaps:` raises `TypeError: 'NoneType' object is not iterable`.
  - If dict: `for swap in resolved_swaps:` loops over keys (strings). Calling `swap.get("pool_address", "")` raises `AttributeError: 'str' object has no attribute 'get'`.
  In both cases, `/api/nonukes/pools` and `/api/nonukes/pool_details` crash with HTTP 500 tracebacks.
- **Mitigation**: Add type guards:
  ```python
  if not isinstance(resolved_swaps, list):
      resolved_swaps = []
  ```

### [High] Challenge 5: Type Confusion Vulnerability in `treasury_tokens_unknown_minter.json`
- **Assumption challenged**: `treasury_tokens_unknown_minter.json` is always parsed into a dictionary.
- **Attack scenario**: The file is corrupted to contain `[]`. A client ignores an address not found in other minter files, triggering the fallback write block in `/api/ignore`.
- **Blast radius**: The server tries to set `tokens[address] = { ... }`. Since `tokens` is a list, it raises `TypeError: list indices must be integers or slices, not str`, throwing a 500 traceback crash.
- **Mitigation**: Add type guard:
  ```python
  if not isinstance(tokens, dict):
      tokens = {}
  ```

---

## Stress Test Results (Simulated/Predicted based on logic trace)

Below is the verification trace matrix of the 11 tests from `tests/e2e/test_nonukes_adversarial.py`:

| Test Name | Attack Scenario | Expected Behavior | Actual Behavior | Result |
|-----------|-----------------|-------------------|-----------------|--------|
| `test_01_api_pools_malformed_address_odd_length` | `/api/pools?address=0x1` | HTTP 400 (Graceful Reject) | HTTP 500 with unhandled `ValueError` traceback | **FAIL** (Bugs Detected) |
| `test_02_api_pools_malformed_address_missing_prefix` | `/api/pools?address=174A0ad...` | HTTP 400 (Graceful Reject) | HTTP 500 with unhandled `ValueError` traceback | **FAIL** (Bugs Detected) |
| `test_03_api_pools_malformed_address_non_hex_chars` | `/api/pools?address=0x...g` | HTTP 400 (Graceful Reject) | HTTP 500 with unhandled `ValueError` traceback | **FAIL** (Bugs Detected) |
| `test_04_api_pool_details_extremely_long_address` | `/api/nonukes/pool_details?address=0x` + 100k "a" | HTTP 400 (Graceful Reject) | HTTP 400 (Graceful Reject) | **PASS** |
| `test_05_api_pools_extremely_long_address` | `/api/pools?address=0x` + 100k "a" | HTTP 400 (Graceful Reject) | HTTP 500 with unhandled `ValueError` traceback | **FAIL** (Bugs Detected) |
| `test_06_corrupt_pools_json_type_confusion` | `nonukes_pools.json` = `[]` | HTTP 500/400 (Graceful Reject) | HTTP 500 with unhandled `AttributeError` traceback | **FAIL** (Bugs Detected) |
| `test_07_corrupt_reserves_json_type_confusion` | `nonukes_pulsex_reserves.json` = `[]` | HTTP 500/400 (Graceful Reject) | HTTP 500 with unhandled `AttributeError` traceback | **FAIL** (Bugs Detected) |
| `test_08_corrupt_resolved_swaps_json_null` | `resolved_swaps.json` = `null` | HTTP 500/400 (Graceful Reject) | HTTP 500 with unhandled `TypeError` traceback | **FAIL** (Bugs Detected) |
| `test_09_corrupt_resolved_swaps_json_type_confusion_dict` | `resolved_swaps.json` = `{"key": "value"}` | HTTP 500/400 (Graceful Reject) | HTTP 500 with unhandled `AttributeError` traceback | **FAIL** (Bugs Detected) |
| `test_10_api_ignore_corrupt_unknown_minter_json` | `treasury_tokens_unknown_minter.json` = `[]` | HTTP 500/400 (Graceful Reject) | HTTP 500 with unhandled `TypeError` traceback | **FAIL** (Bugs Detected) |
| `test_11_concurrent_requests_stress` | Concurrent requests under load | HTTP 200 (Success) | HTTP 200 (Success) | **PASS** |

---

## Unchallenged Areas
- **Daemon connection reliability**: We did not challenge contract RPC connectivity failures or timeouts.
- **Frontend client behavior on 500**: We did not test browser rendering behavior when server endpoints return tracebacks.
