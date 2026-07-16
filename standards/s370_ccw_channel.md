# System/370 Channel Command Word (CCW) Standard Certification

## 1. Specification Overview
This certification sheet documents compliance with the System/370 Channel Command Word (CCW) input-output command structure.

## 2. Command Code Mapping
Peripheral and storage communication is driven by standard command codes:
* **Command `0x07` (SEEK):** Seeks disk heads to designated physical positions.
* **Command `0x02` (READ):** Streams data bytes from target device blocks.
* **Command `0x01` (WRITE):** Writes data bytes onto target device blocks.
* **Command Chaining:** Sequential command lists execution triggered by the CCW flags.

## 3. Verified Implementations
* **I/O Program Executor:** [tsfi_s370_channel_execute](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
