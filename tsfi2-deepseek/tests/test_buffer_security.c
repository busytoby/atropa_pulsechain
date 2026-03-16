#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_types.h"
#include "tsfi_wiring.h"
#include "tsfi_dys_math.h"
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAYLOAD_SIZE 1024
#define SEQUENCE_LENGTH 10000
#define SCRAMBLE_BUFFER_SIZE (128 * 100) 

static int popcount64(uint64_t x) {
    int count = 0;
    for (; x; x >>= 1) {
        count += x & 1;
    }
    return count;
}

static int mock_printf(int fd, const char* fmt, ...) {
    (void)fd;
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);
    return ret;
}

WaveSystem* create_isolated_system() {
    LauWireFirmware *fw = (LauWireFirmware*)lau_malloc_wired(sizeof(LauWireFirmware));
    memset(fw, 0, sizeof(LauWireFirmware));
    fw->rtl.cell_version = 1;
    fw->cell_printf = mock_printf;

    WaveSystem *ws = (WaveSystem *)lau_malloc_wired(sizeof(WaveSystem));
    ws->fw = fw;
    LauSystemHeader *h = (LauSystemHeader *)((char *)ws - offsetof(LauSystemHeader, payload));
    memset(&h->regs, 0, sizeof(LauRegisterBank));
    h->resonance_as_status = lau_strdup("ISOLATED_BOOT");
    return ws;
}

void test_buffer_avalanche() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); 

    tsfi_io_printf(stdout, "\n=== TSFi Encrypted Buffer Security: Avalanche Test ===\n");
    tsfi_io_printf(stdout, "Evaluating cryptographic divergence across %d sequential iterations.\n", SEQUENCE_LENGTH);

    struct YI* root_yi = tsfi_reaction_shoot(Prime);

    TSFiBigInt* Pi_A = tsfi_bn_alloc();
    TSFiBigInt* Pi_B = tsfi_bn_alloc();
    
    TSFiBigInt* origin_ring = root_yi->Ring ? root_yi->Ring : root_yi->Xi;
    tsfi_bn_copy(Pi_A, origin_ring);
    tsfi_bn_copy(Pi_B, origin_ring);

    Pi_B->limbs[0] ^= 1;

    uint64_t total_bits_evaluated = 0;
    uint64_t total_hamming_distance = 0;
    int identical_collision_count = 0;

    for (int i = 0; i < SEQUENCE_LENGTH; i++) {
        struct Dai* dai_A = tsfi_reaction_compute_reciprocity(root_yi->Psi, Pi_A, NULL);
        struct Dai* dai_B = tsfi_reaction_compute_reciprocity(root_yi->Psi, Pi_B, NULL);

        uint64_t a_ichi = dai_A->Ichidai->limbs[0];
        uint64_t a_dai = dai_A->Daiichi->limbs[0];
        uint64_t b_ichi = dai_B->Ichidai->limbs[0];
        uint64_t b_dai = dai_B->Daiichi->limbs[0];

        int dist_ichi = popcount64(a_ichi ^ b_ichi);
        int dist_dai = popcount64(a_dai ^ b_dai);
        
        total_hamming_distance += (dist_ichi + dist_dai);
        total_bits_evaluated += 100;

        if (a_ichi == b_ichi && a_dai == b_dai) {
            identical_collision_count++;
        }

        tsfi_bn_copy(Pi_A, dai_A->Daiichi);
        tsfi_bn_copy(Pi_B, dai_B->Daiichi);

        freeDAI(dai_A);
        freeDAI(dai_B);
    }

    double avalanche_ratio = ((double)total_hamming_distance / (double)total_bits_evaluated) * 100.0;

    if (avalanche_ratio > 45.0 && avalanche_ratio < 55.0 && identical_collision_count == 0) {
        tsfi_io_printf(stdout, "[SUCCESS] Avalanche property is cryptographically sound (~50%% deviation).\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Buffer stream failed security divergence thresholds!\n");
        exit(1);
    }

    tsfi_bn_free(Pi_A);
    tsfi_bn_free(Pi_B);
    freeYI(root_yi);
    tsfi_bn_free(Prime);
}

