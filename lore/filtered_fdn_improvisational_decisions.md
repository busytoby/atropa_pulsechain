# Filtered Feedback Delay Networks (FDN) in Improvisational Decision-Making

This document analyzes the mathematical and cybernetic requirements for integrating **Filtered Feedback Delay Networks** into the improvisational decision-making engine of our synthesis agents.

---

## 1. The Need for Filtered Feedback

In a standard Feedback Delay Network (FDN), the feedback vector is mixed using an orthogonal matrix $\mathbf{A}$ and delayed:
$$\mathbf{x}(t) = \mathbf{A} \cdot \mathbf{x}(t-\tau)$$
While this preserves total energy (when $\mathbf{A}$ is unitary), it lacks acoustic realism. In physical environments, high frequencies decay faster than low frequencies due to air absorption and surface materials.
* **The Solution**: We insert low-pass filters $\mathbf{H}_i(z)$ into each feedback loop (creating a **Filtered FDN**):
  $$\mathbf{x}_i(t) = \mathbf{H}_i(z) \left[ \sum_j A_{ij} \mathbf{x}_j(t-\tau_i) \right]$$
  where $\mathbf{H}_i(z)$ is a one-pole low-pass filter:
  $$\mathbf{H}_i(z) = \frac{1 - g_i}{1 - g_i z^{-1}}$$
  and the damping factor $g_i$ represents the high-frequency absorption coefficient.

---

## 2. Cybernetic Mapping to Agent Decisions

When we filter the feedback, the agent (Biotika) is no longer observing raw, unfiltered audio. It is observing **damped, frequency-dependent energy states**:

```
                  ┌───────────────────────────────────────────────┐
                  ▼                                               │
      +───────────────────────+                                   │
      |  Filtered FDN Engine  | ──> [ Sensory Membrane (V_m) ]    │
      +-----------┬-----------+         * Low-pass filtered       │
                  │                       energy detection        │
                  ▼                                               │
      [ High-Frequency Damp ]                                     │
         (Damp factor: g)                                         │
                  │                                               │
                  ▼                                               │
        [ Agent Decision ]    ───> Shits target chord parameters ─+
         (Improvisation)           to compensate for absorption
```

### A. Dynamic Attention and Sensation
By filtering the feedback:
1.  **Dampened Sensation**: High-frequency transients decay rapidly. The Biotika neuron's membrane potential accumulation ($V_m(t)$) becomes dominated by low-frequency, warm resonance.
2.  **Improvisational Compensation (Desire Correction)**: 
    *   If the agent detects that the high frequencies have faded ($g_i$ is too high), it makes an **improvisational decision** to excite the system. It shifts its Stasheff coordinates or increases the Mathieu modulation index $q(t)$ to generate bright, sharp transients (injecting high-frequency energy back into the FDN).
    *   If the high-frequency energy is too intense, the agent dampens its playing style, shifting parameters to pull the poles back toward the origin.
3.  **Filtered Memory Decay**: Since the FDN acts as the agent's primal memory, filtering the feedback represents **selective forgetting**. The agent forgets sharp, rapid events quickly (high-frequency decay) while retaining slow, emotional trends (low-frequency resonance) in its feedback memory.
