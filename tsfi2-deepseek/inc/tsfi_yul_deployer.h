#ifndef TSFI_YUL_DEPLOYER_H
#define TSFI_YUL_DEPLOYER_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    YUL_DEPLOY_TARGET_23_TREE_NODE = 1,
    YUL_DEPLOY_TARGET_ANVIL_EVM    = 2,
    YUL_DEPLOY_TARGET_IN_MEM_THUNK = 3
} tsfi_yul_deploy_target_t;

typedef struct {
    char contract_address[43]; // dynamic_<address>
    uint32_t bytecode_len;
    tsfi_yul_deploy_target_t target;
    int is_deployed;
} tsfi_yul_deploy_result_t;

/* Deploy a compiled Yul contract over AUTODIN to 2-3 Tree Node, Anvil EVM, or In-Memory Thunk */
int tsfi_yul_deploy_contract(
    const char *yul_src_path,
    tsfi_yul_deploy_target_t target,
    tsfi_yul_deploy_result_t *out_result
);

#endif // TSFI_YUL_DEPLOYER_H
