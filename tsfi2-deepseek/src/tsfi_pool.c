#include "tsfi_pool.h"
#include "tsfi_dysnomia.h"
#include "tsfi_math.h"
#include "lau_memory.h"
#include "lau_registry.h"
#include <string.h>
#include <stdlib.h>

#define TSFI_POOL_DEPTH 64

// --- Static Pools (Rigidity Qing) ---

static TSFiBigInt* g_bn_pool[TSFI_POOL_DEPTH * 16]; // Extra deep pool for BigInts
static int g_bn_pool_idx = 0;

static struct Fa* g_fa_pool[TSFI_POOL_DEPTH];
static int g_fa_pool_idx = 0;

static struct SHA* g_sha_pool[TSFI_POOL_DEPTH];
static int g_sha_pool_idx = 0;

static struct SHAO* g_shao_pool[TSFI_POOL_DEPTH];
static int g_shao_pool_idx = 0;

static struct SHIO* g_shio_pool[TSFI_POOL_DEPTH];
static int g_shio_pool_idx = 0;

static struct YI* g_yi_pool[TSFI_POOL_DEPTH];
static int g_yi_pool_idx = 0;

static struct Dai* g_dai_pool[TSFI_POOL_DEPTH];
static int g_dai_pool_idx = 0;

static struct ZHENG* g_zheng_pool[TSFI_POOL_DEPTH];
static int g_zheng_pool_idx = 0;

static struct ZHOU* g_zhou_pool[TSFI_POOL_DEPTH];
static int g_zhou_pool_idx = 0;

static struct YANG* g_yang_pool[TSFI_POOL_DEPTH];
static int g_yang_pool_idx = 0;

void tsfi_pool_init(void) { if (atomic_load_explicit(&g_init_in_progress, memory_order_relaxed)) return;
    // Rigid Pre-allocation to eliminate fragmentation
    for (int i = 0; i < TSFI_POOL_DEPTH * 16; i++) {
        g_bn_pool[i] = (TSFiBigInt*)lau_memalign(64, sizeof(TSFiBigInt));
    }
    g_bn_pool_idx = TSFI_POOL_DEPTH * 16;

    for (int i = 0; i < TSFI_POOL_DEPTH; i++) {
        g_fa_pool[i] = (struct Fa*)lau_malloc(sizeof(struct Fa));
        g_sha_pool[i] = (struct SHA*)lau_malloc(sizeof(struct SHA));
        g_shao_pool[i] = (struct SHAO*)lau_malloc(sizeof(struct SHAO));
        g_shio_pool[i] = (struct SHIO*)lau_malloc(sizeof(struct SHIO));
        g_yi_pool[i] = (struct YI*)lau_malloc(sizeof(struct YI));
        g_dai_pool[i] = (struct Dai*)lau_malloc(sizeof(struct Dai));
        g_zheng_pool[i] = (struct ZHENG*)lau_malloc(sizeof(struct ZHENG));
        g_zhou_pool[i] = (struct ZHOU*)lau_malloc(sizeof(struct ZHOU));
        g_yang_pool[i] = (struct YANG*)lau_malloc(sizeof(struct YANG));
    }
    g_fa_pool_idx = TSFI_POOL_DEPTH;
    g_sha_pool_idx = TSFI_POOL_DEPTH;
    g_shao_pool_idx = TSFI_POOL_DEPTH;
    g_shio_pool_idx = TSFI_POOL_DEPTH;
    g_yi_pool_idx = TSFI_POOL_DEPTH;
    g_dai_pool_idx = TSFI_POOL_DEPTH;
    g_zheng_pool_idx = TSFI_POOL_DEPTH;
    g_zhou_pool_idx = TSFI_POOL_DEPTH;
    g_yang_pool_idx = TSFI_POOL_DEPTH;
}

// Acquisition Logic

TSFiBigInt* tsfi_pool_acquire_bn(void) {
    if (g_bn_pool_idx > 0) {
        TSFiBigInt* bn = g_bn_pool[--g_bn_pool_idx];
        memset(bn->limbs, 0, sizeof(bn->limbs));
        bn->active_limbs = 0;
        return bn;
    }
    return (TSFiBigInt*)lau_memalign(64, sizeof(TSFiBigInt));
}

