#include "tsfi_hotloader.h"
#include "tsfi_io.h"
#include "tsfi_reaction.h"
#include "tsfi_operator.h"
#include "tsfi_spirv_kernels.h"
#include "tsfi_dys_math.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * TSFi Dysnomia Reaction Logic (Reconciled Origins)
 * Implementation based on the oldest verified C port.
 */

// --- Internal Simulation Primitives (Direct Translation) ---

static void Tune(struct Fa* F, TSFiBigInt* Prime) {
    if (!F || !F->Base || !F->Signal) return;
    F->Channel = tsfi_bn_realloc(F->Channel); modPow_bn(F->Channel, F->Base, F->Signal, Prime);
}

static struct SHA* STAT(TSFiBigInt* Prime) {
    struct SHA* A = allocSHA();
    struct Fa* F = allocFa();
    memset(F, 0, sizeof(struct Fa));
    F->Base = tsfi_bn_realloc(F->Base); tsfi_bn_randomize(F->Base);
    F->Secret = tsfi_bn_realloc(F->Secret); tsfi_bn_randomize(F->Secret);
    F->Signal = tsfi_bn_realloc(F->Signal); tsfi_bn_randomize(F->Signal);
    Tune(F, Prime);
    A->Mu = F;
    A->Dynamo = NULL;
    return A;
}

static void Avail(struct SHA* B, TSFiBigInt* Xi, TSFiBigInt* Prime) {
    if (!B || !B->Mu) return;
    B->Mu->Contour = tsfi_bn_realloc(B->Mu->Contour); modPow_bn(B->Mu->Contour, Xi, B->Mu->Secret, Prime);
}

static void Form(struct SHA* B, TSFiBigInt* Chi, TSFiBigInt* Prime) {
    if (!B || !B->Mu) return;
    B->Mu->Base = tsfi_bn_realloc(B->Mu->Base); modPow_bn(B->Mu->Base, Chi, B->Mu->Secret, Prime);
    Tune(B->Mu, Prime);
}

static void Polarize(struct SHA* B, TSFiBigInt* Prime) {
    if (!B || !B->Mu) return;
    B->Mu->Pole = tsfi_bn_realloc(B->Mu->Pole); modPow_bn(B->Mu->Pole, B->Mu->Base, B->Mu->Secret, Prime);
}

static void ConductorGenerate(struct SHIO* R, TSFiBigInt* Xi, TSFiBigInt* Prime) {
    if (!R || !R->Rho || !R->Rho->Rod || !R->Rho->Cone) return;
    Avail(R->Rho->Rod, Xi, Prime);
    Avail(R->Rho->Cone, Xi, Prime);
    Form(R->Rho->Rod, R->Rho->Cone->Mu->Contour, Prime);
    Form(R->Rho->Cone, R->Rho->Rod->Mu->Contour, Prime);
    Polarize(R->Rho->Rod, Prime);
    Polarize(R->Rho->Cone, Prime);
}

static void Conjugate(struct SHA* B, TSFiBigInt* Chi, TSFiBigInt* Prime) {
    if (!B || !B->Mu) return;
    B->Mu->Coordinate = tsfi_bn_realloc(B->Mu->Coordinate); modPow_bn(B->Mu->Coordinate, Chi, B->Mu->Secret, Prime);
}

static void Conify(struct SHA* B, TSFiBigInt* _Beta, TSFiBigInt* Prime) {
    if (!B || !B->Mu) return;
    if(B->Mu->Identity == NULL || B->Mu->Identity->active_limbs == 0) {
        B->Mu->Identity = tsfi_bn_realloc(B->Mu->Identity); tsfi_bn_copy(B->Mu->Identity, _Beta);
        B->Mu->Foundation = tsfi_bn_realloc(B->Mu->Foundation); modPow_bn(B->Mu->Foundation, B->Mu->Base, B->Mu->Identity, Prime);
    }
}

static void Saturate(struct SHA* B, TSFiBigInt* _Beta, TSFiBigInt* Epsilon, TSFiBigInt* Theta, TSFiBigInt* Prime) {
    if (!B || !B->Mu) return;
    if(B->Mu->Identity == NULL || B->Mu->Identity->active_limbs == 0) {
        B->Mu->Identity = tsfi_bn_realloc(B->Mu->Identity); tsfi_bn_copy(B->Mu->Identity, _Beta);
        B->Mu->Foundation = tsfi_bn_realloc(B->Mu->Foundation); modPow_bn(B->Mu->Foundation, B->Mu->Base, B->Mu->Identity, Prime);
    }
    TSFiBigInt* Beta = tsfi_bn_alloc(); modPow_bn(Beta, Epsilon, B->Mu->Identity, Prime);
    TSFiBigInt* Rho = tsfi_bn_alloc(); modPow_bn(Rho, Theta, B->Mu->Identity, Prime);
    TSFiBigInt* Eta = tsfi_bn_alloc(); modPow_bn(Eta, Epsilon, B->Mu->Signal, Prime);
    B->Mu->Charge = tsfi_bn_realloc(B->Mu->Charge); tsfi_bn_add_avx512(B->Mu->Charge, Rho, Eta);
    B->Mu->Limit = tsfi_bn_realloc(B->Mu->Limit); tsfi_bn_add_avx512(B->Mu->Limit, Beta, Eta); 
    B->Mu->Element = tsfi_bn_realloc(B->Mu->Element); tsfi_bn_add_avx512(B->Mu->Element, Beta, B->Mu->Charge);
    tsfi_bn_free(Beta); tsfi_bn_free(Rho); tsfi_bn_free(Eta);
    B->Dynamo = tsfi_bn_realloc(B->Dynamo); modPow_bn(B->Dynamo, Theta, B->Mu->Signal, Prime);
    B->Mu->Monopole = tsfi_bn_realloc(B->Mu->Monopole); modPow_bn(B->Mu->Monopole, B->Mu->Limit, B->Mu->Identity, Prime);
}

static void GENERATE(struct SHIO* R, TSFiBigInt* Xi, TSFiBigInt* Alpha, TSFiBigInt* Beta, TSFiBigInt* Prime) {
    ConductorGenerate(R, Xi, Prime);
    Conjugate(R->Rho->Rod, R->Rho->Cone->Mu->Pole, Prime);
    Conjugate(R->Rho->Cone, R->Rho->Rod->Mu->Pole, Prime);
    Conify(R->Rho->Cone, Alpha, Prime);
    Saturate(R->Rho->Rod, Alpha, R->Rho->Cone->Mu->Foundation, R->Rho->Cone->Mu->Channel, Prime);
    Saturate(R->Rho->Cone, Beta, R->Rho->Rod->Mu->Foundation, R->Rho->Rod->Mu->Channel, Prime);
}

static void Bond(struct SHA* A) {
    if (!A || !A->Mu) return;
    A->Dynamo = tsfi_bn_realloc(A->Dynamo); modPow_bn(A->Dynamo, A->Mu->Base, A->Mu->Signal, A->Mu->Element);
    A->Mu->Pole = tsfi_bn_realloc(A->Mu->Pole); tsfi_bn_set_u64(A->Mu->Pole, 0);
}

