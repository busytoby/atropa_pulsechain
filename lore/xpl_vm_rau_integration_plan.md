# XplOS VM RAU Integration Plan

This integration plan outlines embedding the **Stateless Coaxial RAU** backplane as the core register-routing foundation of the custom XplOS VM execution engine.

---

## 1. VM State Structure (`xpl_vm_context.h`)

The VM context maintains a unified linear memory page, with the coaxial register channels mapped to a dedicated block.

```c
#ifndef XPL_VM_CONTEXT_H
#define XPL_VM_CONTEXT_H

#include <stdint.h>

#define COAXIAL_BUS_BASE 0x4000
#define WMQ_BASE         0x4800
#define MEMORY_SIZE      0x10000 // 64KB Page

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint32_t active_window_offset; // Supports dynamic SPARC/MVS shifts
    uint32_t active_network_id;     // Active isolated coaxial network segment
} XplVMContext;

#endif
```

---

## 2. Register Resolution Interface (`xpl_rau.h`)

All register accesses (`V0` to `V31`) are compiled down to stateless pointer offsets on the active coaxial bus page.

```c
#ifndef XPL_RAU_H
#define XPL_RAU_H

#include "xpl_vm_context.h"
#include <string.h>

// Resolve register index to memory offset with window shifting
static inline uint32_t xpl_resolve_reg_offset(XplVMContext *ctx, uint32_t v_reg) {
    if (v_reg == 0) return 0; // V0 Invariant (handled at read/write boundary)
    
    // Remap index based on active register window offset
    uint32_t mapped_reg = (v_reg + ctx->active_window_offset) % 32;
    return COAXIAL_BUS_BASE + (mapped_reg * 32);
}

// Peek (Read) virtual register value
static inline uint64_t xpl_rau_peek(XplVMContext *ctx, uint32_t v_reg) {
    if (v_reg == 0) return 0; // V0 is hardwired to constant 0
    
    uint32_t offset = xpl_resolve_reg_offset(ctx, v_reg);
    uint64_t val = 0;
    memcpy(&val, &ctx->memory[offset], sizeof(uint64_t));
    return val;
}

// Poke (Write) virtual register value
static inline void xpl_rau_poke(XplVMContext *ctx, uint32_t v_reg, uint64_t val) {
    if (v_reg == 0) return; // Writes to V0 are no-ops
    
    uint32_t offset = xpl_resolve_reg_offset(ctx, v_reg);
    memcpy(&ctx->memory[offset], &val, sizeof(uint64_t));
}

#endif
```

---

## 3. Instruction Execution Integration

The VM interpreter loop bypasses local registers. Operators directly query the RAU backplane interface.

```c
#include "xpl_rau.h"

void xpl_execute_instruction(XplVMContext *ctx, uint8_t opcode, uint32_t reg_a, uint32_t reg_b, uint32_t reg_dest) {
    switch (opcode) {
        case 0x01: { // ADD reg_dest = reg_a + reg_b
            uint64_t val_a = xpl_rau_peek(ctx, reg_a);
            uint64_t val_b = xpl_rau_peek(ctx, reg_b);
            xpl_rau_poke(ctx, reg_dest, val_a + val_b);
            break;
        }
        case 0x02: { // MOV reg_dest = reg_a
            uint64_t val = xpl_rau_peek(ctx, reg_a);
            xpl_rau_poke(ctx, reg_dest, val);
            break;
        }
        default:
            break;
    }
}
```

---

## 4. WinchesterMQ Integration

Low-level register synchronization packs register data directly to the memory-mapped SCSI port offsets, routing the event over the active bus.

```c
static inline void xpl_rau_sync_core(XplVMContext *ctx, uint32_t target_core_id, uint32_t v_reg) {
    uint64_t val = xpl_rau_peek(ctx, v_reg);
    
    // Route to WinchesterMQ SCSI Ports
    memcpy(&ctx->memory[WMQ_BASE], &target_core_id, sizeof(uint32_t));
    memcpy(&ctx->memory[WMQ_BASE + 0x20], &v_reg, sizeof(uint32_t));
    memcpy(&ctx->memory[WMQ_BASE + 0x40], &val, sizeof(uint64_t));
}
```
