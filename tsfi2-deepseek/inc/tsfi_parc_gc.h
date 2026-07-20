#ifndef TSFI_PARC_GC_H
#define TSFI_PARC_GC_H

#include <stdint.h>
#include "tsfi_parc_smalltalk.h"

typedef struct {
    oop_t oop;
    int ref_count;
} tsfi_parc_gc_entry_t;

typedef struct {
    tsfi_parc_gc_entry_t table[128];
    int entry_count;
} tsfi_parc_gc_t;

/* Initialize Smalltalk reference-counting GC environment */
int tsfi_parc_gc_init(tsfi_parc_gc_t *gc);

/* Increment object reference count */
int tsfi_parc_gc_add_ref(tsfi_parc_gc_t *gc, oop_t oop);

/* Decrement object reference count and dynamically reclaim memory if it hits 0 */
int tsfi_parc_gc_release_ref(tsfi_parc_gc_t *gc, tsfi_parc_smalltalk_vm_t *vm, oop_t oop);

#endif // TSFI_PARC_GC_H
