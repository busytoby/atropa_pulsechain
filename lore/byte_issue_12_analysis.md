# Lloyd Rice's Formant Speech Synthesis (BYTE Magazine, August 1976) & TSFi2 Integration

This document details the mapping, theoretical alignment, and practical integration of speech synthesis concepts from **Lloyd Rice's "Friends, Humans, Countryrobots: Lend me your ears"** (published in *BYTE* Magazine, August 1976, Vol. 1, No. 12, pages 16–24) into the **TSFi2** (Bijective Helmholtz / Yul Speech Synthesizer) architecture.

---

## 1. Concept Mapping & Architecture Matrix

| BYTE Issue #12 Concept | Original Hardware Implementation | TSFi2 Target Subsystem | Implementation Translation |
| :--- | :--- | :--- | :--- |
| **Vocal Tract Formants** | Three parallel bandpass filters modeling resonances $F_1$, $F_2$, $F_3$. | **SID MusicMaker Engine** | Parallel routing to Voice 1, Voice 2, and Voice 3 registers on `musicMaker.yul`. |
| **Glottal Pulse Excitation** | Analog pulse/sawtooth generator simulating vocal cord vibrations. | **SID Sawtooth Waveform** | Waveform control registers set to Sawtooth (17) for voiced vowel generation. |
| **Formant Frequency Control** | Voltage-controlled filters driven by 8-bit DAC ports (Computalker CT-1). | **Yul writeFormants Method** | Selector `0x07c01cb1` translating frequency values to SID frequency words ($freq = Hz \times 10$). |

---

## 2. Theoretical Principles of Formant Synthesis

Lloyd Rice's article describes speech synthesis by modeling the acoustic resonances of the human vocal tract. The shape of the vocal tract determines peaks in the frequency spectrum, known as **formants**:
1.  **Formant 1 ($F_1$)**: Resonates primarily with the tongue height and jaw opening (range: 200 Hz to 1000 Hz).
2.  **Formant 2 ($F_2$)**: Resonates primarily with tongue advancement (range: 800 Hz to 2500 Hz).
3.  **Formant 3 ($F_3$)**: Represents finer resonant details and retroflexion (range: 1500 Hz to 3000 Hz).

By mixing three independent frequencies ($F_1, F_2, F_3$) corresponding to these resonance peaks, the synthesizer can produce distinct vowel sounds. For example:
-   **Vowel /i/ ("beet")**: Low $F_1$ (~270 Hz), High $F_2$ (~2290 Hz).
-   **Vowel /a/ ("father")**: High $F_1$ (~730 Hz), Low $F_2$ (~1090 Hz).

---

## 3. On-Chain TSFi2 Implementation

We implemented the multi-formant synthesis routing natively within our [speechSynthesizer.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/speechSynthesizer.yul) contract under the method `writeFormants(uint256 f1, uint256 f2, uint256 f3)`.

### Register Routing Logic
When `writeFormants` is called with frequencies in Hz, the contract scales the values to SID frequency constants ($Hz \times 10$) and pokes the target `musicMaker` registers:

*   **Voice 1 ($F_1$)**:
    - Freq Low: `54272`
    - Freq High: `54273`
    - Control: `54276` (Sawtooth Waveform: `17`)
*   **Voice 2 ($F_2$)**:
    - Freq Low: `54279`
    - Freq High: `54280`
    - Control: `54283` (Sawtooth Waveform: `17`)
*   **Voice 3 ($F_3$)**:
    - Freq Low: `54286`
    - Freq High: `54287`
    - Control: `54290` (Sawtooth Waveform: `17`)

---

## 4. Verification and Execution

Verification of this implementation is handled by the automated test suite [test_formant_speech.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_formant_speech.js):
1.  **Deployment**: Compiles and deploys the contract to the local test network.
2.  **Execution**: Calls `writeFormants(500, 1500, 2500)` representing a typical vowel resonance state.
3.  **Validation**: Peeks the registers of the `musicMaker` contract to verify that the frequencies were scaled and written correctly to all 3 voice channels.
