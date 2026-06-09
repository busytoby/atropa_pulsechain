# Advanced Infocom Technologies Integration Blueprint

We will integrate all four advanced Infocom-inspired systems directly with our on-chain Z-machine ecosystem.

---

## 1. Rationale and Motivation

*   **Gas Efficiency & Data Overhead**: Packing game layouts into 5-bit ZSCII representations reduces dynamic storage writes, allowing complex narrative worlds to reside on-chain at a fraction of the cost of standard contract architectures.
*   **Fully Decentralized Game States**: Emulating the Z-machine on-chain guarantees that game universes are permissionless, moddable, and resistant to server shutdowns.
*   **Asset Interoperability**: Mapping ERC-20 balances directly to the virtual Object Table links standard DeFi assets dynamically to gameplay mechanics.

---

## 2. TSFi2 Core Component Compatibility

This Z-machine interpreter leverages the standard TSFi2 peripheral architecture:

| Infocom Subsystem | Target Version | Compatible TSFi2 Contract / Tech | Interaction Channel |
| :--- | :--- | :--- | :--- |
| **SID Synth Audio** | Z6 (YZIP) | [musicMaker.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/musicMaker.yul) | Intercepts `sound_effect` opcodes; triggers note frequencies on voice registers. |
| **VIC-II Visuals** | Z5 (XZIP) | [graphicsSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/graphicsSystem.yul) | Intercepts window-split/draw calls; maps sprites directly to the canvas frame. |
| **ZILCH Compiler** | Z3/Z5 | Browser Compiler Engine | Compiles textual definitions directly to on-chain ROM storage. |
| **Invisiclues Hints** | Z3/Z5 | On-Chain Key-Value Registry | Interacts with [keySystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/keySystem.yul) to verify user signatures and release hint payloads. |

---

## 3. Integration Architecture

### Z6 Sound Engine (SID Synthesis)
When a Z6 story file triggers `sound_effect ID volume`:
1.  The interpreter maps the sound ID to specific ADSR envelope parameters.
2.  It executes a low-level call to the TSFi2 `MusicMaker` contract (`musicMaker.yul`).
3.  The SID registers generate the audio output dynamically.

### Z5 Graphics Viewport Mapping
When a Z5 file requests room rendering:
1.  The interpreter parses the room description offset.
2.  If the room has a graphic icon property, the interpreter extracts its compressed bitmap coordinates.
3.  It calls `graphicsSystem.yul` to draw the bitmap on-chain.

### Token-Gated Invisiclues
The player queries hints on-chain:
1.  The contract verifies payment using standard ERC-20 `transferFrom` calls.
2.  Once verified, the contract decrypts the progress stage text using the player's public address key via [keySystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/keySystem.yul).
