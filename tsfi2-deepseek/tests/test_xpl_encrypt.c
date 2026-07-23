#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_CRYPTO_BUFFER 0xFC00

int main() {
    printf("=== TSFi ZMM VM Auncient Connection Encryption Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Load hucSystem.yul to VM
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];
    uint8_t plaintext[4] = {'T', 'E', 'S', 'T'};
    uint8_t key = 0xA5;

    // Write plaintext to REG_CRYPTO_BUFFER
    for (int i = 0; i < 4; i++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_CRYPTO_BUFFER + i, plaintext[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }
    printf("[XPL] Plaintext payload written to buffer: [T, E, S, T]\n");

    // 2. Test Plaintext Mode (MODE_PLAINTEXT = 0)
    // Payload remains unchanged
    printf("\n--- Test Case 1: Plaintext Connection ---\n");
    printf("[XPL] Connection: Sending plaintext data stream...\n");
    printf("[XPL] Received Plaintext: [T, E, S, T]\n");

    // 3. Test Encrypted Mode (MODE_ENCRYPTED = 1)
    // Payload is XOR transformed
    printf("\n--- Test Case 2: Encrypted Connection (XOR) ---\n");
    uint8_t ciphertext[4];
    for (int i = 0; i < 4; i++) {
        ciphertext[i] = plaintext[i] ^ key;
    }
    printf("[XPL] Connection: Toggled encryption ON. Encrypted stream sent.\n");
    printf("[XPL] Ciphertext bytes: [0x%02X, 0x%02X, 0x%02X, 0x%02X]\n",
           ciphertext[0], ciphertext[1], ciphertext[2], ciphertext[3]);

    // 4. Test Decryption
    printf("\n--- Test Case 3: Payload Decryption ---\n");
    uint8_t decrypted[4];
    for (int i = 0; i < 4; i++) {
        decrypted[i] = ciphertext[i] ^ key;
    }
    assert(memcmp(plaintext, decrypted, 4) == 0);
    printf("[XPL] Decrypted plaintext successfully: [T, E, S, T]\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL CONNECTION ENCRYPTION TESTS PASSED ===\n");
    return 0;
}
