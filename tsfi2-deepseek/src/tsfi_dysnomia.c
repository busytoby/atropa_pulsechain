#include "tsfi_dysnomia.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DYS_POOL_SIZE 2048

#define DECLARE_POOL(T, NAME) \
    static T* g_##NAME##_pool[DYS_POOL_SIZE]; \
    static int g_##NAME##_pool_count = 0; \
    T* alloc##NAME(void) { \
        if (g_##NAME##_pool_count > 0) { \
            T* obj = g_##NAME##_pool[--g_##NAME##_pool_count]; \
            memset(obj, 0, sizeof(T)); \
            return obj; \
        } \
        T* obj = (T*)lau_malloc(sizeof(T)); \
        memset(obj, 0, sizeof(T)); \
        return obj; \
    } \
    void cleanup##NAME(void) { \
        while (g_##NAME##_pool_count > 0) lau_free(g_##NAME##_pool[--g_##NAME##_pool_count]); \
    } \
    static void recycle##NAME(T* obj) { \
        if (!obj) return; \
        if (g_##NAME##_pool_count < DYS_POOL_SIZE) { \
            g_##NAME##_pool[g_##NAME##_pool_count++] = obj; \
        } else { \
            lau_free(obj); \
        } \
    }

DECLARE_POOL(struct Fa, Fa)
DECLARE_POOL(struct SHA, SHA)
DECLARE_POOL(struct SHAO, SHAO)
DECLARE_POOL(struct SHIO, SHIO)
DECLARE_POOL(struct YI, YI)
DECLARE_POOL(struct Dai, Dai)
DECLARE_POOL(struct ZHENG, ZHENG)
DECLARE_POOL(struct ZHOU, ZHOU)
DECLARE_POOL(struct YANG, YANG)

void freeFa(struct Fa* F) {
    if (!F) return;
    tsfi_bn_free(F->Base);
    tsfi_bn_free(F->Secret);
    tsfi_bn_free(F->Signal);
    tsfi_bn_free(F->Channel);
    tsfi_bn_free(F->Contour);
    tsfi_bn_free(F->Pole);
    tsfi_bn_free(F->Identity);
    tsfi_bn_free(F->Foundation);
    tsfi_bn_free(F->Element);
    tsfi_bn_free(F->Coordinate);
    tsfi_bn_free(F->Charge);
    tsfi_bn_free(F->Limit);
    tsfi_bn_free(F->Monopole);
    recycleFa(F);
}

void freeSHA(struct SHA* F) {
    if (!F) return;
    freeFa(F->Mu);
    tsfi_bn_free(F->Dynamo);
    recycleSHA(F);
}

void freeSHAO(struct SHAO* F) {
    if (!F) return;
    freeSHA(F->Rod);
    freeSHA(F->Cone);
    tsfi_bn_free(F->Barn);
    recycleSHAO(F);
}

void freeSHIO(struct SHIO* F) {
    if (!F) return;
    freeSHAO(F->Rho);
    tsfi_bn_free(F->Manifold);
    tsfi_bn_free(F->Monopole);
    recycleSHIO(F);
}

void freeYI(struct YI* F) {
    if (!F) return;
    freeSHIO(F->Psi);
    tsfi_bn_free(F->Xi);
    tsfi_bn_free(F->Ring);
    recycleYI(F);
}

void freeDAI(struct Dai* D) {
    if (!D) return;
    tsfi_bn_free(D->Ichidai);
    tsfi_bn_free(D->Daiichi);
    recycleDai(D);
}

void freeZHENG(struct ZHENG* Z) {
    if (!Z) return;
    if (Z->d) freeZHENG(Z->d);
    if (Z->n) freeZHENG(Z->n);
    recycleZHENG(Z);
}

void freeZHOU(struct ZHOU* Z) {
    if (!Z) return;
    recycleZHOU(Z);
}

void freeYANG(struct YANG* F) {
    if (!F) return;
    recycleYANG(F);
}

void printFa(struct Fa* F) {
    if (!F) return;
    printf("Fa[Base=%p, Secret=%p]\n", (void*)F->Base, (void*)F->Secret);
}

void printSHA(struct SHA* F) {
    if (!F) return;
    printFa(F->Mu);
}

void printSHAO(struct SHAO* F) {
    if (!F) return;
    printSHA(F->Rod);
    printSHA(F->Cone);
}

void printSHIO(struct SHIO* F) {
    if (!F) return;
    printSHAO(F->Rho);
}

void printYI(struct YI* F) {
    if (!F) return;
    printSHIO(F->Psi);
}

void tsfi_dysnomia_drain_pools(void) {
    cleanupFa(); cleanupSHA(); cleanupSHAO(); cleanupSHIO();
    cleanupYI(); cleanupDai(); cleanupZHENG(); cleanupZHOU(); cleanupYANG();
}