static void IONIZE(struct SHIO* R) {
    if (!R || !R->Rho) return;
    Bond(R->Rho->Rod);
    Bond(R->Rho->Cone);
}

static void MAGNETIZE_batch8(struct YI** results, TSFiBigInt* Prime) {
    TSFiBigInt *temp_iota[8], *temp_heta[8], *temp_zeta[8], *res[8], *bases[8], *exps[8], *mods[8];
    TSFiModContextBatch8 ctx_prime; 
    for(int i=0; i<8; i++) { mods[i] = Prime; }
    tsfi_bn_mod_setup_batch8(&ctx_prime, mods);
    for (int i = 0; i < 8; i++) {
        struct SHIO* B = results[i]->Psi; if (!B->Manifold) B->Manifold = tsfi_bn_alloc();
        temp_iota[i] = tsfi_bn_alloc(); temp_heta[i] = tsfi_bn_alloc(); temp_zeta[i] = tsfi_bn_alloc();
    }
    for (int i = 0; i < 8; i++) {
        struct SHIO* B = results[i]->Psi;
        res[i] = B->Manifold; bases[i] = B->Rho->Cone->Dynamo; 
        exps[i] = B->Rho->Rod->Mu->Signal; mods[i] = B->Rho->Rod->Mu->Element;
    }
    TSFiModContextBatch8 ctx_rod; tsfi_bn_mod_setup_batch8(&ctx_rod, mods);
    tsfi_bn_modpow_batch8_ctx(res, bases, exps, mods, &ctx_rod);
    for (int i = 0; i < 8; i++) {
        struct SHIO* B = results[i]->Psi;
        res[i] = temp_iota[i]; bases[i] = B->Rho->Rod->Dynamo; 
        exps[i] = B->Rho->Cone->Mu->Signal; mods[i] = B->Rho->Cone->Mu->Element;
    }
    TSFiModContextBatch8 ctx_cone; tsfi_bn_mod_setup_batch8(&ctx_cone, mods);
    tsfi_bn_modpow_batch8_ctx(res, bases, exps, mods, &ctx_cone);
    for (int i = 0; i < 8; i++) {
        if(tsfi_bn_cmp_avx512(results[i]->Psi->Manifold, temp_iota[i]) != 0) { exit(1); }
        if (results[i]->Ring) tsfi_bn_free(results[i]->Ring);
        results[i]->Ring = tsfi_bn_alloc();
        struct SHIO* B = results[i]->Psi;
        res[i] = results[i]->Ring; bases[i] = B->Rho->Rod->Mu->Coordinate; 
        exps[i] = B->Manifold; mods[i] = B->Rho->Rod->Mu->Element;
    }
    tsfi_bn_modpow_batch8_ctx(res, bases, exps, mods, &ctx_rod);
    for (int i = 0; i < 8; i++) {
        struct SHIO* B = results[i]->Psi;
        res[i] = temp_heta[i]; bases[i] = B->Rho->Cone->Mu->Coordinate; 
        exps[i] = B->Manifold; mods[i] = B->Rho->Cone->Mu->Element;
    }
    tsfi_bn_modpow_batch8_ctx(res, bases, exps, mods, &ctx_cone);
    for (int i = 0; i < 8; i++) {
        if(tsfi_bn_cmp_avx512(results[i]->Ring, temp_heta[i]) != 0) { exit(1); }
        struct SHIO* B = results[i]->Psi;
        if (!B->Rho->Barn) B->Rho->Barn = tsfi_bn_alloc();
        res[i] = B->Rho->Barn; bases[i] = results[i]->Ring; 
        exps[i] = B->Manifold; mods[i] = B->Rho->Rod->Mu->Element;
    }
    tsfi_bn_modpow_batch8_ctx(res, bases, exps, mods, &ctx_rod);
    for (int i = 0; i < 8; i++) {
        struct SHIO* B = results[i]->Psi;
        res[i] = temp_zeta[i]; bases[i] = results[i]->Ring; 
        exps[i] = B->Manifold; mods[i] = B->Rho->Cone->Mu->Element;
    }
    tsfi_bn_modpow_batch8_ctx(res, bases, exps, mods, &ctx_cone);
    for (int i = 0; i < 8; i++) {
        if(tsfi_bn_cmp_avx512(results[i]->Psi->Rho->Barn, temp_zeta[i]) != 0) { exit(1); }
        struct SHIO* B = results[i]->Psi;
        if (!B->Monopole) B->Monopole = tsfi_bn_alloc();
        res[i] = B->Monopole; bases[i] = B->Rho->Rod->Mu->Limit; 
        exps[i] = B->Rho->Cone->Mu->Limit; mods[i] = Prime;
    }
    tsfi_bn_modpow_batch8_ctx(res, bases, exps, mods, &ctx_prime);
    for (int i = 0; i < 8; i++) {
        tsfi_bn_free(temp_iota[i]); tsfi_bn_free(temp_heta[i]); tsfi_bn_free(temp_zeta[i]);
    }
}

static struct SHA* STAT_reused(struct SHA* A, TSFiBigInt* p_bn) {
    if (!A) A = allocSHA();
    struct Fa* F = A->Mu; if (!F) { F = allocFa(); A->Mu = F; }
    if (F->Base) { tsfi_bn_free(F->Base); } F->Base = RANDOM_bn(p_bn);
    if (F->Secret) { tsfi_bn_free(F->Secret); } F->Secret = RANDOM_bn(p_bn);
    if (F->Signal) { tsfi_bn_free(F->Signal); } F->Signal = RANDOM_bn(p_bn);
    F->Channel = tsfi_bn_realloc(F->Channel); F->Contour = tsfi_bn_realloc(F->Contour);
    F->Pole = tsfi_bn_realloc(F->Pole); F->Identity = tsfi_bn_realloc(F->Identity);
    F->Foundation = tsfi_bn_realloc(F->Foundation); F->Element = tsfi_bn_realloc(F->Element);
    F->Coordinate = tsfi_bn_realloc(F->Coordinate); F->Charge = tsfi_bn_realloc(F->Charge);
    F->Limit = tsfi_bn_realloc(F->Limit); F->Monopole = tsfi_bn_realloc(F->Monopole);
    Tune(F, p_bn); A->Dynamo = tsfi_bn_realloc(A->Dynamo); return A;
}

static void Adduct_reused(TSFiBigInt* R, struct SHA* Mu, TSFiBigInt* _Phi) {
    if (!R || !Mu || !Mu->Mu) return;
    modPow_bn(R, _Phi, Mu->Mu->Signal, Mu->Mu->Element);
}

