#define _POSIX_C_SOURCE 200809L
#include "tsfi_helmholtz_transitive_deduction_solver.h"
#include "tsfi_totient_zero_nullstellensatz_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

bool tsfi_helmholtz_deduction_init(TsfiHelmholtzDeductionList *list) {
    if (!list) return false;
    memset(list, 0, sizeof(TsfiHelmholtzDeductionList));
    return true;
}

bool tsfi_helmholtz_deduction_append_step(
    TsfiHelmholtzDeductionList *list,
    TsfiHelmholtzStepKind kind,
    const char *descriptor,
    int64_t in_val,
    uint32_t ast_proof_hash
) {
    if (!list || list->step_count >= TSFI_HELMHOLTZ_MAX_STEPS || !descriptor) return false;

    TsfiHelmholtzDeductionNode *node = &list->steps[list->step_count];
    node->step_index = list->step_count + 1;
    node->kind = kind;
    strncpy(node->descriptor, descriptor, sizeof(node->descriptor) - 1);
    node->in_register = in_val;
    node->ast_proof_hash = ast_proof_hash;
    node->step_verified = false;

    list->step_count++;
    return true;
}

bool tsfi_helmholtz_deduction_execute_in_order(TsfiHelmholtzDeductionList *list) {
    if (!list || list->step_count == 0) return false;

    double start_ns = get_time_ns();
    int64_t running_accumulator = 0;

    for (uint32_t i = 0; i < list->step_count; i++) {
        TsfiHelmholtzDeductionNode *node = &list->steps[i];

        switch (node->kind) {
            case TSFI_STEP_KIND_SOVEREIGN_SEED:
                // Sovereign participant seed: Must initialize 1,000,000 Saat default endowment
                if (node->in_register <= 0) {
                    list->total_deduction_latency_ns = get_time_ns() - start_ns;
                    list->pipeline_verified = false;
                    return false;
                }
                node->out_register = 1000000; // Standard Hogan Bank endowment
                running_accumulator = node->out_register;
                node->step_verified = true;
                break;

            case TSFI_STEP_KIND_INITIAL_ORDERS1_PRE:
                // EDSAC Initial Orders 1 Opcode Prefetch Firewall evaluation
                // Must not contain prohibited opcode masks ('X', illegal branches)
                if (node->in_register < 0 || (node->in_register & 0xFF) == 'X') {
                    list->total_deduction_latency_ns = get_time_ns() - start_ns;
                    list->pipeline_verified = false;
                    return false;
                }
                node->out_register = (node->in_register & 0x3F); // 6-bit FIELDATA boundary
                running_accumulator += node->out_register;
                node->step_verified = true;
                break;

            case TSFI_STEP_KIND_MOTZKIN_MODPOW: {
                // 3-term non-preferential recurrence over MotzkinPrime field
                int64_t base = (node->in_register > 0) ? (node->in_register % MOTZKIN_PRIME_SYS) : 3;
                int64_t exp = 7;
                int64_t res = 1;
                while (exp > 0) {
                    if (exp & 1) res = (__int128)res * base % MOTZKIN_PRIME_SYS;
                    base = (__int128)base * base % MOTZKIN_PRIME_SYS;
                    exp >>= 1;
                }
                node->out_register = res;
                running_accumulator = (running_accumulator + res) % MOTZKIN_PRIME_SYS;
                node->step_verified = true;
                break;
            }

            case TSFI_STEP_KIND_TOTIENT_NULLSTELLEN: {
                // Totient = 0 radical annihilation check
                TsfiTotientZeroProofParams params = {
                    .totient_val = node->in_register,
                    .radical_power_m = 4,
                    .accumulator_energy = (running_accumulator >= 0) ? running_accumulator : 0,
                    .cics_writer_id = 1000000,
                    .simulate_fault = 0,
                    .k_param = 3
                };
                if (tsfi_verify_totient_zero_nullstellensatz(&params) != TSFI_NULLSTELLENSATZ_QUALIFIED) {
                    list->total_deduction_latency_ns = get_time_ns() - start_ns;
                    list->pipeline_verified = false;
                    return false;
                }
                node->out_register = 0; // Radical variety vanishes identically
                node->step_verified = true;
                break;
            }

            case TSFI_STEP_KIND_EDO22_HELMHOLTZ_ROPE: {
                // Helmholtz acoustic wave & EDO-22 microtonal rotary phase mapping
                int64_t tone_step = node->in_register % 22;
                // Non-preferential rotary contraction under gamma = 875/1000
                int64_t contracted_phase = (tone_step * 875) / 1000;
                node->out_register = contracted_phase;
                running_accumulator += contracted_phase;
                node->step_verified = true;
                break;
            }

            case TSFI_STEP_KIND_ACCUMULATOR_CLAMP:
                // Non-preferential accumulator energy clamping: E_acc >= 0
                if (running_accumulator < 0) running_accumulator = 0;
                node->out_register = running_accumulator;
                node->step_verified = true;
                break;
        }
    }

    // Compute In-Order Deductive Merkle Digest across all pipeline nodes
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx) {
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) == 1) {
            for (uint32_t i = 0; i < list->step_count; i++) {
                EVP_DigestUpdate(ctx, &list->steps[i].step_index, sizeof(list->steps[i].step_index));
                EVP_DigestUpdate(ctx, &list->steps[i].kind, sizeof(list->steps[i].kind));
                EVP_DigestUpdate(ctx, list->steps[i].descriptor, strlen(list->steps[i].descriptor));
                EVP_DigestUpdate(ctx, &list->steps[i].in_register, sizeof(list->steps[i].in_register));
                EVP_DigestUpdate(ctx, &list->steps[i].out_register, sizeof(list->steps[i].out_register));
                EVP_DigestUpdate(ctx, &list->steps[i].ast_proof_hash, sizeof(list->steps[i].ast_proof_hash));
            }
            unsigned int len = 0;
            EVP_DigestFinal_ex(ctx, list->deduction_merkle_root, &len);
        }
        EVP_MD_CTX_free(ctx);
    }

    list->total_deduction_latency_ns = get_time_ns() - start_ns;
    list->pipeline_verified = true;
    return true;
}
