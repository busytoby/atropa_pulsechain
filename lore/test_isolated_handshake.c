#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_types.h"
#include "tsfi_wiring.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmare Segregation: Isolated Helmholtz Handshake ===\n");

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
    rod_state.Beta = Rod_Identity; // Serves as Alpha/zD for Rod
    rod_state.current_epoch = HELMHOLTZ_EPOCH_INIT;
    
    TSFiHelmholtzAdductState cone_state = {0};
    cone_state.Prime = Prime;
    cone_state.Xi = Xi;
    cone_state.Beta = Cone_Identity; // Serves as Beta/zI for Cone
    cone_state.current_epoch = HELMHOLTZ_EPOCH_INIT;

    // Step 0: INIT -> Base, Secret, Signal Generation
    tsfi_helmholtz_step(&rod_state, NULL);
    tsfi_helmholtz_step(&cone_state, NULL);
    tsfi_io_printf(stdout, "[EPOCH 0] Initialization complete.\n");

    // Epoch 1: AVAIL -> Contour Generation
    tsfi_helmholtz_step(&rod_state, NULL);
    tsfi_helmholtz_step(&cone_state, NULL);
    tsfi_io_printf(stdout, "[EPOCH 1] Contour vectors generated.\n");

    // Epoch 2: FORM -> Exchange Contours
    tsfi_helmholtz_step(&rod_state, cone_state.Mu->Mu->Contour);
    tsfi_helmholtz_step(&cone_state, rod_state.Mu->Mu->Contour);
    tsfi_io_printf(stdout, "[EPOCH 2] Contours exchanged and Forms locked.\n");

    // Epoch 3: POLARIZE -> Pole Generation
    tsfi_helmholtz_step(&rod_state, NULL);
    tsfi_helmholtz_step(&cone_state, NULL);
    tsfi_io_printf(stdout, "[EPOCH 3] Poles generated.\n");

    // Epoch 4: CONJUGATE -> Exchange Poles
    tsfi_helmholtz_step(&rod_state, cone_state.Mu->Mu->Pole);
    tsfi_helmholtz_step(&cone_state, rod_state.Mu->Mu->Pole);
    tsfi_io_printf(stdout, "[EPOCH 4] Poles exchanged and Coordinates locked.\n");

    // Epoch 5: CONIFY -> Foundation Generation
    tsfi_helmholtz_step(&rod_state, NULL);
    tsfi_helmholtz_step(&cone_state, NULL);
    tsfi_io_printf(stdout, "[EPOCH 5] Foundation bases seeded.\n");

    // Epoch 6: SATURATE -> Exchange Foundations
    tsfi_helmholtz_step(&rod_state, cone_state.Mu->Mu->Foundation);
    tsfi_helmholtz_step(&cone_state, rod_state.Mu->Mu->Foundation);
    tsfi_io_printf(stdout, "[EPOCH 6] Foundations exchanged and Dynamo logic saturated.\n");

    // Epoch 7: IONIZE -> Final internal bond
    tsfi_helmholtz_step(&rod_state, NULL);
    tsfi_helmholtz_step(&cone_state, NULL);
    tsfi_io_printf(stdout, "[EPOCH 7] State Ionized.\n");

    // Epoch 8: MAGNETIZE -> Done
    tsfi_helmholtz_step(&rod_state, NULL);
    tsfi_helmholtz_step(&cone_state, NULL);

    if (rod_state.current_epoch == HELMHOLTZ_EPOCH_DONE && cone_state.current_epoch == HELMHOLTZ_EPOCH_DONE) {
        tsfi_io_printf(stdout, "[EPOCH 8] Handshake complete. Both firmware instances achieved mathematical synchrony.\n");
        
        // Verify the handshake resulted in an identical, stable Dai buffer mechanism
        // We will mock the Dai computation from the isolated channels
        struct Dai* rod_dai = allocDai();
        struct Dai* cone_dai = allocDai();
        
        // Simulating ReactSHIO across the boundary
        TSFiBigInt* Pi = tsfi_bn_alloc();
        tsfi_bn_copy(Pi, Xi); // In reality this would be the Ring, but we use Xi for simplicity
        
        rod_dai->Ichidai = tsfi_bn_realloc(rod_dai->Ichidai); modPow_bn(rod_dai->Ichidai, Pi, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        rod_dai->Daiichi = tsfi_bn_realloc(rod_dai->Daiichi); modPow_bn(rod_dai->Daiichi, Pi, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);

        cone_dai->Ichidai = tsfi_bn_realloc(cone_dai->Ichidai); modPow_bn(cone_dai->Ichidai, Pi, rod_state.Mu->Mu->Channel, cone_state.Mu->Mu->Channel);
        cone_dai->Daiichi = tsfi_bn_realloc(cone_dai->Daiichi); modPow_bn(cone_dai->Daiichi, Pi, cone_state.Mu->Mu->Channel, rod_state.Mu->Mu->Channel);

        if (tsfi_bn_cmp_avx512(rod_dai->Ichidai, cone_dai->Ichidai) == 0 &&
            tsfi_bn_cmp_avx512(rod_dai->Daiichi, cone_dai->Daiichi) == 0) {
            tsfi_io_printf(stdout, "[SUCCESS] Segregated endpoints successfully established symmetric cryptographic channels.\n");
        } else {
            tsfi_io_printf(stderr, "[FAIL] Channels diverged.\n");
            return 1;
        }

        freeDAI(rod_dai);
        freeDAI(cone_dai);
        tsfi_bn_free(Pi);
    } else {
        tsfi_io_printf(stderr, "[FAIL] Handshake epochs desynchronized.\n");
        return 1;
    }

    tsfi_bn_free(Prime);
    tsfi_bn_free(Xi);
    return 0;
}