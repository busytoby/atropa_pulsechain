#ifndef TSFI_AUTODIN_TAPE_PROOF_H
#define TSFI_AUTODIN_TAPE_PROOF_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char volume_id[7];
    char file_id[18];
    uint8_t security_level;
    char state_hash_hex[65];
    char signature_key[32];
    int proof_valid;
    int sig_valid;
} tsfi_autodin_tape_proof_result_t;

/* Verify complete AUTODIN cryptographic signature and Merkle proof of state for a .dat.bin tape file */
int tsfi_autodin_verify_tape_file(const char *file_path, uint8_t process_clearance, tsfi_autodin_tape_proof_result_t *out_result);

#endif // TSFI_AUTODIN_TAPE_PROOF_H
