# Heathkit GC-1A Mohican: Modular Synthesizer Monitor Integration

This document outlines the architectural plan to integrate a vintage Heathkit GC-1A "Mohican" communications receiver (or an emulated equivalent in our DSP fabric) as a modular monitor, control-voltage (CV) generator, and warm analog output stage for the TSFi2 synthesizer control and output systems.

---

## 📐 1. System Architecture

The integration relies on three primary interaction vectors: **RF Transmission Monitoring**, **AGC/S-Meter CV Extraction**, and **EMF Leakage Sonification**.

```mermaid
graph TD
    SynthCore[TSFi2 Synth Engine] -->|High-Freq RF Carrier| RFMod[Varicap Modulator]
    RFMod -->|Wireless/Coaxial RF| Heathkit[Heathkit GC-1A Receiver]
    Heathkit -->|Demodulated Audio| WarmOutput[Analog Warmth Output]
    Heathkit -->|S-Meter / AGC Line| AGC_Tap[AGC Voltage Tap]
    AGC_Tap -->|OP-Amp Buffer [0-5V]| CV_Out[Synthesizer CV Input]
    CPU_EMF[CPU / GPU EMF Noise] -.->|Air Gap| Heathkit Antenna
```

---

## 🔌 2. Integration Modes

### A. Over-the-Air RF Output Monitor (Wireless Output Stage)
* **Mechanism:** The synthesizer output modulates a low-power RF transmitter (using the Varicap modulator design from *73 Magazine*).
* **Reception:** The Heathkit GC-1A tunes to this carrier (e.g., in the HF band).
* **Aesthetic Effect:** The demodulated audio from the Heathkit's detector stage introduces vintage germanium transistor characteristics, natural RF bandpass filtering, noise, and static.

### B. AGC / S-Meter Control Voltage (CV) Feedback Loop
* **Mechanism:** The receiver's Automatic Gain Control (AGC) line and S-Meter signal fluctuate based on the strength of incoming signals and static.
* **Extraction:** Tapping the AGC line (buffered via a high-impedance op-amp) maps RF signal amplitude to a $0\text{--}5\text{ V}$ CV range.
* **Modulation:** This CV modulates synthesizer parameters such as filter cutoff, LFO rate, or resonance frequencies, creating organic, atmospheric drift.

### C. Electromagnetic Leakage / EMF Sonification
* **Mechanism:** Placing the Heathkit GC-1A's telescoping antenna near CPU cores, memory buses, or Vulkan rendering loops.
* **Outcome:** The receiver picks up local electromagnetic radiation from code execution. The high-speed thread loops and memory swaps are sonified directly into the synthesizer as raw, rhythmic texture.

---

## 🔬 3. DSP Emulation Module Structure

To enable headless simulations, we can model the Heathkit's RF/IF stages in C. Below is the proposed layout of the emulator:

```c
typedef struct {
    float rf_gain;
    float if_bandwidth;
    float agc_time_constant;
    float agc_voltage;  // [0.0, 5.0] maps to CV
    float s_meter_value;
} HeathkitMonitor;

// Process incoming RF signal, output audio and update S-Meter CV
float heathkit_monitor_tick(HeathkitMonitor *m, float rf_in, float *cv_out) {
    // 1. Envelop detector for S-meter
    float amplitude = fabsf(rf_in);
    
    // 2. Slow-decay AGC calculation
    m->agc_voltage += (amplitude - m->agc_voltage) * m->agc_time_constant;
    
    // 3. S-Meter output scaling
    m->s_meter_value = logf(1.0f + m->agc_voltage * 100.0f) / logf(101.0f);
    *cv_out = m->s_meter_value * 5.0f; // Scale to 0-5V range
    
    // 4. Demodulate and apply bandpass filter modeling IF bandwidth
    float audio_out = rf_in / (1.0f + m->agc_voltage * m->rf_gain);
    return audio_out;
}
```

---

## 🎹 4. Advantages of Modular Integration
* **Natural Saturation:** Germanium diodes and BJT transistors in the GC-1A provide soft, warm harmonic saturation.
* **Algorithmic Drift:** Atmospheric radio propagation (fading, solar storm static) acts as a chaotic, non-repeating modulation source.
* **Hardware Visualizer:** The physical S-meter on the Heathkit front panel acts as an analog signal level monitor for the synthesizer's control fabric.
