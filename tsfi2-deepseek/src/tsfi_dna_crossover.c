#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "tsfi_genetic.h"
#include "tsfi_yang_registry.h"
#include "tsfi_helmholtz.h"
#include "tsfi_merkle.h"
#include "lau_memory.h"
#include "tsfi_math.h"

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <parent_a_dna> <parent_b_dna> <output_dna>\n", argv[0]);
        return 1;
    }

    tsfi_helmholtz_init();

    // 1. Load DNA Parents
    GeneticNode *ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    GeneticNode *ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    GeneticNode *Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));

    ParentA->type = GENETIC_TYPE_YI; ParentA->dys_ptr = allocYI();
    ParentB->type = GENETIC_TYPE_YI; ParentB->dys_ptr = allocYI();

    FILE *fA = fopen(argv[1], "rb");
    FILE *fB = fopen(argv[2], "rb");
    if (!fA || !fB) {
        perror("fopen");
        return 1;
    }

    uint8_t bufA[4096], bufB[4096];
    size_t nA = fread(bufA, 1, sizeof(bufA), fA);
    size_t nB = fread(bufB, 1, sizeof(bufB), fB);
    fclose(fA); fclose(fB);

    ((struct YI*)ParentA->dys_ptr)->Xi = tsfi_bn_alloc();
    ((struct YI*)ParentB->dys_ptr)->Xi = tsfi_bn_alloc();
    tsfi_bn_from_bytes(((struct YI*)ParentA->dys_ptr)->Xi, bufA, nA);
    tsfi_bn_from_bytes(((struct YI*)ParentB->dys_ptr)->Xi, bufB, nB);

    // 2. Perform AVX-512 Crossover
    Fourier_UniversalCrossover(ParentA, ParentB, Child);

    // 3. Save Child DNA
    FILE *fOut = fopen(argv[3], "wb");
    if (fOut) {
        uint8_t child_buf[4096];
        tsfi_bn_to_bytes(((struct YI*)Child->dys_ptr)->Xi, child_buf, 4096);
        fwrite(child_buf, 1, 4096, fOut);
        fclose(fOut);
    }

    // 4. Update Acoustic Registry
    int shm_fd = shm_open("/tsfi_acoustic_space", O_RDWR, 0666);
    if (shm_fd != -1) {
        TsfiAcousticRegistry *reg = mmap(NULL, sizeof(TsfiAcousticRegistry), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (reg != MAP_FAILED) {
            // Calculate "Acoustic Pressure" based on DNA entropy
            uint8_t hash[64];
            tsfi_keccak512(bufA, nA, hash); // Just a placeholder for actual entropy check
            
            reg->pdai_manifold_pressure = 0.1f + (float)(hash[0] % 100) / 500.0f;
            reg->yang_heartbeat_hz = 40.0f + reg->pdai_manifold_pressure * 20.0f;
            reg->yang_static_amplitude = (reg->pdai_manifold_pressure > 0.5f) ? 0.3f : 0.05f;
            
            snprintf(reg->agent_monologue, 256, "[CROSSOVER] Evolved DNA Gen %s. Pressure: %.2f", 
                     Child->generation_id, reg->pdai_manifold_pressure);
            
            munmap(reg, sizeof(TsfiAcousticRegistry));
        }
        close(shm_fd);
    }

    // 5. Cleanup
    freeYI((struct YI*)ParentA->dys_ptr); lau_free(ParentA);
    freeYI((struct YI*)ParentB->dys_ptr); lau_free(ParentB);
    if (Child->type == GENETIC_TYPE_YI) freeYI((struct YI*)Child->dys_ptr);
    lau_free(Child);

    return 0;
}
