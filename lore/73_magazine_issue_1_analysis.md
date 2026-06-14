# 73 Magazine: Issue #1 (October 1960) Technical Analysis

The inaugural issue of *73 Magazine*, published in October 1960 by Wayne Green (W2NSD), represents a pivotal moment in amateur radio history. It focused on homebrew construction, early transistorization, surplus conversion, and technical fundamentals, contrasting with the more formal style of other publications of the era.

---

## 📅 Historical Context & Philosophy
* **Launch Date:** October 1960
* **Editor/Publisher:** Wayne Green, W2NSD (former editor of *CQ Magazine*)
* **Editorial Philosophy:** Focus on practical, inexpensive home construction projects, surplus equipment conversions, and readable technical theory rather than high-end commercial products.

---

## 🛠️ Key Technical Article Tear-Downs

### 1. "Something New in FM" (W7CSD)
This article was pioneering in its introduction of the **Varicap** (variable capacitance diode / varactor) for direct frequency modulation of a transmitter oscillator.
* **Core Concept:** Utilizing the voltage-dependent junction capacitance of a semiconductor diode to modulate the resonant frequency of an LC tank circuit.
* **Significance:** Replaced bulky reactance tube modulators with a single solid-state component, paving the way for compact VHF FM gear.

```mermaid
graph LR
    AudioIn[Audio Input] --> Varicap[Varicap Diode]
    Varicap -->|Capacitance Shift| LCTank[LC Tank Circuit]
    LCTank -->|Frequecy Modulated RF| Oscillator[VFO Stage]
```

### 2. "Audio Booster" (K5JKX/6)
An early solid-state audio processing project designed to increase effective "talk power."
* **Topology:** A three-transistor preamplifier/clipper.
* **Function:** Boosted weak microphone signals and applied controlled peak clipping to keep average modulation high without exceeding legal modulation limits (preventing splatter).
* **Components:** PNP germanium transistors (typical of the era, such as the 2N107 or CK722).

### 3. "Bantam Converters" (W9DUT)
VHF converter designs enabling existing HF receivers to tune into higher frequency bands.
* **Frequencies Covered:** 50, 108, 144, 152, and 220 MHz.
* **Architecture:** Two-tube designs using high-transconductance pentodes/triodes (e.g., 6DJ8/ECC88 frame-grid tubes) to achieve a low noise figure on VHF.

### 4. "Tubeless Electronic Keyer" (W1TUW)
A project exploring early automation in telegraphy.
* **Design:** Utilized polar relays and RC timing networks to generate self-completing dots and dashes without relying on vacuum tube circuits or active multivibrators.

---

## 📻 Surplus & Test Equipment Reviews

### Digest of Surplus (W1MEG)
Amateur radio in the 1950s and 1960s was heavily defined by WWII military surplus. This article provided a guide to navigating the surplus depots and converting classic military radios (like the ARC-5 series or BC-348) for civilian amateur bands.

### Testing the Heathkit GC-1A "Mohican"
The issue featured a hands-on review of Heathkit's first fully-transistorized general coverage communications receiver.
* **Features:** 10 transistors, battery power capability, and ceramic IF filters.
* **Verdict:** Highlighted the shifts in portable operation and receiver design paradigms away from power-hungry vacuum tubes.

---

## ⚡ Safety & Fundamentals
* **"Shock" (Bates):** Addressed the biological effects of electric shock, high-voltage safety rules for tube transmitters, and modern resuscitation techniques.
* **"Modulation Fundamentals" (W0TKU):** A mathematical and graphical overview of Amplitude Modulation (AM) sidebands and carrier relationships.
