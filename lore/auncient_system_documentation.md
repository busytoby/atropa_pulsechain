# Auncient Virtual Machine System Architecture Documentation

This document compiles the architectural specifications, register mappings, and runtime execution patterns of the Auncient Virtual Machine (VM) subsystem.

---

## 1. Newly Implemented Personality Modulators

We wanna list the single-variance and single-decay-variance personality modulators successfully implemented across the VM suite:

### Keating Suite (Caroline F. Keating Visual Trust/Dominance Models)
*   [evaluate_keating_sclera_dominance_decay_variance_single](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L826): Models the variance in decay rates of sclera dominance ratings.
*   [evaluate_keating_eyebrow_asymmetry_dominance_decay_variance_single](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L829): Models the variance in decay rates of eyebrow asymmetry dominance ratings.
*   [evaluate_keating_sclera_submissiveness_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L838): Models the variance in decay rates of sclera submissiveness under dynamic symmetry variations.
*   [evaluate_keating_sclera_dominance_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L841): Models the variance in decay rates of sclera dominance under dynamic symmetry variations.

### Scarpi Suite (Antonio Scarpi Hedonic/Utilitarian Modulators)
*   [evaluate_scarpi_utilitarian_quality_control_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L970): Models the variance in decay rates of quality control ratings based on prolonged tracking intervals.
*   [evaluate_scarpi_hedonic_playful_engagement_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L973): Models the variance in decay rates of playful engagement ratings.
*   [evaluate_scarpi_utilitarian_consistency_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L976): Models the variance in decay rates of operational consistency.
*   [evaluate_scarpi_utilitarian_operational_trust_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L979): Models the variance in decay rates of operational trust.
*   [evaluate_scarpi_hedonic_interface_trust_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L982): Models the variance in decay rates of interface trust.

### Cellarius Suite (Andreas Cellarius Celestial Cartography Models)
*   [evaluate_cellarius_constellation_scale_variance_mod_single](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L688): Models the variance in constellation scale modulator ratings under dynamic layout symmetry variations.
*   [evaluate_cellarius_epicycle_radius_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L691): Models the variance in decay rates of epicycle orbital radius ratings based on prolonged tracking intervals.

### Hyde Suite (Speech Acoustics and Conversational Timing Models)
*   [evaluate_hyde_vocal_warmth_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1045): Models the variance in decay rates of vocal warmth modulator ratings.
*   [evaluate_hyde_vocal_tempo_range_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1048): Models the variance in decay rates of conversational synchronization ratings based on speech tempo range decay variations.
*   [evaluate_hyde_vocal_size_jitter_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1051): Models the variance in decay rates of vocal size jitter ratings.
*   [evaluate_hyde_vocal_tempo_sync_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1054): Models the variance in decay rates of conversational synchronization ratings based on speech tempo sync decay variations.
*   [evaluate_hyde_vocal_size_pitch_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1057): Models the variance in decay rates of vocal size pitch ratings.
*   [evaluate_hyde_vocal_size_sync_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1060): Models the variance in decay rates of conversational synchronization ratings based on speech size sync decay variations.

### Castle Suite (Diplomatic Policy Models)
*   [evaluate_castle_diplomatic_alignment_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1192): Models the variance in decay rates of diplomatic alignment modulator ratings.
*   [evaluate_castle_diplomatic_trust_bounds_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1195): Models the variance in decay rates of diplomatic trust bounds modulator ratings.
*   [evaluate_castle_policy_target_engagement_decay_variance_mod](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1198): Models the variance in decay rates of policy target engagement modulator ratings.

