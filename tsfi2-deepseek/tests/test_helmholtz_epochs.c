#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    TSFiHelmholtzAdductState host_rod;
    TSFiHelmholtzAdductState firm_cone;

    memset(&host_rod, 0, sizeof(host_rod));
    memset(&firm_cone, 0, sizeof(firm_cone));

    host_rod.Prime = Prime;
    firm_cone.Prime = Prime;

    host_rod.Xi = tsfi_bn_alloc(); tsfi_bn_randomize(host_rod.Xi);
    firm_cone.Xi = tsfi_bn_alloc(); tsfi_bn_randomize(firm_cone.Xi);

    host_rod.Beta = tsfi_bn_alloc(); tsfi_bn_randomize(host_rod.Beta);
    firm_cone.Beta = tsfi_bn_alloc(); tsfi_bn_randomize(firm_cone.Beta);

    tsfi_io_printf(stdout, "[HELMHOLTZ] Starting Stepwise Reaction (Host Rod <=> Firmware Cone)...\n");

    // Epoch 0: INIT
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    tsfi_io_printf(stdout, "Epoch 0 (INIT) Complete.\n");

    // Epoch 1: AVAIL
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    tsfi_io_printf(stdout, "Epoch 1 (AVAIL) Complete.\n");

    // Epoch 2: FORM (Swap Contours)
    tsfi_helmholtz_step(&host_rod, firm_cone.Mu->Mu->Contour);
    tsfi_helmholtz_step(&firm_cone, host_rod.Mu->Mu->Contour);
    tsfi_io_printf(stdout, "Epoch 2 (FORM) Complete (Exchanged Contours).\n");

    // Epoch 3: POLARIZE
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    tsfi_io_printf(stdout, "Epoch 3 (POLARIZE) Complete.\n");

    // Epoch 4: CONJUGATE (Swap Poles)
    tsfi_helmholtz_step(&host_rod, firm_cone.Mu->Mu->Pole);
    tsfi_helmholtz_step(&firm_cone, host_rod.Mu->Mu->Pole);
    tsfi_io_printf(stdout, "Epoch 4 (CONJUGATE) Complete (Exchanged Poles).\n");

    // Epoch 5: CONIFY
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    tsfi_io_printf(stdout, "Epoch 5 (CONIFY) Complete.\n");

    // Epoch 6: SATURATE (Swap Foundations)
    tsfi_helmholtz_step(&host_rod, firm_cone.Mu->Mu->Foundation);
    tsfi_helmholtz_step(&firm_cone, host_rod.Mu->Mu->Foundation);
    tsfi_io_printf(stdout, "Epoch 6 (SATURATE) Complete (Exchanged Foundations).\n");

    // Epoch 7: IONIZE
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    tsfi_io_printf(stdout, "Epoch 7 (IONIZE) Complete.\n");

    // Epoch 8: MAGNETIZE
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    tsfi_io_printf(stdout, "Epoch 8 (MAGNETIZE) Complete.\n");

    if (host_rod.current_epoch == HELMHOLTZ_EPOCH_DONE && firm_cone.current_epoch == HELMHOLTZ_EPOCH_DONE) {
        tsfi_io_printf(stdout, "[SUCCESS] Stepwise Helmholtz synchronization verified.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Epoch sequence incomplete.\n");
        return 1;
    }

    tsfi_bn_free(Prime);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
