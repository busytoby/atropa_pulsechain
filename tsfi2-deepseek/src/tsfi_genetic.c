#include "tsfi_genetic.h"
#include "tsfi_vision.h"
#include "lau_memory.h"
#include "tsfi_io.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void tsfi_k0rn_evolve_vision(K0RnStream *s, const TSFiResonanceAnalysis *vis, float aggressiveness) {
    if (!s || !vis) return;
    float intensity = aggressiveness;
    if (vis->visual_delta > 0.1f) intensity *= 0.5f;
    if (vis->smoothness < 0.5f) intensity *= 0.2f;
    if (vis->content_fps < 10.0f) intensity *= 2.0f;

    for (uint32_t i = 0; i < s->op_count; i++) {
        K0RnOp *op = &s->ops[i];
        float r = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        op->x += r * 0.01f * intensity;
        r = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        op->y += r * 0.01f * intensity;
        r = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        op->p1 += r * 0.005f * intensity;
        if (op->p1 < 0.01f) op->p1 = 0.01f;
        if (op->p1 > 0.5f) op->p1 = 0.5f;
    }
}

int tsfi_genetic_load_approved_seeds(const char *csv_path, int *seeds, int max_seeds) {
    FILE *f = fopen(csv_path, "r");
    if (!f) return 0;
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_seeds) {
        if (strstr(line, ",like")) {
            char *comma = strchr(line, ',');
            if (comma) seeds[count++] = atoi(comma + 1);
        }
    }
    fclose(f);
    return count;
}


TSFiBigInt* MixDNA_bn(TSFiBigInt* A, TSFiBigInt* B) {
    if (!A && !B) return tsfi_bn_alloc();
    if (!A) { TSFiBigInt* res = tsfi_bn_alloc(); tsfi_bn_copy(res, B); return res; }
    if (!B) { TSFiBigInt* res = tsfi_bn_alloc(); tsfi_bn_copy(res, A); return res; }
    TSFiBigInt* res = tsfi_bn_alloc();
    int n = (A->active_limbs > B->active_limbs) ? A->active_limbs : B->active_limbs;
    for (int i = 0; i < n; i++) {
        uint64_t valA = (i < (int)A->active_limbs) ? A->limbs[i] : B->limbs[i];
        uint64_t valB = (i < (int)B->active_limbs) ? B->limbs[i] : A->limbs[i];
        res->limbs[i] = ((valA + valB) / 2) & TSFI_LIMB_MASK;
    }
    res->active_limbs = n;
    while(res->active_limbs > 0 && res->limbs[res->active_limbs-1] == 0) res->active_limbs--;
    return res;
}

void tsfi_k0rn_mutate(K0RnStream *s, float rate) { (void)s; (void)rate; }
void tsfi_k0rn_crossover(const K0RnStream *a, const K0RnStream *b, K0RnStream *child) { (void)a; (void)b; (void)child; }

void tsfi_k0rn_symmetrize(K0RnStream *s) {
    if (!s || s->op_count == 0) return;
    bool *matched = (bool*)lau_calloc(s->op_count, sizeof(bool));

    for (uint32_t i = 0; i < s->op_count; i++) {
        if (matched[i]) continue;
        K0RnOp *a = &s->ops[i];

        // Check if self-mirrored (centered on X=0)
        if (fabsf(a->x + a->z) < 0.05f) {
            float mid_x = (a->x + a->z) / 2.0f;
            a->x -= mid_x; a->z -= mid_x; // Center it
            matched[i] = true;
            continue;
        }

        // Search for a mirrored partner
        for (uint32_t j = i + 1; j < s->op_count; j++) {
            if (matched[j]) continue;
            K0RnOp *b = &s->ops[j];
            if (a->type != b->type) continue;

            // Distance between a and mirror(b)
            float d = fabsf(a->x + b->x) + fabsf(a->y - b->y) + 
                      fabsf(a->z + b->z) + fabsf(a->w - b->w);
            
            if (d < 0.3f) { // Found a mirror pair
                float avg_x = (fabsf(a->x) + fabsf(b->x)) / 2.0f;
                float avg_y = (a->y + b->y) / 2.0f;
                float avg_z = (fabsf(a->z) + fabsf(b->z)) / 2.0f;
                float avg_w = (a->w + b->w) / 2.0f;
                float avg_p1 = (a->p1 + b->p1) / 2.0f;

                a->x = (a->x < 0) ? -avg_x : avg_x;
                b->x = (b->x < 0) ? -avg_x : avg_x;
                a->y = b->y = avg_y;
                a->z = (a->z < 0) ? -avg_z : avg_z;
                b->z = (b->z < 0) ? -avg_z : avg_z;
                a->w = b->w = avg_w;
                a->p1 = b->p1 = avg_p1;

                matched[i] = matched[j] = true;
                break;
            }
        }
    }
    lau_free(matched);
}

