#include "tsfi_hotloader.h"
#include "lau_thunk.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>

static void* (*g_thunk_dlopen)(const char*, int) = NULL;
static void* (*g_thunk_dlsym)(void*, const char*) = NULL;
static char* (*g_thunk_dlerror)(void) = NULL;
static int   (*g_thunk_dlclose)(void*) = NULL;
static ThunkProxy *g_thunk_proxy_ptr = NULL;
static void* (*g_tsfi_memset)(void*, int, size_t) = NULL;
static bool g_init_active = false;

void tsfi_dl_thunks_init(void) {
    if (g_thunk_dlopen) return;
    g_init_active = true;

    ThunkProxy *p = ThunkProxy_create();
    if (!p) { g_init_active = false; return; }
    g_thunk_proxy_ptr = p;

    g_thunk_dlopen = (void* (*)(const char*, int))ThunkProxy_emit_wave_dlopen(p, NULL);
    g_thunk_dlsym  = (void* (*)(void*, const char*))ThunkProxy_emit_wave_dlsym(p, NULL);
    
    // dlerror and dlclose can be baked as well
    extern void* dlerror(void);
    extern int   dlclose(void*);
    g_thunk_dlerror = (char* (*)(void))ThunkProxy_emit_baked(p, (void*)dlerror, 0);
    g_thunk_dlclose = (int (*)(void*))ThunkProxy_emit_baked(p, (void*)dlclose, 1);
    
    g_tsfi_memset = (void* (*)(void*, int, size_t))ThunkProxy_emit_memset(p);

    ThunkProxy_seal(p);
    g_init_active = false;
}

void* tsfi_memset(void *s, int c, size_t n) {
    if (g_init_active || !g_tsfi_memset) {
        if (!g_init_active && !g_tsfi_memset) tsfi_dl_thunks_init();
        if (!g_tsfi_memset) return __builtin_memset(s, c, n);
    }
    return g_tsfi_memset(s, c, n);
}

void* tsfi_dlopen(const char *filename, int flags) {
    tsfi_dl_thunks_init();
    return g_thunk_dlopen(filename, flags);
}

void* tsfi_dlsym(void *handle, const char *symbol) {
    tsfi_dl_thunks_init();
    return g_thunk_dlsym(handle, symbol);
}

char* tsfi_dlerror(void) {
    tsfi_dl_thunks_init();
    return g_thunk_dlerror();
}

int tsfi_dlclose(void *handle) {
    tsfi_dl_thunks_init();
    

    
    printf("[HOTLOADER] Unsealing dynamic plugin matrix for dlclose...\n");
    int res = g_thunk_dlclose(handle);
    
    extern void lau_assert_zero_unsealed_leaks(const char*, void*);
    printf("[HOTLOADER] Plugin detached. Mathematically sweeping dynamic execution map for unlinked geometry...\n");
    lau_assert_zero_unsealed_leaks("tsfi_dlclose", handle);
    
    return res;
}

void tsfi_dl_thunks_teardown(void) {
    if (g_thunk_proxy_ptr) {
        ThunkProxy_unseal(g_thunk_proxy_ptr);
        ThunkProxy_destroy(g_thunk_proxy_ptr);
        g_thunk_proxy_ptr = NULL;
    }
}
