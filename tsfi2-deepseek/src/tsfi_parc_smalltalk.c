#include <stdio.h>
#include <string.h>
#include "tsfi_parc_smalltalk.h"

int tsfi_parc_st_init_vm(tsfi_parc_smalltalk_vm_t *vm) {
    if (!vm) return -1;
    vm->class_count = 0;
    vm->heap_count = 0;
    vm->active_context_oop = 0;
    return 0;
}

oop_t tsfi_parc_st_define_class(tsfi_parc_smalltalk_vm_t *vm, const char *name, oop_t superclass_oop) {
    if (!vm || vm->class_count >= MAX_CLASSES) return 0;
    int idx = vm->class_count++;
    st_class_t *c = &vm->classes[idx];
    strncpy(c->name, name, sizeof(c->name) - 1);
    c->superclass_oop = superclass_oop;
    c->method_count = 0;
    return (oop_t)(idx + 1); // class class index offset by 1
}

int tsfi_parc_st_bind_method(tsfi_parc_smalltalk_vm_t *vm, oop_t class_oop, const char *selector, const uint8_t *bytecodes, int count) {
    if (!vm) return -1;
    int c_idx = (int)class_oop - 1;
    if (c_idx < 0 || c_idx >= vm->class_count) return -2;
    st_class_t *c = &vm->classes[c_idx];

    if (c->method_count >= MAX_METHODS) return -3;
    int m_idx = c->method_count++;
    st_method_t *m = &c->methods[m_idx];
    strncpy(m->selector, selector, sizeof(m->selector) - 1);
    m->bytecode_count = count < 64 ? count : 64;
    memcpy(m->bytecodes, bytecodes, m->bytecode_count);
    return 0;
}

int tsfi_parc_st_send_message(tsfi_parc_smalltalk_vm_t *vm, oop_t receiver_oop, const char *selector, uint32_t *result_out) {
    if (!vm) return -1;
    int obj_idx = -1;
    for (int i = 0; i < vm->heap_count; i++) {
        if (vm->heap[i].oop == receiver_oop) {
            obj_idx = i;
            break;
        }
    }
    if (obj_idx == -1) return -2; // object not found
    st_object_t *obj = &vm->heap[obj_idx];

    // Dynamic Method Lookup starting at object class climbing to superclasses
    oop_t current_class_oop = obj->class_oop;
    st_method_t *method = NULL;

    while (current_class_oop != 0) {
        int c_idx = (int)current_class_oop - 1;
        if (c_idx < 0 || c_idx >= vm->class_count) break;
        st_class_t *c = &vm->classes[c_idx];

        for (int m = 0; m < c->method_count; m++) {
            if (strcmp(c->methods[m].selector, selector) == 0) {
                method = &c->methods[m];
                break;
            }
        }
        if (method) break;
        current_class_oop = c->superclass_oop;
    }

    if (!method) return -3; // message not understood

    // Smalltalk stack interpreter sandbox
    uint32_t stack[16];
    int sp = 0;

    for (int pc = 0; pc < method->bytecode_count; pc++) {
        uint8_t code = method->bytecodes[pc];
        uint8_t op = code >> 4;
        uint8_t arg = code & 0x0F;

        switch (op) {
            case 0: // Load field (push field value onto stack)
                if (sp < 16 && arg < 8) {
                    stack[sp++] = obj->fields[arg];
                }
                break;
            case 1: // Store field (pop value into field)
                if (sp > 0 && arg < 8) {
                    obj->fields[arg] = stack[--sp];
                }
                break;
            case 2: // Load constant (push arg integer onto stack)
                if (sp < 16) {
                    stack[sp++] = (uint32_t)arg;
                }
                break;
            case 3: // Add (pop two, add, push)
                if (sp >= 2) {
                    uint32_t b = stack[--sp];
                    uint32_t a = stack[--sp];
                    stack[sp++] = a + b;
                }
                break;
            case 4: // Subtract (pop two, subtract, push)
                if (sp >= 2) {
                    uint32_t b = stack[--sp];
                    uint32_t a = stack[--sp];
                    stack[sp++] = a - b;
                }
                break;
            case 5: // Halt/Return (terminate interpreter)
                if (sp > 0 && result_out) {
                    *result_out = stack[--sp];
                }
                return 0;
        }
    }

    if (sp > 0 && result_out) {
        *result_out = stack[sp - 1];
    }
    return 0;
}
