#ifndef TSFI_ZMM_RPC_H
#define TSFI_ZMM_RPC_H

#include "tsfi_zmm_vm.h"
#include <stddef.h>

// Dispatch a JSON-RPC command
// Returns 1 if handled, 0 if not a valid command
int tsfi_zmm_rpc_dispatch(TsfiZmmVmState *state, const char *json_in, char *output_buf, size_t out_max);

#endif