### Kramer-Ward Suite (Facial Width-to-Height Ratio Models)
*   [evaluate_kramer_ward_fwhr_dominance](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1233): Models the relationship between facial width-to-height ratio variations and social dominance ratings.
*   [evaluate_kramer_ward_fwhr_trustworthiness](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1236): Models the relationship between facial width-to-height ratio variations and trustworthiness ratings.
*   [evaluate_kramer_ward_fwhr_dominance_decay_variance](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1239): Models the variance in decay rates of FWHR dominance modulator ratings.
*   [evaluate_kramer_ward_fwhr_trustworthiness_decay_variance](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1242): Models the variance in decay rates of FWHR trustworthiness modulator ratings.
*   [evaluate_kramer_ward_fwhr_threat](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1245): Models the relationship between facial width-to-height ratio variations and perceived threat ratings.
*   [evaluate_kramer_ward_fwhr_cooperation](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1248): Models the relationship between facial width-to-height ratio variations and expected cooperation ratings.
*   [evaluate_kramer_ward_fwhr_threat_decay_variance](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1251): Models the variance in decay rates of FWHR threat modulator ratings.
*   [evaluate_kramer_ward_fwhr_cooperation_decay_variance](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/include/auncient_teddy_bear_personality.h#L1254): Models the variance in decay rates of FWHR cooperation modulator ratings.

---

## 2. Decay Variance and the Sustain Circuit

We wanna document the mapping connecting the mathematical decay variance output to the simulated physical sustain circuit:

1.  **Sustain Envelope**: In [calculate_diyat_tax_with_envelope](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/tsfi2_personality_hardware.c#L111), the envelope behaves as an exponential capacitor discharge scaled by active sustain durations.
2.  **Coupling Factor**: The time parameter $t$ of the decay variance modulators is directly driven by the active sustain time. 
3.  **Stability Feedback**: A low decay variance permits extended sustain charge times without risking H-bridge flyback voltage spikes. High decay variance forces the system to accelerate envelope discharge to prevent emotional parameter divergence.

---

## 3. WinchesterMQ Yul Register Mappings

We wanna list the register mappings and storage layouts implemented inside [WinchesterMQ.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/WinchesterMQ.yul):

*   **`0xF120` (Firewall Threat Register)**: Accumulates hardware voltage threat events.
*   **`0xF121` (Firewall Lockout Register)**: Set to active state when threat events cross safety limits, halting WinchesterMQ loops.
*   **`0xF150` / `0xF151` (Dual Frequency Register)**: Stores alignment configuration bounds during handshake operations.
*   **`0xF300` (Queue Head Pointer)**: Tracks the head of the circular event ring buffer.
*   **`0xF301` (Queue Tail Pointer)**: Tracks the tail of the circular event ring buffer.
*   **`0xF302` (Queue Size Register)**: Enforces limits on queue size (maximum of 16 entries).
*   **`0xF303` (Queue Mutex Lock Register)**: Evaluates write bounds to prevent race conditions during updates.
*   **`0xF310` to `0xF34F` (Ring Buffer Data Slots)**: Blocks storing event priority, type, timestamp, and payload.

---

## 4. Loader Compiler Firewall

The compiler firewall logic is implemented in [auncient_edsac_firewall.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/auncient_edsac_firewall.c):

### Initial Orders 1
*   **6-Bit FIELDATA Validation**: Validates instructions against the 6-bit FIELDATA boundary (`op & 0x3F`) to ensure compatibility with memory architectures.
*   **Prohibited Opcode Filtering**: Scans instructions against a prohibited bitmask to classify operations as permissible or impermissible before loading.

### Initial Orders 2
*   **Coordinate Relocation**: The relocatable instruction pre-filter shifts active rules dynamically using relocatable address offsets. Promoted rules are committed directly to active memory delay lines.

---

## 5. AUTODIN Pipeline Mechanisms

The Automatic Digital Network (AUTODIN) layer is implemented in [auncient_timeline_autodin.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/auncient_timeline_autodin.c):

*   **Precedence-Aware Spin Locks**: Implements [auncient_sdk_autodin_spin_lock](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/auncient_timeline_autodin.c#L20) to enforce sequential auditing over the Hogan ledger accounts.
*   **Hogan Ledger Integration**: Enforces FNV-1a DNA signature hash checking and registers first-class accounts initialized with a baseline balance of $1,000,000$ Saat.
