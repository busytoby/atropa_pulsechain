#ifndef TSFI_REACTION_H
#define TSFI_REACTION_H

#include "tsfi_fused_operator.h"
#include "tsfi_dysnomia.h"
#include "tsfi_thunks.h"

// --- Reaction Builders ---

// SHAO: Basic Reaction (Wave Shader)
// Uses Rod/Cone Dynamo to drive Intensity/Decay.
TSFiFusedOp* tsfi_reaction_build_shao(const char* name, struct SHAO* shao);

// SHIO: Intermediate Reaction (SHAO + Wave Morph)
// Uses Manifold to drive Time/secret.
TSFiFusedOp* tsfi_reaction_build_shio(const char* name, struct SHIO* shio);

// YI: Advanced Reaction (SHIO + CRC)
// Uses Xi for Salt.
TSFiFusedOp* tsfi_reaction_build_yi(const char* name, struct YI* yi, DysnomiaReactiveState* state);

// --- Helmholtz Stepwise Reaction (Epochs) ---

typedef enum {
    HELMHOLTZ_EPOCH_INIT = 0,      // STAT (Randomize Base/Secret/Signal)
    HELMHOLTZ_EPOCH_AVAIL = 1,     // Avail (Xi -> Contour)
    HELMHOLTZ_EPOCH_FORM = 2,      // Form (Contour -> Base)
    HELMHOLTZ_EPOCH_POLARIZE = 3,  // Polarize (Base -> Pole)
    HELMHOLTZ_EPOCH_CONJUGATE = 4, // Conjugate (Pole -> Coordinate)
    HELMHOLTZ_EPOCH_CONIFY = 5,    // Conify (Coordinate -> Foundation)
    HELMHOLTZ_EPOCH_SATURATE = 6,  // Saturate (Foundation -> Element/Charge/Dynamo)
    HELMHOLTZ_EPOCH_IONIZE = 7,    // IONIZE (Dynamo -> Final Bond)
    HELMHOLTZ_EPOCH_MAGNETIZE = 8, // MAGNETIZE (Cross-Endpoint Verification)
    HELMHOLTZ_EPOCH_DONE = 9
} HELMHOLTZ_EPOCH;

typedef enum {
    HELMHOLTZ_ADDUCT_ROD = 0,
    HELMHOLTZ_ADDUCT_CONE = 1
} HELMHOLTZ_ADDUCT;

// Stepwise State for a single endpoint (Host or Firmware)
typedef struct {
    struct SHA* Mu;
    TSFiBigInt* Xi;
    TSFiBigInt* Beta;
    TSFiBigInt* Alpha;
    TSFiBigInt* Prime;
    HELMHOLTZ_EPOCH current_epoch;
} TSFiHelmholtzAdductState;

// Execute a single stepwise transition for a given adduct.
// In a secure split system, the other adduct's required state (e.g. Contour, Pole)
// is passed in as 'external_input'.
void tsfi_helmholtz_step(TSFiHelmholtzAdductState* state, TSFiBigInt* external_input);
void tsfi_helmholtz_synchronize(TSFiHelmholtzAdductState* rod, TSFiHelmholtzAdductState* cone);

// High Level API
struct YI* tsfi_reaction_shoot(TSFiBigInt* Prime);
struct YI* tsfi_reaction_fuse(TSFiBigInt* Xi, TSFiBigInt* Rho, TSFiBigInt* Ohm, struct SHAO* orig_rho, TSFiBigInt* Prime);
struct YI* tsfi_reaction_shoot_bn_reused(struct YI* R, TSFiBigInt* p_bn, int shot_idx);
void tsfi_reaction_boot_firmware_yi(struct YI* R);
void tsfi_reaction_shoot_batch8(struct YI** results, TSFiBigInt* p_bn);
void tsfi_reaction_shoot_batch8_fused(struct YI** results, TSFiBigInt* p_bn);
void tsfi_reaction_fuse_batch8_fused(struct YI** results, TSFiBigInt** Xis, TSFiBigInt** Rhos, TSFiBigInt** Ohms, struct SHAO** orig_rhos, TSFiBigInt* p_bn);

// Reciprocal Verification: Verifies Eta (Ichidai) and Kappa (Daiichi) relationship.
struct Dai* tsfi_reaction_compute_reciprocity(struct SHIO* shio, TSFiBigInt* Pi, TSFiBigInt* Theta);
struct Dai* ReactSHIO_bn_reused(struct Dai* dI, struct SHIO* Mu, TSFiBigInt* pi_bn);

#endif // TSFI_REACTION_H
