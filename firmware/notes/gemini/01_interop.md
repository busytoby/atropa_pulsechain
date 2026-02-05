# Dysnomia / TSFi Interop Integration
**Date:** Wednesday, February 4, 2026
**Status:** Completed

## Objective
Enable the C# `Dysnomia` library to utilize the AVX-512 accelerated math library (`tsfi_math`) from `tsfi2` firmware integration.

## Implementation
1.  **Shared Library (`libtsfi_interop.so`)**:
    - Created `firmware/tsfi_lib/tsfi_interop.c` to wrap `tsfi_math` functions with C-friendly signatures (byte arrays).
    - Created `firmware/tsfi_lib/build_interop.sh` to build the shared library linking `lau_memory` and `tsfi_math`.
    - Implemented Endianness conversion (C# Little-Endian <-> TSFi Big-Endian).

2.  **Dysnomia Integration (`Dysnomia/Lib/Math.cs`)**:
    - Added `[DllImport]` for `Interop_ModPow`.
    - Updated `ModPow` to attempt calling the native library.
    - Added fallback mechanism to legacy `BigInteger.ModPow` if the library is missing or fails.
    - Updated `Dysnomia.csproj` to include `libtsfi_interop.so` in the build output.

3.  **Solidity Compilation**:
    - Created `solidity/compile_linux.sh` to compile all contracts in `dysnomia/` (excluding interfaces and old code) using the local `solc` (0.8.21).
    - Verified successful compilation of all active contracts.

## Verification
- **Test Harness**: Created `tests/InteropTest` console application.
- **Test Case**: `123456789^65537 % 1000000007` -> `560583526`.
- **Result**: PASSED. Confirmed `Dysnomia.Math.ModPow` returns correct results using the interop path.

## Artifacts
- `firmware/tsfi_lib/libtsfi_interop.so` (deployed to Dysnomia/bin)
- `tests/InteropTest` (source code)
- `solidity/compile_linux.sh`
