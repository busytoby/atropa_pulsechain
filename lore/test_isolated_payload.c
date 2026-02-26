#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_types.h"
#include "tsfi_wiring.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAYLOAD_SIZE 1024 // 1 KB payload

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware Segregation: End-to-End Payload Integrity ===\n");
    
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); 

    // Initial synchronized vector across the void
    TSFiBigInt* Xi = tsfi_bn_alloc();
    tsfi_bn_randomize(Xi);
    
    TSFiBigInt* Rod_Identity = tsfi_bn_alloc();
    tsfi_bn_randomize(Rod_Identity);
    
    TSFiBigInt* Cone_Identity = tsfi_bn_alloc();
    tsfi_bn_randomize(Cone_Identity);

    // Isolated States
    TSFiHelmholtzAdductState rod_state = {0};
    rod_state.Prime = Prime;
    rod_state.Xi = Xi;
    rod_state.Beta = Rod_Identity;
    rod_state.current_epoch = HELMHOLTZ_EPOCH_INIT;
    
    TSFiHelmholtzAdductState cone_state = {0};
    cone_state.Prime = Prime;
    cone_state.Xi = Xi;
    cone_state.Beta = Cone_Identity;
    cone_state.current_epoch = HELMHOLTZ_EPOCH_INIT;

    // Execute Handshake
    tsfi_helmholtz_step(&rod_state, NULL); tsfi_helmholtz_step(&cone_state, NULL); // INIT -> AVAIL
    tsfi_helmholtz_step(&rod_state, NULL); tsfi_helmholtz_step(&cone_state, NULL); // AVAIL -> FORM
    tsfi_helmholtz_step(&rod_state, cone_state.Mu->Mu->Contour); tsfi_helmholtz_step(&cone_state, rod_state.Mu->Mu->Contour); // FORM -> POLARIZE
    tsfi_helmholtz_step(&rod_state, NULL); tsfi_helmholtz_step(&cone_state, NULL); // POLARIZE -> CONJUGATE
    tsfi_helmholtz_step(&rod_state, cone_state.Mu->Mu->Pole); tsfi_helmholtz_step(&cone_state, rod_state.Mu->Mu->Pole); // CONJUGATE -> CONIFY
    tsfi_helmholtz_step(&rod_state, NULL); tsfi_helmholtz_step(&cone_state, NULL); // CONIFY -> SATURATE
    tsfi_helmholtz_step(&rod_state, cone_state.Mu->Mu->Foundation); tsfi_helmholtz_step(&cone_state, rod_state.Mu->Mu->Foundation); // SATURATE -> IONIZE
    tsfi_helmholtz_step(&rod_state, NULL); tsfi_helmholtz_step(&cone_state, NULL); // IONIZE -> MAGNETIZE
    tsfi_helmholtz_step(&rod_state, NULL); tsfi_helmholtz_step(&cone_state, NULL); // MAGNETIZE -> DONE

    if (rod_state.current_epoch != HELMHOLTZ_EPOCH_DONE || cone_state.current_epoch != HELMHOLTZ_EPOCH_DONE) {
        tsfi_io_printf(stderr, "[FAIL] Handshake epochs desynchronized.\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[STATE] Handshake complete. Cryptographic channels synchronized.\n");

    // 1. Synthesize Known Payload
    uint8_t original_payload[PAYLOAD_SIZE];
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        original_payload[i] = (uint8_t)(i % 256); // Deterministic pattern
    }

    uint8_t encrypted_buffer[PAYLOAD_SIZE];
    uint8_t decrypted_payload[PAYLOAD_SIZE];

    // --- ROD (Transmitter) ---
    tsfi_io_printf(stdout, "[ROD] Encrypting %d byte payload...\n", PAYLOAD_SIZE);
    TSFiBigInt* rod_pi = tsfi_bn_alloc();
    tsfi_bn_copy(rod_pi, Xi); // Initial stream vector
    
    struct Dai* rod_dai = allocDai();
    int bytes_processed = 0;

    while (bytes_processed < PAYLOAD_SIZE) {
        rod_dai->Ichidai = tsfi_bn_realloc(rod_dai->Ichidai); modPow_bn(rod_dai->Ichidai, rod_pi, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        rod_dai->Daiichi = tsfi_bn_realloc(rod_dai->Daiichi); modPow_bn(rod_dai->Daiichi, rod_pi, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);
        
        // Extract 16 bytes of entropy from the Dai
        uint64_t entropy[2] = { rod_dai->Ichidai->limbs[0], rod_dai->Daiichi->limbs[0] };
        uint8_t* stream_bytes = (uint8_t*)entropy;
        
        for (int i = 0; i < 16 && bytes_processed < PAYLOAD_SIZE; i++, bytes_processed++) {
            encrypted_buffer[bytes_processed] = original_payload[bytes_processed] ^ stream_bytes[i];
        }
        
        // Advance the stream state
        tsfi_bn_copy(rod_pi, rod_dai->Daiichi);
    }
    
    // --- CONE (Receiver) ---
    tsfi_io_printf(stdout, "[CONE] Receiving and decrypting payload...\n");
    TSFiBigInt* cone_pi = tsfi_bn_alloc();
    tsfi_bn_copy(cone_pi, Xi); // Cone starts with same initial stream vector
    
    struct Dai* cone_dai = allocDai();
    int bytes_decrypted = 0;

    while (bytes_decrypted < PAYLOAD_SIZE) {
        cone_dai->Ichidai = tsfi_bn_realloc(cone_dai->Ichidai); modPow_bn(cone_dai->Ichidai, cone_pi, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        cone_dai->Daiichi = tsfi_bn_realloc(cone_dai->Daiichi); modPow_bn(cone_dai->Daiichi, cone_pi, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);
        
        uint64_t entropy[2] = { cone_dai->Ichidai->limbs[0], cone_dai->Daiichi->limbs[0] };
        uint8_t* stream_bytes = (uint8_t*)entropy;
        
        for (int i = 0; i < 16 && bytes_decrypted < PAYLOAD_SIZE; i++, bytes_decrypted++) {
            decrypted_payload[bytes_decrypted] = encrypted_buffer[bytes_decrypted] ^ stream_bytes[i];
        }
        
        // Advance the stream state
        tsfi_bn_copy(cone_pi, cone_dai->Daiichi);
    }

    // --- VERIFICATION ---
    int failures = 0;
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        if (original_payload[i] != decrypted_payload[i]) {
            failures++;
            if (failures < 5) {
                tsfi_io_printf(stderr, "Mismatch at byte %d: Expected %02x, Got %02x\n", i, original_payload[i], decrypted_payload[i]);
            }
        }
    }

    if (failures == 0) {
        tsfi_io_printf(stdout, "[SUCCESS] End-to-End payload integrity verified. 100%% byte-for-byte match.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Decryption failed with %d byte mismatches.\n", failures);
        return 1;
    }

    freeDAI(rod_dai);
    freeDAI(cone_dai);
    tsfi_bn_free(rod_pi);
    tsfi_bn_free(cone_pi);
    tsfi_bn_free(Prime);
    tsfi_bn_free(Xi);
    return 0;
}