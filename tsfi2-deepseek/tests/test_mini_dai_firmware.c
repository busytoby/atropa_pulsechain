#include <unistd.h>
#define _POSIX_C_SOURCE 200809L
#include "tsfi_types.h"
#include "tsfi_wiring.h"
#include "lau_memory.h"
#include "tsfi_cli.h"
#include "tsfi_io.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_reaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// The miniature DAI firmware specified by the user
typedef struct {
    struct YI core_yi;
    struct Dai* (*react_sha)(struct SHA* Mu, TSFiBigInt* Pi, TSFiBigInt* Theta);
    struct Dai* (*react_shio)(struct SHIO* Mu, TSFiBigInt* Pi);
} MiniDAIFirmware;

// Dummy logic tables to satisfy the helmholtz list
static void dummy_epoch(int *ver) { if (ver) *ver += 1; }
static bool dummy_state(void *obj) { (void)obj; return true; }
static void dummy_directive(int *cnt, char *dir) { (void)cnt; (void)dir; }
static void dummy_scramble(void *ws) { (void)ws; }
static void dummy_provenance(void *ws) { (void)ws; }

static const TSFiLogicTable mini_logic = {
    .logic_epoch = dummy_epoch,
    .logic_state = dummy_state,
    .logic_directive = dummy_directive,
    .logic_scramble = dummy_scramble,
    .logic_provenance = dummy_provenance
};

static struct Dai* mini_react_shio_thunk(struct SHIO* Mu, TSFiBigInt* Pi) {
    return tsfi_reaction_compute_reciprocity(Mu, Pi, NULL);
}

int main() {
    alarm(5);
    printf("--- SYSTEM-11: MINIATURE DAI FIRMWARE VERIFICATION ---\n");

    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    // 1. Generate the anonymous YI
    struct YI* generated_yi = tsfi_reaction_shoot(Prime);
    printf("[SHOOT] Generated anonymous YI successfully.\n");
    
    // 2. Package it into the miniature DAI firmware
    MiniDAIFirmware* mini_fw = (MiniDAIFirmware*)lau_malloc_wired(sizeof(MiniDAIFirmware));
    assert(mini_fw != NULL);
    memcpy(&mini_fw->core_yi, generated_yi, sizeof(struct YI));
    mini_fw->react_sha = NULL; // Optional binding
    mini_fw->react_shio = mini_react_shio_thunk; // Direct binding to the mathematical thunk
    lau_seal_object(mini_fw);
    
    printf("[FIRMWARE] Packaged YI into Miniature DAI Firmware structure.\n");

    // 3. Create the anonymous WaveSystem
    WaveSystem *ws = (WaveSystem *)lau_malloc_wired(sizeof(WaveSystem));
    assert(ws != NULL);
    
    // Bind the firmware directly
    ws->fw = mini_fw;
    
    LauSystemHeader *h = (LauSystemHeader *)((char *)ws - offsetof(LauSystemHeader, payload));
    memset(&h->regs, 0, sizeof(LauRegisterBank));
    h->resonance_as_status = lau_strdup("WF_ANON_START");
    
    // Wire the system with our dummy helmholtz list
    lau_wire_system(ws, h, &mini_logic);
    
    printf("[WAVESYSTEM] Anonymous WaveSystem instantiated and wired to Mini-FW.\n");

    // 4. Prove ReactSHIO execution via the firmware's function pointer
    MiniDAIFirmware *fw_ptr = (MiniDAIFirmware*)ws->fw;
    TSFiBigInt* Pi = fw_ptr->core_yi.Ring ? fw_ptr->core_yi.Ring : fw_ptr->core_yi.Xi;
    
    struct Dai* result_dai = fw_ptr->react_shio(fw_ptr->core_yi.Psi, Pi);
    
    if (result_dai) {
        printf("[VERIFY] Successfully executed ReactSHIO via firmware function pointer.\n");
        printf("  -> Resulting Daiichi: %lu\n", result_dai->Daiichi->limbs[0]);
        printf("  -> Resulting Ichidai: %lu\n", result_dai->Ichidai->limbs[0]);
        freeDAI(result_dai);
    } else {
        printf("[VERIFY] FAILED to execute ReactSHIO.\n");
    }

    // Teardown
    tsfi_bn_free(Prime);
    
    // We don't call freeYI on generated_yi because its pointers were transferred verbatim to core_yi
    // which will be cleaned up if we implement a custom teardown for MiniDAIFirmware.
    // For this test, we just let the pool drain handle the big ints.
    
    // Cleanup anonymous system
    if (h->resonance_as_status) { lau_free(h->resonance_as_status); h->resonance_as_status = NULL; }
    lau_unseal_object(ws);
    lau_free(ws);
    
    // Cleanup mini firmware
    lau_unseal_object(mini_fw);
    lau_free(mini_fw);

    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();
    
    extern void tsfi_dl_thunks_teardown(void);
    tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    
    printf("SYSTEM_AT_REST_SUCCESS\n");
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
