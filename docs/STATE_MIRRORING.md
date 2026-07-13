# Auncient VM: GGUF Metadata State Mirroring

This document describes the protocol and registers used to mirror external GGUF metadata commitments directly into the local virtual machine execution state.

## 1. The Mirroring Protocol
To synchronize off-chain neural model files with the active transaction ledger, the system mirrors GGUF key-value metadata directly to the ZMM VM active registers at boot time:

1. **Metadata Discovery:** During model loading, the GGUF loader (`tsfi_gguf_loader.c`) parses the model header and extracts the `auncient.acab.root` array metadata key.
2. **Buffer Storage:** The 32-byte root hash is saved in the global variable `g_gguf_acab_root`.
3. **Register Mirroring:** The virtual machine copies the global buffer directly into storage slot `55100` (representing the active local ACAB commitment).

```
[GGUF Model File]
   | (Metadata: auncient.acab.root)
   v
[tsfi_gguf_loader.c] -> Parses & sets g_gguf_acab_root
   |
   v
[ZMM VM / Yul Emulator] -> Copies to Storage Slot 55100
```

## 2. Register Definitions
* **g_gguf_acab_root (C Global):** Host-level 32-byte array buffer populated by the GGUF loader.
* **Storage Slot 55100 (VM Register):** Local ZMM VM active activity root slot used to verify incoming spent proofs and transaction spent events.
* **OP_EQUALVERIFY (Bitcoin Script):** The JIT thunk verifier compares incoming witness paths against the mirrored value in Slot 55100, rejecting blocks if the model file is out of sync with the transaction ledger.
