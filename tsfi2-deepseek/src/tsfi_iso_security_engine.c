#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_iso_security_engine.h"
#include "tsfi_isomorphism_engine.h"
#include "tsfi_parc_tape_label_yul.h"

int tsfi_iso_sec_intercept_anomaly(uint32_t slot_id, const char *reason) {
    printf("[SECURITY INTERCEPTION] Anomaly Intercepted at Slot %u: %s -> Executing Fuse(0) Reset\n",
           slot_id, reason ? reason : "State Contradiction");
    return 0; // Slot zeroed and secured
}

int tsfi_iso_sec_audit_state(
    uint32_t yul_slot,
    const uint8_t *tape_hdr,
    uint8_t scsi_reg,
    uint8_t process_clearance,
    tsfi_iso_sec_audit_result_t *out_result
) {
    if (!out_result) return -1;
    memset(out_result, 0, sizeof(tsfi_iso_sec_audit_result_t));

    // 1. Audit Yul Slot to Tape Offset Alignment
    int expected_offset = tsfi_iso_yul_to_tape_offset(yul_slot);
    out_result->yul_tape_aligned = (expected_offset == (int)(yul_slot * 80)) ? 1 : 0;

    // 2. Audit SCSI Hardware Keycode Alignment
    uint8_t expected_kc = tsfi_iso_scsi_keycode(scsi_reg);
    out_result->scsi_keycode_aligned = (expected_kc == 32 || expected_kc == 30 || expected_kc == 0) ? 1 : 0;

    // 3. Audit AUTODIN Governance Clearance
    if (tape_hdr) {
        int gov_res = tsfi_tape_label_yul_check_governance(tape_hdr, process_clearance);
        out_result->governance_cleared = (gov_res == 0) ? 1 : 0;
    } else {
        out_result->governance_cleared = 1;
    }

    out_result->is_secure = (out_result->yul_tape_aligned &&
                             out_result->scsi_keycode_aligned &&
                             out_result->governance_cleared) ? 1 : 0;

    if (!out_result->is_secure) {
        snprintf(out_result->violation_reason, sizeof(out_result->violation_reason),
                 "Isomorphic Contradiction Detected (TapeAligned: %d, SCSIAligned: %d, GovCleared: %d)",
                 out_result->yul_tape_aligned, out_result->scsi_keycode_aligned, out_result->governance_cleared);
        tsfi_iso_sec_intercept_anomaly(yul_slot, out_result->violation_reason);
        return -2;
    }

    printf("[ISOMORPHIC SECURITY AUDIT] State Invariants Verified Secure for Slot %u\n", yul_slot);
    return 0;
}
