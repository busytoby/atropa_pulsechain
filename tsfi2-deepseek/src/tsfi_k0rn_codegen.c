#include "tsfi_k0rn_ir.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

K0RnModule* k0rn_module_create() {
    K0RnModule* m = (K0RnModule*)lau_malloc(sizeof(K0RnModule));
    memset(m, 0, sizeof(K0RnModule));
    m->capacity = 64;
    m->code = (K0RnInstr*)lau_malloc(sizeof(K0RnInstr) * m->capacity);
    return m;
}

void k0rn_emit(K0RnModule* m, K0RnIROpcode op, uint8_t dst, uint8_t s1, uint8_t s2, uint32_t imm) {
    if (m->count >= m->capacity) {
        m->capacity *= 2;
        m->code = (K0RnInstr*)lau_realloc(m->code, sizeof(K0RnInstr) * m->capacity);
    }
    m->code[m->count++] = (K0RnInstr){op, dst, s1, s2, 0, imm};
}

void k0rn_module_destroy(K0RnModule* m) {
    if (!m) return;
    if (m->code) lau_free(m->code);
    lau_free(m);
}

static void* compile_avx512(K0RnModule* m) {
    (void)m;
    ThunkProxy* p = ThunkProxy_create();
    // Implementation pending
    return (void*)p;
}

static uint32_t* compile_spirv(K0RnModule* m, size_t* out_size) {
    (void)m;
    // Implementation pending
    *out_size = 0;
    return NULL;
}

void k0rn_compile_dual(K0RnModule* m, void** out_thunk, uint32_t** out_spv, size_t* out_spv_size) {
    if (out_thunk) *out_thunk = compile_avx512(m);
    if (out_spv) *out_spv = compile_spirv(m, out_spv_size);
}
