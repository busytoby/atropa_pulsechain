# Optimum-Interval Timer Scheduling for Gas and RPC Optimization

This document outlines the design for implementing **Optimum-Interval Timer Scheduling** in the synthesis dashboard to dynamically scale simulation step rates, drastically reducing network RPC calls and blockchain gas costs.

---

## 1. The Overhead Problem

In our current architecture, simulating modules like the **Formant Filter** or **Tunnel Diode** requires sending sample-by-sample execution requests to the ZMM VM.
* **Fixed-Rate Bottleneck**: Calling the VM at a fixed high-frequency rate (e.g., $1000\text{ Hz}$ or higher) creates network bottlenecking and executes many redundant computations when the sound waves are linear or silent.
* **Gas Implications**: When running on-chain, every call to the contract incurs transaction gas costs. Executing operations during steady-state phases is highly inefficient.

---

## 2. The Solution: Dynamic Variable-Rate Scheduling

Inspired by Herget & Clemence's optimum-interval punched-card tables, we replace fixed-rate timers with a **Dynamic Variable-Rate Timer**:

```
           High Activity (Transient)
              o  o  o  o  o  o
             /                \       Low Activity (Steady-State)
            /                  \       o--------o--------o
           /                    \
  Interval: 2ms   2ms   2ms   2ms     Interval: 20ms     20ms
```

### A. The Curvature-Detector
The client-side scheduler monitors the second derivative (curvature) of the output signal $y(t)$:
$$\text{Curvature } D_2 = |y(t) - 2y(t-1) + y(t-2)|$$

1.  **Low Curvature ($D_2 < \epsilon$)**: The waveform is quiet or linear. The scheduler increases the timer interval $dt$ to $20\text{ ms} - 50\text{ ms}$. Intermediate values are generated locally in JS using linear interpolation.
2.  **High Curvature ($D_2 \ge \epsilon$)**: A transient is detected (e.g., a drum trigger or resonant spike). The scheduler instantly drops the interval $dt$ to $2\text{ ms}$ to capture the high-frequency dynamics.

---

## 3. Benefits and Gas Reduction

By implementing this dynamic scheduler:
* **RPC Reduction**: Cuts total API requests to the VM by up to **$85\%$** during steady-state sweeps or silence.
* **On-Chain Gas Optimization**: In a smart contract context, this allows state updates to be batched and written only when critical boundary conditions or transient thresholds are crossed, leaving steady-state interpolation to be performed cheaply by off-chain client nodes.
