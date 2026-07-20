# OT Accumulator basic model, ER/EER bridges, and improvements

This document describes the design and verification specifications of the **Auncient** OT Accumulator character model, its Entity-Relationship (ER) and Enhanced Entity-Relationship (EER) database bridging systems, and the four system-level improvements.

---

## 1. The Basic Model: `OT Baud LLM DAT On The ACAB`

In this configuration, **OT** refers to the **OT Accumulator** (rather than "Oregon Trail"):
* **State Ingestion:** Instantiates a `TSFiOTAccumulator` mapping undecidable boundary coordinate paths and weights.
* **ACAB Seeding:** Modulates coordinate weights directly using raw bytes from the **Auncient** Coaxial Activity Bus (ACAB) root register.
* **Baudot Transcoding:** Formats cumulative potential into character streams and translates them to 5-bit Baudot (Baud) codes.
* **LLM DAT Serialization:** Packs the Baudot token arrays into binary block-ledger assets (.dat.bin) for disk persistence.

---

## 2. ER and EER Database Bridging

* **ER Modeling:** Extracts cumulative potential values from the basic model and maps them to standard relational entities: an `Incident` entity representing coordinate boundary strain, and responding agencies.
* **EER Enhanced Subclassing:** Categorizes incidents based on strain levels. Potentials $\ge 50.0f$ are subclassed as critical system outages or `NuclearAlert` incidents mapped to NORAD; lower potentials are categorized as minor `TaxAuditConflict` incidents handled by the IRS.

---

## 3. Implemented Improvements

1. **Hamming(7,4) FEC:** Encodes bytes into 7-bit error-correcting codes, enabling SAGE PLL loops to correct single-bit transmission corruptions without triggering Kermit NAK cycles.
2. **Dynamic Tone Wheel:** Provides simulated mechanical solenoid-punch write capabilities to dynamically rewrite character tracks on rotating card memory blocks.
3. **Declarative Datalog EER:** Resolves responder agency mapping dynamically using trie compile and search rules via the native Datalog relation engine.
4. **Vulkan PLL Projection:** Projects PLL errors and tracking voltages to Vulkan Normalized Device Coordinates (NDC) to facilitate real-time oscilloscope rendering.
