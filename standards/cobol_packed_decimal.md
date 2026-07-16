# COBOL COMP-3 Packed Decimal Standard Certification

## 1. Specification Overview
This certification sheet documents compliance with the COBOL COMP-3 packed BCD storage layout specification.

## 2. Structural Mapping
Numeric data is encoded with two binary-coded decimal (BCD) digits per byte. The last byte contains the final digit in its high nibble and the sign flag in its low nibble:
* **Positive Sign Representation:** Mapped to low-nibble `0xC` (standard) or `0xF`/`0xA`/`0xE`.
* **Negative Sign Representation:** Mapped to low-nibble `0xD` (standard) or `0xB`.
* **Padding:** Even digit counts are padded with a leading `0` in the first high nibble to maintain byte alignment.

## 3. Verified Implementations
* **Packing Solver:** [tsfi_s370_pack](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
* **Unpacking Solver:** [tsfi_s370_unpack](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
* **Arithmetic Units:** [tsfi_s370_packed_add](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
