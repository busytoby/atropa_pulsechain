#ifndef LAU_THUNK_H
#define LAU_THUNK_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t *thunk_pool;
    uint8_t *thunk_cursor;
    size_t pool_size;
} ThunkProxy;

ThunkProxy* ThunkProxy_create(void);
void* ThunkProxy_emit_baked(ThunkProxy *p, void *fn, int argc, ...);
void* ThunkProxy_emit_mixed(ThunkProxy *p, void *fn, void *baked_ptr);
void ThunkProxy_rebind(void *thunk_ptr, void *new_target);
void ThunkProxy_destroy(ThunkProxy *p);

#endif // LAU_THUNK_H
