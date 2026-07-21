#ifndef TSFI_ISO_SECURITY_ENGINE_H
#define TSFI_ISO_SECURITY_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int is_secure;
    int yul_tape_aligned;
    int scsi_keycode_aligned;
    int governance_cleared;
    char violation_reason[128];
} tsfi_iso_sec_audit_result_t;

/* Perform real-time isomorphic audit across Yul contracts, tape headers, and SCSI hardware */
int tsfi_iso_sec_audit_state(
    uint32_t yul_slot,
    const uint8_t *tape_hdr,
    uint8_t scsi_reg,
    uint8_t process_clearance,
    tsfi_iso_sec_audit_result_t *out_result
);

/* Intercept state anomaly and reset corrupted register slot using Fuse(0) zeroing */
int tsfi_iso_sec_intercept_anomaly(uint32_t slot_id, const char *reason);

#endif // TSFI_ISO_SECURITY_ENGINE_H
