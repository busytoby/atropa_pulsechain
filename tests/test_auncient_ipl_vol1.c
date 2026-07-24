#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define RECORD_SIZE 80

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

// Volume Label (VOL1) representation in I/PL
typedef struct {
    char label_id[5];      // Must be "VOL1"
    char volume_serial[7]; // Volume serial identifier (e.g. "SYSRES")
    uint8_t boot_block_hash[HASH_SIZE]; // Expected Merkle root of IPL boot payload
} ipl_vol1_record_t;

// -------------------------------------------------------------
// VOL1 Header Verification and Authentication
// -------------------------------------------------------------
bool verify_ipl_volume_label(const ipl_vol1_record_t *record, 
                             const char *expected_serial,
                             const uint8_t *payload,
                             size_t payload_len) {
    // 1. Verify Identifier
    if (strncmp(record->label_id, "VOL1", 4) != 0) {
        return false; // Not a valid volume label
    }

    // 2. Match Volume Serial Number
    if (strncmp(record->volume_serial, expected_serial, 6) != 0) {
        return false; // Volume serial mismatch
    }

    // 3. Compute and authenticate boot payload signature
    uint8_t calculated_hash[HASH_SIZE];
    sha256(payload, payload_len, calculated_hash);

    if (memcmp(record->boot_block_hash, calculated_hash, HASH_SIZE) != 0) {
        return false; // Security signature verification failed
    }

    return true; // Verification passed. Safe to I/PL boot.
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT I/PL VOLUME LABEL (VOL1) BOOT VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Set up simulated boot payload
    const char *boot_payload = "AUNCIENT_KERN_SYSRES_BOOT_SECTOR_DATA";
    size_t payload_len = strlen(boot_payload);

    uint8_t expected_hash[HASH_SIZE];
    sha256(boot_payload, payload_len, expected_hash);

    // Initialize valid VOL1 record
    ipl_vol1_record_t valid_record = {
        .label_id = "VOL1",
        .volume_serial = "SYSRES"
    };
    memcpy(valid_record.boot_block_hash, expected_hash, HASH_SIZE);

    // 1. Verify compliant volume label -> Should pass and boot
    printf("[TEST] Verifying compliant VOL1 volume label...\n");
    fflush(stdout);
    bool ok = verify_ipl_volume_label(&valid_record, "SYSRES", (const uint8_t *)boot_payload, payload_len);
    assert(ok == true);
    printf("   ✓ Volume Label validated. Boot signature matches target.\n");
    fflush(stdout);

    // 2. Verify mismatched volume serial -> Should fail
    printf("[TEST] Verifying label with serial mismatch...\n");
    fflush(stdout);
    ok = verify_ipl_volume_label(&valid_record, "ALTRES", (const uint8_t *)boot_payload, payload_len);
    assert(ok == false);
    printf("   ✓ Mismatched serial successfully blocked from boot.\n");
    fflush(stdout);

    // 3. Verify corrupted payload -> Should fail hash check
    printf("[TEST] Verifying corrupted boot payload...\n");
    fflush(stdout);
    const char *corrupted_payload = "CORRUPTED_KERN_SYSRES_BOOT_SECTOR_DATA";
    ok = verify_ipl_volume_label(&valid_record, "SYSRES", (const uint8_t *)corrupted_payload, strlen(corrupted_payload));
    assert(ok == false);
    printf("   ✓ Payload signature mismatch successfully blocked from boot.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("I/PL VOL1 BOOT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
