# Keys Of Ong: Cryptographic Constant-Entropy Prediction Game

* **Contract Address**: [`0x9CCc2de565da893AB1300674F174545A3E568F93`](https://scan.pulsechain.com/address/0x9CCc2de565da893AB1300674F174545A3E568F93)
* **Token Identifier**: `dynamic_0x9ccc2de565da893ab1300674f174545a3e568f93`
* **Game Token Name**: `Keys Of Ong`
* **Token Symbol**: `ARCADIA`
* **Decimals**: `18`
* **Linked Arithmetic Engine**: [`dynamic_0xb680f0cc810317933f234f67eb6a9e923407f05d`](https://scan.pulsechain.com/address/0xB680F0cc810317933F234f67EB6A9E923407f05D) (`libAtropaMath v1.1`)
* **Primary Function Selector**: `0xbce3821f(uint256)`

---

## Executive Overview: The Ong Interdimensional Prediction Matrix

`Keys Of Ong` is an on-chain cryptographic prediction game deployed on PulseChain. Inspired by the legendary ARG mythos of Ong's Hat and interdimensional gateway keys, the game functions as an empirical difficulty-testing harness for constant-entropy pseudo-random number generator (RNG) architectures in the Dysnomia VM ecosystem.

```
+-----------------------------------------------------------------------------------+
|                        Keys Of Ong Gameplay Architecture                          |
+-----------------------------------------------------------------------------------+
|  1. Underlying Engine: Deterministic modpow modular exponentiation sequence       |
|  2. Player Input: Submits predicted RNG state via 0xbce3821f(uint256)             |
|  3. Mathlib Verification: Evaluates guess against active round state in mathlib   |
|  4. Reward Emission: Mints exactly 1.0 ARCADIA on verified state match            |
|  5. Empirical Entropy Audit: Benchmarks resistance against predictive solvers     |
+-----------------------------------------------------------------------------------+
```

---

## Core Game Mechanics

1. **Deterministic `modpow` Pseudo-Random Generator**:
   * The underlying RNG relies on pure modular exponentiation (`modpow`) over fixed Dysnomia system prime fields.
   * While fully deterministic in pure mathematics, calculating the active round output ahead of time under variable transaction ordering requires anticipating modular trajectories.
2. **Player Interaction & Submission Loop**:
   * A player submits their candidate `uint256` prediction number as the sole argument to function selector `0xbce3821f`.
   * The transaction executes within a fixed gas budget profile (~155,443 gas), evaluating the submitted parameter against the active random state produced by [`dynamic_0xb680f0cc810317933f234f67eb6a9e923407f05d`](https://scan.pulsechain.com/address/0xB680F0cc810317933F234f67EB6A9E923407f05D) (`libAtropaMath v1.1`).
3. **Reward Invariant (1.0 ARCADIA / Win)**:
   * When the submitted prediction matches the RNG output, the contract triggers an ERC-20 `Transfer(0x0, player, 1000000000000000000)` event.
   * Exactly `1.0` ARCADIA token (`10^{18}` base units) is minted and awarded directly to the winning player's wallet address.

---

## Historical Win Records & Most Recent Token Win

On-chain transaction logs record continuous public gameplay interactions and successful prediction mints:

```
[Latest Game Win Audit Record]
+-----------------------------------------------------------------------------------+
| Transaction Hash : 0xd9082aba66cf820232d384ce444061e24b2cf4b7ce474b76d985e196ebfa1013
| Block Number     : 26,125,925                                                     |
| Timestamp (UTC)  : 2026-03-27 02:45:35 UTC                                        |
| Winning Player   : 0x142133Ba22D2480d6B76073Fe0eC0ea1c70a6BC0                     |
| Submitted Guess  : 0x1a869e14b229b (466,634,809,769,627)                          |
| Gas Consumed     : 155,455 gas                                                    |
| Reward Emitted   : 1.000000000000000000 ARCADIA                                   |
+-----------------------------------------------------------------------------------+
```

* **Simultaneous Math & RNG Inflow**: Each win simultaneously mints `1.0` [`dynamic_0xb680f0cc810317933f234f67eb6a9e923407f05d`](https://scan.pulsechain.com/address/0xB680F0cc810317933F234f67EB6A9E923407f05D) (`MATH`) and `1.0` [`dynamic_0xa96bcbed7f01de6ceed14fc86d90f21a36de2143`](https://scan.pulsechain.com/address/0xa96BcbeD7F01de6CEEd14fC86d90F21a36dE2143) (`RNG`), maintaining synchronization across the entropy network.

---

## System Role in Dysnomia VM & Empirical Entropy Auditing

1. **Benchmarking Constant-Entropy Resilience**:
   * The game serves as a live, adversarial test harness.
   * By incentivizing external actors to predict on-chain random sequences with real token emissions, it empirically benchmarks the computational cost and resistance of constant-entropy modular state generators against brute-force or analytical prediction techniques.
2. **Pastoral & Mythic Arcadia**:
   * The symbol `ARCADIA` embodies the classical harmonious realm, unlocked when a traveler finds the correct mathematical key to traverse the interdimensional threshold.

---

## Technical & On-Chain Audit Profile

* **Deployer Address**: [`0xBF182955401aF3f2f7e244cb31184E93E74a2501`](https://scan.pulsechain.com/address/0xBF182955401aF3f2f7e244cb31184E93E74a2501)
* **Deployment Transaction**: [`0xd3efe3bc95c1635210ac5b16e0b6d480358fdf7bce56219381f5ab5650d99fc5`](https://scan.pulsechain.com/tx/0xd3efe3bc95c1635210ac5b16e0b6d480358fdf7bce56219381f5ab5650d99fc5) (Block `19,401,967`, `2024-02-10 17:15:28 UTC`)
* **Total Supply**: `7,717.999999999999999969` ARCADIA
* **Treasury Holding**: [`0xBF182955401aF3f2f7e244cb31184E93E74a2501`](https://scan.pulsechain.com/address/0xBF182955401aF3f2f7e244cb31184E93E74a2501) holds `0.308524793814153095` ARCADIA (`0.00399747076722%` custody)
* **Token Audit Documentation**: [`lore/tokens/0x9CCc2de565da893AB1300674F174545A3E568F93.md`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/lore/tokens/0x9CCc2de565da893AB1300674F174545A3E568F93.md)
