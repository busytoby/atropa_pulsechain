# Blueprint: Flexible TCG Game Rules Framework

This document outlines a modular rules system for the NoNukes TCG. By utilizing the dynamic parameters produced by the on-chain contracts (`ZI`, `PANG`, `RING`, `META`), we can establish game rules that evolve automatically based on the mathematical state of the battlefield.

---

## 1. Global Rule Mutators (Scaled by Entropy)

Instead of static game modes, the rules of a match are governed by the aggregate entropy ($E_{total}$) of the active Qings in play:

### A. Phase State: High Entropy Mode ($E_{total} \ge 10,000$)
*   **Theme**: *Chaos & Volatility*.
*   **Rule Changes**:
    *   **Unstable Math**: Damage values are calculated with a random modifier: $\text{ATK}_{actual} = \text{ATK} \pm (\text{Random} \pmod{\text{Entropy}})$.
    *   **Panic Dampening**: Division by zero does *not* trigger card destruction; it instead swaps the card's active ATK and DEF values.

### B. Phase State: Zero-Point Mode ($E_{total} < 2,000$)
*   **Theme**: *Precision & Determinism*.
*   **Rule Changes**:
    *   **Absolute Math**: All attacks deal exact, non-random damage.
    *   **Singularity Reverts**: Any division-by-zero panic calculated during a combat phase immediately "reverts" the card, destroying it and dealing damage to its owner. Players must carefully budget their entropy output.

---

## 2. Dynamic Field Rules (Attached to Active Qings)

Since all cards have attached **Qings**, the presence of specific cards on the board modifies the active ruleset for all players:

*   **Tethys Qing (Void Control)**: 
    *   *Rule*: All active cards must divide their combat damage by their own room's entropy before applying it to an enemy card.
*   **Fornax Qing (Energy Core)**:
    *   *Rule*: Actions no longer consume set mana; instead, activating an ability increases the player's entropy by $500$ points.
*   **META Qing (Beat Synchronizer)**:
    *   *Rule*: The combat round only triggers attacks when `Yeo` (initiative) is an odd number.

---

## 3. Mathematical Card Traits

Cards carry passive mathematical traits that interact directly with the game engine rules:

1.  **Prime-Bound**: This card is immune to damage unless the incoming attack value is a prime number.
2.  **Asymptotic**: This card's defense increases as its health approaches zero: $\text{DEF}_{current} = \text{DEF}_{base} + \lfloor 1000 / \text{HP} \rfloor$.
3.  **Non-Commutative**: The order of combat actions matters. If this card attacks after a friendly card, its attack power is multiplied by `Yeo`. If it attacks first, its attack power is halved.
