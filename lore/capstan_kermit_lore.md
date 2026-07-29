# CAPSTAN KERMIT: Hardware-Bound Transactional File Transfer Protocol

CAPSTAN KERMIT is a specialized packet transfer protocol where the protocol state machine (sequence control, window buffering, and error recovery) is bound directly to the physical hardware registers of an ACID-compliant capstan shaft tape device in the XPL layer.

```
+--------------------------------------------------------------+
|                       CAPSTAN KERMIT                         |
+--------------------------------------------------------------+
|  KERMIT Packet Sequence (Seq)  <--->  Physical Sector ID      |
|  Sliding Window Buffer         <--->  Continuous Capstan Run  |
|  ACK / NACK Signals            <--->  RAW Head Parity Status  |
|  Transaction Rollback          <--->  Motor Reversal & Rewind |
+--------------------------------------------------------------+
```

### 1. Protocol Architecture & Hardware Binding
* **Sequence-to-Sector Binding (Consistency):** The packet sequence number ($Seq$) maps 1-to-1 to the physical shaft encoder step (`CAPSTAN_ENCODER`). The protocol verifies sequence alignment by asserting that the incoming packet's $Seq$ matches the current encoder step before engaging the write head.
* **Window-to-Capstan Control (Atomicity):** The sliding window size is bounded by the tape layout constraints. During window transmission, the capstan disengages its mechanical brake (`CAPSTAN_BRAKE = 0`), engages the solenoid clamp (`CAPSTAN_SOLENOID = 1`), and runs the motor forward (`CAPSTAN_CONTROL = 1`) continuously.
* **ACK/NACK via RAW Parity (Isolation):** Parity checks are performed by a Read-After-Write (RAW) head downstream. A parity pass sets `RAW_HEAD_STATUS = 1` (equivalent to a hardware ACK). A parity failure sets `RAW_HEAD_STATUS = 0` (equivalent to a hardware NACK).

### 2. ACID Transaction Enforcement in XPL
* **Atomicity:** A packet group transfer is atomic. If the RAW verification head detects a parity failure (NACK) on *any* packet within the current window:
  1. The capstan motor immediately stops (`CAPSTAN_CONTROL = 0`).
  2. The mechanical brake engages (`CAPSTAN_BRAKE = 1`).
  3. The motor reverses (`CAPSTAN_CONTROL = 2`) to rewind the tape exactly to the starting sector of the current window.
  4. The sender is instructed to retransmit the window, ensuring no partial or corrupted packets remain on disk.
* **Consistency:** Out-of-sequence packets physically violate the sector mapping boundary. Any deviation instantly trips the supervisor interrupt (`XPLSM_INTERRUPT`), locking the capstan brake to halt progress.
* **Isolation:** Separate connection channels are assigned to distinct physical tracks or separate virtual tape drives. Access is serialized via hardware lock registers (`MUTEX_REG`).
* **Durability:** A transaction is committed only when the tape brake caliper locks the capstan shaft over the next sector boundary, securing the magnetic flux transitions permanently on the non-volatile media.
