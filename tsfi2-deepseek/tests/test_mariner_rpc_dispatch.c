#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_zmm_rpc.h"
#include "../inc/tsfi_zmm_vm.h"
#include "../inc/tsfi_chancery_docket.h"

int main(void) {
    printf("====================================================================\n");
    printf("PROJECT MARINER PHASE 2: JSON-RPC VECTOR DISPATCH ENDPOINT TEST\n");
    printf("====================================================================\n\n");

    // 1. Initialize ZMM VM State
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 2. Format JSON-RPC Dispatch Request
    const char *rpc_request = 
        "{\"jsonrpc\": \"2.0\", \"method\": \"mariner.dispatch_vector\", \"params\": {\"channel_id\": 2, \"element_count\": 128, \"k_exponent\": 8}, \"id\": 8080}";

    printf("1. Dispatching JSON-RPC Request:\n%s\n\n", rpc_request);

    char rpc_response[4096] = {0};
    int handled = tsfi_zmm_rpc_dispatch(&vm, rpc_request, rpc_response, sizeof(rpc_response));

    assert(handled == 1);
    printf("2. Received JSON-RPC Response:\n%s\n", rpc_response);

    // Assert that response is valid JSON-RPC 2.0 with success status
    assert(strstr(rpc_response, "\"jsonrpc\": \"2.0\"") != NULL);
    assert(strstr(rpc_response, "\"status\": \"success\"") != NULL);
    assert(strstr(rpc_response, "\"id\": 8080") != NULL);

    tsfi_zmm_vm_destroy(&vm);

    // 3. File Resolution on Chancery Docket
    printf("3. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_id = tsfi_chancery_docket_file(
        &docket,
        "Mariner Phase 2 JSON-RPC Multi-Fabric SIMD Dispatch Endpoint",
        "solidity/dysnomia/domain/std/motzkin_3term_recurrence_handshake.algol61",
        2026
    );
    assert(doc_id == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_id, 0, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t audit_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(audit_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("MARINER PHASE 2 JSON-RPC DISPATCH TEST PASSED & SEALED ON LEDGER\n");
    printf("====================================================================\n");

    return 0;
}
