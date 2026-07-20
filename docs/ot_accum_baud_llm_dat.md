# OT Pipeline Implementations, ER/EER bridges, and improvements

This document describes the design and verification specifications of the **Auncient** OT character models, their Entity-Relationship (ER) and Enhanced Entity-Relationship (EER) database bridging systems, and three generations of system-level improvements.

---

## 1. Pipeline Configurations for the Basic Model: `OT Baud LLM DAT On The ACAB`

Three operational definitions of **OT** are supported within the pipeline:

### 1.1 Oregon Trail (OT)
* **State Ingestion:** Ingests simulated game attributes (days, pace, rations, health).
* **ACAB Seeding:** Seeding cash, oxen, and resources out-of-band using bytes from the **Auncient** ACAB epoch root.

### 1.2 OT Accumulator (OT)
* **State Ingestion:** Aggregates undecidable boundary coordinate paths and weights.
* **ACAB Seeding:** Modulates coordinate weights directly using the ACAB root register.

### 1.3 Cryptographic Oblivious Transfer (OT)
* **State Ingestion:** Executes a 1-out-of-2 Oblivious Transfer protocol where a receiver chooses selection $b \in \{0,1\}$ to retrieve secret messages.
* **ACAB Seeding:** Modulates public key bases using the ACAB root prime modulus.

All configurations serialize output status text through Baudot encoders, tokenize the stream (LLM), and write the payload to binary block-ledger assets (.dat.bin).

---

## 2. ER and EER Database Bridging

* **ER Modeling:** Extracts simulation status payloads and maps them to standard relational entities: an `Incident` entity representing coordinates, and responding agencies.
* **EER Enhanced Subclassing:** Categorizes incidents dynamically based on payload contents. Critical states are subclassed as `NuclearAlert` incidents mapped to NORAD; minor states are categorized as `TaxAuditConflict` incidents handled by the IRS.

---

## 3. Implemented Improvements

### Generation 1
1. **Hamming(7,4) FEC:** Corrects single-bit transmission corruptions.
2. **Dynamic Tone Wheel:** Modulates character tracks on rotating card memory blocks.
3. **Declarative Datalog EER:** Resolves responder agency mapping dynamically via the native Datalog relation engine.
4. **Vulkan PLL Projection:** Projects PLL errors and tracking voltages to Vulkan NDC for oscilloscope rendering.

### Generation 2
1. **SECDED Hamming(8,4):** Adds overall parity to support Single Error Correction and Double Error Detection.
2. **Lock-free SPSC Queue:** Simulates non-blocking memory barrier writes for thread-safe insertions.
3. **Cascading Datalog Cascade:** Resolves EER subclassing using multi-fact logical evaluation.
4. **PI PLL Loop Tuning:** Integrates proportional-integral tuning to demodulation loops.

### Generation 3
1. **Adaptive Baudot Maps:** Permits on-the-fly dictionary shifting to minimize transmission shifts.
2. **BCH(15,7) DEC-TED:** Protects data paths with Double Error Correction and Triple Error Detection.
3. **PID PLL with AGC:** Regulates lock stability and signal gain under drift.
4. **Referential Integrity Constraints:** Cascades deletions to prevent orphaned records in the EER database.

### Generation 4 (Galois and Kalman Permanently Banned)
1. **Dynamic STANAG Routing:** Routes packets dynamically by Service Access Point (SAP) handlers.
2. **LRC(15,11) Checksum Recovery:** Enforces systematic longitudinal redundancy parity byte checks to correct single-byte transmission errors, completely replacing Galois-based Reed-Solomon codecs.
3. **Exponential Moving Average (EMA):** Smoothes PLL tracking using low-pass EMA filters, completely replacing Kalman estimation.
4. **Declarative Relational Invariant Audits:** Automatically verifies global relational assertions across EER tables to prevent incorrect or non-compliant states.

