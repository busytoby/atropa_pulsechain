#ifndef TSFI_TRIE_DISPATCHER_H
#define TSFI_TRIE_DISPATCHER_H

#include "tsfi_trie.h"

// Initialize the JSON-RPC method dispatch Trie router
tsfi_trie_node* tsfi_trie_init_rpc_router(void);

// Resolve RPC method name to its corresponding type code using Trie lookup
int tsfi_trie_resolve_rpc(tsfi_trie_node *router, const char *method_name);

// Initialize the ABI function selector Trie router
tsfi_trie_node* tsfi_trie_init_abi_router(void);

// Resolve ABI function selector (hex string) to its metadata using Trie lookup
const char* tsfi_trie_resolve_abi(tsfi_trie_node *router, const char *selector);

// Initialize SCSI WinchesterMQ command prefix router
tsfi_trie_node* tsfi_trie_init_scsi_router(void);

// Resolve low-level SCSI handshake actions using Trie prefix match
const char* tsfi_trie_resolve_scsi(tsfi_trie_node *router, const char *scsi_cmd);

#endif // TSFI_TRIE_DISPATCHER_H