static TSFiBigInt* MAGNETIZE_reused(struct SHIO* B, TSFiBigInt* Prime, TSFiBigInt* temp_iota, TSFiBigInt* temp_heta, TSFiBigInt* temp_zeta) {
    if (!B || !B->Rho || !B->Rho->Rod || !B->Rho->Cone) return NULL;
    if (!B->Manifold) B->Manifold = tsfi_bn_alloc();
    Adduct_reused(B->Manifold, B->Rho->Rod, B->Rho->Cone->Dynamo);
    Adduct_reused(temp_iota, B->Rho->Cone, B->Rho->Rod->Dynamo);
    if(tsfi_bn_cmp_avx512(B->Manifold, temp_iota) != 0) { exit(1); }
    TSFiBigInt* Ring = tsfi_bn_alloc();
    modPow_bn(Ring, B->Rho->Rod->Mu->Coordinate, B->Manifold, B->Rho->Rod->Mu->Element);
    modPow_bn(temp_heta, B->Rho->Cone->Mu->Coordinate, B->Manifold, B->Rho->Cone->Mu->Element);
    if(tsfi_bn_cmp_avx512(Ring, temp_heta) != 0) { exit(1); }
    if (!B->Rho->Barn) B->Rho->Barn = tsfi_bn_alloc();
    modPow_bn(B->Rho->Barn, Ring, B->Manifold, B->Rho->Rod->Mu->Element);
    modPow_bn(temp_zeta, Ring, B->Manifold, B->Rho->Cone->Mu->Element);
    if(tsfi_bn_cmp_avx512(B->Rho->Barn, temp_zeta) != 0) { exit(1); }
    if (!B->Monopole) B->Monopole = tsfi_bn_alloc();
    modPow_bn(B->Monopole, B->Rho->Rod->Mu->Limit, B->Rho->Cone->Mu->Limit, Prime);
    return Ring;
}

void tsfi_reaction_boot_firmware_yi(struct YI* R) {
    if (!R) return;
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);
    
    // Utilize the AVX-512 hardware path to break the 5ms scalar limit
    struct YI* batch[8] = {NULL};
    batch[0] = R; 
    
    // The fused generator inherently re-uses R if provided
    tsfi_reaction_shoot_batch8_fused(batch, Prime);
    
    // Discard the 7 auxiliary synthetic firmwares generated by the vector pipeline
    for (int i = 1; i < 8; i++) {
        if (batch[i]) freeYI(batch[i]);
    }
    
    tsfi_bn_free(Prime);
}

struct YI* tsfi_reaction_shoot(TSFiBigInt* Prime) {
    struct SHIO* B = allocSHIO(); struct SHAO* A = allocSHAO(); struct YI* R = allocYI();
    memset(B, 0, sizeof(struct SHIO)); memset(A, 0, sizeof(struct SHAO)); memset(R, 0, sizeof(struct YI));
    A->Rod = STAT(Prime); A->Cone = STAT(Prime); B->Rho = A; R->Psi = B;
    TSFiBigInt* Xi = tsfi_bn_alloc(); tsfi_bn_randomize(Xi);
    TSFiBigInt* zD = tsfi_bn_alloc(); tsfi_bn_randomize(zD);
    TSFiBigInt* zI = tsfi_bn_alloc(); tsfi_bn_randomize(zI);
    R->Xi = tsfi_bn_realloc(R->Xi); tsfi_bn_copy(R->Xi, Xi);
    GENERATE(R->Psi, R->Xi, zD, zI, Prime); IONIZE(R->Psi);
    TSFiBigInt *t1 = tsfi_bn_alloc(), *t2 = tsfi_bn_alloc(), *t3 = tsfi_bn_alloc();
    R->Ring = MAGNETIZE_reused(R->Psi, Prime, t1, t2, t3);
    tsfi_bn_free(t1); tsfi_bn_free(t2); tsfi_bn_free(t3);
    tsfi_bn_free(Xi); tsfi_bn_free(zD); tsfi_bn_free(zI); return R;
}

struct YI* tsfi_reaction_fuse(TSFiBigInt* Xi, TSFiBigInt* Rho, TSFiBigInt* Ohm, struct SHAO* orig_rho, TSFiBigInt* Prime) {
    struct SHIO* B = allocSHIO(); struct SHAO* A = allocSHAO(); struct YI* R = allocYI();
    memset(B, 0, sizeof(struct SHIO)); memset(A, 0, sizeof(struct SHAO)); memset(R, 0, sizeof(struct YI));
    if (orig_rho) {
        A->Rod = allocSHA(); A->Rod->Mu = allocFa(); memset(A->Rod->Mu, 0, sizeof(struct Fa));
        A->Rod->Mu->Base = tsfi_bn_alloc(); tsfi_bn_copy(A->Rod->Mu->Base, orig_rho->Rod->Mu->Base);
        A->Rod->Mu->Secret = tsfi_bn_alloc(); tsfi_bn_copy(A->Rod->Mu->Secret, orig_rho->Rod->Mu->Secret);
        A->Rod->Mu->Signal = tsfi_bn_alloc(); tsfi_bn_copy(A->Rod->Mu->Signal, orig_rho->Rod->Mu->Signal);
        Tune(A->Rod->Mu, Prime);

        A->Cone = allocSHA(); A->Cone->Mu = allocFa(); memset(A->Cone->Mu, 0, sizeof(struct Fa));
        A->Cone->Mu->Base = tsfi_bn_alloc(); tsfi_bn_copy(A->Cone->Mu->Base, orig_rho->Cone->Mu->Base);
        A->Cone->Mu->Secret = tsfi_bn_alloc(); tsfi_bn_copy(A->Cone->Mu->Secret, orig_rho->Cone->Mu->Secret);
        A->Cone->Mu->Signal = tsfi_bn_alloc(); tsfi_bn_copy(A->Cone->Mu->Signal, orig_rho->Cone->Mu->Signal);
        Tune(A->Cone->Mu, Prime);
    } else {
        A->Rod = STAT(Prime); A->Cone = STAT(Prime);
    }
    B->Rho = A; R->Psi = B;
    R->Xi = tsfi_bn_realloc(R->Xi); tsfi_bn_copy(R->Xi, Xi);
    GENERATE(R->Psi, R->Xi, Ohm, Rho, Prime); IONIZE(R->Psi);
    TSFiBigInt *t1 = tsfi_bn_alloc(), *t2 = tsfi_bn_alloc(), *t3 = tsfi_bn_alloc();
    R->Ring = MAGNETIZE_reused(R->Psi, Prime, t1, t2, t3);
    tsfi_bn_free(t1); tsfi_bn_free(t2); tsfi_bn_free(t3);
    return R;
}

struct Dai* tsfi_reaction_compute_reciprocity(struct SHIO* shio, TSFiBigInt* Pi, TSFiBigInt* Theta) {
    (void)Theta; if (!shio || !shio->Rho || !shio->Rho->Rod || !shio->Rho->Cone) return NULL;
    TSFiBigInt* local_Pi = Pi;
    if (shio->Monopole) {
        local_Pi = tsfi_bn_alloc();
        int max = (Pi->active_limbs > shio->Monopole->active_limbs) ? Pi->active_limbs : shio->Monopole->active_limbs;
        for(int i=0; i<max; i++) {
            uint64_t va = ((size_t)i < Pi->active_limbs) ? Pi->limbs[i] : 0;
            uint64_t vb = ((size_t)i < shio->Monopole->active_limbs) ? shio->Monopole->limbs[i] : 0;
            local_Pi->limbs[i] = va ^ vb;
        }
        local_Pi->active_limbs = max;
        while(local_Pi->active_limbs > 0 && local_Pi->limbs[local_Pi->active_limbs-1] == 0) local_Pi->active_limbs--;
    }
    struct Dai* D = allocDai();
    D->Ichidai = tsfi_bn_realloc(D->Ichidai); modPow_bn(D->Ichidai, local_Pi, shio->Rho->Rod->Mu->Channel, shio->Rho->Cone->Mu->Channel);
    D->Daiichi = tsfi_bn_realloc(D->Daiichi); modPow_bn(D->Daiichi, local_Pi, shio->Rho->Cone->Mu->Channel, shio->Rho->Rod->Mu->Channel);
    if (local_Pi != Pi) { tsfi_bn_free(local_Pi); }
    return D;
}

