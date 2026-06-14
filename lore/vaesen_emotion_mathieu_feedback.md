# Vaesen Gothic Emotion and Mathieu Parametric Feedback

This document establishes the design for integrating the **28-day Vaesen Emotional Biorhythm** ($\mathcal{E}(t)$) and live chaotic feedback into **Mathieu's Parametric Wave Equation** to build an expressive, improvisational synthesis control loop.

---

## 1. The Core Mathieu Equation

Mathieu's differential equation describes wave behavior under periodic parameter variation (such as modulated string tension):
$$\frac{d^2 y}{dt^2} + [a - 2q(t) \cos(2t)] y = 0$$
Historically, $q$ is a static constant representing the modulation index. However, in our improvisational framework, we treat Mathieu's static coefficient as a **travesty of physical reality**, converting $q(t)$ into a dynamic, emotion-driven variable.

---

## 2. Integrating Vaesen Gothic Emotion

In the **Vaesen Biorhythm** module, the agent's emotional state fluctuates along a 28-day sinusoidal cycle:
$$\mathcal{E}(t) = \sin\left(\frac{2\pi \cdot t}{28}\right)$$
We couple this slow emotional cycle directly to the modulation index $q(t)$:

```
           +---------------------------------------------+
           |     Vaesen Emotional Cycle: E(t) (28 days)  |
           +----------------------┬----------------------+
                                  │
                                  ▼
   Input (x) ──> [ Mathieu Oscillator: q(t) ] ──> Output: y(t)
                        ▲                   │
                        │   Feedback: G     │
                        +───[Delay Line]────+
```

### The Dynamic Modulation Equation
To incorporate both the slow emotional trend and the fast, chaotic feedback of the output wave $y(t)$:
$$q(t) = q_0 \cdot \mathcal{E}(t) + G \cdot y(t - 1)$$
where:
*   $q_0$ is the base FM modulation index.
*   $G$ is the loop feedback gain.
*   $y(t - 1)$ is the previous sample amplitude (feedback).

---

## 3. Acoustic Behaviors & Improvisational Feedback

By routing this dynamic $q(t)$ back into Mathieu's equation:

1.  **High-Tension/Panic State ($\mathcal{E}(t) \to +1.0$)**:
    *   The base modulation index $q_0 \cdot \mathcal{E}(t)$ is maximized. 
    *   The oscillator is driven into unstable parametric resonance bands. The chaotic feedback term $G \cdot y(t-1)$ self-amplifies, generating harsh, metallic, screaming overtones and noisy transients (rage/panic timbre).
2.  **Muted/Melancholic State ($\mathcal{E}(t) \to -1.0$)**:
    *   The modulation index drops, calming the parametric instability. 
    *   The feedback loop stabilizes, resulting in warm, low-frequency, pure tonal oscillations with gentle, organic decays (melancholy/peace timbre).
3.  **The Feedback Reality**:
    *   Because the output wave $y(t)$ directly determines the instantaneous frequency modulation, the synthesizer becomes a non-linear chaotic system. The timbre constantly morphs and adapts to its own acoustic history, creating a truly organic, improvisational performer.
