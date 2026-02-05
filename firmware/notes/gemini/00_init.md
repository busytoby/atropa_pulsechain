# TSFi Integration Notes

## Objective
Integrate `tsfi2` hardened memory allocator (`lau_memory`) and AVX-512 math library (`tsfi_math`) into `atropa_pulsechain` firmware components.

## Targets
- `gemalloc.h`: Replace custom slab allocator with `lau_memory`.
- `mathint.h`: Replace scalar big integer math with `tsfi_math` (AVX-512).

## Verification
A test harness `test_mathint.c` exists in the firmware root. We will use `build_test.sh` to verify correctness and performance of the integrated solution.

### Results (2026-02-04)
**Baseline (Legacy):**
- Result: `0` (INCORRECT) - The scalar `modPow` implementation failed for the test case `123456789^65537 % 1000000007`.

**Integrated (TSFi):**
- Result: `560583526` (CORRECT) - The AVX-512 accelerated implementation produced the correct result.
- Memory Audit: `lau_memory` successfully tracked allocations with zero leaks reported during the test.

## Conclusion
The `tsfi_math` library not only provides AVX-512 acceleration but also fixes a functional correctness bug in the legacy big integer arithmetic. Integration is recommended for all AVX-512 capable hosts.