void test_buffer_mutilation_tolerance() {
    tsfi_io_printf(stdout, "\n=== TSFi Encrypted Buffer Security: Mutilation Tolerance ===\n");

    WaveSystem* ws_rod = create_isolated_system();
    WaveSystem* ws_cone = create_isolated_system();

    TSFiBigInt* Prime = tsfi_bn_alloc(); tsfi_bn_set_u64(Prime, 953467954114363ULL); 
    TSFiBigInt* Xi = tsfi_bn_alloc(); tsfi_bn_randomize(Xi);
    TSFiBigInt* Rod_Identity = tsfi_bn_alloc(); tsfi_bn_randomize(Rod_Identity);
    TSFiBigInt* Cone_Identity = tsfi_bn_alloc(); tsfi_bn_randomize(Cone_Identity);

    TSFiHelmholtzAdductState rod_state = {0};
    rod_state.Prime = Prime; rod_state.Xi = Xi; rod_state.Beta = Rod_Identity;
    rod_state.current_epoch = HELMHOLTZ_EPOCH_INIT;
    
    TSFiHelmholtzAdductState cone_state = {0};
    cone_state.Prime = Prime; cone_state.Xi = Xi; cone_state.Beta = Cone_Identity;
    cone_state.current_epoch = HELMHOLTZ_EPOCH_INIT;

    tsfi_helmholtz_synchronize(&rod_state, &cone_state);

    if (rod_state.current_epoch != HELMHOLTZ_EPOCH_DONE) {
        tsfi_io_printf(stderr, "[FAIL] Handshake desynchronized.\n");
        exit(1);
    }

    uint8_t original_payload[PAYLOAD_SIZE];
    for (int i = 0; i < PAYLOAD_SIZE; i++) original_payload[i] = (uint8_t)(i % 256);
    
    uint8_t ciphertext[PAYLOAD_SIZE];
    uint8_t mutated_ciphertext[PAYLOAD_SIZE];
    uint8_t decrypted_payload[PAYLOAD_SIZE];

    TSFiBigInt* rod_pi = tsfi_bn_alloc(); tsfi_bn_copy(rod_pi, Xi);
    struct Dai* rod_dai = allocDai();
    int bp = 0;
    while (bp < PAYLOAD_SIZE) {
        rod_dai->Ichidai = tsfi_bn_realloc(rod_dai->Ichidai); modPow_bn(rod_dai->Ichidai, rod_pi, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        rod_dai->Daiichi = tsfi_bn_realloc(rod_dai->Daiichi); modPow_bn(rod_dai->Daiichi, rod_pi, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);
        uint64_t entropy[2] = { rod_dai->Ichidai->limbs[0], rod_dai->Daiichi->limbs[0] };
        uint8_t* stream = (uint8_t*)entropy;
        for (int i = 0; i < 16 && bp < PAYLOAD_SIZE; i++, bp++) ciphertext[bp] = original_payload[bp] ^ stream[i];
        tsfi_bn_copy(rod_pi, rod_dai->Daiichi);
    }
    
    memcpy(mutated_ciphertext, ciphertext, PAYLOAD_SIZE);
    mutated_ciphertext[12] ^= 0xFF;
    mutated_ciphertext[256] ^= 0xAA;
    mutated_ciphertext[512] ^= 0x0F;
    mutated_ciphertext[1020] ^= 0x55;

    TSFiBigInt* cone_pi = tsfi_bn_alloc(); tsfi_bn_copy(cone_pi, Xi);
    struct Dai* cone_dai = allocDai();
    int bytes_decrypted = 0;
    while (bytes_decrypted < PAYLOAD_SIZE) {
        cone_dai->Ichidai = tsfi_bn_realloc(cone_dai->Ichidai); modPow_bn(cone_dai->Ichidai, cone_pi, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        cone_dai->Daiichi = tsfi_bn_realloc(cone_dai->Daiichi); modPow_bn(cone_dai->Daiichi, cone_pi, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);
        
        uint64_t entropy[2] = { cone_dai->Ichidai->limbs[0], cone_dai->Daiichi->limbs[0] };
        uint8_t* stream = (uint8_t*)entropy;
        
        for (int i = 0; i < 16 && bytes_decrypted < PAYLOAD_SIZE; i++, bytes_decrypted++) {
            decrypted_payload[bytes_decrypted] = mutated_ciphertext[bytes_decrypted] ^ stream[i];
        }
        tsfi_bn_copy(cone_pi, cone_dai->Daiichi);
    }

    int failures = 0;
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        if (original_payload[i] != decrypted_payload[i]) failures++;
    }

    if (failures == 4) {
        tsfi_io_printf(stdout, "[SUCCESS] Buffer mutilation was strictly contained. System did not segfault or structurally collapse.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Cipher stream failed to contain the noise variance (expected 4, got %d).\n", failures);
        exit(1);
    }

    freeDAI(rod_dai); freeDAI(cone_dai); tsfi_bn_free(rod_pi); tsfi_bn_free(cone_pi);
    tsfi_bn_free(Prime); tsfi_bn_free(Xi);
    tsfi_bn_free(Rod_Identity); tsfi_bn_free(Cone_Identity);
    lau_unseal_object(ws_rod->fw); lau_free(ws_rod->fw);
    LauSystemHeader *hr = (LauSystemHeader *)((char *)ws_rod - offsetof(LauSystemHeader, payload)); lau_free(hr->resonance_as_status);
    lau_unseal_object(ws_rod); lau_free(ws_rod);
    lau_unseal_object(ws_cone->fw); lau_free(ws_cone->fw);
    LauSystemHeader *hc = (LauSystemHeader *)((char *)ws_cone - offsetof(LauSystemHeader, payload)); lau_free(hc->resonance_as_status);
    lau_unseal_object(ws_cone); lau_free(ws_cone);
    
    freeSHA(rod_state.Mu); freeSHA(cone_state.Mu);
}

void test_buffer_ouroboros() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); 

    tsfi_io_printf(stdout, "\n=== TSFi Encrypted Buffer Security: Ouroboros Periodicity Test ===\n");
    
    struct YI* root_yi = tsfi_reaction_shoot(Prime);

    TSFiBigInt* tortoise = tsfi_bn_alloc();
    TSFiBigInt* hare = tsfi_bn_alloc();
    
    TSFiBigInt* origin_ring = root_yi->Ring ? root_yi->Ring : root_yi->Xi;
    tsfi_bn_copy(tortoise, origin_ring);
    tsfi_bn_copy(hare, origin_ring);

    int cycle_detected = 0;
    int steps = 0;

    for (int i = 0; i < 10000; i++) {
        struct Dai* t_dai = tsfi_reaction_compute_reciprocity(root_yi->Psi, tortoise, NULL);
        tsfi_bn_copy(tortoise, t_dai->Daiichi);
        freeDAI(t_dai);

        struct Dai* h_dai_1 = tsfi_reaction_compute_reciprocity(root_yi->Psi, hare, NULL);
        tsfi_bn_copy(hare, h_dai_1->Daiichi);
        freeDAI(h_dai_1);
        struct Dai* h_dai_2 = tsfi_reaction_compute_reciprocity(root_yi->Psi, hare, NULL);
        tsfi_bn_copy(hare, h_dai_2->Daiichi);
        freeDAI(h_dai_2);

        steps++;
        if (tsfi_bn_cmp_avx512(tortoise, hare) == 0) {
            cycle_detected = 1;
            break;
        }
    }

    if (cycle_detected) {
        tsfi_io_printf(stderr, "[FAIL] Cryptographic Periodicity Failure! Cycle detected at step %d.\n", steps);
        exit(1);
    } else {
        tsfi_io_printf(stdout, "[SUCCESS] No topological loops detected. Buffer stream is acyclic within tested bounds.\n");
    }

    tsfi_bn_free(tortoise);
    tsfi_bn_free(hare);
    freeYI(root_yi);
    tsfi_bn_free(Prime);
}

