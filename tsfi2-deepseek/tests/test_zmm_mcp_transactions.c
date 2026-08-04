#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_zmm_rpc.h"

extern void tsfi_wire_firmware_init(void);

int main() {
    tsfi_wire_firmware_init();
    
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);
    
    // Load cpu6502 (needed for checkpointing)
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");
    
    // Load mvs_tape_certifier
    tsfi_zmm_vm_exec(&vm, "YULINIT \"mvs_tape_certifier\", \"../solidity/dysnomia/domain/bin/mvs_tape_certifier.yul\", 5");
    
    char output[8192];
    
    // 1. Begin transaction (lock sector 40 for caller 1)
    printf("[MCP_TEST] Beginning transaction on sector 40...\n");
    const char *begin_req = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.begin_transaction\", \"params\": {\"sector\": 40, \"callerId\": 1}, \"id\": 100}";
    int ok = tsfi_zmm_rpc_dispatch(&vm, begin_req, output, sizeof(output));
    assert(ok);
    printf("Begin Response: %s\n", output);
    assert(strstr(output, "\"status\":\"success\"") != NULL || strstr(output, "\"status\": \"success\"") != NULL);
    
    // 2. Write group (3 sectors, starting at 10, value 99, failIndex -1)
    printf("[MCP_TEST] Writing sector group...\n");
    const char *write_req = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.write_sectors\", \"params\": {\"startSec\": 10, \"count\": 3, \"dataVal\": 99, \"failIndex\": -1}, \"id\": 101}";
    ok = tsfi_zmm_rpc_dispatch(&vm, write_req, output, sizeof(output));
    assert(ok);
    printf("Write Response: %s\n", output);
    assert(strstr(output, "\"status\":\"success\"") != NULL || strstr(output, "\"status\": \"success\"") != NULL);
    
    // 3. Commit transaction
    printf("[MCP_TEST] Committing transaction...\n");
    const char *commit_req = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.commit_transaction\", \"params\": {}, \"id\": 102}";
    ok = tsfi_zmm_rpc_dispatch(&vm, commit_req, output, sizeof(output));
    assert(ok);
    printf("Commit Response: %s\n", output);
    assert(strstr(output, "\"status\":\"committed\"") != NULL || strstr(output, "\"status\": \"committed\"") != NULL);
    
    // 4. Begin transaction 2
    printf("[MCP_TEST] Beginning transaction 2 on sector 50...\n");
    const char *begin_req2 = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.begin_transaction\", \"params\": {\"sector\": 50, \"callerId\": 1}, \"id\": 103}";
    ok = tsfi_zmm_rpc_dispatch(&vm, begin_req2, output, sizeof(output));
    assert(ok);
    printf("Begin Response: %s\n", output);
    
    // 5. Write group with forced failure (fail at index 1)
    printf("[MCP_TEST] Writing sector group with forced failure (expecting auto-rollback)...\n");
    const char *write_req2 = "{\"jsonrpc\": \"2.0\", \"method\": \"spool.write_sectors\", \"params\": {\"startSec\": 10, \"count\": 3, \"dataVal\": 88, \"failIndex\": 1}, \"id\": 104}";
    ok = tsfi_zmm_rpc_dispatch(&vm, write_req2, output, sizeof(output));
    assert(ok);
    printf("Write Response: %s\n", output);
    assert(strstr(output, "rolled back") != NULL);
    
    tsfi_zmm_vm_destroy(&vm);
    printf("[MCP_TEST] === ALL MCP TRANSACTION TESTS PASSED ===\n");
    return 0;
}
