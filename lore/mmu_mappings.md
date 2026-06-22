# ZMM Shared Virtual Memory (SVM) & MMU Register Mappings

This document details the exact memory register structures and coordination between the guest 6502 card cores and the host-side C zero-copy paging thunk system.

## 1. MMU Register Interface ($DF10–$DF13)

The guest virtual machine cards interface with the host-side Shared Virtual Memory (SVM) paging system using 4 memory-mapped I/O (MMIO) registers located in the MMIO page `0xDF`:

| Address | Symbolic Name  | Mode | Purpose / Description |
|---------|----------------|------|-----------------------|
| `$DF10` | `MMU_CMD`      | W    | Trigger command: `0x01` = Mount, `0x02` = Lock, `0x03` = Unlock |
| `$DF11` | `MMU_SRC_CARD` | R/W  | Source Card ID for zero-copy mounts |
| `$DF12` | `MMU_PAGE`     | R/W  | Target Page Index to mount/lock/unlock |
| `$DF13` | `MMU_STATUS`   | R    | Status return: `0` = Success, negative (unsigned byte) = Error (e.g., `254` for `-2`) |

---

## 2. Host-Side C Paging Thunks

The memory paging and mapping operations are performed entirely in host-side C memory thunks to avoid Yul/on-chain overhead and optimize execution performance.

### Paging Directory Structures
Located in [tsfi_zmm_svm.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tests/tsfi_zmm_svm.h):
```c
typedef struct {
    uint8_t* pages[256];
    uint8_t page_locks[256]; // 1 = Locked, 0 = Unlocked
} CardMemoryMap;
```

### Core Operations
1. **`tsfi_zmm_svm_mount(card_a, card_b, page_idx)`**:
   - Maps Card B's page pointer to point directly to Card A's physical buffer (zero-copy sharing).
   - Validates that Card B's page is not locked (`page_locks[page_idx] == 0`).
2. **`tsfi_zmm_svm_lock_page(card_id, page_idx)`**:
   - Sets the page lock bit, preventing any writes or mounting changes on this page index.
3. **`tsfi_zmm_svm_unlock_page(card_id, page_idx)`**:
   - Clears the lock bit, allowing standard writes and mountings.

---

## 3. 6502 Guest MMU Driver

The assembly driver [winchester_mmu_driver.asm](file:///home/mariarahel/.gemini/antigravity-cli/brain/53112525-9898-4973-a6fc-c208cc0d018f/scratch/winchester_mmu_driver.asm) exposes these subroutines:

### Subroutines

```assembly
; Input: A = Source Card ID, Y = Page Index
; Output: A = Status Code (0 on success)
MMU_MOUNT:
        STA MMU_SRC_CARD
        STY MMU_PAGE
        LDA #$01
        STA MMU_CMD
        LDA MMU_STATUS
        RTS

; Input: Y = Page Index
; Output: A = Status Code (0 on success)
MMU_LOCK:
        STY MMU_PAGE
        LDA #$02
        STA MMU_CMD
        LDA MMU_STATUS
        RTS

; Input: Y = Page Index
; Output: A = Status Code (0 on success)
MMU_UNLOCK:
        STY MMU_PAGE
        LDA #$03
        STA MMU_CMD
        LDA MMU_STATUS
        RTS
```

---

## 4. Context Switch Integration

When a context switch occurs, the paging table pointer modifications remain persistent inside the global `g_vm_memory` structures.
Because the pointers themselves are shared, any read/write updates to the mapped pages automatically propagate instantly between cards without needing re-mapping overhead during the process switch cycle.
