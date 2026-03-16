#ifndef TSFI_K0RN_IR_H
#define TSFI_K0RN_IR_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    K0RN_IR_LOAD_STREAM,  // vDst = Stream[i]
    K0RN_IR_STORE_STREAM, // Stream[i] = vSrc
    K0RN_IR_LOAD_UNIFORM, // vDst = Broadcast(Uniform[idx])
    K0RN_IR_ADD,          // vDst = vA + vB
    K0RN_IR_SUB,          // vDst = vA - vB
    K0RN_IR_MUL,          // vDst = vA * vB
    K0RN_IR_FMA,          // vDst = vA * vB + vC
    K0RN_IR_MAX,          // vDst = max(vA, vB)
    K0RN_IR_MIN           // vDst = min(vA, vB)
} K0RnIROpcode;

typedef struct {
    K0RnIROpcode op;
    uint8_t dst; // Logical Register 0..15
    uint8_t src1;
    uint8_t src2;
    uint8_t src3;
    uint32_t imm; // Immediate or secret
} K0RnInstr;

typedef struct {
    K0RnInstr* code;
    size_t count;
    size_t capacity;
} K0RnModule;

K0RnModule* k0rn_module_create();
void k0rn_emit(K0RnModule* m, K0RnIROpcode op, uint8_t dst, uint8_t s1, uint8_t s2, uint32_t imm);
void k0rn_compile_dual(K0RnModule* m, void** out_thunk, uint32_t** out_spv, size_t* out_spv_size);
void k0rn_module_destroy(K0RnModule* m);

#endif
