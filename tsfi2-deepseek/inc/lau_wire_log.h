#ifndef LAU_WIRE_LOG_H
#define LAU_WIRE_LOG_H

#include "tsfi_types.h"
#include <stdatomic.h>

DEFINE_MAPPED_STRUCT(LauWireLog,
    char *buffer;
    uint32_t *head;
    uint32_t *valid_head;
    uint32_t *tail;
    _Atomic uint32_t *lock;
    uint32_t capacity;
)

void lau_wire_log(LauWireLog *log);

#endif // LAU_WIRE_LOG_H