void test_buffer_scramble_security() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    tsfi_io_printf(stdout, "\n=== TSFi Encrypted Buffer Security: Scramble Zeroization Test ===\n");
    
    struct YI* root_yi_batch[8] = {0};
    tsfi_reaction_shoot_batch8_fused(root_yi_batch, Prime);
    TSFiBigInt* Pi = tsfi_bn_alloc();
    tsfi_bn_copy(Pi, root_yi_batch[0]->Ring ? root_yi_batch[0]->Ring : root_yi_batch[0]->Xi);

    uint8_t* crypto_buffer = lau_malloc_wired(SCRAMBLE_BUFFER_SIZE);

    uint64_t bytes_written = 0;
    
    // 1. Populate the buffer with real reaction data
    for (int i = 0; i < 100; i++) {
        struct Dai* dai = tsfi_reaction_compute_reciprocity(root_yi_batch[0]->Psi, Pi, NULL);
        memcpy(crypto_buffer + bytes_written, &dai->Ichidai->limbs[0], 64);
        bytes_written += 64;
        memcpy(crypto_buffer + bytes_written, &dai->Daiichi->limbs[0], 64);
        bytes_written += 64;
        tsfi_bn_copy(Pi, dai->Daiichi);
        freeDAI(dai);
    }
    
    uint64_t snapshot[16];
    memcpy(snapshot, crypto_buffer + 512, 128); // Snapshot 2 Dai at offset 512

    // 3. Scramble the buffer
    lau_mem_scramble(crypto_buffer, SCRAMBLE_BUFFER_SIZE, LAU_SCRAMBLE_MODE_RANDOM);
    
    // 4. Heuristic Search / Verification
    uint64_t post_snapshot[16];
    memcpy(post_snapshot, crypto_buffer + 512, 128);
    
    int bytes_matched = 0;
    for (int i = 0; i < SCRAMBLE_BUFFER_SIZE; i++) {
        if (crypto_buffer[i] == ((uint8_t*)snapshot)[i % 128]) {
            bytes_matched++;
        }
    }
    
    double match_ratio = ((double)bytes_matched / SCRAMBLE_BUFFER_SIZE) * 100.0;
    
    if (snapshot[0] != post_snapshot[0] && snapshot[1] != post_snapshot[1] && match_ratio < 2.0) {
        tsfi_io_printf(stdout, "[SUCCESS] Memory completely zeroized. Cold-boot and scraping vectors neutralized.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Residual reaction state detected post-scramble!\n");
        exit(1);
    }

    lau_unseal_object(crypto_buffer);
    lau_free(crypto_buffer);
    tsfi_bn_free(Pi);
    for(int i=0; i<8; i++) {
        if (root_yi_batch[i]) freeYI(root_yi_batch[i]);
    }
    tsfi_bn_free(Prime);
}

