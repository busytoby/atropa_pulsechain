#include "lau_thunk.h"
#include "lau_memory.h"
#include "tsfi_resonance.h"
#include "tsfi_io.h"
#include "lau_audit.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>
#include "tsfi_hotloader.h"

#if !defined(__x86_64__)
#error "LAU Thunk JIT is only supported on x86_64 architectures."
#endif

typedef uint64_t u64_u __attribute__((aligned(1)));
typedef uint32_t u32_u __attribute__((aligned(1)));
typedef uint16_t u16_u __attribute__((aligned(1)));

_Thread_local int g_in_thunk_create = 0;

ThunkProxy* ThunkProxy_create(void) {
    size_t pg = sysconf(_SC_PAGESIZE);
    ThunkProxy *proxy = (ThunkProxy *)lau_malloc(sizeof(ThunkProxy));
    if (!proxy) return NULL;
    
    g_in_thunk_create = 1;
    // Use WIRED to ensure hardware tracking and correct sealing integration
    proxy->thunk_pool = (uint8_t *)lau_memalign_wired(pg, pg);
    g_in_thunk_create = 0;
    
    if (!proxy->thunk_pool) { lau_free(proxy); return NULL; }
    
    // Link the thunk pool's wired header back to this proxy for correct sealing integration
    extern LauWiredHeader* get_wired_header_external(void *payload);
    LauWiredHeader *h = get_wired_header_external(proxy->thunk_pool);
    if (h) h->proxy = proxy;
    
    // Initial State: RWX for generation.
    lau_mprotect(proxy->thunk_pool, PROT_READ | PROT_WRITE | PROT_EXEC);
    
    memset(proxy->thunk_pool, 0, pg);
    proxy->thunk_cursor = proxy->thunk_pool;
    proxy->pool_size = pg;
    return proxy;
}

void thunk_check_bounds(ThunkProxy *p, size_t needed) {
    if ((size_t)(p->thunk_cursor - p->thunk_pool) + needed > p->pool_size) {
        fprintf(stderr, "[LAU_THUNK] FATAL: JIT Buffer Overflow!\n");
        abort();
    }
}

