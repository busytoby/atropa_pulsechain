#include "lau_thunk.h"
#include "lau_memory.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>

#if !defined(__x86_64__)
#error "LAU Thunk JIT is only supported on x86_64 architectures."
#endif

ThunkProxy* ThunkProxy_create(void) {
    size_t pg = sysconf(_SC_PAGESIZE);
    ThunkProxy *proxy = (ThunkProxy *)lau_malloc(sizeof(ThunkProxy));
    if (!proxy) return NULL;
    
    // Use lau_memalign for the pool to ensure page alignment for mprotect
    proxy->thunk_pool = (uint8_t *)lau_memalign(pg, pg);
    if (!proxy->thunk_pool) {
        lau_free(proxy);
        return NULL;
    }
    
    memset(proxy->thunk_pool, 0, pg);
    proxy->thunk_cursor = proxy->thunk_pool;
    proxy->pool_size = pg;
    return proxy;
}

// Helper to check bounds
static void thunk_check_bounds(ThunkProxy *p, size_t needed) {
    if ((size_t)(p->thunk_cursor - p->thunk_pool) + needed > p->pool_size) {
        fprintf(stderr, "[LAU_THUNK] FATAL: JIT Buffer Overflow!\n");
        abort();
    }
}

void* ThunkProxy_emit_baked(ThunkProxy *p, void *fn, int argc, ...) {
    if (argc > 6) {
        fprintf(stderr, "[LAU_THUNK] FATAL: Too many arguments for JIT (Max 6)\n");
        abort();
    }
    thunk_check_bounds(p, (argc * 32) + 32 + 2); // +2 for metadata

    // 1. Write Metadata (Patch Offset)
    // We reserve 2 bytes before the executable code to store the offset 
    // to the function pointer immediate.
    uint8_t *meta = p->thunk_cursor;
    p->thunk_cursor += 2; 

    uint8_t *c = p->thunk_cursor; 
    void *start = (void*)c;
    va_list args; 
    va_start(args, argc);
    
    // System V AMD64 ABI: rdi, rsi, rdx, rcx, r8, r9
    for (int i = 0; i < argc; i++) {
        void *val = va_arg(args, void*);
        
        // MOV r64, imm64 (REX.W + B8+rd)
        *c++ = 0x48; 
        
        switch (i) {
            case 0: *c++ = 0xbf; break; // MOV RDI (BF)
            case 1: *c++ = 0xbe; break; // MOV RSI (BE)
            case 2: *c++ = 0xba; break; // MOV RDX (BA)
            case 3: *c++ = 0xb9; break; // MOV RCX (B9)
            case 4: *c++ = 0xb8; break; // MOV R8  (B8)
            case 5: *c++ = 0xb9; break; // MOV R9  (B9)
        }
        
        // Patch REX prefix to REX.WB (0x49) for R8/R9
        if (i >= 4) {
             c[-2] = 0x49; 
        }

        // Fix: Use memcpy for unaligned write
        memcpy(c, &val, sizeof(void*));
        c += 8;
    }
    va_end(args);
    
    // MOV RAX, imm64 (Address of function)
    *c++ = 0x48; *c++ = 0xb8; 
    
    // CALCULATE OFFSET
    // The immediate value starts here.
    // Offset is relative to 'start'.
    uint16_t offset = (uint16_t)(c - (uint8_t*)start);
    
    // STORE OFFSET IN METADATA (Little Endian)
    memcpy(meta, &offset, sizeof(uint16_t));
    
    // Fix: Use memcpy for unaligned write
    memcpy(c, &fn, sizeof(void*));
    c += 8;
    
    *c++ = 0xff; *c++ = 0xe0; // jmp *%rax
    p->thunk_cursor = c; 
    return start;
}

void* ThunkProxy_emit_mixed(ThunkProxy *p, void *fn, void *baked_ptr) {
    thunk_check_bounds(p, 64 + 2); // +2 for metadata

    // 1. Write Metadata
    uint8_t *meta = p->thunk_cursor;
    p->thunk_cursor += 2;

    uint8_t *c = p->thunk_cursor; 
    void *start = (void*)c;
    
    // MOV %RDI, %RSI  (Arg1 -> Arg2)
    // Correct Opcode: 48 89 FE
    *c++ = 0x48; *c++ = 0x89; *c++ = 0xfe; 

    *c++ = 0x48; *c++ = 0xbf; 
    // Fix: Use memcpy for unaligned write
    memcpy(c, &baked_ptr, sizeof(void*));
    c += 8; // mov baked -> %rdi (Arg1)
    
    *c++ = 0x48; *c++ = 0xb8; 
    
    // CALCULATE OFFSET
    uint16_t offset = (uint16_t)(c - (uint8_t*)start);
    memcpy(meta, &offset, sizeof(uint16_t));
    
    // Fix: Use memcpy for unaligned write
    memcpy(c, &fn, sizeof(void*));
    c += 8;
    
    *c++ = 0xff; *c++ = 0xe0;
    p->thunk_cursor = c; 
    return start;
}

void ThunkProxy_rebind(void *thunk_ptr, void *new_target) {
    if (!thunk_ptr || !new_target) return;
    
    uint8_t *p = (uint8_t*)thunk_ptr;
    
    // ROBUST REBIND: Read metadata offset
    // The offset is stored 2 bytes before the code start.
    uint16_t offset;
    memcpy(&offset, p - 2, sizeof(uint16_t));
    
    // Sanity Check: Offset should be reasonable
    if (offset > 256) {
         fprintf(stderr, "[LAU_THUNK] ERROR: Invalid rebind offset %d (Corrupt metadata?)\n", offset);
         return;
    }
    
    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t addr = (uintptr_t)(p + offset);
    uintptr_t page_start = addr & ~(page_size - 1);
    
    // Debug
    // printf("[LAU_THUNK] Rebinding %p (off=%u) to %p.\n", thunk_ptr, offset, new_target);

    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        perror("[LAU_THUNK] mprotect failed during rebind");
        return;
    }
    
    // ATOMIC WRITE
    // We are writing a 64-bit pointer. 
    // Note: It might cross a cache line boundary, but on x86 aligned writes are atomic.
    // Unaligned writes are atomic if within a cache line.
    // If it crosses a page boundary, we are in trouble if we didn't unlock both pages.
    // Given the pool alignment and size, it's unlikely to cross a page boundary unless 
    // the instruction sequence is perfectly aligned to do so. 
    // (FIXME: Ideally check if (addr + 8) crosses page)
    
    if ((addr & ~(page_size - 1)) != ((addr + 7) & ~(page_size - 1))) {
        // Crosses page boundary! Unlock next page too.
        mprotect((void*)(page_start + page_size), page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
    }

    // ATOMIC WRITE (Best effort on unaligned, followed by sync)
    memcpy(p + offset, &new_target, sizeof(void*));
    
    mprotect((void*)page_start, page_size, PROT_READ | PROT_EXEC);
    if ((addr & ~(page_size - 1)) != ((addr + 7) & ~(page_size - 1))) {
         mprotect((void*)(page_start + page_size), page_size, PROT_READ | PROT_EXEC);
    }
    
    __builtin___clear_cache((char*)(p + offset), (char*)(p + offset + 8));
}

void ThunkProxy_destroy(ThunkProxy *p) {
    if (!p) return;
    if (p->thunk_pool) {
        // Ensure memory is writable before free (which scrambles it)
        // If it was PROT_EXEC, scrambling would segfault.
        mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_WRITE);
        lau_free(p->thunk_pool);
    }
    lau_free(p);
}