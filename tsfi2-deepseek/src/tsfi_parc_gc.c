#include "tsfi_parc_gc.h"

int tsfi_parc_gc_init(tsfi_parc_gc_t *gc) {
    if (!gc) return -1;
    gc->entry_count = 0;
    for (int i = 0; i < 128; i++) {
        gc->table[i].oop = 0;
        gc->table[i].ref_count = 0;
    }
    return 0;
}

int tsfi_parc_gc_add_ref(tsfi_parc_gc_t *gc, oop_t oop) {
    if (!gc || oop == 0) return -1;

    // Search for existing entry
    for (int i = 0; i < gc->entry_count; i++) {
        if (gc->table[i].oop == oop) {
            gc->table[i].ref_count++;
            return gc->table[i].ref_count;
        }
    }

    // Allocate new track entry
    if (gc->entry_count >= 128) return -2;
    int idx = gc->entry_count++;
    gc->table[idx].oop = oop;
    gc->table[idx].ref_count = 1;
    return 1;
}

int tsfi_parc_gc_release_ref(tsfi_parc_gc_t *gc, tsfi_parc_smalltalk_vm_t *vm, oop_t oop) {
    if (!gc || !vm || oop == 0) return -1;

    for (int i = 0; i < gc->entry_count; i++) {
        if (gc->table[i].oop == oop) {
            gc->table[i].ref_count--;
            if (gc->table[i].ref_count <= 0) {
                // Reclaim object from Smalltalk VM heap
                int target_idx = -1;
                for (int h = 0; h < vm->heap_count; h++) {
                    if (vm->heap[h].oop == oop) {
                        target_idx = h;
                        break;
                    }
                }
                if (target_idx != -1) {
                    // Shift heap items left to prune dead object
                    for (int h = target_idx; h < vm->heap_count - 1; h++) {
                        vm->heap[h] = vm->heap[h + 1];
                    }
                    vm->heap_count--;
                }
                // Clear table entry
                gc->table[i] = gc->table[gc->entry_count - 1];
                gc->entry_count--;
                return 0; // successfully garbage collected
            }
            return gc->table[i].ref_count;
        }
    }
    return -2; // object not tracked
}
