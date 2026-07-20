#ifndef TSFI_PARC_SMALLTALK_H
#define TSFI_PARC_SMALLTALK_H

#include <stdint.h>

#define MAX_METHODS 32
#define MAX_CLASSES 16

typedef uint32_t oop_t; // Object Oriented Pointer

typedef struct {
    char selector[32];
    uint8_t bytecodes[64];
    int bytecode_count;
} st_method_t;

typedef struct {
    char name[32];
    oop_t superclass_oop;
    st_method_t methods[MAX_METHODS];
    int method_count;
} st_class_t;

typedef struct {
    oop_t oop;
    oop_t class_oop;
    uint32_t fields[8];
} st_object_t;

typedef struct {
    st_class_t classes[MAX_CLASSES];
    int class_count;
    st_object_t heap[128];
    int heap_count;
    oop_t active_context_oop;
} tsfi_parc_smalltalk_vm_t;

/* Initialize Smalltalk sandbox virtual machine */
int tsfi_parc_st_init_vm(tsfi_parc_smalltalk_vm_t *vm);

/* Register a class in Smalltalk VM environment */
oop_t tsfi_parc_st_define_class(tsfi_parc_smalltalk_vm_t *vm, const char *name, oop_t superclass_oop);

/* Bind a compiled method block to class selector table */
int tsfi_parc_st_bind_method(tsfi_parc_smalltalk_vm_t *vm, oop_t class_oop, const char *selector, const uint8_t *bytecodes, int count);

/* Send a message to an object causing method selector lookup and bytecode execution */
int tsfi_parc_st_send_message(tsfi_parc_smalltalk_vm_t *vm, oop_t receiver_oop, const char *selector, uint32_t *result_out);

#endif // TSFI_PARC_SMALLTALK_H
