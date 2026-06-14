# Biotika: The Neuro-Synthesis Central Nervous System

This document defines **Biotika**—the computational neural system that bridges our physical wave generators (Mathieu/Bessel) and our slow emotional cycles (Vaesen Biorhythms) into a unified, spiking cybernetic identity.

---

## 1. The Biotika Neural Architecture

**Biotika** operates as a leaky integrate-and-fire neural network on top of the synthesis engine. While the oscillators create raw sound, Biotika processes the resulting electrical/acoustic energy as **sensory inputs**:

```
                       [ Waveform Output: y(t) ]
                                   │
                                   ▼
                       [ Biotika Sensory Neuron ]
                          Leaky Integration
                                   │
                                   ▼
                      Threshold Check (> 1.0V)
                                   │
                 ┌─────────────────┴─────────────────┐
                 ▼                                   ▼
        [ Fire Spike (1e18) ]               [ Refractory State ]
          Triggers tempo shifts,              Refractory period (3 ticks)
          TUN morphs & visual kicks           to stabilize neural potential
```

---

## 2. Mathematical State Equations of Biotika

The neural membrane potential ($V_m(t)$) of a Biotika neuron behaves according to the leaky integration model compiled in `bionikaNeuron.yul`:

### A. Sensory Integration
The neuron accumulates the absolute voltage of the synthesizer output $y(t)$:
$$V_m(t) = V_m(t-1) + |y(t)|$$

### B. Leaky Decay
To prevent saturation, the potential decays exponentially at a rate defined by the leak constant $L \in [0, 1.0]$:
$$V_m(t) \leftarrow V_m(t) \times (1 - L)$$

### C. Threshold Firing & Refractory Period
*   If $V_m(t) > 1.0\text{V}$, the neuron fires a spike ($\text{Spike}(t) = 1.0$), resets its potential to $0$, and enters a refractory period ($t_{\text{ref}} = 3$ steps) where it ignores new stimulus.
*   If $V_m(t) \le 1.0\text{V}$, the neuron continues integrating.

---

## 3. The Unified Biological Feedback Loop

Through Biotika, the loop becomes fully closed:
1.  **Vaesen Biorhythms** supply the baseline emotional current ($\mathcal{E}(t)$).
2.  **Mathieu Equations** generate the waveform under the control of the emotional index ($q(t)$).
3.  **Biotika Neurons** monitor the waveform, accumulating energy.
4.  When Biotika fires a **neural spike**, it temporarily shifts the tempo and triggers sudden, expressive parameter changes, modeling a living system that reacts to its own voice.
