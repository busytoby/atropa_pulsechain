#ifndef TSFI_AUTODIN_ANVIL_ORACLE_H
#define TSFI_AUTODIN_ANVIL_ORACLE_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_autodin_zmm_tx.h"

typedef struct {
    int is_isomorphic;
    int state_roots_match;
    int nonces_match;
    int signatures_match;
    char proof_digest_hex[65];
} tsfi_autodin_anvil_isom_proof_t;

/* Execute dual-path verification reproducing Anvil's transactional security inside AUTODIN */
int tsfi_autodin_verify_anvil_isom(
    const tsfi_autodin_zmm_tx_req_t *tx_req,
    tsfi_autodin_anvil_isom_proof_t *out_proof
);

#endif // TSFI_AUTODIN_ANVIL_ORACLE_H
