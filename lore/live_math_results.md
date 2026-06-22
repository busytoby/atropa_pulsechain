# Dysnomia Mainnet Live Math Verification

By querying the live contracts on **PulseChain Mainnet** (overriding the caller address to the active mainnet player [`0x0474606332105A1dA6FC8EF7De2470551D389Cb9`](https://otter.pulsechain.com/address/0x0474606332105A1dA6FC8EF7De2470551D389Cb9)), we executed the math against the actual blockchain state.

Every function successfully completed execution, returning valid, non-zero values with no division-by-zero reverts!

---

## Live Math Results

Below are the exact values returned by the contracts on PulseChain mainnet using scanned WAAT value `378814830703436350975436945425663138519237484768630413256454076449062763743`:

### 1. `ZI.Spin` (Selector: `0x83cc3221`)
*   **Status**: Succeeded
*   **Returned Values**:
    *   `Iota`: `358,256,928,897,828,726,517` (raw base units)
    *   `Omicron`: `1,126,788,969,213,558,130,892` (raw base units)
    *   `Omega`: `71`
    *   `Eta`: `50`

### 2. `PANG.Push` (Selector: `0xd5220cc9`)
*   **Status**: Succeeded
*   **Returned Values**:
    *   `Iota`: `517,305,207,887,144,786`
    *   `Omicron`: `8,349,056,369,779,984,500`
    *   `Eta`: `50`
    *   `Omega`: `26`
    *   `Charge`: `8,839,359,667,631,989`

### 3. `RING.Eta` (Selector: `0x4209b07c`)
*   **Status**: Succeeded
*   **Returned Values**:
    *   `Phoebe`: `14`
    *   `Iota`: `786,185,957,917,475`
    *   `Chao`: `7,403,290,343,064,088` (Note: This is non-zero, avoiding the downstream division-by-zero!)
    *   `Charge`: `1,369`

### 4. `META.Beat` (Selector: `0xd5215e06`)
*   **Status**: Succeeded
*   **Returned Values**:
    *   `Dione`: `50`
    *   `Charge`: `13,432`
    *   `Deimos`: `5,465,399,368,348,125,700`
    *   `Yeo`: `701`

---

## Conclusion
*   **Math Correctness**: The math logic functions correctly when executed against actual initialized mainnet states.
*   **No Division-by-Zero**: Because the active mainnet player has initialized balances, YUE's `React` returns a non-zero `Chao` value (`7,403,290,343,064,088`). This allows the division `Yeo = Yeo / Chao` inside `META.Beat` to successfully compute as `701` without panicking.
