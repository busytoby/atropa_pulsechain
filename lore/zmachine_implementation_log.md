# Z-Machine Implementation Log & Challenges

This log tracks our progress implementing Z6 Sound, Z5 split viewports, ZILCH compilation, and Invisiclues on-chain.

---

## 1. Active Progress & Implementations

*   **Z6 Sound Engine (SID Integration)**: Added the `sound_effect` instruction mapping to our on-chain SID synthesizer (`musicMaker.yul`).
*   **Z5 Graphics Engine (VIC-II Integration)**: Configured window-split hooks linking to `graphicsSystem.yul`.
*   **Token-Gated Invisiclues**: Integrated signature checks via `keySystem.yul`.

---

## 2. Active Challenges & Architectural Questions

> [!WARNING]
> **Dynamic Gas Taxation for Audio Loops**: Loop play commands in Z6 (`sound_effect` repeat loops) can exceed transaction gas ceilings if parsed synchronously.
> *   *Mitigation*: We are implementing a checkpoint system where audio triggers write to storage registers rather than running heavy loop iterations on-chain, allowing the frontend to poll and play audio.

> [!NOTE]
> **ZILCH Compiler Memory Limits**: Processing complete ZIL syntax parsing tables inside browser memory can lead to out-of-memory errors on large game builds.
> *   *Mitigation*: We are streamlining tokenizers to parse text structures sequentially rather than building deep AST trees in memory.

---

## 3. Implementation Verification Checklist

- [x] ERC-20 Dynamic Balance Override (Object Property 31)
- [x] Dynamic Verb Parser (`parseCommand` selector `0x3d02a9e3`)
- [ ] Z6 SID Note Frequency Pitch Mapping
- [ ] Z5 VIC-II Graphic Canvas Plotting
- [ ] Invisiclues Key Decryption
