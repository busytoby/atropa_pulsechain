#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../src/tsfi_computel_secure_kermit.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Secure Kermit Key Exchange test suite...\n");

    SecureKermitState alice;
    SecureKermitState bob;

    // Execute Seed transition
    execute_secure_kermit_seed(&alice, 123456789ULL);
    execute_secure_kermit_seed(&bob, 987654321ULL);

    // Swap Base parameters to align generators (Diffie-Hellman model requires shared generator/base)
    // In our Yul model, the Base is shared or agreed upon
    bob.base = alice.base; 

    // Derive public parameters
    uint64_t alice_pub = derive_public_parameter(&alice);
    uint64_t bob_pub = derive_public_parameter(&bob);

    // Compute shared keys
    compute_shared_session_key(&alice, bob_pub);
    compute_shared_session_key(&bob, alice_pub);

    printf("[TEST] Alice Shared Key: %lu\n", alice.shared_key);
    printf("[TEST] Bob Shared Key:   %lu\n", bob.shared_key);

    // Keys must match
    assert(alice.shared_key == bob.shared_key);
    assert(alice.is_handshake_complete == true);
    assert(bob.is_handshake_complete == true);

    // Test encryption / decryption loopback
    char message[] = "Secret Z-Machine Save State Payload";
    uint8_t len = strlen(message);

    printf("[TEST] Plaintext:  \"%s\"\n", message);

    // Encrypt at Alice's end
    crypt_kermit_payload(&alice, (uint8_t *)message, len);

    // Decrypt at Bob's end (XOR loopback)
    crypt_kermit_payload(&bob, (uint8_t *)message, len);

    printf("[TEST] Decrypted:  \"%s\"\n", message);
    assert(strcmp(message, "Secret Z-Machine Save State Payload") == 0);

    printf("[SUCCESS] Secure Kermit Key Exchange and Crypt loopback passed successfully.\n");
    return 0;
}
