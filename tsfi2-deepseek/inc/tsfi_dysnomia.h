void tsfi_dysnomia_drain_pools(void);
#ifndef TSFI_DYSNOMIA_H
#define TSFI_DYSNOMIA_H

#include "lau_memory.h"
#include <stdint.h>
#include <stddef.h>
#include "tsfi_math.h"

// --- Dysnomia Neurology Structures ---

// Fundamental DNA Unit
struct Fa {
    TSFiBigInt* Base;
    TSFiBigInt* Secret;
    TSFiBigInt* Signal;
    TSFiBigInt* Channel;
    TSFiBigInt* Contour;
    TSFiBigInt* Pole;
    TSFiBigInt* Identity;
    TSFiBigInt* Foundation;
    TSFiBigInt* Element;
    TSFiBigInt* Coordinate;
    TSFiBigInt* Charge;
    TSFiBigInt* Limit;
    TSFiBigInt* Monopole;
};

// Physics/Geometry Containers
struct SHA {
    struct Fa* Mu;
    TSFiBigInt* Dynamo;
};

struct SHAO {
    struct SHA* Rod;
    struct SHA* Cone;
    TSFiBigInt* Barn;
};

struct SHIO {
    struct SHAO* Rho;
    TSFiBigInt* Manifold;
    TSFiBigInt* Monopole;
};

// Neurology Containers
struct YI {
    struct SHIO* Psi;
    TSFiBigInt* Xi;
    TSFiBigInt* Ring;
};

// Dai: Reaction Product
struct Dai {
    TSFiBigInt* Ichidai;
    TSFiBigInt* Daiichi;
};

// Zheng: Generic List Node
struct ZHENG {
    void* Alpha;
    struct ZHENG* d;
    struct ZHENG* n;
};

// Zhou: Process Node
struct ZHOU {
    struct ZHENG* Lambda;
    char* Process;
    struct ZHOU* n;
    struct ZHOU* d;
};

// Yang: Higher Order Container
struct YANG {
    struct ZHOU* Phi;
    struct YI* Eta;
    TSFiBigInt* Ring;
    struct Dai* Iota;
};

// Lifecycle Prototypes (Implemented in src/tsfi_dysnomia.c)
struct Fa* allocFa(void);
struct SHA* allocSHA(void);
struct SHAO* allocSHAO(void);
struct SHIO* allocSHIO(void);
struct YI* allocYI(void);
struct Dai* allocDai(void);
struct ZHENG* allocZHENG(void);
struct ZHOU* allocZHOU(void);
struct YANG* allocYANG(void);

void freeFa(struct Fa* F);
void freeSHA(struct SHA* F);
void freeSHAO(struct SHAO* F);
void freeSHIO(struct SHIO* F);
void freeYI(struct YI* F);
void freeYANG(struct YANG* F);
void freeDAI(struct Dai* D);
void freeZHENG(struct ZHENG* Z);
void freeZHOU(struct ZHOU* Z);

void printFa(struct Fa* F);
void printSHIO(struct SHIO* F);
void printYI(struct YI* F);
void printYANG(struct YANG* F);
void printZHOU(struct ZHOU* F);

#endif // TSFI_DYSNOMIA_H