static void* emit_baked_core(ThunkProxy *p, void *fn, bool is_call, int argc, va_list args) {
    thunk_check_bounds(p, (argc * 12) + 12 + 2);
    uint8_t *meta = p->thunk_cursor; p->thunk_cursor += 2; 
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0xf3; *c++ = 0x0f; *c++ = 0x1e; *c++ = 0xfa; // ENDBR64
    for (int i = 0; i < argc; i++) {
        void *val = va_arg(args, void*);
        if (i < 4) *c++ = 0x48; else *c++ = 0x49;
        switch (i) {
            case 0: *c++ = 0xbf; break;
            case 1: *c++ = 0xbe; break;
            case 2: *c++ = 0xba; break;
            case 3: *c++ = 0xb9; break;
            case 4: *c++ = 0xb8; break;
            case 5: *c++ = 0xb9; break;
        }
        *(u64_u*)c = (uintptr_t)val; c += 8;
    }
    *c++ = 0x48; *c++ = 0xb8; uint8_t *ptr_slot = c;
    *(u64_u*)c = (uintptr_t)fn; c += 8;
    if (is_call) { *c++ = 0xff; *c++ = 0xd0; } else { *c++ = 0xff; *c++ = 0xe0; }
    *(u16_u*)meta = (uint16_t)(ptr_slot - (uint8_t*)start);
    p->thunk_cursor = c;
    __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_baked(ThunkProxy *p, void *fn, int argc, ...) {
    va_list args; va_start(args, argc);
    void* res = emit_baked_core(p, fn, false, argc, args);
    va_end(args); return res;
}

void* ThunkProxy_emit_baked_no_ret(ThunkProxy *p, void *fn, int argc, ...) {
    va_list args; va_start(args, argc);
    void* res = emit_baked_core(p, fn, true, argc, args);
    va_end(args); return res;
}

void* ThunkProxy_emit_mixed(ThunkProxy *p, void *fn, void *baked_ptr) {
    thunk_check_bounds(p, 64 + 2); 
    uint8_t *meta = p->thunk_cursor; p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xfe; *c++ = 0x48; *c++ = 0xbf; 
    *(u64_u*)c = (uintptr_t)baked_ptr; c += 8;
    *c++ = 0x48; *c++ = 0xb8; 
    *(u16_u*)meta = (uint16_t)(c - (uint8_t*)start);
    *(u64_u*)c = (uintptr_t)fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_forwarding(ThunkProxy *p, void *fn, void *baked_ptr) {
    thunk_check_bounds(p, 128 + 2); 
    uint8_t *meta = p->thunk_cursor; p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x4d; *c++ = 0x89; *c++ = 0xc1; *c++ = 0x49; *c++ = 0x89; *c++ = 0xc8; 
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xd1; *c++ = 0x48; *c++ = 0x89; *c++ = 0xf2; 
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xfe; *c++ = 0x48; *c++ = 0xbf; 
    *(u64_u*)c = (uintptr_t)baked_ptr; c += 8;
    *c++ = 0x48; *c++ = 0xb8; 
    *(u16_u*)meta = (uint16_t)(c - (uint8_t*)start);
    *(u64_u*)c = (uintptr_t)fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_avx_setup(ThunkProxy *p, void *kernel_fn, void *sgpr_bank) {
    thunk_check_bounds(p, 128 + 2); 
    uint8_t *meta = p->thunk_cursor; p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)sgpr_bank; c += 8;
    *c++ = 0x49; *c++ = 0x89; *c++ = 0xc7;
    *c++ = 0x48; *c++ = 0xb8; uint8_t *ptr_slot = c;
    *(u64_u*)c = (uintptr_t)kernel_fn; c += 8;
    *(u16_u*)meta = (uint16_t)(ptr_slot - (uint8_t*)start);
    *c++ = 0xff; *c++ = 0xe0;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_vgpr_load(ThunkProxy *p, void *fn, void *ptr_to_ptr, int zmm_idx) {
    (void)fn; thunk_check_bounds(p, 32 + 2);
    p->thunk_cursor += 2; 
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)ptr_to_ptr; c += 8;
    *c++ = 0x48; *c++ = 0x8b; *c++ = 0x00;
    *c++ = 0x62; *c++ = 0xf1; *c++ = 0x7c; *c++ = 0x48; *c++ = 0x10; *c++ = (uint8_t)(0x00 | (zmm_idx << 3)); 
    *c++ = 0xc3; 
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_vgpr_store(ThunkProxy *p, void *fn, void *ptr_to_ptr, int zmm_idx) {
    (void)fn; thunk_check_bounds(p, 32 + 2);
    p->thunk_cursor += 2; 
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)ptr_to_ptr; c += 8;
    *c++ = 0x48; *c++ = 0x8b; *c++ = 0x00;
    *c++ = 0x62; *c++ = 0xf1; *c++ = 0x7c; *c++ = 0x48; *c++ = 0x11; *c++ = (uint8_t)(0x00 | (zmm_idx << 3)); 
    *c++ = 0xc3; 
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_vgpr_kernel_call(ThunkProxy *p, void *avx_kernel_fn, void *scalar_context_ptr) {
    thunk_check_bounds(p, 32 + 2);
    uint8_t *meta = p->thunk_cursor; p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x48; *c++ = 0xbf; *(u64_u*)c = (uintptr_t)scalar_context_ptr; c += 8;
    *c++ = 0x48; *c++ = 0xb8; uint8_t *ptr_slot = c;
    *(u64_u*)c = (uintptr_t)avx_kernel_fn; c += 8;
    *c++ = 0xff; *c++ = 0xd0; *c++ = 0xc3; 
    *(u16_u*)meta = (uint16_t)(ptr_slot - (uint8_t*)start);
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_vgpr_load_relative(ThunkProxy *p, int secret_bytes, int zmm_idx) {
    thunk_check_bounds(p, 32 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x62; uint8_t p1 = 0xf1; if (zmm_idx >= 8) p1 &= ~0x80; p1 &= ~0x20; 
    *c++ = p1; *c++ = 0x7c; *c++ = 0x48; *c++ = 0x10; 
    *c++ = (uint8_t)(0x87 | ((zmm_idx & 7) << 3)); 
    *(u32_u*)c = (uint32_t)secret_bytes; c += 4; *c++ = 0xc3; 
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_vgpr_store_relative(ThunkProxy *p, int secret_bytes, int zmm_idx) {
    thunk_check_bounds(p, 32 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x62; uint8_t p1 = 0xf1; if (zmm_idx >= 8) p1 &= ~0x80; p1 &= ~0x20; 
    *c++ = p1; *c++ = 0x7c; *c++ = 0x48; *c++ = 0x11; 
    *c++ = (uint8_t)(0x87 | ((zmm_idx & 7) << 3)); 
    *(u32_u*)c = (uint32_t)secret_bytes; c += 4; *c++ = 0xc3; 
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_set_r15_and_call(ThunkProxy *p, void *target_fn, void *r15_val) {
    thunk_check_bounds(p, 32 + 2);
    uint8_t *meta = p->thunk_cursor; p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x49; *c++ = 0xbf; *(u64_u*)c = (uintptr_t)r15_val; c += 8;
    *c++ = 0x48; *c++ = 0xb8; uint8_t *ptr_slot = c;
    *(u64_u*)c = (uintptr_t)target_fn; c += 8; *c++ = 0xff; *c++ = 0xe0;
    *(u16_u*)meta = (uint16_t)(ptr_slot - (uint8_t*)start);
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_zmm_density_op(ThunkProxy *p, int source_zmm) {
    thunk_check_bounds(p, 64 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x62; *c++ = 0xf1; *c++ = 0x7c; *c++ = 0x48; *c++ = 0x58; *c++ = (uint8_t)(0xc0 | (source_zmm << 0)); 
    *c++ = 0xc3; p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_zmm_fill(ThunkProxy *p, void *target_ptr, float value) {
    p->thunk_cursor = (uint8_t*)(((uintptr_t)p->thunk_cursor + 15) & ~15);
    thunk_check_bounds(p, 64 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)target_ptr; c += 8;
    union { float f; uint32_t u; } u; u.f = value;
    *c++ = 0x41; *c++ = 0xba; *(u32_u*)c = u.u; c += 4;
    *c++ = 0xc4; *c++ = 0xc1; *c++ = 0x79; *c++ = 0x6e; *c++ = 0xc2; 
    *c++ = 0x62; *c++ = 0xf2; *c++ = 0x7d; *c++ = 0x48; *c++ = 0x18; *c++ = 0xc0; 
    *c++ = 0x62; *c++ = 0xf1; *c++ = 0x7c; *c++ = 0x48; *c++ = 0x11; *c++ = 0x00; *c++ = 0xc3;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_zmm_spill(ThunkProxy *p, void *buffer, int secret_bytes, int zmm_idx) {
    thunk_check_bounds(p, 32 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    uint8_t *addr = (uint8_t*)buffer + secret_bytes;
    *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)addr; c += 8;
    *c++ = 0x62; *c++ = 0xf1; *c++ = 0x7c; *c++ = 0x48; *c++ = 0x2b; *c++ = (uint8_t)(0x00 | (zmm_idx << 3));
    *c++ = 0xc3; p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_zmm_fill_slot(ThunkProxy *p, void *buffer, int secret_bytes, int zmm_idx) {
    thunk_check_bounds(p, 32 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    uint8_t *addr = (uint8_t*)buffer + secret_bytes;
    *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)addr; c += 8;
    *c++ = 0x62; *c++ = 0xf1; *c++ = 0x7c; *c++ = 0x48; *c++ = 0x10; *c++ = (uint8_t)(0x00 | (zmm_idx << 3));
    *c++ = 0xc3; p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>
#endif

__attribute__((no_sanitize("address")))
void ThunkProxy_rebind(void *thunk_ptr, void *new_target) {
    if (!thunk_ptr || !new_target) return;
    uint8_t *p = (uint8_t*)thunk_ptr; uint16_t secret = *(u16_u*)(p - 2);
    if (secret > 256) return;
    
    size_t pg = sysconf(_SC_PAGESIZE);
    uintptr_t addr = (uintptr_t)p & ~(pg - 1);

    // Direct Patching Model: Bypass lau_mprotect to avoid structural recursion/latency
    mprotect((void*)addr, pg, PROT_READ | PROT_WRITE | PROT_EXEC);
    *(u64_u*)(p + secret) = (uintptr_t)new_target;
    __builtin___clear_cache((char*)(p + secret), (char*)(p + secret + 8));
}

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

void* ThunkProxy_emit_wave_dlopen(ThunkProxy *p, void *dlopen_fn) {
    thunk_check_bounds(p, 32 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    void *actual_fn = dlopen_fn ? dlopen_fn : dlsym(RTLD_DEFAULT, "dlopen");
    *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)actual_fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0; p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_wave_dlsym(ThunkProxy *p, void *dlsym_fn) {
    thunk_check_bounds(p, 32 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    void *actual_fn = dlsym_fn ? dlsym_fn : dlsym(RTLD_DEFAULT, "dlsym");
    *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)actual_fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0; p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_wave_dlclose(ThunkProxy *p, void *hdl, void *td_fn, int strikes, void **ptrs, void *hdr, size_t sz) {
    thunk_check_bounds(p, (strikes * 48) + 160 + 2); p->thunk_cursor += 2;
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    void *act_td = td_fn ? td_fn : dlsym(RTLD_DEFAULT, "dlclose");
    void *mm_fn = dlsym(RTLD_DEFAULT, "munmap");
    *c++ = 0x55; *c++ = 0x48; *c++ = 0x89; *c++ = 0xe5; *c++ = 0x48; *c++ = 0x83; *c++ = 0xe4; *c++ = 0xf0;
    for (int i = 0; i < strikes; i++) {
        *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)ptrs[i]; c += 8;
        *c++ = 0x48; *c++ = 0x8b; *c++ = 0x38; *c++ = 0x48; *c++ = 0x85; *c++ = 0xff;
        *c++ = 0x75; *c++ = 0x02; *c++ = 0xeb; *c++ = 0x1b;
        *c++ = 0x48; *c++ = 0xc7; *c++ = 0xc6; *(u32_u*)c = 96; c += 4;
        *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)mm_fn; c += 8; *c++ = 0xff; *c++ = 0xd0;
        *c++ = 0x48; *c++ = 0xbf; *(u64_u*)c = (uintptr_t)ptrs[i]; c += 8;
        *c++ = 0x48; *c++ = 0x31; *c++ = 0xc0; *c++ = 0x48; *c++ = 0x89; *c++ = 0x07;
    }
    if (hdl && act_td) {
        *c++ = 0x48; *c++ = 0xbf; *(u64_u*)c = (uintptr_t)hdl; c += 8;
        *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)act_td; c += 8; *c++ = 0xff; *c++ = 0xd0;
    }
    if (hdr && sz > 0) {
        *c++ = 0x48; *c++ = 0xbf; *(u64_u*)c = (uintptr_t)hdr; c += 8;
        *c++ = 0x48; *c++ = 0xbe; *(u64_u*)c = (uintptr_t)sz; c += 8;
        *c++ = 0x48; *c++ = 0xb8; *(u64_u*)c = (uintptr_t)mm_fn; c += 8; *c++ = 0xff; *c++ = 0xd0;
    }
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xec; *c++ = 0x5d; *c++ = 0xc3;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void ThunkProxy_destroy_authoritative(ThunkProxy *p) {
    if (!p) return;
    if (p->thunk_pool) {
        lau_mprotect(p->thunk_pool, PROT_READ | PROT_WRITE);
        
        // Break recursion: clear the proxy link in the pool's header before freeing the pool
        extern LauWiredHeader* get_wired_header_external(void *payload);
        LauWiredHeader *h = get_wired_header_external(p->thunk_pool);
        if (h && h->proxy == p) h->proxy = NULL;

        void *pool = p->thunk_pool;
        p->thunk_pool = NULL;
        extern void lau_free_authoritative(void *ptr);
        lau_free_authoritative(pool);
    }
    extern void lau_free_authoritative(void *ptr);
    lau_free_authoritative(p);
}

void ThunkProxy_destroy(ThunkProxy *p) {
    if (!p) return;
    
    // 1. Restore write access FIRST to allow breaking ontological links
    if (p->thunk_pool) {
        lau_mprotect(p->thunk_pool, PROT_READ | PROT_WRITE);
        
        // 2. Break ontological links
        extern LauWiredHeader* get_wired_header_external(void *payload);
        LauWiredHeader *h = get_wired_header_external(p->thunk_pool);
        if (h && h->proxy == p) {
            h->proxy = NULL;
        }
        
        // 3. Reclaim pool
        void *pool = p->thunk_pool;
        p->thunk_pool = NULL;
        lau_free(pool);
    }

    // 4. Reclaim proxy structure
    lau_free(p);
}

void* ThunkProxy_emit_timeline_guard(ThunkProxy *p, uint64_t *sem_addr, uint64_t target_val) {
    thunk_check_bounds(p, 32);
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x48; *c++ = 0xb8; *(uint64_t**)c = sem_addr; c += 8;
    *c++ = 0x48; *c++ = 0x8b; *c++ = 0x00; *c++ = 0x48; *c++ = 0xba; *(uint64_t*)c = target_val; c += 8;
    *c++ = 0x48; *c++ = 0x39; *c++ = 0xd0; *c++ = 0x73; *c++ = 0x01; *c++ = 0xc3;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_ansi_parser(ThunkProxy *p, int secret_bytes) {
    thunk_check_bounds(p, 64); uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x62; *c++ = 0xf1; *c++ = 0xfe; *c++ = 0x48; *c++ = 0x6f; *c++ = 0x07;
    *c++ = 0x62; *c++ = 0xd1; *c++ = 0xfe; *c++ = 0x48; *c++ = 0x7f; *c++ = 0x87;
    *(u32_u*)c = (uint32_t)secret_bytes; c += 4; *c++ = 0xc3;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_ret(ThunkProxy *p) {
    thunk_check_bounds(p, 2); uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0xc3; p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_memset(ThunkProxy *p) {
    thunk_check_bounds(p, 128); 
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    // x86_64: RDI=dest, RSI=val, RDX=count
    *c++ = 0x55;                     // push rbp
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xe5; // mov rbp, rsp
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xf8; // mov rax, rdi (return value)
    
    // Broadcast byte (ESI) to ZMM0
    *c++ = 0x40; *c++ = 0x0f; *c++ = 0xb6; *c++ = 0xf6; // movzx esi, sil
    *c++ = 0x62; *c++ = 0xf2; *c++ = 0x7d; *c++ = 0x48; *c++ = 0x7c; *c++ = 0xc6; // vpbroadcastb zmm0, esi
    
    // Main loop: 64-byte blocks
    uint8_t *loop_head = c;
    *c++ = 0x48; *c++ = 0x81; *c++ = 0xfa; *c++ = 0x40; *c++ = 0x00; *c++ = 0x00; *c++ = 0x00; // cmp rdx, 64
    *c++ = 0x72; *c++ = 0x0e;                         // jb .small
    
    // vmovdqu64 [rdi], zmm0 (AVX-512)
    *c++ = 0x62; *c++ = 0xf1; *c++ = 0x7f; *c++ = 0x48; *c++ = 0x7f; *c++ = 0x07;
    *c++ = 0x48; *c++ = 0x83; *c++ = 0xc7; *c++ = 0x40; // add rdi, 64
    *c++ = 0x48; *c++ = 0x83; *c++ = 0xea; *c++ = 0x40; // sub rdx, 64
    uint8_t rel = (uint8_t)(loop_head - (c + 2));
    *c++ = 0xeb; *c++ = rel; // jmp .loop
    
    // .small: Handle remaining 0-63 bytes with rep stosb
    *c++ = 0x48; *c++ = 0x85; *c++ = 0xd2;             // test rdx, rdx
    *c++ = 0x74; *c++ = 0x07;                         // jz .done
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xd1;             // mov rcx, rdx
    *c++ = 0x89; *c++ = 0xf0;                         // mov eax, esi
    *c++ = 0xf3; *c++ = 0xaa;                         // rep stosb
    
    // .done:
    *c++ = 0x5d;                                     // pop rbp
    *c++ = 0xc3;                                     // ret
    
    p->thunk_cursor = c;
    __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

extern void tsfi_video_write_frame(void *pixels, size_t size);

void* ThunkProxy_emit_video_export(ThunkProxy *p, void *px_ptr, size_t sz, int *f_cnt, int limit) {
    thunk_check_bounds(p, 128); uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x55; *c++ = 0x48; *c++ = 0x89; *c++ = 0xe5; *c++ = 0x48; *c++ = 0xb8; *(uint64_t**)c = (uint64_t*)f_cnt; c += 8;
    *c++ = 0x8b; *c++ = 0x00; *c++ = 0x3d; *(uint32_t*)c = (uint32_t)limit; c += 4; *c++ = 0x7d; *c++ = 0x1a; 
    *c++ = 0x48; *c++ = 0xbf; *(uint64_t*)c = (uintptr_t)px_ptr; c += 8;
    *c++ = 0x48; *c++ = 0xbe; *(uint64_t*)c = (uint64_t)sz; c += 8;
    *c++ = 0x48; *c++ = 0xb8; *(uint64_t*)c = (uintptr_t)tsfi_video_write_frame; c += 8;
    *c++ = 0xff; *c++ = 0xd0; *c++ = 0x48; *c++ = 0xb8; *(uint64_t**)c = (uint64_t*)f_cnt; c += 8;
    *c++ = 0xf0; *c++ = 0xff; *c++ = 0x00; *c++ = 0x48; *c++ = 0x89; *c++ = 0xec; *c++ = 0x5d; *c++ = 0xc3;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_io_poll(ThunkProxy *p, int count) {
    (void)count;
    thunk_check_bounds(p, 256);
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    
    *c++ = 0x55;                     // push rbp
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xe5; // mov rbp, rsp
    
    // .loop:
    uint8_t *loop_head = c;
    *c++ = 0x4d; *c++ = 0x31; *c++ = 0xc0; // xor r8, r8 (index = 0)
    
    // .check_one:
    uint8_t *check_head = c;
    *c++ = 0x4c; *c++ = 0x39; *c++ = 0xc2; // cmp rdx, r8 (count vs index)
    uint8_t *jle_done = c;
    *c++ = 0x7e; *c++ = 0;                 // jle .done_check
    
    *c++ = 0x4e; *c++ = 0x8b; *c++ = 0x0c; *c++ = 0xc7; // mov r9, [rdi + r8*8]
    *c++ = 0x45; *c++ = 0x8b; *c++ = 0x11;             // mov r10d, [r9]
    *c++ = 0x46; *c++ = 0x3b; *c++ = 0x14; *c++ = 0x86; // cmp r10d, [rsi + r8*4] (REX.X=1 for R8)
    uint8_t *jne_found = c;
    *c++ = 0x75; *c++ = 0;                         // jne .found
    
    *c++ = 0x49; *c++ = 0xff; *c++ = 0xc0;             // inc r8
    uint8_t rel_c = (uint8_t)(check_head - (c + 2));
    *c++ = 0xeb; *c++ = rel_c;                         // jmp .check_one
    
    // .done_check:
    uint8_t *done_check_ptr = c;
    jle_done[1] = (uint8_t)(done_check_ptr - (jle_done + 2));
    
    *c++ = 0xf3; *c++ = 0x90;                         // pause
    *c++ = 0x48; *c++ = 0xff; *c++ = 0xc9;             // dec rcx
    uint8_t rel_l = (uint8_t)(loop_head - (c + 2));
    *c++ = 0x75; *c++ = rel_l;                         // jnz .loop
    
    *c++ = 0x48; *c++ = 0xc7; *c++ = 0xc0;             // mov rax, -1
    *c++ = 0xff; *c++ = 0xff; *c++ = 0xff; *c++ = 0xff;
    *c++ = 0x5d; *c++ = 0xc3;                         // pop rbp, ret
    
    // .found:
    uint8_t *found_ptr = c;
    jne_found[1] = (uint8_t)(found_ptr - (jne_found + 2));
    
    *c++ = 0x4c; *c++ = 0x89; *c++ = 0xc0;             // mov rax, r8
    *c++ = 0x5d; *c++ = 0xc3;                         // pop rbp, ret
    
    p->thunk_cursor = c;
    __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_plier_foundation(ThunkProxy *p, void *payload, size_t size) {
    thunk_check_bounds(p, 256);
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    
    // Function signature: int plier_strike(void)
    // Returns 0 if foundation is steady, 1 if fractured.
    
    *c++ = 0x55;                     // push rbp
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xe5; // mov rbp, rsp
    
    // We will perform a simple 64-bit XOR sum of the payload as a "Plier Strike"
    // RDI = payload, RSI = size
    *c++ = 0x48; *c++ = 0xbf; *(uint64_t*)c = (uintptr_t)payload; c += 8;
    *c++ = 0x48; *c++ = 0xbe; *(uint64_t*)c = (uint64_t)size; c += 8;
    
    *c++ = 0x48; *c++ = 0x31; *c++ = 0xc0; // xor rax, rax (sum)
    *c++ = 0x48; *c++ = 0x31; *c++ = 0xc9; // xor rcx, rcx (index)
    
    // .loop:
    uint8_t *loop_head = c;
    *c++ = 0x48; *c++ = 0x39; *c++ = 0xf1; // cmp rsi, rcx
    *c++ = 0x7e; *c++ = 0x08;             // jle .done
    
    *c++ = 0x48; *c++ = 0x33; *c++ = 0x04; *c++ = 0x0f; // xor rax, [rdi + rcx]
    *c++ = 0x48; *c++ = 0x83; *c++ = 0xc1; *c++ = 0x08; // add rcx, 8
    uint8_t rel_l = (uint8_t)(loop_head - (c + 2));
    *c++ = 0xeb; *c++ = rel_l;                         // jmp .loop
    
    // .done:
    // Foundation is steady if sum matches expected (simplified for now to just returning sum)
    // Real implementation would compare against a stored secret.
    *c++ = 0x5d; *c++ = 0xc3;             // pop rbp, ret
    
    p->thunk_cursor = c;
    __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_yi_accessor(ThunkProxy *p) {
    thunk_check_bounds(p, 64);
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    
    struct YI *yi = NULL;
    ThunkProxy_extract_ontological_structuring(p, &yi, NULL);
    if (!yi) return NULL;

    *c++ = 0x48; *c++ = 0xb8; *(uint64_t*)c = (uintptr_t)yi; c += 8; // mov rax, imm64 (yi)
    *c++ = 0xc3;                                                 // ret
    
    p->thunk_cursor = c;
    __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_yi_compare(ThunkProxy *p) {
    thunk_check_bounds(p, 256);
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;

    *c++ = 0x55;                     // push rbp
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xe5; // mov rbp, rsp
    *c++ = 0x41; *c++ = 0x54;         // push r12
    *c++ = 0x41; *c++ = 0x55;         // push r13

    *c++ = 0x49; *c++ = 0x89; *c++ = 0xfc; // mov r12, rdi
    *c++ = 0x49; *c++ = 0x89; *c++ = 0xf5; // mov r13, rsi

    extern int tsfi_bn_cmp_avx512(const TSFiBigInt *a, const TSFiBigInt *b);
    uintptr_t cmp_fn = (uintptr_t)tsfi_bn_cmp_avx512;

    // 1. Compare Xi (offset 8)
    *c++ = 0x49; *c++ = 0x8b; *c++ = 0x7c; *c++ = 0x24; *c++ = 0x08; // mov rdi, [r12+8]
    *c++ = 0x49; *c++ = 0x8b; *c++ = 0x75; *c++ = 0x08;             // mov rsi, [r13+8]
    *c++ = 0x48; *c++ = 0xb8; *(uint64_t*)c = cmp_fn; c += 8;      // mov rax, cmp_fn
    *c++ = 0xff; *c++ = 0xd0;                                     // call rax
    *c++ = 0x48; *c++ = 0x85; *c++ = 0xc0;                         // test rax, rax
    *c++ = 0x75; *c++ = 31;                                        // jne .not_equal (31 bytes forward)

    // 2. Compare Ring (offset 16)
    *c++ = 0x49; *c++ = 0x8b; *c++ = 0x7c; *c++ = 0x24; *c++ = 0x10; // mov rdi, [r12+16]
    *c++ = 0x49; *c++ = 0x8b; *c++ = 0x75; *c++ = 0x10;             // mov rsi, [r13+16]
    *c++ = 0x48; *c++ = 0xb8; *(uint64_t*)c = cmp_fn; c += 8;      // mov rax, cmp_fn
    *c++ = 0xff; *c++ = 0xd0;                                     // call rax
    *c++ = 0x48; *c++ = 0x85; *c++ = 0xc0;                         // test rax, rax
    *c++ = 0x75; *c++ = 5;                                         // jne .not_equal (5 bytes forward)

    // Equal
    *c++ = 0x48; *c++ = 0x31; *c++ = 0xc0; // xor rax, rax
    *c++ = 0xeb; *c++ = 0x07;             // jmp .done

    // .not_equal:
    *c++ = 0x48; *c++ = 0xc7; *c++ = 0xc0; // mov rax, 1
    *c++ = 0x01; *c++ = 0x00; *c++ = 0x00; *c++ = 0x00;

    // .done:
    *c++ = 0x41; *c++ = 0x5d;         // pop r13
    *c++ = 0x41; *c++ = 0x5c;         // pop r12
    *c++ = 0x5d; *c++ = 0xc3;         // pop rbp, ret

    p->thunk_cursor = c;
    __builtin___clear_cache((char*)start, (char*)c);
    return start;
}
void* ThunkProxy_emit_relever_alignment(ThunkProxy *p, void *target_fn) {
    thunk_check_bounds(p, 128);
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    
    *c++ = 0x55;                     // push rbp
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xe5; // mov rbp, rsp
    
    *c++ = 0x53;                     // push rbx
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xe3; // mov rbx, rsp
    
    // Aligns RSP such that (RSP + 8) is 64-byte aligned
    // 1. sub rsp, 8
    *c++ = 0x48; *c++ = 0x83; *c++ = 0xec; *c++ = 0x08;
    // 2. and rsp, -64
    *c++ = 0x48; *c++ = 0x81; *c++ = 0xe4;
    *c++ = 0xc0; *c++ = 0xff; *c++ = 0xff; *c++ = 0xff;
    // 3. add rsp, 8 (This restores the +8 offset required by ABI)
    *c++ = 0x48; *c++ = 0x83; *c++ = 0xc4; *c++ = 0x08;
    
    // movabs rax, target_fn
    *c++ = 0x48; *c++ = 0xb8;
    *(uint64_t*)c = (uintptr_t)target_fn; c += 8;
    
    // call rax
    *c++ = 0xff; *c++ = 0xd0;
    
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xdc; // mov rsp, rbx
    *c++ = 0x5b;                     // pop rbx
    *c++ = 0x5d; *c++ = 0xc3;         // pop rbp, ret
    
    p->thunk_cursor = c;
    __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_safety_chain(ThunkProxy *p, bool (*state_fn)(void*), void (*epoch_fn)(void*), void *wt) {
    thunk_check_bounds(p, 128); p->thunk_cursor = (uint8_t*)(((uintptr_t)p->thunk_cursor + 15) & ~15);
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0xf3; *c++ = 0x0f; *c++ = 0x1e; *c++ = 0xfa;
    *c++ = 0x48; *c++ = 0xbf; *(uint64_t*)c = (uintptr_t)wt; c += 8;
    *c++ = 0x48; *c++ = 0xb8; *(uint64_t*)c = (uintptr_t)state_fn; c += 8; *c++ = 0xff; *c++ = 0xd0;
    *c++ = 0x84; *c++ = 0xc0; *c++ = 0x75; *c++ = 0x01; *c++ = 0xc3;
    *c++ = 0x48; *c++ = 0xbf; *(uint64_t*)c = (uintptr_t)wt; c += 8;
    *c++ = 0x48; *c++ = 0xb8; *(uint64_t*)c = (uintptr_t)epoch_fn; c += 8; *c++ = 0xff; *c++ = 0xd0; *c++ = 0xc3;
    p->thunk_cursor = c; __builtin___clear_cache((char*)start, (char*)c);
    return start;
}


void ThunkProxy_seal(ThunkProxy *p) {
    if (!p) {
        return;
    }
    if (!p->thunk_pool) return;
    lau_mprotect(p->thunk_pool, PROT_READ | PROT_EXEC);
}

void ThunkProxy_unseal(ThunkProxy *p) {
    if (!p || !p->thunk_pool) return;
    lau_mprotect(p->thunk_pool, PROT_READ | PROT_WRITE | PROT_EXEC);
}

// --- Ontological Layering (Dysnomia Integration) ---

// Emits a special preamble containing the raw data pointers within the executable block.
// This allows the ThunkProxy to structurally carry ontological payload without modifying C data layouts.
void ThunkProxy_emit_ontological_structuring(ThunkProxy *p, struct YI *yi, struct Dai *dai) {
    if (!p) return;
    thunk_check_bounds(p, 32);
    uint8_t *c = p->thunk_cursor;
    
    // We emit a special magic byte sequence that acts as a NOP to execution but can be scanned:
    // NOP (0x90) followed by specific immediate moves to scratch registers.
    // 0x90, 0x49 0xBC (mov r12, imm64), 0x49 0xBD (mov r13, imm64)
    *c++ = 0x90;
    
    *c++ = 0x49; *c++ = 0xbc; 
    *(uint64_t*)c = (uintptr_t)yi; c += 8;
    
    *c++ = 0x49; *c++ = 0xbd; 
    *(uint64_t*)c = (uintptr_t)dai; c += 8;
    
    p->thunk_cursor = c;
}

// Scans the ThunkProxy executable pool for the ontological magic sequence to recover the pointers.
void ThunkProxy_extract_ontological_structuring(ThunkProxy *p, struct YI **yi_out, struct Dai **dai_out) {
    if (yi_out) *yi_out = NULL;
    if (dai_out) *dai_out = NULL;
    if (!p || !p->thunk_pool || !p->thunk_cursor) return;
    
    // Preamble is 21 bytes: 0x90 (1) + 0x49 0xBC <ptr> (10) + 0x49 0xBD <ptr> (10)
    if (p->thunk_cursor < p->thunk_pool + 21) return;

    uint8_t *c = p->thunk_pool;
    uint8_t *end = p->thunk_cursor - 21;
    while (c <= end) {
        if (c[0] == 0x90 && c[1] == 0x49 && c[2] == 0xbc && c[11] == 0x49 && c[12] == 0xbd) {
            if (yi_out) *yi_out = (struct YI*)(uintptr_t)(*(uint64_t*)(c + 3));
            if (dai_out) *dai_out = (struct Dai*)(uintptr_t)(*(uint64_t*)(c + 13));
            return;
        }
        c++;
    }
}

void* ThunkProxy_emit_layer1_seal(ThunkProxy *p, void *target_fn) {
    struct YI *yi = NULL;
    ThunkProxy_extract_ontological_structuring(p, &yi, NULL);
    
    if (!yi || !yi->Psi) return target_fn; // No layer-0 genesis, no layer-1 structuring

    // Dynamically allocate the DAI (Layer-1) driven by the YI (Layer-0)
    extern struct Dai* ReactSHIO_bn_reused(struct Dai* dI, struct SHIO* Mu, void* pi_bn);
    struct Dai* local_dai = ReactSHIO_bn_reused(NULL, yi->Psi, yi->Xi);
    
    if (local_dai) {
        // Re-emit the ontological structure to include the newly formed DAI
        ThunkProxy_emit_ontological_structuring(p, yi, local_dai);
    }

    // Bake the DAI directly into the executable memory as mathematical proof-of-seal
    thunk_check_bounds(p, 64);
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    
    // mov r10, local_dai->Ichidai
    *c++ = 0x49; *c++ = 0xba; *(uint64_t*)c = (uintptr_t)(local_dai ? local_dai->Ichidai : NULL); c += 8;
    // mov r11, local_dai->Daiichi
    *c++ = 0x49; *c++ = 0xbb; *(uint64_t*)c = (uintptr_t)(local_dai ? local_dai->Daiichi : NULL); c += 8;
    
    // jmp target_fn
    if (target_fn) {
        *c++ = 0x48; *c++ = 0xb8; *(uint64_t*)c = (uintptr_t)target_fn; c += 8;
        *c++ = 0xff; *c++ = 0xe0;
    } else {
        *c++ = 0xc3; // ret
    }
    
    p->thunk_cursor = c;
    __builtin___clear_cache((char*)start, (char*)c);
    return start;
}

void* ThunkProxy_emit_mapped(ThunkProxy *p, MappedCommon *c, LauWiredHeader *h) {
    thunk_check_bounds(p, 1024);
    
    if (h->schema_count > 0 && h->schema != NULL) {
        // Zero-Cost Dynamic Mapping
        for (int i = 0; i < h->schema_count; i++) {
            void **target_member = (void**)((char*)c + h->schema[i].offset);
            switch (h->schema[i].type) {
                case THUNK_BAKED:
                    *target_member = ThunkProxy_emit_baked(p, h->schema[i].target_fn, h->schema[i].arity, c);
                    break;
                case THUNK_FORWARDING:
                    *target_member = ThunkProxy_emit_forwarding(p, h->schema[i].target_fn, c);
                    break;
                case THUNK_ZMM:
                    *target_member = ThunkProxy_emit_vgpr_load(p, h->schema[i].target_fn, c, 0); // basic zmm load mapping
                    break;
                case THUNK_IOCTL:
                    break;
            }
        }
    } else {
        // Fallback to legacy static MappedCommon topography
        c->step_safety_epoch = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_epoch, 1, h->version);
        c->step_safety_state = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_state, 1, &h->ftw);
        c->step_executor_directive = (void(*)(char*))ThunkProxy_emit_mixed(p, h->logic_directive, &h->counter);
        c->scramble = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_scramble, 1, h->payload);
        c->provenance = (void(*)(void))ThunkProxy_emit_baked(p, h->logic_provenance, 1, h->payload);
        c->hilbert_eval = (void(*)(float, float, float*))ThunkProxy_emit_mixed(p, h->logic_hilbert, h->payload);
        c->hilbert_batch = (void(*)(void*, const float*, float*, int))ThunkProxy_emit_forwarding(p, h->logic_hilbert_batch, h->payload);
    }
    
    return (void*)c->step_safety_epoch;
}
