#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "lau_audit.h"
#include "tsfi_io.h"

static FILE *g_log_file = NULL;
static atomic_flag g_init_lock = ATOMIC_FLAG_INIT;

static void lau_cleanup_log(void) {
    if (g_log_file) {
        tsfi_io_flush(g_log_file);
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

#include "tsfi_vae_firmware.h"
#include "tsfi_vessel_anchor.h"

void lau_log_status(const char *status) {
    if (!g_log_file) {
        // Simple spinlock for initialization
        while (atomic_flag_test_and_set(&g_init_lock));
        
        if (!g_log_file) {
            g_log_file = fopen("lau_audit.log", "a");
            if (g_log_file) {
                atexit(lau_cleanup_log);
            }
        }
        
        atomic_flag_clear(&g_init_lock);
        
        if (!g_log_file) return;
    }

    // VAE Auditing verification
    static LauVaeFirmware_State audit_vae;
    static NandTrapState audit_nand;
    static bool vae_init = false;
    if (!vae_init) {
        memset(&audit_nand, 0, sizeof(NandTrapState));
        audit_nand.magic = NAND_TRAP_MAGIC;
        audit_nand.version = NAND_TRAP_VERSION;
        // Inject baseline Atropa vessel anchors
        tsfi_vessel_anchor_inject(&audit_nand, 0x999);
        tsfi_vae_firmware_boot(&audit_vae, &audit_nand);
        vae_init = true;
    }

    // Run VAE evaluation step
    LauVaeFirmware_eval_combinatorial(&audit_vae);
    LauVaeFirmware_eval_sequential(&audit_vae);

    // Audit logs are stamped with the VAE's active resonance feedback
    tsfi_io_log(g_log_file, TSFI_LOG_INFO, "AUDIT", "%s [VAE_RESO: %.5f]", status, audit_vae.spectral_feedback);

    // Check if VAE spectral resonance exceeds boundary limits (indicating potential buffer or stack frame drift)
    if (audit_vae.spectral_feedback > 10.0f || audit_vae.spectral_feedback < -10.0f) {
        tsfi_io_log(g_log_file, TSFI_LOG_WARN, "VAE_SECURITY", "VAE structural DNA drift detected! Resetting stasis...");
        audit_vae.reset_strobe = true;
        LauVaeFirmware_eval_sequential(&audit_vae);
        audit_vae.reset_strobe = false;
    }

    tsfi_io_flush(g_log_file);
}