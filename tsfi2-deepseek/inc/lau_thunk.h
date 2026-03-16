#ifndef LAU_THUNK_H
#define LAU_THUNK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "tsfi_dysnomia.h"
#include "tsfi_dai.h"

typedef struct {
    uint8_t *thunk_pool;
    uint8_t *thunk_cursor;
    size_t pool_size;
    int (*classify)(void *obj);
} ThunkProxy;

struct LauWiredHeader;
struct MappedCommon;

ThunkProxy* ThunkProxy_create(void);
void* ThunkProxy_emit_baked(ThunkProxy *p, void *fn, int argc, ...);
void* ThunkProxy_emit_baked_no_ret(ThunkProxy *p, void *fn, int argc, ...);
void* ThunkProxy_emit_ret(ThunkProxy *p);
void* ThunkProxy_emit_memset(ThunkProxy *p);
void* ThunkProxy_emit_mapped(ThunkProxy *p, struct MappedCommon *c, struct LauWiredHeader *h);

// --- Ontological Layering (Dysnomia Integration) ---
struct YI;
struct Dai;

// Embed the YI and DAI entities into the JIT assembly via immediate constants
void ThunkProxy_emit_ontological_structuring(ThunkProxy *p, struct YI *yi, struct Dai *dai);

// Extract the embedded YI and DAI pointers by disassembling the proxy pool
void ThunkProxy_extract_ontological_structuring(ThunkProxy *p, struct YI **yi_out, struct Dai **dai_out);

// Emit the Layer-1 seal, which dynamically allocates a DAI from the YI
// and bakes it into the execution pathway assembly.
void* ThunkProxy_emit_layer1_seal(ThunkProxy *p, void *target_fn);

void* ThunkProxy_emit_mixed(ThunkProxy *p, void *fn, void *baked_ptr);
void* ThunkProxy_emit_forwarding(ThunkProxy *p, void *fn, void *baked_ptr); 
void* ThunkProxy_emit_avx_setup(ThunkProxy *p, void *kernel_fn, void *sgpr_bank);
void* ThunkProxy_emit_vgpr_load(ThunkProxy *p, void *fn, void *ptr_to_ptr, int zmm_idx);
void* ThunkProxy_emit_vgpr_store(ThunkProxy *p, void *fn, void *ptr_to_ptr, int zmm_idx);
void* ThunkProxy_emit_vgpr_kernel_call(ThunkProxy *p, void *avx_kernel_fn, void *scalar_context_ptr);
void* ThunkProxy_emit_prefetch_l1(ThunkProxy *p, void *ptr_to_ptr);
void* ThunkProxy_emit_prefetch_l2(ThunkProxy *p, void *ptr_to_ptr);
void* ThunkProxy_emit_prefetch_l3(ThunkProxy *p, void *ptr_to_ptr);
void* ThunkProxy_emit_stream_store(ThunkProxy *p, void *fn, void *ptr_to_ptr, int zmm_idx);

// Zero-Copy Switching: Context-Relative Access (R15 + secret)
void* ThunkProxy_emit_vgpr_load_relative(ThunkProxy *p, int secret_bytes, int zmm_idx);
void* ThunkProxy_emit_vgpr_store_relative(ThunkProxy *p, int secret_bytes, int zmm_idx);

// Helper to set R15 and call a target function
void* ThunkProxy_emit_set_r15_and_call(ThunkProxy *p, void *target_fn, void *r15_val);

// Internal Helper (Exposed for extension thunks)
void thunk_check_bounds(ThunkProxy *p, size_t needed);

// ZMM Specialized Thunks
void* ThunkProxy_emit_zmm_density_op(ThunkProxy *p, int source_zmm); // VADDPS (Accum)
void* ThunkProxy_emit_zmm_fill(ThunkProxy *p, void *target_ptr, float value);
void* ThunkProxy_emit_zmm_spill(ThunkProxy *p, void *buffer, int secret_bytes, int zmm_idx);
void* ThunkProxy_emit_zmm_fill_slot(ThunkProxy *p, void *buffer, int secret_bytes, int zmm_idx);

// Pure WAVE Arithmetic Emitters
void* ThunkProxy_emit_wave_vmulps(ThunkProxy *p, int zmm_dest, int zmm_src1, int zmm_src2);
void* ThunkProxy_emit_wave_vsubps(ThunkProxy *p, int zmm_dest, int zmm_src1, int zmm_src2);
void* ThunkProxy_emit_wave_vfmadd213ps(ThunkProxy *p, int zmm_dest, int zmm_src1, int zmm_src2);
void* ThunkProxy_emit_wave_vmaxps(ThunkProxy *p, int zmm_dest, int zmm_src1, int zmm_src2);

// Wave Lifecycle: Custom teardown with surgical leak neutralization
void* ThunkProxy_emit_wave_dlopen(ThunkProxy *p, void *dlopen_fn);
void* ThunkProxy_emit_wave_dlsym(ThunkProxy *p, void *dlsym_fn);
void* ThunkProxy_emit_wave_dlclose(ThunkProxy *p, void *handle, void *teardown_fn, int num_surgical_strikes, void **ptr_locations, void *header_addr, size_t header_size);

// Hardware-Synced Orchestration (Zhong Timeline Integration)
// Emits a thunk that reads a 64-bit value from *sem_addr and compares it to target_val.
// If current < target, the thunk exits (RET) early, signaling a wait-free yield.
void* ThunkProxy_emit_timeline_guard(ThunkProxy *p, uint64_t *sem_addr, uint64_t target_val);

// Visual Logic Emitters
void* ThunkProxy_emit_video_export(ThunkProxy *p, void *pixels_ptr, size_t size_bytes, int *frame_counter, int loop_limit);

// Parallel ANSI BASE Parser (AVX-512)
// Loads 64 bytes from src_ptr, stores to BASE_ptr via secret offset.
void* ThunkProxy_emit_ansi_parser(ThunkProxy *p, int BASE_secret_bytes);

// Helmholtz Wavefront Polling (Bypasses select/poll)
void* ThunkProxy_emit_io_poll(ThunkProxy *p, int count);

// Deterministic Access Tracking (Replaces Bloom)
// Sets bit (leaf_idx / 4) in a 512-bit bitmask (64 bytes)
void* ThunkProxy_emit_access_mark(ThunkProxy *p, uint8_t *mask_ptr, int leaf_idx);
void* ThunkProxy_emit_access_mark_no_ret(ThunkProxy *p, uint8_t *mask_ptr, int leaf_idx);

// Plier Foundation (Seal-2 Checksum/Integrity)
void* ThunkProxy_emit_plier_foundation(ThunkProxy *p, void *payload, size_t size);
void* ThunkProxy_emit_yi_accessor(ThunkProxy *p);
void* ThunkProxy_emit_yi_compare(ThunkProxy *p);

// Relever (Step 3): Stack Alignment Guard
void* ThunkProxy_emit_relever_alignment(ThunkProxy *p, void *target_fn);

// NEW: Simplified C/JIT Hybrid Safety Chain
void* ThunkProxy_emit_safety_chain(ThunkProxy *p, bool (*state_fn)(void*), void (*epoch_fn)(void*), void *wt);

void ThunkProxy_rebind(void *thunk_ptr, void *new_target);
void ThunkProxy_seal(ThunkProxy *p);
void ThunkProxy_unseal(ThunkProxy *p);
void ThunkProxy_destroy(ThunkProxy *p);

#endif // LAU_THUNK_H