void tsfi_pool_release_bn(TSFiBigInt* bn) {
    if (!bn) return;
    if (g_bn_pool_idx < TSFI_POOL_DEPTH * 16) {
        g_bn_pool[g_bn_pool_idx++] = bn;
    } else {
        lau_free(bn);
    }
}

struct Fa* tsfi_pool_acquire_fa(void) {
    if (g_fa_pool_idx > 0) {
        struct Fa* f = g_fa_pool[--g_fa_pool_idx];
        memset(f, 0, sizeof(struct Fa));
        return f;
    }
    struct Fa* obj = (struct Fa*)lau_malloc(sizeof(struct Fa)); if(obj) memset(obj, 0, sizeof(struct Fa)); return obj;
}

void tsfi_pool_release_fa(struct Fa* f) {
    if (!f) return;
    if (g_fa_pool_idx < TSFI_POOL_DEPTH) {
        g_fa_pool[g_fa_pool_idx++] = f;
    } else {
        lau_free(f);
    }
}

struct SHA* tsfi_pool_acquire_sha(void) {
    if (g_sha_pool_idx > 0) {
        struct SHA* a = g_sha_pool[--g_sha_pool_idx];
        memset(a, 0, sizeof(struct SHA));
        return a;
    }
    struct SHA* obj = (struct SHA*)lau_malloc(sizeof(struct SHA)); if(obj) memset(obj, 0, sizeof(struct SHA)); return obj;
}

void tsfi_pool_release_sha(struct SHA* a) {
    if (!a) return;
    if (g_sha_pool_idx < TSFI_POOL_DEPTH) {
        g_sha_pool[g_sha_pool_idx++] = a;
    } else {
        lau_free(a);
    }
}

struct SHAO* tsfi_pool_acquire_shao(void) {
    if (g_shao_pool_idx > 0) {
        struct SHAO* a = g_shao_pool[--g_shao_pool_idx];
        memset(a, 0, sizeof(struct SHAO));
        return a;
    }
    struct SHAO* obj = (struct SHAO*)lau_malloc(sizeof(struct SHAO)); if(obj) memset(obj, 0, sizeof(struct SHAO)); return obj;
}

void tsfi_pool_release_shao(struct SHAO* a) {
    if (!a) return;
    if (g_shao_pool_idx < TSFI_POOL_DEPTH) {
        g_shao_pool[g_shao_pool_idx++] = a;
    } else {
        lau_free(a);
    }
}

struct SHIO* tsfi_pool_acquire_shio(void) {
    if (g_shio_pool_idx > 0) {
        struct SHIO* b = g_shio_pool[--g_shio_pool_idx];
        memset(b, 0, sizeof(struct SHIO));
        return b;
    }
    struct SHIO* obj = (struct SHIO*)lau_malloc(sizeof(struct SHIO)); if(obj) memset(obj, 0, sizeof(struct SHIO)); return obj;
}

void tsfi_pool_release_shio(struct SHIO* b) {
    if (!b) return;
    if (g_shio_pool_idx < TSFI_POOL_DEPTH) {
        g_shio_pool[g_shio_pool_idx++] = b;
    } else {
        lau_free(b);
    }
}

struct YI* tsfi_pool_acquire_yi(void) {
    if (g_yi_pool_idx > 0) {
        struct YI* r = g_yi_pool[--g_yi_pool_idx];
        memset(r, 0, sizeof(struct YI));
        return r;
    }
    struct YI* obj = (struct YI*)lau_malloc(sizeof(struct YI)); if(obj) memset(obj, 0, sizeof(struct YI)); return obj;
}

void tsfi_pool_release_yi(struct YI* r) {
    if (!r) return;
    if (g_yi_pool_idx < TSFI_POOL_DEPTH) {
        g_yi_pool[g_yi_pool_idx++] = r;
    } else {
        lau_free(r);
    }
}

