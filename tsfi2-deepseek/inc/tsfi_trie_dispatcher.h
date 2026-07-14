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

// Initialize IP CIDR Trie router for network route optimization
tsfi_trie_node* tsfi_trie_init_cidr_router(void);

// Insert a network route (e.g. "192.168.1.0/24") into the CIDR router
void tsfi_trie_add_cidr_route(tsfi_trie_node *router, const char *cidr, const char *as_path);

// Match an IP address to its longest matching CIDR prefix route
const char* tsfi_trie_route_ip(tsfi_trie_node *router, const char *ip_str);

// Initialize the ZMM interpreter contract namespace address router
tsfi_trie_node* tsfi_trie_init_contract_namespace_router(void);

// Resolve contract address namespace (e.g. dynamic_*, sys_*, usr_*) using prefix match
const char* tsfi_trie_resolve_contract_namespace(tsfi_trie_node *router, const char *contract_address);

// Initialize font ligature Trie router
tsfi_trie_node* tsfi_trie_init_ligature_router(void);

// Resolve font ligature mapping (returns glyph index or 0 if not found)
int tsfi_trie_resolve_ligature(tsfi_trie_node *router, const char *sequence);

// Initialize telemetry topic wildcard Trie router
tsfi_trie_node* tsfi_trie_init_topic_router(void);

// Resolve a specific topic to its routing destination using wildcard matches
const char* tsfi_trie_resolve_topic(tsfi_trie_node *router, const char *topic);

// Initialize phoneme-to-formant parameter Trie router
tsfi_trie_node* tsfi_trie_init_formant_router(void);

// Resolve phoneme target parameters (F1, F2, F3, zero_mix, voicing_strength) using Trie lookup
int tsfi_trie_resolve_formant(tsfi_trie_node *router, const char *phoneme, double *f1, double *f2, double *f3, double *zero_mix, double *voicing);

#endif // TSFI_TRIE_DISPATCHER_H
