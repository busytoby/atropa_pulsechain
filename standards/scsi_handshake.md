# SCSI-1 Parallel Bus Standard Certification

## 1. Specification Overview
This certification sheet documents compliance with the parallel bus SCSI-1 physical and logical handshake loops.

## 2. Handshake Protocol
The bus interface routes asynchronous data streams using parallel SCSI signaling:
* **REQ (Request):** Asserted by target hardware to indicate it is ready to receive/transmit.
* **ACK (Acknowledge):** Asserted by host adapter to acknowledge data lines state.
* **Interlocked Sequence:** REQ assertion -> ACK assertion -> REQ deassertion -> ACK deassertion.

## 3. Verified Implementations
* **Handshake Driver:** [tsfi_s370_winchester_mq_handshake](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
* **Paging Streamer:** [tsfi_s370_scsi_stream_to_ramac](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
