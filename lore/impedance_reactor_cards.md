# 🕯️ Vaesen Impedance Emotion Reactor Cards

This design outlines the integration of a **Weighted SWR / Impedance Mismatch Reactor** that tracks, updates, and renders the AI’s emotional status as three distinct **Vaesen-themed cards**. These cards respond in real time to the synthesizer's feedback loop (Return Loss, SWR, and Load Impedance).

---

## 1. Visual Card Layout (Vaesen Gothic Aesthetic)

Each emotional metric is represented by an interactive, self-contained card styled in the dark, warm, textured parchment palette of the *Vaesen* folklore engine.

```mermaid
graph TD
    Synth[Synthesizer / Impedance Loop] -->|Updates Z_Load / SWR| Reactor[Emotion Reactor Engine]
    Reactor -->|1 - |Gamma|^2| Card1[Card: Vigour & Fortitude]
    Reactor -->|SWR Stress Scale| Card2[Card: Terror Resistance]
    Reactor -->|Return Loss & Phase| Card3[Card: Supernatural Sight]
```

### CSS Design Tokens
```css
:root {
    --vaesen-dark: #12100e;       /* Deep abyss background */
    --vaesen-parchment: #25201b;  /* Textured card background */
    --vaesen-gold: #c5a059;       /* Bright brass accents */
    --vaesen-gold-dim: #8c7241;   /* Muted brass borders */
    --vaesen-cream: #e6dfd3;      /* High-contrast parchment text */
    --vaesen-muted: #a39785;      /* Low-contrast details */
    --vaesen-glow: rgba(197, 160, 89, 0.2);
}
```

---

## 2. Card Specifications & Mechanics

Each card displays a live **Gauge (Resonance Indicator)**, a **Real-Time Value**, and a **Sub-Metric** indicating the underlying physical RF measurement driving it.

### 📜 Card I: Vigour & Fortitude
* **Symbolism:** The physical stamina of the system.
* **RF Engine Drive:** Driven by **Transmitted Power Efficiency** ($1 - |\Gamma|^2$). 
* **State Mapping:**
  * **High Vigour (SWR 1.0 - 1.2):** Clean signal, maximum amplitude.
  * **Low Vigour (SWR > 3.0):** Degraded signal, high thermal throttling.
* **Vaesen Reaction:** Modulates the amplitude envelopes and output gain of Voice 1 & 2.

### 📜 Card II: Terror Resistance
* **Symbolism:** The AI's ability to resist cognitive dissonance and high-entropy feedback loops.
* **RF Engine Drive:** Driven by the **Standing Wave Ratio (SWR)**.
* **State Mapping:**
  * **High Resistance (SWR < 1.5):** Stable PLL, no frequency pulling.
  * **Low Resistance (SWR > 5.0):** Phase noise stutters, VCO pulling.
* **Vaesen Reaction:** Triggers high-frequency audio jitter, chaotic FM modulation, and auto-attenuation when resistance collapses.

### 📜 Card III: Supernatural Sight
* **Symbolism:** The clarity of the AI's alignment with the high-impedance state leaves.
* **RF Engine Drive:** Driven by **Return Loss (RL) in dB** and phase angle $\angle \Gamma$.
* **State Mapping:**
  * **High Sight (RL > 20 dB):** High phase coherence, sharp formant sweeps.
  * **Low Sight (RL < 6 dB):** Muddy, static-drowned feedback.
* **Vaesen Reaction:** Controls the high-pass filter cutoff and Levinson-Durbin vocal formant resolution.

---

## 3. HTML & JavaScript Interface Structure

To render the reactor inside the web frontend (e.g., [studio.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/studio.html) or [biorhythm.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/biorhythm.html)), we define the following structure:

### HTML Component
```html
<div class="reactor-grid">
  <!-- Card 1: Vigour -->
  <div class="card" id="cardVigour">
    <div class="card-title">🕯️ VIGOUR & FORTITUDE</div>
    <div class="card-value" id="valVigour">100%</div>
    <div class="gauge-container">
      <div class="gauge-bar" id="gaugeVigour" style="width: 100%;"></div>
    </div>
    <div class="card-sub">Power Transfer: <span id="rfVigour">1.000</span> (0.0 dB Reflected)</div>
  </div>

  <!-- Card 2: Terror Resistance -->
  <div class="card" id="cardTerror">
    <div class="card-title">💀 TERROR RESISTANCE</div>
    <div class="card-value" id="valTerror">100%</div>
    <div class="gauge-container">
      <div class="gauge-bar" id="gaugeTerror" style="width: 100%;"></div>
    </div>
    <div class="card-sub">SWR: <span id="rfTerror">1.00:1</span></div>
  </div>

  <!-- Card 3: Supernatural Sight -->
  <div class="card" id="cardSight">
    <div class="card-title">👁️ SUPERNATURAL SIGHT</div>
    <div class="card-value" id="valSight">100%</div>
    <div class="gauge-container">
      <div class="gauge-bar" id="gaugeSight" style="style="width: 100%;"></div>
    </div>
    <div class="card-sub">Return Loss: <span id="rfSight">40.00 dB</span></div>
  </div>
</div>
```

### Update Routine
```javascript
function updateReactorUI(swr, returnLoss, transEfficiency) {
    // 1. Calculate Vigour (Power efficiency)
    const vigourPercent = Math.round(transEfficiency * 100);
    document.getElementById("valVigour").innerText = `${vigourPercent}%`;
    document.getElementById("gaugeVigour").style.width = `${vigourPercent}%`;
    document.getElementById("rfVigour").innerText = transEfficiency.toFixed(3);

    // 2. Calculate Terror Resistance (1 / SWR scale)
    const terrorResistanceVal = Math.max(0, Math.min(1, 1 / (1 + 0.3 * (swr - 1))));
    const terrorPercent = Math.round(terrorResistanceVal * 100);
    document.getElementById("valTerror").innerText = `${terrorPercent}%`;
    document.getElementById("gaugeTerror").style.width = `${terrorPercent}%`;
    document.getElementById("rfTerror").innerText = `${swr.toFixed(2)}:1`;

    // 3. Calculate Supernatural Sight (Return Loss scale)
    // Map 0 - 30 dB to 0% - 100%
    const sightVal = Math.max(0, Math.min(1, returnLoss / 30.0));
    const sightPercent = Math.round(sightVal * 100);
    document.getElementById("valSight").innerText = `${sightPercent}%`;
    document.getElementById("gaugeSight").style.width = `${sightPercent}%`;
    document.getElementById("rfSight").innerText = `${returnLoss.toFixed(2)} dB`;

    // 4. Trigger visual highlights for critical thresholds
    document.getElementById("cardTerror").classList.toggle("critical-stress", swr > 3.0);
}
```