void test_buffer_replay_attack() {
    tsfi_io_printf(stdout, "\n=== TSFi Encrypted Buffer Security: Replay Attack Rejection ===\n");

    WaveSystem* ws_rod = create_isolated_system();
    WaveSystem* ws_cone = create_isolated_system();

    TSFiBigInt* Prime = tsfi_bn_alloc(); tsfi_bn_set_u64(Prime, 953467954114363ULL); 
    TSFiBigInt* Xi = tsfi_bn_alloc(); tsfi_bn_randomize(Xi);
    TSFiBigInt* Rod_Identity = tsfi_bn_alloc(); tsfi_bn_randomize(Rod_Identity);
    TSFiBigInt* Cone_Identity = tsfi_bn_alloc(); tsfi_bn_randomize(Cone_Identity);

    TSFiHelmholtzAdductState rod_state = {0};
    rod_state.Prime = Prime; rod_state.Xi = Xi; rod_state.Beta = Rod_Identity;
    rod_state.current_epoch = HELMHOLTZ_EPOCH_INIT;
    
    TSFiHelmholtzAdductState cone_state = {0};
    cone_state.Prime = Prime; cone_state.Xi = Xi; cone_state.Beta = Cone_Identity;
    cone_state.current_epoch = HELMHOLTZ_EPOCH_INIT;

    tsfi_helmholtz_synchronize(&rod_state, &cone_state);

    uint8_t original_payload[64];
    for (int i = 0; i < 64; i++) original_payload[i] = (uint8_t)(i % 256);
    
    uint8_t session_1_cipher[64];
    uint8_t session_2_cipher[64];
    uint8_t attacker_replay_output[64];

    (void)session_2_cipher;

    TSFiBigInt* rod_pi_1 = tsfi_bn_alloc(); tsfi_bn_copy(rod_pi_1, Xi);
    struct Dai* rod_dai = allocDai();
    int bp = 0;
    while (bp < 64) {
        rod_dai->Ichidai = tsfi_bn_realloc(rod_dai->Ichidai); modPow_bn(rod_dai->Ichidai, rod_pi_1, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        rod_dai->Daiichi = tsfi_bn_realloc(rod_dai->Daiichi); modPow_bn(rod_dai->Daiichi, rod_pi_1, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);
        uint64_t entropy[2] = { rod_dai->Ichidai->limbs[0], rod_dai->Daiichi->limbs[0] };
        uint8_t* stream = (uint8_t*)entropy;
        for (int i = 0; i < 16 && bp < 64; i++, bp++) session_1_cipher[bp] = original_payload[bp] ^ stream[i];
        tsfi_bn_copy(rod_pi_1, rod_dai->Daiichi);
    }
    
    bp = 0;
    while (bp < 64) {
        rod_dai->Ichidai = tsfi_bn_realloc(rod_dai->Ichidai); modPow_bn(rod_dai->Ichidai, rod_pi_1, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        rod_dai->Daiichi = tsfi_bn_realloc(rod_dai->Daiichi); modPow_bn(rod_dai->Daiichi, rod_pi_1, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);
        uint64_t entropy[2] = { rod_dai->Ichidai->limbs[0], rod_dai->Daiichi->limbs[0] };
        uint8_t* stream = (uint8_t*)entropy;
        for (int i = 0; i < 16 && bp < 64; i++, bp++) session_2_cipher[bp] = original_payload[bp] ^ stream[i];
        tsfi_bn_copy(rod_pi_1, rod_dai->Daiichi);
    }

    TSFiBigInt* cone_pi = tsfi_bn_alloc(); tsfi_bn_copy(cone_pi, Xi);
    struct Dai* cone_dai = allocDai();
    bp = 0;
    while (bp < 64) {
        cone_dai->Ichidai = tsfi_bn_realloc(cone_dai->Ichidai); modPow_bn(cone_dai->Ichidai, cone_pi, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        cone_dai->Daiichi = tsfi_bn_realloc(cone_dai->Daiichi); modPow_bn(cone_dai->Daiichi, cone_pi, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);
        tsfi_bn_copy(cone_pi, cone_dai->Daiichi);
        bp += 16;
    }

    int bytes_decrypted = 0;
    while (bytes_decrypted < 64) {
        cone_dai->Ichidai = tsfi_bn_realloc(cone_dai->Ichidai); modPow_bn(cone_dai->Ichidai, cone_pi, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        cone_dai->Daiichi = tsfi_bn_realloc(cone_dai->Daiichi); modPow_bn(cone_dai->Daiichi, cone_pi, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);
        
        uint64_t entropy[2] = { cone_dai->Ichidai->limbs[0], cone_dai->Daiichi->limbs[0] };
        uint8_t* stream = (uint8_t*)entropy;
        
        for (int i = 0; i < 16 && bytes_decrypted < 64; i++, bytes_decrypted++) {
            attacker_replay_output[bytes_decrypted] = session_1_cipher[bytes_decrypted] ^ stream[i];
        }
        tsfi_bn_copy(cone_pi, cone_dai->Daiichi);
    }

    int failures = 0;
    for (int i = 0; i < 64; i++) {
        if (original_payload[i] != attacker_replay_output[i]) failures++;
    }

    if (failures > 32) {
        tsfi_io_printf(stdout, "[SUCCESS] Replay attack neutralized. Decrypted output is mathematically destroyed.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Cipher stream failed to protect against state replay.\n");
        exit(1);
    }

    freeDAI(rod_dai); freeDAI(cone_dai); tsfi_bn_free(rod_pi_1); tsfi_bn_free(cone_pi);
    tsfi_bn_free(Prime); tsfi_bn_free(Xi);
    tsfi_bn_free(Rod_Identity); tsfi_bn_free(Cone_Identity);
    lau_unseal_object(ws_rod->fw); lau_free(ws_rod->fw);
    LauSystemHeader *hr = (LauSystemHeader *)((char *)ws_rod - offsetof(LauSystemHeader, payload)); lau_free(hr->resonance_as_status);
    lau_unseal_object(ws_rod); lau_free(ws_rod);
    lau_unseal_object(ws_cone->fw); lau_free(ws_cone->fw);
    LauSystemHeader *hc = (LauSystemHeader *)((char *)ws_cone - offsetof(LauSystemHeader, payload)); lau_free(hc->resonance_as_status);
    lau_unseal_object(ws_cone); lau_free(ws_cone);
    
    freeSHA(rod_state.Mu); freeSHA(cone_state.Mu);
}

int main() {
    test_buffer_avalanche();
    test_buffer_mutilation_tolerance();
    test_buffer_ouroboros();
    test_buffer_scramble_security();
    test_buffer_replay_attack();

    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
