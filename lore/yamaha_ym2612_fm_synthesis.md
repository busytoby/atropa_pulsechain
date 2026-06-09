# Yamaha YM2612 FM Synthesis Architecture

The **Yamaha YM2612 (OPN2)** is a six-channel frequency modulation (FM) sound synthesizer chip that gives the Sega Genesis its distinct, metallic, and punchy audio signature. This document details its 4-operator voices and the 8 algorithm routing configurations.

---

## 1. 4-Operator Voice Design

Each YM2612 FM channel is composed of **4 Operators (Op1, Op2, Op3, Op4)**. An operator is a digital sine wave generator modified by an envelope generator (controlling volume over time via Attack, Decay, Sustain, and Release).

Rather than simply mixing four audio waves, operators are routed so that the output of one operator modulates the frequency (phase) of the next:

$$\text{Output} = \sin(\phi + \beta \times \text{Modulator})$$

Where $\phi$ is the carrier phase, $\beta$ is modulation strength, and $\text{Modulator}$ is the signal from the preceding operator.

---

## 2. FM Algorithms (Routing Configurations)

The relationship between the four operators is determined by one of **8 Algorithms (0 to 7)**:

```
[ Alg 0: Serial Cascade ]      [ Alg 4: Mixed Parallel ]      [ Alg 7: Pure Parallel ]
       ┌───────┐                      ┌───────┐ ┌───────┐            ┌───┐ ┌───┐ ┌───┐ ┌───┐
       │  Op1  │                      │  Op1  │ │  Op2  │            │Op1│ │Op2│ │Op3│ │Op4│
       └───┬───┘                      └───┬───┘ └───┬───┘            └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
           ▼                              │         │                  │     │     │     │
       ┌───────┐                          └────┬────┘                  └─────┼─────┼─────┘
       │  Op2  │                               ▼                             ▼     ▼
       └───┬───┘                           ┌───────┐                        ┌─────────┐
           ▼                               │  Op3  │                        │  Sound  │
       ┌───────┐                           └───┬───┘                        │ Output  │
       │  Op3  │                               ▼                            └─────────┘
       └───┬───┘                           ┌───────┐
           ▼                               │  Op4  │
       ┌───────┐                           └───┬───┘
       │  Op4  │                               ▼
       └───┬───┘                          ┌─────────┐
           ▼                              │  Sound  │
      ┌─────────┐                         │ Output  │
      │  Sound  │                         └─────────┘
      │ Output  │
      └─────────┘
```

* **Algorithm 0**: Full serial cascade. Yields complex, highly distorted, metallic, and brassy synth sounds.
* **Algorithm 4**: Op1 and Op2 modulate Op3, which modulates Op4. Good for pianos and bell attacks.
* **Algorithm 7**: All operators run in parallel directly to the output. Useful for organ sounds or multi-tone mixing.

---

## 3. Simulation Implementation

To demonstrate this system, we have created a simulator script: [test_ym2612.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_ym2612.js).

This script:
1. Defines the structures for Operators (phase, frequency, envelope multipliers).
2. Implements operator phase modulation equations.
3. Simulates the 8 routing algorithms, generating output audio sample values based on operator configurations.
