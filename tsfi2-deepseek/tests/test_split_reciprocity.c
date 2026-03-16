#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    struct YI* ref_yi = tsfi_reaction_shoot(Prime);
    struct Dai* ref_dai = tsfi_reaction_compute_reciprocity(ref_yi->Psi, ref_yi->Ring ? ref_yi->Ring : ref_yi->Xi, NULL);

    tsfi_io_printf(stdout, "[RECIPROCITY] Reference Ichidai: %lu\n", ref_dai->Ichidai->limbs[0]);
    tsfi_io_printf(stdout, "[RECIPROCITY] Reference Daiichi: %lu\n", ref_dai->Daiichi->limbs[0]);

    TSFiHelmholtzAdductState host_rod;
    TSFiHelmholtzAdductState firm_cone;
    memset(&host_rod, 0, sizeof(host_rod));
    memset(&firm_cone, 0, sizeof(firm_cone));

    host_rod.Prime = Prime;
    firm_cone.Prime = Prime;
    
    host_rod.Xi = tsfi_bn_alloc(); tsfi_bn_randomize(host_rod.Xi);
    firm_cone.Xi = tsfi_bn_alloc(); tsfi_bn_copy(firm_cone.Xi, host_rod.Xi);
    
    host_rod.Beta = tsfi_bn_alloc(); tsfi_bn_randomize(host_rod.Beta);
    firm_cone.Beta = tsfi_bn_alloc(); tsfi_bn_randomize(firm_cone.Beta);

    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    
    tsfi_helmholtz_step(&host_rod, firm_cone.Mu->Mu->Contour);
    tsfi_helmholtz_step(&firm_cone, host_rod.Mu->Mu->Contour);
    
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    
    tsfi_helmholtz_step(&host_rod, firm_cone.Mu->Mu->Pole);
    tsfi_helmholtz_step(&firm_cone, host_rod.Mu->Mu->Pole);
    
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);
    
    tsfi_helmholtz_step(&host_rod, firm_cone.Mu->Mu->Foundation);
    tsfi_helmholtz_step(&firm_cone, host_rod.Mu->Mu->Foundation);
    
    tsfi_helmholtz_step(&host_rod, NULL);
    tsfi_helmholtz_step(&firm_cone, NULL);

    struct SHIO* split_shio = allocSHIO();
    split_shio->Rho = allocSHAO();
    split_shio->Rho->Rod = host_rod.Mu;
    split_shio->Rho->Cone = firm_cone.Mu;
    
    struct Dai* split_dai = tsfi_reaction_compute_reciprocity(split_shio, host_rod.Xi, NULL);

    tsfi_io_printf(stdout, "[RECIPROCITY] Split Ichidai: %lu\n", split_dai->Ichidai->limbs[0]);
    tsfi_io_printf(stdout, "[RECIPROCITY] Split Daiichi: %lu\n", split_dai->Daiichi->limbs[0]);

    if (tsfi_bn_cmp_avx512(split_dai->Ichidai, split_dai->Daiichi) == 0) {
        tsfi_io_printf(stderr, "[ERROR] Ichidai == Daiichi (Degenerate case). Check entropy.\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[SUCCESS] Split Reciprocity verified.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