void tsfi_genetic_benchmark_run(int iterations, char* buffer, size_t max_len) {
    if (buffer && max_len > 0) snprintf(buffer, max_len, "Genetic benchmark: %d iterations", iterations);
    tsfi_io_printf(stderr, "[GENETIC] Benchmark Running: %d iterations...\n", iterations);
}

void Fourier_UniversalCrossover(GeneticNode* A, GeneticNode* B, GeneticNode* Child) {

    if (!A || !B || !Child) return;

    

    // Dysnomia Hybrid Rule: YI is dominant over YANG

    if (A->type == GENETIC_TYPE_YI || B->type == GENETIC_TYPE_YI) {

        Child->type = GENETIC_TYPE_YI;

    } else {

        Child->type = (rand() % 2 == 0) ? A->type : B->type;

    }

    

    Child->react_shio = (rand() % 2 == 0) ? A->react_shio : B->react_shio;



    Child->react_sha = (rand() % 2 == 0) ? A->react_sha : B->react_sha;

    snprintf(Child->generation_id, 63, "XO_%08X", (unsigned int)rand());



        if (Child->type == GENETIC_TYPE_YI) {



            Child->dys_ptr = allocYI();



            memset(Child->dys_ptr, 0, sizeof(struct YI));



            struct YI* yiA = (struct YI*)A->dys_ptr;



            struct YI* yiB = (struct YI*)B->dys_ptr;



            struct YI* yiC = (struct YI*)Child->dys_ptr;



    

        if (yiA && yiB && yiC) {

            yiC->Xi = MixDNA_bn(yiA->Xi, yiB->Xi);

            yiC->Ring = MixDNA_bn(yiA->Ring, yiB->Ring);

            // Deep crossover for Psi (SHIO) if both have it

            if (yiA->Psi && yiB->Psi) {

                yiC->Psi = (struct SHIO*)allocSHIO();

                memset(yiC->Psi, 0, sizeof(struct SHIO));

                if (yiA->Psi->Rho && yiB->Psi->Rho) {

                    yiC->Psi->Rho = (struct SHAO*)allocSHAO();

                    memset(yiC->Psi->Rho, 0, sizeof(struct SHAO));

                    if (yiA->Psi->Rho->Rod && yiB->Psi->Rho->Rod) {

                        yiC->Psi->Rho->Rod = (struct SHA*)allocSHA();

                        memset(yiC->Psi->Rho->Rod, 0, sizeof(struct SHA));

                        if (yiA->Psi->Rho->Rod->Mu && yiB->Psi->Rho->Rod->Mu) {

                            yiC->Psi->Rho->Rod->Mu = (struct Fa*)allocFa();

                            memset(yiC->Psi->Rho->Rod->Mu, 0, sizeof(struct Fa));

                            yiC->Psi->Rho->Rod->Mu->Base = MixDNA_bn(yiA->Psi->Rho->Rod->Mu->Base, yiB->Psi->Rho->Rod->Mu->Base);

                        }

                    }

                }

            }

        }

        } else if (Child->type == GENETIC_TYPE_YANG) {

            Child->dys_ptr = allocYANG();

            memset(Child->dys_ptr, 0, sizeof(struct YANG));

            struct YANG* yangA = (struct YANG*)A->dys_ptr;

            struct YANG* yangB = (struct YANG*)B->dys_ptr;

            struct YANG* yangC = (struct YANG*)Child->dys_ptr;

    

        if (yangA && yangB && yangC) {

            yangC->Ring = MixDNA_bn(yangA->Ring, yangB->Ring);

        }

    }

}
