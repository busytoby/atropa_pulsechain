#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_zmm_rpc.h"

extern void tsfi_wire_firmware_init(void);

int main() {
    printf("=== ZMM STANAG GMTI Block-Ledger Transaction Rollback Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize Capstan Shaft CPU & Memory Certifier Yul State Machines
    printf("[CAPSTAN_ZMM] Loading Yul VM CPU and tape certifier components...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"mvs_tape_certifier\", \"../solidity/dysnomia/domain/bin/mvs_tape_certifier.yul\", 5");

    char output[8192];

    // ==========================================
    // TRANSACTION 1: Commit Valid GMTI Block
    // ==========================================
    printf("\n[TRANSACTION_1] Initiating transaction to write Valid GMTI Block...\n");
    // Begin sector locking transaction
    const char *begin_req = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.begin_transaction\", \"params\": {\"sector\": 20, \"callerId\": 1}, \"id\": 200}";
    int ok = tsfi_zmm_rpc_dispatch(&vm, begin_req, output, sizeof(output));
    assert(ok);
    printf("  -> Begin Response: %s\n", output);

    // Write valid GMTI data sectors (failIndex = -1, no failure) within bounds 0-49
    printf("[TRANSACTION_1] Writing valid GMTI target tracking records to ledger sectors...\n");
    const char *write_req = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.write_sectors\", \"params\": {\"startSec\": 20, \"count\": 3, \"dataVal\": 4607, \"failIndex\": -1}, \"id\": 201}";
    ok = tsfi_zmm_rpc_dispatch(&vm, write_req, output, sizeof(output));
    assert(ok);
    printf("  -> Write Response: %s\n", output);

    // Commit Valid GMTI Block
    const char *commit_req = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.commit_transaction\", \"params\": {}, \"id\": 202}";
    ok = tsfi_zmm_rpc_dispatch(&vm, commit_req, output, sizeof(output));
    assert(ok);
    printf("  -> Commit Response: %s\n", output);
    assert(strstr(output, "\"status\":\"committed\"") != NULL || strstr(output, "\"status\": \"committed\"") != NULL);
    printf("[SUCCESS] Valid GMTI Block permanently committed to Capstan block-ledger.\n");

    // ==========================================
    // TRANSACTION 2: Write Adversary Spoofed Block with Forced Rollback
    // ==========================================
    printf("\n[TRANSACTION_2] Initiating transaction to write Spoofed GMTI Block...\n");
    const char *begin_req2 = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.begin_transaction\", \"params\": {\"sector\": 30, \"callerId\": 1}, \"id\": 203}";
    ok = tsfi_zmm_rpc_dispatch(&vm, begin_req2, output, sizeof(output));
    assert(ok);
    printf("  -> Begin Response: %s\n", output);

    // Write target records containing spoofed data at sector index 1 (failIndex = 1)
    // Red Box validation detects spoofing vector and rejects the block write, triggering automatic transaction rollback.
    printf("[TRANSACTION_2] Writing target records containing spoofing vector at sector offset 1 (expecting auto-rollback)...\n");
    const char *write_req2 = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.write_sectors\", \"params\": {\"startSec\": 30, \"count\": 3, \"dataVal\": 9999, \"failIndex\": 1}, \"id\": 204}";
    ok = tsfi_zmm_rpc_dispatch(&vm, write_req2, output, sizeof(output));
    assert(ok);
    printf("  -> Write Response: %s\n", output);
    assert(strstr(output, "rolled back") != NULL);
    printf("[SUCCESS] Spoofed GMTI Block successfully intercepted and rolled back by Capstan validation loops.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ZMM STANAG GMTI ROLLBACK TRANSACTION TESTS PASSED ===\n");
    return 0;
}
