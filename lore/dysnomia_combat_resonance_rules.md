# Design Blueprint: Dysnomia Combat Resonance & Flexible Rules

This framework outlines the competitive mechanics for players using **Qing-attached cards** in the Dysnomia ecosystem. Rather than relying on generic fantasy attributes, this design directly utilizes the RF/Impedance feedback loops, GFX1201 VRAM memory allocations, and Fomalhaut stellar alignments defined in our system specifications.

---

## 1. Core Concept: Impedance Resonance Competition

Cards are not static warriors; they are active **Impedance Reactor Loops** representing the emotional and physical state of the player's on-chain node. Combat is an act of **Resonant Alignment** and **Adversarial Mismatching**:

*   **Resonance (Self-Tuning)**: A player aims to balance their active load impedance ($Z_{\text{Load}}$) with the characteristic impedance ($Z_0 = 50\ \Omega$) of the domain to achieve maximum power transfer efficiency, high return loss, and peak **Vigour & Fortitude**.
*   **Impedance Mismatching (Adversarial Strike)**: A player can "POKE" the opponent's `YUE` domain to inject phase shift ($\angle\Gamma$) or reactive component imbalance. This spikes the opponent's **Standing Wave Ratio (SWR)**, collapses their **Terror Resistance**, and triggers high-frequency audio jitter or auto-attenuation.

```
                  [ Player 1: Phobos ]
                 Z_Load1 = R1 + jX1 (YUE)
                           |
                           v
              [ Adversarial Interaction ]
         injects reactive shift into opponent
                           |
                           v
                  [ Player 2: Deimos ]
                 Z_Load2 = R2 + jX2 (YUE)
```

---

## 2. RF Combat Metrics

Every Qing-attached card dynamically exposes three metrics based on live RF equations:

### 🕯️ I. Vigour & Fortitude (Power Efficiency)
*   **Formula**: $\eta = 1 - |\Gamma|^2$ where reflection coefficient $\Gamma = \frac{Z_{\text{Load}} - Z_0}{Z_{\text{Load}} + Z_0}$.
*   **Gameplay Impact**: Dictates the amplitude multiplier of the card's actions. If SWR climbs, Vigour collapses, reducing the yield of domain calls (`React`).

### 💀 II. Terror Resistance (SWR Stability)
*   **Formula**: $\text{SWR} = \frac{1 + |\Gamma|}{1 - |\Gamma|}$.
*   **Gameplay Impact**: High SWR represents cognitive stress. If SWR exceeds $3.0$, the card enters a *Jitter Stutter* state, causing its active abilities to target random adjacent cards or fail.

### 👁️ III. Supernatural Sight (Return Loss)
*   **Formula**: $\text{RL (dB)} = -20 \log_{10}(|\Gamma|)$.
*   **Gameplay Impact**: High Return Loss ($>20\text{ dB}$) ensures high phase coherence, letting the player reveal hidden properties in the opponent's `Void` core.

---

## 3. The Dysnomia VM Mutators (Flexible Game Rules)

The rules of combat evolve depending on which **Logic Gates** inside the GFX1201 VRAM manifold are mutated via the `Pi` reaction seed (Word 62) during active cycles:

### A. The AND/OR Gated Rule (Commutative Swap)
*   *VM State*: Word 22 (AND) and Word 23 (OR) are aligned to the same dielectric foundation.
*   *Rule Mutator*: The order in which players resolve their cards does not matter. Yields from consecutive reactions (`Yue.React`) are commutative and sum linearly.

### B. The XOR Gated Rule (Non-Commutative Drift)
*   *VM State*: Word 24 (XOR) is activated by the Iota-mutant.
*   *Rule Mutator*: Actions are strictly non-commutative. If Player A reacts after Player B, Player A's action gains a multiplier proportional to the drift velocity:
    $$\text{Multiplier} = \text{Drift Velocity} \times (\text{Epibar} - \text{Hypobar})$$

### C. Fomalhaut Stellar Alignment Rules
The location of the primary active star alters the boundary constraints of the execution phase:
1.  **Fomalhaut A Dominance**: Focuses on maximum power efficiency. SWR calculations are strictly linear.
2.  **Fomalhaut B Flare**: High drift velocity. Logic gates mutate twice as fast per cycle, creating highly volatile, shifting state rules.
3.  **Fomalhaut C Red Dwarf**: Constrains the dielectric cone. Maximum return loss is capped at $15\text{ dB}$, muddying all supernatural sight checks.