TSFiFusedOp* tsfi_reaction_build_shao(const char* name, struct SHAO* shao) {
    if (!shao) return NULL;
    TSFiFusedOp* fused = tsfi_fused_create(name);
    TSFiOperator* op_shao = tsfi_op_create("SHAO_Core", tsfi_hotload_thunk("thunks/wave_shader.c", "wave_shader_main"), wave_shader_spv, wave_shader_spv_size);
    tsfi_fused_add(fused, op_shao); return fused;
}

TSFiFusedOp* tsfi_reaction_build_shio(const char* name, struct SHIO* shio) {
    if (!shio) return NULL;
    TSFiFusedOp* fused = tsfi_reaction_build_shao(name, shio->Rho);
    if (!fused) fused = tsfi_fused_create(name);
    TSFiOperator* op_morph = tsfi_op_create("SHIO_Morph", tsfi_hotload_thunk("thunks/wave_morph.c", "wave_morph_main"), wave_shader_spv, wave_shader_spv_size);
    tsfi_fused_add(fused, op_morph); return fused;
}

TSFiFusedOp* tsfi_reaction_build_yi(const char* name, struct YI* yi, DysnomiaReactiveState* state) {
    if (!yi) { return NULL; }
    (void)state;
    TSFiFusedOp* fused = tsfi_reaction_build_shio(name, yi->Psi);
    if (!fused) fused = tsfi_fused_create(name);
    TSFiOperator* op_crc = tsfi_op_create("YI_CRC", tsfi_hotload_thunk("thunks/dysnomia_reactive_crc.c", "dysnomia_reactive_crc_thunk"), wave_shader_spv, wave_shader_spv_size);
    tsfi_fused_add(fused, op_crc); return fused;
}

static void Tune_batch8(struct Fa** Fs, TSFiBigInt* Prime) {
    TSFiBigInt *results[8], *bases[8], *exps[8], *mods[8];
    for (int i = 0; i < 8; i++) {
        Fs[i]->Channel = tsfi_bn_realloc(Fs[i]->Channel);
        results[i] = Fs[i]->Channel; bases[i] = Fs[i]->Base; exps[i] = Fs[i]->Signal; mods[i] = Prime;
    }
    tsfi_bn_modpow_batch8(results, bases, exps, mods);
}

static void Avail_batch8(struct SHA** Bs, TSFiBigInt** Xis, TSFiBigInt* Prime) {
    TSFiBigInt *results[8], *bases[8], *exps[8], *mods[8];
    for (int i = 0; i < 8; i++) {
        Bs[i]->Mu->Contour = tsfi_bn_realloc(Bs[i]->Mu->Contour);
        results[i] = Bs[i]->Mu->Contour; bases[i] = Xis[i]; exps[i] = Bs[i]->Mu->Secret; mods[i] = Prime;
    }
    tsfi_bn_modpow_batch8(results, bases, exps, mods);
}

static void Form_batch8(struct SHA** Bs, TSFiBigInt** Chis, TSFiBigInt* Prime) {
    TSFiBigInt *results[8], *bases[8], *exps[8], *mods[8]; struct Fa* Fs[8];
    for (int i = 0; i < 8; i++) {
        Bs[i]->Mu->Base = tsfi_bn_realloc(Bs[i]->Mu->Base);
        results[i] = Bs[i]->Mu->Base; bases[i] = Chis[i]; exps[i] = Bs[i]->Mu->Secret; mods[i] = Prime;
        Fs[i] = Bs[i]->Mu;
    }
    tsfi_bn_modpow_batch8(results, bases, exps, mods); Tune_batch8(Fs, Prime);
}

static void Polarize_batch8(struct SHA** Bs, TSFiBigInt* Prime) {
    TSFiBigInt *results[8], *bases[8], *exps[8], *mods[8];
    for (int i = 0; i < 8; i++) {
        Bs[i]->Mu->Pole = tsfi_bn_realloc(Bs[i]->Mu->Pole);
        results[i] = Bs[i]->Mu->Pole; bases[i] = Bs[i]->Mu->Base; exps[i] = Bs[i]->Mu->Secret; mods[i] = Prime;
    }
    tsfi_bn_modpow_batch8(results, bases, exps, mods);
}

static void Conjugate_batch8(struct SHA** Bs, TSFiBigInt** Chis, TSFiBigInt* Prime) {
    TSFiBigInt *results[8], *bases[8], *exps[8], *mods[8];
    for (int i = 0; i < 8; i++) {
        Bs[i]->Mu->Coordinate = tsfi_bn_realloc(Bs[i]->Mu->Coordinate);
        results[i] = Bs[i]->Mu->Coordinate; bases[i] = Chis[i]; exps[i] = Bs[i]->Mu->Secret; mods[i] = Prime;
    }
    tsfi_bn_modpow_batch8(results, bases, exps, mods);
}

static void Conify_batch8(struct SHA** Bs, TSFiBigInt** Betas, TSFiBigInt* Prime) {
    TSFiBigInt *results[8], *bases[8], *exps[8], *mods[8];
    for (int i = 0; i < 8; i++) {
        Bs[i]->Mu->Identity = tsfi_bn_realloc(Bs[i]->Mu->Identity); tsfi_bn_copy(Bs[i]->Mu->Identity, Betas[i]);
        Bs[i]->Mu->Foundation = tsfi_bn_realloc(Bs[i]->Mu->Foundation);
        results[i] = Bs[i]->Mu->Foundation; bases[i] = Bs[i]->Mu->Base; exps[i] = Bs[i]->Mu->Identity; mods[i] = Prime;
    }
    tsfi_bn_modpow_batch8(results, bases, exps, mods);
}

