# System/370 Dynamic Address Translation (DAT) Paging Standard Certification

## 1. Specification Overview
This certification sheet documents compliance with the System/370 31-bit Dynamic Address Translation (DAT) virtual memory paging layout.

## 2. Paging Architecture
Virtual addresses are translated to physical memory spaces using hierarchical segment-page table lookups:
* **Segment Index (SX):** Virtual address bits 1..11 (11 bits) mapping to Segment Table entries.
* **Page Index (PX):** Virtual address bits 12..19 (8 bits) mapping to Page Table entries.
* **Byte Offset (BX):** Virtual address bits 20..31 (12 bits) indicating page offset.
* **TLB Acceleration:** Direct-mapped caching of virtual pages to bypass table walk latencies.

## 3. Verified Implementations
* **DAT Walk Translation:** [tsfi_s370_dat_translate](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
* **TLB Cached Translation:** [tsfi_s370_dat_translate_with_tlb](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
