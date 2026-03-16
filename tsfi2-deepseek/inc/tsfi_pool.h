#ifndef TSFI_POOL_H
#define TSFI_POOL_H

#include "tsfi_dysnomia.h"
#include "tsfi_math.h"

// --- Genetic Pool (Rigidity Qing) ---
// Pre-allocated pools for core reaction structures to eliminate fragmentation.

void tsfi_pool_init(void);
void tsfi_pool_teardown(void);

TSFiBigInt*  tsfi_pool_acquire_bn(void);
void         tsfi_pool_release_bn(TSFiBigInt* bn);

struct Fa*   tsfi_pool_acquire_fa(void);
void         tsfi_pool_release_fa(struct Fa* f);

struct SHA*  tsfi_pool_acquire_sha(void);
void         tsfi_pool_release_sha(struct SHA* a);

struct SHAO* tsfi_pool_acquire_shao(void);
void         tsfi_pool_release_shao(struct SHAO* a);

struct SHIO* tsfi_pool_acquire_shio(void);
void         tsfi_pool_release_shio(struct SHIO* b);

struct YI*   tsfi_pool_acquire_yi(void);
void         tsfi_pool_release_yi(struct YI* r);

struct Dai*   tsfi_pool_acquire_dai(void);
void          tsfi_pool_release_dai(struct Dai* d);

struct ZHENG* tsfi_pool_acquire_zheng(void);
void          tsfi_pool_release_zheng(struct ZHENG* z);

struct ZHOU*  tsfi_pool_acquire_zhou(void);
void          tsfi_pool_release_zhou(struct ZHOU* z);

struct YANG*  tsfi_pool_acquire_yang(void);
void          tsfi_pool_release_yang(struct YANG* y);

#endif