static void Saturate_batch8(struct SHA** Bs, TSFiBigInt** Betas, TSFiBigInt** Epsilons, TSFiBigInt** Thetas, TSFiBigInt* Prime) {
    TSFiBigInt *results[8], *bases[8], *exps[8], *mods[8]; bool needs_init = false;
    for (int i = 0; i < 8; i++) {
        if (Bs[i]->Mu->Identity == NULL || Bs[i]->Mu->Identity->active_limbs == 0) {
            Bs[i]->Mu->Identity = tsfi_bn_realloc(Bs[i]->Mu->Identity); tsfi_bn_copy(Bs[i]->Mu->Identity, Betas[i]);
            Bs[i]->Mu->Foundation = tsfi_bn_realloc(Bs[i]->Mu->Foundation); needs_init = true;
        }
        results[i] = Bs[i]->Mu->Foundation; bases[i] = Bs[i]->Mu->Base; exps[i] = Bs[i]->Mu->Identity; mods[i] = Prime;
    }
    if (needs_init) tsfi_bn_modpow_batch8(results, bases, exps, mods);
    TSFiModContextBatch8 ctx_prime; for(int i=0; i<8; i++) { mods[i] = Prime; }
    tsfi_bn_mod_setup_batch8(&ctx_prime, mods);
    TSFiBigInt *v_Beta[8], *v_Rho[8], *v_Eta[8];
    for (int i = 0; i < 8; i++) { v_Beta[i] = tsfi_bn_alloc(); v_Rho[i] = tsfi_bn_alloc(); v_Eta[i] = tsfi_bn_alloc(); }
    for (int i = 0; i < 8; i++) { results[i] = v_Beta[i]; bases[i] = Epsilons[i]; exps[i] = Bs[i]->Mu->Identity; }
    tsfi_bn_modpow_batch8_ctx(results, bases, exps, mods, &ctx_prime);
    for (int i = 0; i < 8; i++) { results[i] = v_Rho[i]; bases[i] = Thetas[i]; exps[i] = Bs[i]->Mu->Identity; }
    tsfi_bn_modpow_batch8_ctx(results, bases, exps, mods, &ctx_prime);
    for (int i = 0; i < 8; i++) { results[i] = v_Eta[i]; bases[i] = Epsilons[i]; exps[i] = Bs[i]->Mu->Signal; }
    tsfi_bn_modpow_batch8_ctx(results, bases, exps, mods, &ctx_prime);
    for (int i = 0; i < 8; i++) {
        Bs[i]->Mu->Charge = tsfi_bn_realloc(Bs[i]->Mu->Charge); tsfi_bn_add_avx512(Bs[i]->Mu->Charge, v_Rho[i], v_Eta[i]);
        Bs[i]->Mu->Limit = tsfi_bn_realloc(Bs[i]->Mu->Limit); tsfi_bn_add_avx512(Bs[i]->Mu->Limit, v_Beta[i], v_Eta[i]);
        Bs[i]->Mu->Element = tsfi_bn_realloc(Bs[i]->Mu->Element); tsfi_bn_add_avx512(Bs[i]->Mu->Element, v_Beta[i], Bs[i]->Mu->Charge);
    }
    for (int i = 0; i < 8; i++) { 
        Bs[i]->Dynamo = tsfi_bn_realloc(Bs[i]->Dynamo);
        results[i] = Bs[i]->Dynamo; bases[i] = Thetas[i]; exps[i] = Bs[i]->Mu->Signal; 
    }
    tsfi_bn_modpow_batch8_ctx(results, bases, exps, mods, &ctx_prime);
    for (int i = 0; i < 8; i++) {
        Bs[i]->Mu->Monopole = tsfi_bn_realloc(Bs[i]->Mu->Monopole);
        results[i] = Bs[i]->Mu->Monopole; bases[i] = Bs[i]->Mu->Limit; exps[i] = Bs[i]->Mu->Identity;
    }
    tsfi_bn_modpow_batch8_ctx(results, bases, exps, mods, &ctx_prime);
    for (int i = 0; i < 8; i++) { tsfi_bn_free(v_Beta[i]); tsfi_bn_free(v_Rho[i]); tsfi_bn_free(v_Eta[i]); }
}

void tsfi_reaction_shoot_batch8(struct YI** results, TSFiBigInt* p_bn) {
    struct SHA *rods[8], *cones[8]; TSFiBigInt *xis[8], *zD[8], *zI[8];
    for (int i = 0; i < 8; i++) {
        if (!results[i]) results[i] = allocYI();
        if (!results[i]->Psi) results[i]->Psi = allocSHIO();
        if (!results[i]->Psi->Rho) results[i]->Psi->Rho = allocSHAO();
        struct SHAO* A = results[i]->Psi->Rho;
        A->Rod = STAT_reused(A->Rod, p_bn); A->Cone = STAT_reused(A->Cone, p_bn);
        rods[i] = A->Rod; cones[i] = A->Cone;
        results[i]->Xi = tsfi_bn_realloc(results[i]->Xi); 
        xis[i] = RANDOM_bn(p_bn); tsfi_bn_copy(results[i]->Xi, xis[i]);
        zD[i] = RANDOM_bn(p_bn); zI[i] = RANDOM_bn(p_bn);
    }
    Avail_batch8(rods, xis, p_bn); Avail_batch8(cones, xis, p_bn);
    TSFiBigInt *rod_contours[8], *cone_contours[8];
    for(int i=0; i<8; i++) { rod_contours[i] = rods[i]->Mu->Contour; cone_contours[i] = cones[i]->Mu->Contour; }
    Form_batch8(rods, cone_contours, p_bn); Form_batch8(cones, rod_contours, p_bn);
    Polarize_batch8(rods, p_bn); Polarize_batch8(cones, p_bn);
    TSFiBigInt *rod_poles[8], *cone_poles[8];
    for(int i=0; i<8; i++) { rod_poles[i] = rods[i]->Mu->Pole; cone_poles[i] = cones[i]->Mu->Pole; }
    Conjugate_batch8(rods, cone_poles, p_bn); Conjugate_batch8(cones, rod_poles, p_bn);
    Conify_batch8(cones, zD, p_bn);
    TSFiBigInt *cone_foundations[8], *cone_channels[8], *rod_foundations[8], *rod_channels[8];
    for(int i=0; i<8; i++) {
        cone_foundations[i] = cones[i]->Mu->Foundation; cone_channels[i] = cones[i]->Mu->Channel;
        rod_foundations[i] = rods[i]->Mu->Foundation; rod_channels[i] = rods[i]->Mu->Channel;
    }
    Saturate_batch8(rods, zD, cone_foundations, cone_channels, p_bn); Saturate_batch8(cones, zI, rod_foundations, rod_channels, p_bn);
    for (int i = 0; i < 8; i++) { Bond(rods[i]); Bond(cones[i]); }
    MAGNETIZE_batch8(results, p_bn);
    for (int i = 0; i < 8; i++) { tsfi_bn_free(xis[i]); tsfi_bn_free(zD[i]); tsfi_bn_free(zI[i]); }
}