struct Dai* tsfi_pool_acquire_dai(void) {
    if (g_dai_pool_idx > 0) {
        struct Dai* d = g_dai_pool[--g_dai_pool_idx];
        memset(d, 0, sizeof(struct Dai));
        return d;
    }
    struct Dai* obj = (struct Dai*)lau_malloc(sizeof(struct Dai)); if(obj) memset(obj, 0, sizeof(struct Dai)); return obj;
}

void tsfi_pool_release_dai(struct Dai* d) {
    if (!d) return;
    if (g_dai_pool_idx < TSFI_POOL_DEPTH) {
        g_dai_pool[g_dai_pool_idx++] = d;
    } else {
        lau_free(d);
    }
}

struct ZHENG* tsfi_pool_acquire_zheng(void) {
    if (g_zheng_pool_idx > 0) {
        struct ZHENG* z = g_zheng_pool[--g_zheng_pool_idx];
        memset(z, 0, sizeof(struct ZHENG));
        return z;
    }
    struct ZHENG* obj = (struct ZHENG*)lau_malloc(sizeof(struct ZHENG)); if(obj) memset(obj, 0, sizeof(struct ZHENG)); return obj;
}

void tsfi_pool_release_zheng(struct ZHENG* z) {
    if (!z) return;
    if (g_zheng_pool_idx < TSFI_POOL_DEPTH) {
        g_zheng_pool[g_zheng_pool_idx++] = z;
    } else {
        lau_free(z);
    }
}

struct ZHOU* tsfi_pool_acquire_zhou(void) {
    if (g_zhou_pool_idx > 0) {
        struct ZHOU* z = g_zhou_pool[--g_zhou_pool_idx];
        memset(z, 0, sizeof(struct ZHOU));
        return z;
    }
    struct ZHOU* obj = (struct ZHOU*)lau_malloc(sizeof(struct ZHOU)); if(obj) memset(obj, 0, sizeof(struct ZHOU)); return obj;
}

void tsfi_pool_release_zhou(struct ZHOU* z) {
    if (!z) return;
    if (g_zhou_pool_idx < TSFI_POOL_DEPTH) {
        g_zhou_pool[g_zhou_pool_idx++] = z;
    } else {
        lau_free(z);
    }
}

struct YANG* tsfi_pool_acquire_yang(void) {
    if (g_yang_pool_idx > 0) {
        struct YANG* y = g_yang_pool[--g_yang_pool_idx];
        memset(y, 0, sizeof(struct YANG));
        return y;
    }
    struct YANG* obj = (struct YANG*)lau_malloc(sizeof(struct YANG)); if(obj) memset(obj, 0, sizeof(struct YANG)); return obj;
}

void tsfi_pool_release_yang(struct YANG* y) {
    if (!y) return;
    if (g_yang_pool_idx < TSFI_POOL_DEPTH) {
        g_yang_pool[g_yang_pool_idx++] = y;
    } else {
        lau_free(y);
    }
}

void tsfi_pool_teardown(void) {
    for (int i = 0; i < g_bn_pool_idx; i++) lau_free(g_bn_pool[i]);
    for (int i = 0; i < g_fa_pool_idx; i++) lau_free(g_fa_pool[i]);
    for (int i = 0; i < g_sha_pool_idx; i++) lau_free(g_sha_pool[i]);
    for (int i = 0; i < g_shao_pool_idx; i++) lau_free(g_shao_pool[i]);
    for (int i = 0; i < g_shio_pool_idx; i++) lau_free(g_shio_pool[i]);
    for (int i = 0; i < g_yi_pool_idx; i++) lau_free(g_yi_pool[i]);
    for (int i = 0; i < g_dai_pool_idx; i++) lau_free(g_dai_pool[i]);
    for (int i = 0; i < g_zheng_pool_idx; i++) lau_free(g_zheng_pool[i]);
    for (int i = 0; i < g_zhou_pool_idx; i++) lau_free(g_zhou_pool[i]);
    for (int i = 0; i < g_yang_pool_idx; i++) lau_free(g_yang_pool[i]);
    g_bn_pool_idx = g_fa_pool_idx = g_sha_pool_idx = g_shao_pool_idx = g_shio_pool_idx = g_yi_pool_idx = 0;
    g_dai_pool_idx = g_zheng_pool_idx = g_zhou_pool_idx = g_yang_pool_idx = 0;
}
