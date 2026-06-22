# Blueprint: Combat Rewards Architecture

This document defines how combat rewards are resolved, calculated, and distributed on-chain within the Dysnomia TCG and Z-machine engine.

---

## 1. The React/Beat On-Chain Reward Loop

Rewards are minted and distributed directly during contract execution. Both the domain and asset contracts utilize **`_mintToCap()`** to manage token issuance during play:

```
[ Combat Action (React / Beat) ]
               |
               v
  Calculates RF Resonance (SWR, RL)
               |
               v
     Executes _mintToCap()
               |
               v
Sends Yield directly to Player Room (Alpha.On.Phi)
```

1.  **Reaction Yields**: Calling `YUE.React(Qing)` triggers `XIE.Power(Waat)`. The computed values `Omicron` (Epibar gain) and `Omega` (Hypobar gain) are directly proportional to the ERC-20 yields distributed.
2.  **Inflation Control**: Every reward check executes `_mintToCap()` inside the domain token contracts, ensuring that game actions never push the token supply above its maximum hard-cap limit ($maxSupply$).

---

## 2. Dynamic Reward Scaling (RF Resonance Multipliers)

To reward skilled gameplay, token distribution is scaled dynamically based on the player's **RF resonance metrics**:

$$\text{Reward Multiplier} = \text{Power Transfer Efficiency} \times \text{Return Loss Scale}$$

*   **Vigour Reward Scaling**: High power transfer efficiency ($\eta = 1 - |\Gamma|^2 \approx 1.0$) applies a $1.0\times$ multiplier to the base reward. If the player's SWR is high (poor tuning), the multiplier decays towards $0$.
*   **Terror Resistance Penalty**: If SWR exceeds $3.0$, the player's transaction suffers a tax or penalty because cognitive noise blocks the clean minting of rewards.
*   **Supernatural Sight Bonus**: A high Return Loss ($>20\text{ dB}$) boosts the player's chance of uncovering rare item drops from the Z-machine loot tables.

---

## 3. Z-Machine Hook Integration (Property Rewards)

When a combat encounter is settled within a text adventure story (Z-machine game session), rewards are resolved using the **`put_prop`** interceptor:

1.  **Objective Completion**: The game code updates a monster's state to "defeated" or sets a chest property:
    `put_prop Chest Opened 1`
2.  **Opcode Interception**: The Yule interpreter (`zmachine.yul`) catches the property modification on-chain.
3.  **Token Push**: If the modified property matches a registered reward contract, the interpreter executes `transfer` to push the designated reward (e.g. `YUE` or `Eris` tokens) from the game contract pool directly to the player's room.

---

## 4. CHOA Chat Yield Mechanics

During live multiplayer interactions, players communicate and trigger reactions via `CHOA.Chat(Qing, MSG)`. This executes:

1.  **Reaction Execution**: Triggers `Chan.ReactYue(Yue, Qing)` which accumulates `Epibar` and `Hypobar` on the `YUE` contract.
2.  **Token Minting**: Triggers `_mintToCap()` to balance supply.
3.  **Dynamic Payout (Mai)**:
    *   Queries `Sei.Chan().Xie().Xia().Mai().React(UserToken.Saat(1), _qing.Waat())`.
    *   Caps the calculated payout `Mai` at `1.0 YUE/CHOA` tokens (`1 * 10 ** decimals()`).
    *   Transfers the calculated `Mai` amount from the `CHOA` treasury address directly to the player's `YUE` pool address.