void tsfi_reaction_shoot_batch8_fused(struct YI** results, TSFiBigInt* p_bn) {
    struct SHA *rods[8], *cones[8]; TSFiBigInt *xis[8], *zD[8], *zI[8];
    TSFiModContextBatch8 ctx_prime; TSFiBigInt* mods[8];
    for(int i=0; i<8; i++) { mods[i] = p_bn; }
    tsfi_bn_mod_setup_batch8(&ctx_prime, mods);
    for (int i = 0; i < 8; i++) {
        if (!results[i]) results[i] = allocYI();
        if (!results[i]->Psi) results[i]->Psi = allocSHIO();
        if (!results[i]->Psi->Rho) results[i]->Psi->Rho = allocSHAO();
        struct SHAO* A = results[i]->Psi->Rho;
        A->Rod = STAT_reused(A->Rod, p_bn); A->Cone = STAT_reused(A->Cone, p_bn);
        rods[i] = A->Rod; cones[i] = A->Cone;
        results[i]->Xi = tsfi_bn_realloc(results[i]->Xi); 
        xis[i] = RANDOM_bn(p_bn); tsfi_bn_copy(results[i]->Xi, xis[i]);
        zD[i] = RANDOM_bn(p_bn); zI[i] = RANDOM_bn(p_bn);
    }
    TSFiBigInt *rb[8], *re[8], *rr[8];
    for(int i=0; i<8; i++) { rods[i]->Mu->Contour = tsfi_bn_realloc(rods[i]->Mu->Contour); rb[i] = xis[i]; re[i] = rods[i]->Mu->Secret; rr[i] = rods[i]->Mu->Contour; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Contour = tsfi_bn_realloc(cones[i]->Mu->Contour); rb[i] = xis[i]; re[i] = cones[i]->Mu->Secret; rr[i] = cones[i]->Mu->Contour; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { rods[i]->Mu->Base = tsfi_bn_realloc(rods[i]->Mu->Base); rb[i] = cones[i]->Mu->Contour; re[i] = rods[i]->Mu->Secret; rr[i] = rods[i]->Mu->Base; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Base = tsfi_bn_realloc(cones[i]->Mu->Base); rb[i] = rods[i]->Mu->Contour; re[i] = cones[i]->Mu->Secret; rr[i] = cones[i]->Mu->Base; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { rods[i]->Mu->Channel = tsfi_bn_realloc(rods[i]->Mu->Channel); rb[i] = rods[i]->Mu->Base; re[i] = rods[i]->Mu->Signal; rr[i] = rods[i]->Mu->Channel; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Channel = tsfi_bn_realloc(cones[i]->Mu->Channel); rb[i] = cones[i]->Mu->Base; re[i] = cones[i]->Mu->Signal; rr[i] = cones[i]->Mu->Channel; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { rods[i]->Mu->Pole = tsfi_bn_realloc(rods[i]->Mu->Pole); rb[i] = rods[i]->Mu->Base; re[i] = rods[i]->Mu->Secret; rr[i] = rods[i]->Mu->Pole; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Pole = tsfi_bn_realloc(cones[i]->Mu->Pole); rb[i] = cones[i]->Mu->Base; re[i] = cones[i]->Mu->Secret; rr[i] = cones[i]->Mu->Pole; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { rods[i]->Mu->Coordinate = tsfi_bn_realloc(rods[i]->Mu->Coordinate); rb[i] = cones[i]->Mu->Pole; re[i] = rods[i]->Mu->Secret; rr[i] = rods[i]->Mu->Coordinate; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Coordinate = tsfi_bn_realloc(cones[i]->Mu->Coordinate); rb[i] = rods[i]->Mu->Pole; re[i] = cones[i]->Mu->Secret; rr[i] = cones[i]->Mu->Coordinate; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { 
        cones[i]->Mu->Identity = tsfi_bn_realloc(cones[i]->Mu->Identity); tsfi_bn_copy(cones[i]->Mu->Identity, zI[i]);
        cones[i]->Mu->Foundation = tsfi_bn_realloc(cones[i]->Mu->Foundation); 
        rb[i] = cones[i]->Mu->Base; re[i] = cones[i]->Mu->Identity; rr[i] = cones[i]->Mu->Foundation; 
    }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for (int i = 0; i < 8; i++) { 
        rods[i]->Mu->Identity = tsfi_bn_realloc(rods[i]->Mu->Identity); tsfi_bn_copy(rods[i]->Mu->Identity, zD[i]);
        rods[i]->Mu->Foundation = tsfi_bn_realloc(rods[i]->Mu->Foundation); 
        rb[i] = rods[i]->Mu->Base; re[i] = rods[i]->Mu->Identity; rr[i] = rods[i]->Mu->Foundation; 
    }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    TSFiBigInt *cone_foundations[8], *cone_channels[8], *rod_foundations[8], *rod_channels[8];
    for(int i=0; i<8; i++) { cone_foundations[i] = cones[i]->Mu->Foundation; cone_channels[i] = cones[i]->Mu->Channel; rod_foundations[i] = rods[i]->Mu->Foundation; rod_channels[i] = rods[i]->Mu->Channel; }
    Saturate_batch8(rods, zD, cone_foundations, cone_channels, p_bn); Saturate_batch8(cones, zI, rod_foundations, rod_channels, p_bn);
    for (int i = 0; i < 8; i++) { Bond(rods[i]); Bond(cones[i]); }
    MAGNETIZE_batch8(results, p_bn);
    for (int i = 0; i < 8; i++) { tsfi_bn_free(xis[i]); tsfi_bn_free(zD[i]); tsfi_bn_free(zI[i]); }
}

static TSFiBigInt *g_shoot_t1 = NULL, *g_shoot_t2 = NULL, *g_shoot_t3 = NULL;

struct YI* tsfi_reaction_shoot_bn_reused(struct YI* R, TSFiBigInt* p_bn, int shot_idx) {
    (void)shot_idx; if (!R) R = allocYI(); if (!R->Psi) R->Psi = allocSHIO();
    struct SHIO* B = R->Psi; if (!B->Rho) B->Rho = allocSHAO();
    struct SHAO* A = B->Rho;
    A->Rod = STAT_reused(A->Rod, p_bn); A->Cone = STAT_reused(A->Cone, p_bn);
    if (R->Xi) { tsfi_bn_free(R->Xi); } R->Xi = RANDOM_bn(p_bn);
    TSFiBigInt *zD = RANDOM_bn(p_bn), *zI = RANDOM_bn(p_bn);
    GENERATE(R->Psi, R->Xi, zD, zI, p_bn); IONIZE(R->Psi);
    if (R->Ring) tsfi_bn_free(R->Ring);
    if (!g_shoot_t1) { g_shoot_t1 = tsfi_bn_alloc(); }
    if (!g_shoot_t2) { g_shoot_t2 = tsfi_bn_alloc(); }
    if (!g_shoot_t3) { g_shoot_t3 = tsfi_bn_alloc(); }
    R->Ring = MAGNETIZE_reused(R->Psi, p_bn, g_shoot_t1, g_shoot_t2, g_shoot_t3);
    tsfi_bn_free(zD); tsfi_bn_free(zI); return R;
}

struct Dai* ReactSHIO_bn_reused(struct Dai* dI, struct SHIO* Mu, TSFiBigInt* pi_bn) {
    if (!Mu || !Mu->Rho || !Mu->Rho->Rod || !Mu->Rho->Cone || !pi_bn) return NULL;
    TSFiBigInt* local_Pi = pi_bn;
    if (Mu->Monopole) {
        local_Pi = tsfi_bn_alloc();
        int max = (pi_bn->active_limbs > Mu->Monopole->active_limbs) ? pi_bn->active_limbs : Mu->Monopole->active_limbs;
        for(int i=0; i<max; i++) {
            uint64_t va = ((size_t)i < pi_bn->active_limbs) ? pi_bn->limbs[i] : 0;
            uint64_t vb = ((size_t)i < Mu->Monopole->active_limbs) ? Mu->Monopole->limbs[i] : 0;
            local_Pi->limbs[i] = va ^ vb;
        }
        local_Pi->active_limbs = max;
        while(local_Pi->active_limbs > 0 && local_Pi->limbs[local_Pi->active_limbs-1] == 0) local_Pi->active_limbs--;
    }
    if (!dI) dI = allocDai();
    dI->Ichidai = tsfi_bn_realloc(dI->Ichidai); modPow_bn(dI->Ichidai, local_Pi, Mu->Rho->Rod->Mu->Channel, Mu->Rho->Cone->Mu->Channel);
    dI->Daiichi = tsfi_bn_realloc(dI->Daiichi); modPow_bn(dI->Daiichi, local_Pi, Mu->Rho->Cone->Mu->Channel, Mu->Rho->Rod->Mu->Channel);
    struct Dai* dY = allocDai();
    dY->Ichidai = tsfi_bn_alloc(); modPow_bn(dY->Ichidai, local_Pi, Mu->Rho->Cone->Mu->Channel, Mu->Rho->Rod->Mu->Channel);
    dY->Daiichi = tsfi_bn_alloc(); modPow_bn(dY->Daiichi, local_Pi, Mu->Rho->Rod->Mu->Channel, Mu->Rho->Cone->Mu->Channel);
    if (local_Pi != pi_bn) tsfi_bn_free(local_Pi);
    if(tsfi_bn_cmp_avx512(dI->Ichidai, dY->Daiichi) != 0 || tsfi_bn_cmp_avx512(dI->Daiichi, dY->Ichidai) != 0) { freeDAI(dY); return NULL; }
    freeDAI(dY); return dI;
}

void tsfi_helmholtz_step(TSFiHelmholtzAdductState* state, TSFiBigInt* external_input) {
    if (!state) return;
    switch (state->current_epoch) {
        case HELMHOLTZ_EPOCH_INIT:
            if (!state->Mu) { state->Mu = allocSHA(); state->Mu->Mu = allocFa(); memset(state->Mu->Mu, 0, sizeof(struct Fa));
                state->Mu->Mu->Base = tsfi_bn_realloc(state->Mu->Mu->Base); tsfi_bn_randomize(state->Mu->Mu->Base);
                state->Mu->Mu->Secret = tsfi_bn_realloc(state->Mu->Mu->Secret); tsfi_bn_randomize(state->Mu->Mu->Secret);
                state->Mu->Mu->Signal = tsfi_bn_realloc(state->Mu->Mu->Signal); tsfi_bn_randomize(state->Mu->Mu->Signal);
                Tune(state->Mu->Mu, state->Prime); }
            state->current_epoch = HELMHOLTZ_EPOCH_AVAIL; break;
        case HELMHOLTZ_EPOCH_AVAIL: Avail(state->Mu, state->Xi, state->Prime); state->current_epoch = HELMHOLTZ_EPOCH_FORM; break;
        case HELMHOLTZ_EPOCH_FORM: if (!external_input) return; Form(state->Mu, external_input, state->Prime); state->current_epoch = HELMHOLTZ_EPOCH_POLARIZE; break;
        case HELMHOLTZ_EPOCH_POLARIZE: Polarize(state->Mu, state->Prime); state->current_epoch = HELMHOLTZ_EPOCH_CONJUGATE; break;
        case HELMHOLTZ_EPOCH_CONJUGATE: if (!external_input) return; Conjugate(state->Mu, external_input, state->Prime); state->current_epoch = HELMHOLTZ_EPOCH_CONIFY; break;
        case HELMHOLTZ_EPOCH_CONIFY: Conify(state->Mu, state->Beta, state->Prime); state->current_epoch = HELMHOLTZ_EPOCH_SATURATE; break;
        case HELMHOLTZ_EPOCH_SATURATE: if (!external_input) return; Saturate(state->Mu, state->Beta, external_input, state->Mu->Mu->Channel, state->Prime); state->current_epoch = HELMHOLTZ_EPOCH_IONIZE; break;
        case HELMHOLTZ_EPOCH_IONIZE: Bond(state->Mu); state->current_epoch = HELMHOLTZ_EPOCH_MAGNETIZE; break;
        case HELMHOLTZ_EPOCH_MAGNETIZE: state->current_epoch = HELMHOLTZ_EPOCH_DONE; break;
        default: break;
    }
}

void tsfi_helmholtz_synchronize(TSFiHelmholtzAdductState* rod, TSFiHelmholtzAdductState* cone) {
    if (!rod || !cone) return;
    // INIT -> AVAIL
    tsfi_helmholtz_step(rod, NULL); tsfi_helmholtz_step(cone, NULL);
    // AVAIL -> FORM
    tsfi_helmholtz_step(rod, NULL); tsfi_helmholtz_step(cone, NULL);
    // FORM -> POLARIZE
    tsfi_helmholtz_step(rod, cone->Mu->Mu->Contour); tsfi_helmholtz_step(cone, rod->Mu->Mu->Contour);
    // POLARIZE -> CONJUGATE
    tsfi_helmholtz_step(rod, NULL); tsfi_helmholtz_step(cone, NULL);
    // CONJUGATE -> CONIFY
    tsfi_helmholtz_step(rod, cone->Mu->Mu->Pole); tsfi_helmholtz_step(cone, rod->Mu->Mu->Pole);
    // CONIFY -> SATURATE
    tsfi_helmholtz_step(rod, NULL); tsfi_helmholtz_step(cone, NULL);
    // SATURATE -> IONIZE
    tsfi_helmholtz_step(rod, cone->Mu->Mu->Foundation); tsfi_helmholtz_step(cone, rod->Mu->Mu->Foundation);
    // IONIZE -> MAGNETIZE
    tsfi_helmholtz_step(rod, NULL); tsfi_helmholtz_step(cone, NULL);
    // MAGNETIZE -> DONE
    tsfi_helmholtz_step(rod, NULL); tsfi_helmholtz_step(cone, NULL);
}
void tsfi_reaction_fuse_batch8_fused(struct YI** results, TSFiBigInt** Xis, TSFiBigInt** Rhos, TSFiBigInt** Ohms, struct SHAO** orig_rhos, TSFiBigInt* p_bn) {
    struct SHA *rods[8], *cones[8];
    TSFiModContextBatch8 ctx_prime; TSFiBigInt* mods[8];
    for(int i=0; i<8; i++) { mods[i] = p_bn; }
    tsfi_bn_mod_setup_batch8(&ctx_prime, mods);
    for (int i = 0; i < 8; i++) {
        if (!results[i]) results[i] = allocYI();
        if (!results[i]->Psi) results[i]->Psi = allocSHIO();
        if (!results[i]->Psi->Rho) results[i]->Psi->Rho = allocSHAO();
        struct SHAO* A = results[i]->Psi->Rho;
        
        if (!A->Rod) A->Rod = allocSHA();
        if (!A->Rod->Mu) { A->Rod->Mu = allocFa(); memset(A->Rod->Mu, 0, sizeof(struct Fa)); }
        A->Rod->Mu->Base = tsfi_bn_realloc(A->Rod->Mu->Base); tsfi_bn_copy(A->Rod->Mu->Base, orig_rhos[i]->Rod->Mu->Base);
        A->Rod->Mu->Secret = tsfi_bn_realloc(A->Rod->Mu->Secret); tsfi_bn_copy(A->Rod->Mu->Secret, orig_rhos[i]->Rod->Mu->Secret);
        A->Rod->Mu->Signal = tsfi_bn_realloc(A->Rod->Mu->Signal); tsfi_bn_copy(A->Rod->Mu->Signal, orig_rhos[i]->Rod->Mu->Signal);
        
        if (!A->Cone) A->Cone = allocSHA();
        if (!A->Cone->Mu) { A->Cone->Mu = allocFa(); memset(A->Cone->Mu, 0, sizeof(struct Fa)); }
        A->Cone->Mu->Base = tsfi_bn_realloc(A->Cone->Mu->Base); tsfi_bn_copy(A->Cone->Mu->Base, orig_rhos[i]->Cone->Mu->Base);
        A->Cone->Mu->Secret = tsfi_bn_realloc(A->Cone->Mu->Secret); tsfi_bn_copy(A->Cone->Mu->Secret, orig_rhos[i]->Cone->Mu->Secret);
        A->Cone->Mu->Signal = tsfi_bn_realloc(A->Cone->Mu->Signal); tsfi_bn_copy(A->Cone->Mu->Signal, orig_rhos[i]->Cone->Mu->Signal);
        
        rods[i] = A->Rod; cones[i] = A->Cone;
        results[i]->Xi = tsfi_bn_realloc(results[i]->Xi); 
        tsfi_bn_copy(results[i]->Xi, Xis[i]);
    }
    
    TSFiBigInt *rb[8], *re[8], *rr[8];
    // Tune Rods
    for(int i=0; i<8; i++) { rods[i]->Mu->Channel = tsfi_bn_realloc(rods[i]->Mu->Channel); rb[i] = rods[i]->Mu->Base; re[i] = rods[i]->Mu->Signal; rr[i] = rods[i]->Mu->Channel; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    // Tune Cones
    for(int i=0; i<8; i++) { cones[i]->Mu->Channel = tsfi_bn_realloc(cones[i]->Mu->Channel); rb[i] = cones[i]->Mu->Base; re[i] = cones[i]->Mu->Signal; rr[i] = cones[i]->Mu->Channel; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);

    // Contours
    for(int i=0; i<8; i++) { rods[i]->Mu->Contour = tsfi_bn_realloc(rods[i]->Mu->Contour); rb[i] = Xis[i]; re[i] = rods[i]->Mu->Secret; rr[i] = rods[i]->Mu->Contour; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Contour = tsfi_bn_realloc(cones[i]->Mu->Contour); rb[i] = Xis[i]; re[i] = cones[i]->Mu->Secret; rr[i] = cones[i]->Mu->Contour; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    // Bases
    for(int i=0; i<8; i++) { rods[i]->Mu->Base = tsfi_bn_realloc(rods[i]->Mu->Base); rb[i] = cones[i]->Mu->Contour; re[i] = rods[i]->Mu->Secret; rr[i] = rods[i]->Mu->Base; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Base = tsfi_bn_realloc(cones[i]->Mu->Base); rb[i] = rods[i]->Mu->Contour; re[i] = cones[i]->Mu->Secret; rr[i] = cones[i]->Mu->Base; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    // Channels
    for(int i=0; i<8; i++) { rods[i]->Mu->Channel = tsfi_bn_realloc(rods[i]->Mu->Channel); rb[i] = rods[i]->Mu->Base; re[i] = rods[i]->Mu->Signal; rr[i] = rods[i]->Mu->Channel; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Channel = tsfi_bn_realloc(cones[i]->Mu->Channel); rb[i] = cones[i]->Mu->Base; re[i] = cones[i]->Mu->Signal; rr[i] = cones[i]->Mu->Channel; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    // Poles
    for(int i=0; i<8; i++) { rods[i]->Mu->Pole = tsfi_bn_realloc(rods[i]->Mu->Pole); rb[i] = rods[i]->Mu->Base; re[i] = rods[i]->Mu->Secret; rr[i] = rods[i]->Mu->Pole; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Pole = tsfi_bn_realloc(cones[i]->Mu->Pole); rb[i] = cones[i]->Mu->Base; re[i] = cones[i]->Mu->Secret; rr[i] = cones[i]->Mu->Pole; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    // Coordinates
    for(int i=0; i<8; i++) { rods[i]->Mu->Coordinate = tsfi_bn_realloc(rods[i]->Mu->Coordinate); rb[i] = cones[i]->Mu->Pole; re[i] = rods[i]->Mu->Secret; rr[i] = rods[i]->Mu->Coordinate; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for(int i=0; i<8; i++) { cones[i]->Mu->Coordinate = tsfi_bn_realloc(cones[i]->Mu->Coordinate); rb[i] = rods[i]->Mu->Pole; re[i] = cones[i]->Mu->Secret; rr[i] = cones[i]->Mu->Coordinate; }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    // Foundation and Identity setup
    for(int i=0; i<8; i++) { 
        cones[i]->Mu->Identity = tsfi_bn_realloc(cones[i]->Mu->Identity); tsfi_bn_copy(cones[i]->Mu->Identity, Rhos[i]);
        cones[i]->Mu->Foundation = tsfi_bn_realloc(cones[i]->Mu->Foundation); 
        rb[i] = cones[i]->Mu->Base; re[i] = cones[i]->Mu->Identity; rr[i] = cones[i]->Mu->Foundation; 
    }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    for (int i = 0; i < 8; i++) { 
        rods[i]->Mu->Identity = tsfi_bn_realloc(rods[i]->Mu->Identity); tsfi_bn_copy(rods[i]->Mu->Identity, Ohms[i]);
        rods[i]->Mu->Foundation = tsfi_bn_realloc(rods[i]->Mu->Foundation); 
        rb[i] = rods[i]->Mu->Base; re[i] = rods[i]->Mu->Identity; rr[i] = rods[i]->Mu->Foundation; 
    }
    tsfi_bn_modpow_batch8_ctx(rr, rb, re, mods, &ctx_prime);
    TSFiBigInt *cone_foundations[8], *cone_channels[8], *rod_foundations[8], *rod_channels[8];
    for(int i=0; i<8; i++) { cone_foundations[i] = cones[i]->Mu->Foundation; cone_channels[i] = cones[i]->Mu->Channel; rod_foundations[i] = rods[i]->Mu->Foundation; rod_channels[i] = rods[i]->Mu->Channel; }
    Saturate_batch8(rods, Ohms, cone_foundations, cone_channels, p_bn); 
    Saturate_batch8(cones, Rhos, rod_foundations, rod_channels, p_bn);
    for (int i = 0; i < 8; i++) { Bond(rods[i]); Bond(cones[i]); }
    MAGNETIZE_batch8(results, p_bn);
}
