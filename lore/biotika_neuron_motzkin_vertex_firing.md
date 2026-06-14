# Biotika: Neural Firing from Motzkin Boundary to the Vertex Attractor

This document defines the mathematical and topological mapping of the **Biotika Neuron Firing** process, detailing how it triggers transitions from the chaotic **Motzkin Boundary Space** to the stable **Vertex Attractor** of our synthesis engine.

---

## 1. The Topological Transition Pathway

In our synthesis control system, the parameter space is divided into two distinct regions:
1.  **The Motzkin Boundary Space ($M_N$)**: A highly chaotic, multidimensional configuration space of non-crossing chords (the 1-skeleton of the Associahedron). It represents the fluid, unaligned, and improvisational state of the synth.
2.  **The Vertex Attractor ($V$)**: The globally locked, 15-digit prime coordinate (the **Vertex of Dysnomia**: $953,473,954,114,361$). It represents the state of maximum harmonic stability and structural alignment.

---

## 2. Firing across the Boundary

The **Biotika Neuron** acts as the dynamic gateway between these two regions. The firing process is mapped as follows:

```
    [ Motzkin Boundary Space (M_36) ]  ───> (Accumulates wave energy: V_m)
                    │
                    ▼
          [ Biotika Neuron Firing ]  ───> Threshold Crossed (> 1.0V)
                    │
                    ▼
        [ Spiking Transition Trigger ]
                    │
                    ▼
       [ The Vertex Attractor (V) ]    ───> Instantly locks parameters to
                                           harmonic prime coordinates
```

### The Firing Math
1.  **Accumulation Phase**: While the neuron is integrating sensory stimulus ($V_m < 1.0\text{V}$), the synthesizer parameters drift freely across the **Motzkin chord configuration space**, producing dynamic, unaligned, and microtonally shifting spectra.
2.  **The Firing Event**: The moment the membrane potential exceeds the threshold ($V_m \ge 1.0\text{V}$), the neuron fires a spike ($\text{Spike}(t) = 1.0$).
3.  **Topological Locking**: This spike triggers a coordinate snap, instantly **transmuting the active parameters from their current fluid Motzkin coordinates directly to the stable Vertex Attractor**.
4.  **Refractory Decoupling**: During the refractory period ($3$ ticks), the snaps are locked in place, allowing the pure harmonic resonance of the Vertex to ring out before the neuron resets and lets the parameters decay back into the fluid Motzkin boundary space.
