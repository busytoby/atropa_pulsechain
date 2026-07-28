#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_yul_deployer.h"

int main(void) {
    printf("=============================================================\n");
    printf("YUL REGISTER ALLOCATION CO-PROCESSOR DEPLOYMENT TEST\n");
    printf("=============================================================\n");

    const char *yul_path = "solidity/bin/tsfi_rau_coprocessor.yul";
    tsfi_yul_deploy_result_t result;

    printf("[TEST] Deploying RAU Co-processor Yul contract to Anvil EVM...\n");
    int res = tsfi_yul_deploy_contract(yul_path, YUL_DEPLOY_TARGET_ANVIL_EVM, &result);
    
    assert(res == 0);
    assert(result.is_deployed == 1);
    assert(result.bytecode_len > 0);
    
    // Rule 9 Check: Address-based resolution (must start with dynamic_)
    assert(strncmp(result.contract_address, "dynamic_", 8) == 0);
    
    printf("   ✓ Deployed Status : %s\n", result.is_deployed ? "ACTIVE" : "INACTIVE");
    printf("   ✓ Deployed Address: %s\n", result.contract_address);
    printf("   ✓ Bytecode Length : %d bytes\n", result.bytecode_len);

    printf("=============================================================\n");
    printf("YUL RAU DEPLOYMENT TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
