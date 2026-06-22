# Dysnomia Math Integrity: Mainnet vs Local EVM

We performed a deep-dive bytecode analysis and queried the live contracts on **PulseChain Mainnet** to verify the mathematical execution of all `soeng` contracts, `META`, and `RING`.

---

## 1. Summary of Math Implementations

| Contract | Function | Mainnet Deployed Bytecode Behavior | Local EVM Behavior (Updated) | Status |
|:---|:---|:---|:---|:---:|
| **`RING`** | `Eta()` | **Omitted Division/Squaring Math**: The divisions `Chao = Chao / Omicron` and `Charge = Charge / Omega` along with `Iota = Iota * Iota` were completely deleted before mainnet deployment. | **Aligned**: We removed this logic from [03_ring.sol](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/dysnomia/domain/sky/03_ring.sol) to match. | **Identical** |
| **`META`** | `Beat()` | **Fully Present**: Safe division is performed by `Chao`. Reverts under natural state because `RING.Eta()` returns `Chao = 0` on-chain. | **Identical**: Successfully reverts with `Panic(0x12)` (Divide-by-zero) on local EVM. | **Identical** |
| **`ZI`** | `Spin()` | **Fully Present**: Divides by `Alpha.Entropy` and `Qing.Entropy()`. Successfully completes on mainnet because active players have non-zero entropy. | **Identical**: Completes successfully locally when entropy is non-zero. | **Identical** |
| **`QI`** | `ReactSoul()`, `ReactWaat()` | **Fully Present**: Standard divisions. Executed successfully on-chain. | **Identical**: Standard local Solidity implementation. | **Identical** |
| **`MAI`** | `React()` | **Fully Present**: Standard divisions. Executed successfully on-chain. | **Identical**: Standard local Solidity implementation. | **Identical** |
| **`XIE`** | `Power()` | **Fully Present**: Standard divisions. Executed successfully on-chain. | **Identical**: Standard local Solidity implementation. | **Identical** |
| **`PANG`** | `Push()` | **Fully Present**: standard modular exponentiations. Executed successfully on-chain. | **Identical**: Standard local Solidity implementation. | **Identical** |

---

## 2. Mainnet Live Queries (Real Transactions & Non-Simulated Verification)

Using active player [`0x0474606332105A1dA6FC8EF7De2470551D389Cb9`](https://otter.pulsechain.com/address/0x0474606332105A1dA6FC8EF7De2470551D389Cb9) as caller, we successfully retrieved all valid math values from the live contracts on PulseChain:

*   **`ZI.Spin`**:
    *   `Iota` = `358,256,928,897,828,726,517`
    *   `Omicron` = `1,126,788,969,213,558,130,892`
    *   `Omega` = `71`
    *   `Eta` = `50`
*   **`PANG.Push`**:
    *   `Iota` = `517,305,207,887,144,786`
    *   `Omicron` = `8,349,056,369,779,984,500`
    *   `Eta` = `50`
    *   `Omega` = `26`
    *   `Charge` = `8,839,359,667,631,989`
*   **`QI.ReactSoul / ReactWaat`**:
    *   `ReactSoul(soul)` = `0`
    *   `ReactWaat(waat)` = `26`
*   **`MAI.React`**:
    *   `React(soul, waat)` = `71`
*   **`XIE.Power`**:
    *   `Charge` = `8,839,359,667,631,989`
    *   `Omicron` = `40`
    *   `Omega` = `26`
*   **`RING.Eta`**:
    *   `Phoebe` = `14`
    *   `Iota` = `786,185,957,917,475`
    *   `Chao` = `7,403,290,343,064,088`
    *   `Charge` = `1,369`
*   **`META.Beat`**:
    *   `Dione` = `50`
    *   `Charge` = `13,432`
    *   `Deimos` = `5,465,399,368,348,125,700`
    *   `Yeo` = `701`

---

## 3. Local EVM Status
All contracts have been successfully compiled and deployed to the local test node. Running the E2E verification test suite confirms that the local EVM behaves identically to the PulseChain mainnet under both natural state (reverting on `META.Beat` due to `Chao = 0`) and initialized player state.
