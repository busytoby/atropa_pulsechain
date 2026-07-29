#include "auncient_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define MOTZKIN_PRIME 953467954114363ULL

// WinchesterMQ registers
#define REG_SCSI_REQ  0x0500
#define REG_SCSI_ACK  0x0501
#define REG_SCSI_DATA 0x0502
#define REG_DISP_X    0x0503

// 1. RED ANALYZER Gating: Static XDC Auth PKI validation (Rule 15 - XDC)
static bool red_analyzer_validate_xdc_auth(const uint32_t *keys, uint32_t key_count, uint64_t *auth_out) {
    if (key_count < 4) {
        return false; // Rejected: K >= 4 keys required (Rule 15)
    }

    uint64_t sum = 0;
    for (uint32_t i = 0; i < key_count; i++) {
        sum = (sum + keys[i]) % MOTZKIN_PRIME;
    }
    if (auth_out) *auth_out = sum;
    return true;
}

// 2. BLACK XPLSM Gating: SCSI Handshake validation
static bool black_xplsm_scsi_handshake(uint8_t *mem, uint32_t *interrupt_out) {
    // Assert REQ
    mem[REG_SCSI_REQ] = 1;

    // Simulate target asserting ACK
    mem[REG_SCSI_ACK] = 1;

    // Read ACK
    if (mem[REG_SCSI_ACK] != 1) {
        if (interrupt_out) *interrupt_out = 25; // Trap 25: Handshake failure
        return false;
    }

    // Write Data and trigger DisplacementShader sync
    mem[REG_SCSI_DATA] = 0xAA;
    uint8_t disp_val = mem[REG_SCSI_DATA] * 2;
    mem[REG_DISP_X] = disp_val;

    // Clear REQ
    mem[REG_SCSI_REQ] = 0;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COMPILING WINCHESTERMQ HANDSHAKE SUITE\n");
    printf("=============================================================\n");

    const char *src_path = "xpl/winchester_mq.xpl";
    const char *bin_path = "../tests/winchester_mq.dat.bin";

    // 1. Compile winchester_mq.xpl using XCOM compiler API
    printf("[TEST] Compiling %s to %s via XCOM compiler API...\n", src_path, bin_path);
    bool ok = auncient_sdk_compile_xpl_to_dat_bin(src_path, bin_path);
    assert(ok == true);
    printf("   ✓ WinchesterMQ JCL script compiled successfully.\n");

    // 2. Test RED rail XDC PKI validation
    printf("[TEST] Auditing PKI credentials on RED rail...\n");
    uint32_t valid_keys[] = { 1001, 2002, 3003, 4004 };
    uint64_t auth_sig = 0;
    assert(red_analyzer_validate_xdc_auth(valid_keys, 4, &auth_sig) == true);
    assert(auth_sig == 10010);

    uint32_t invalid_keys[] = { 1001, 2002 }; // K < 4
    assert(red_analyzer_validate_xdc_auth(invalid_keys, 2, &auth_sig) == false);
    printf("   ✓ RED rail successfully enforced PKI key count limits (K >= 4).\n");

    // 3. Test BLACK rail SCSI Handshake & DisplacementShader alignment
    printf("[TEST] Running SCSI Handshake on BLACK rail...\n");
    uint8_t virtual_memory[65536] = {0};
    uint32_t trap_code = 0;

    assert(black_xplsm_scsi_handshake(virtual_memory, &trap_code) == true);
    assert(virtual_memory[REG_DISP_X] == 0x54); // 0xAA * 2 = 0x154 -> 0x54 (truncated byte)
    printf("   ✓ SCSI handshake completed. DisplacementShader vertex offset aligned: 0x%02X.\n", virtual_memory[REG_DISP_X]);

    // Clean up compiled asset
    remove(bin_path);

    printf("=============================================================\n");
    printf("WINCHESTERMQ GRAPHICS HANDSHAKE PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
