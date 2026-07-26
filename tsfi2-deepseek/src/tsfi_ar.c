#include "tsfi_ar.h"
#include <string.h>
#include <stdio.h>

typedef enum {
    AR_CUTOFF_STATE,
    AR_CONDUC_STATE
} ar_gate_state_t;

typedef struct {
    uint32_t expected_seq;
    uint32_t last_nonce;
    ar_gate_state_t write_gate;
    char rx_buffer[64];
} ar_xcom_receiver_t;

void tsfi_ar_init(TSFiAssetResolver *ar) {
    if (!ar) return;
    memset(ar->resolved_path, 0, sizeof(ar->resolved_path));
}

bool tsfi_ar_resolve(TSFiAssetResolver *ar, const char *uri, char *out_path) {
    if (!ar || !uri || !out_path) return false;
    
    // Check if the URI uses the custom auncient:// scheme
    if (strncmp(uri, "auncient://", 11) == 0) {
        // Resolve only to .dat.bin files to enforce quadtree index restrictions
        snprintf(ar->resolved_path, sizeof(ar->resolved_path), "/root/assets/%s.dat.bin", uri + 11);
        strcpy(out_path, ar->resolved_path);
        return true;
    }
    return false;
}

void tsfi_ar_integrate_xcom(bool resolve_ok, void *xcom_rx_ptr) {
    if (!xcom_rx_ptr) return;
    ar_xcom_receiver_t *rx = (ar_xcom_receiver_t *)xcom_rx_ptr;
    if (resolve_ok) {
        rx->write_gate = AR_CONDUC_STATE;
    } else {
        rx->write_gate = AR_CUTOFF_STATE;
    }
}
