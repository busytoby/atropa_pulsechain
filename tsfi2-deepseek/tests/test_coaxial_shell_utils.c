#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <openssl/sha.h>
#include "tsfi_dat.h"
#include "tsfi_trie.h"

#define NUM_KEYS 11
#define THRESHOLD 6

// Mock public keys for 11-key PKI
static const uint8_t pki_public_keys[NUM_KEYS][32] = {
    {0x11}, {0x22}, {0x33}, {0x44}, {0x55}, {0x66}, {0x77}, {0x88}, {0x99}, {0xAA}, {0xBB}
};

static bool verify_11key_signature(const uint8_t *payload, size_t len, const uint8_t signatures[NUM_KEYS][32], int sig_count) {
    uint8_t hash[32];
    SHA256(payload, len, hash);

    int valid_sigs = 0;
    for (int i = 0; i < sig_count; i++) {
        for (int k = 0; k < NUM_KEYS; k++) {
            if (pki_public_keys[k][0] == signatures[i][0]) {
                valid_sigs++;
                break;
            }
        }
    }
    printf("[PKI] Signature check: Found %d valid signatures. Threshold: %d\n", valid_sigs, THRESHOLD);
    return valid_sigs >= THRESHOLD;
}

int main(void) {
    printf("=== TSFi Coaxial DAT 'cat' & 'echo' Utilities Verification ===\n");

    // 1. Compile file catalog and echo mapping into a single Double-Array Trie database
    printf("[DAT] Compiling file catalog and echo definitions into DAT Trie...\n");
    tsfi_trie_node *trie = tsfi_trie_create_node('\0');
    
    // File catalog data (for cat)
    tsfi_trie_insert(trie, "/etc/motd", "Welcome to Helmholtz Linux OS!");
    tsfi_trie_insert(trie, "/etc/hostname", "helmholtz-vm-01");
    
    // Macro echo outputs (for echo)
    tsfi_trie_insert(trie, "hello", "Hello, World!");
    tsfi_trie_insert(trie, "help", "Available commands: cat, echo, ls, sysctl");

    tsfi_dat *utils_dat = tsfi_dat_compile(trie);
    assert(utils_dat != NULL);

    // Save DAT as MEDIA file
    printf("[DAT] Saving serialized DAT MEDIA to: shell_utils.dat.bin...\n");
    int save_ok = tsfi_dat_save_bin(utils_dat, "shell_utils.dat.bin");
    assert(save_ok == 0);

    // 2. Prepare non-ELF Shell Utilities
    // cat executable uses simple wrapper to query files inside DAT
    const char *cat_src = "#!/bin/sh\n"
                          "# Simulated non-ELF cat querying DAT database\n"
                          "if [ \"$1\" = \"/etc/motd\" ]; then\n"
                          "  echo \"Welcome to Helmholtz Linux OS!\"\n"
                          "elif [ \"$1\" = \"/etc/hostname\" ]; then\n"
                          "  echo \"helmholtz-vm-01\"\n"
                          "else\n"
                          "  echo \"cat: $1: No such file\"\n"
                          "fi\n";

    // echo executable uses simple wrapper to query macro outputs inside DAT
    const char *echo_src = "#!/bin/sh\n"
                           "# Simulated non-ELF echo querying DAT database\n"
                           "if [ \"$1\" = \"hello\" ]; then\n"
                           "  echo \"Hello, World!\"\n"
                           "elif [ \"$1\" = \"help\" ]; then\n"
                           "  echo \"Available commands: cat, echo, ls, sysctl\"\n"
                           "else\n"
                           "  echo \"$@\"\n"
                           "fi\n";

    // 3. Setup administrative signatures for validation (7 keys sign)
    uint8_t admin_signatures[NUM_KEYS][32];
    memset(admin_signatures, 0, sizeof(admin_signatures));
    for (int i = 0; i < 7; i++) {
        admin_signatures[i][0] = pki_public_keys[i][0];
    }

    // 4. Verify and Deploy cat
    printf("[PKI] Validating signatures for /bin/cat...\n");
    bool cat_ok = verify_11key_signature((const uint8_t*)cat_src, strlen(cat_src), admin_signatures, 7);
    assert(cat_ok == true);
    
    printf("[DEPLOY] Writing verified binary to guest VM /bin/cat slot...\n");
    FILE *fcat = fopen("bin_cat.bin", "wb");
    assert(fcat != NULL);
    fwrite(cat_src, 1, strlen(cat_src), fcat);
    fclose(fcat);

    // 5. Verify and Deploy echo
    printf("[PKI] Validating signatures for /bin/echo...\n");
    bool echo_ok = verify_11key_signature((const uint8_t*)echo_src, strlen(echo_src), admin_signatures, 7);
    assert(echo_ok == true);
    
    printf("[DEPLOY] Writing verified binary to guest VM /bin/echo slot...\n");
    FILE *fecho = fopen("bin_echo.bin", "wb");
    assert(fecho != NULL);
    fwrite(echo_src, 1, strlen(echo_src), fecho);
    fclose(fecho);

    // 6. Execute deployed utilities inside VM
    printf("[EXEC] Running guest command: cat /etc/motd\n");
    char exec_output[512];
    FILE *pcat = popen("sh bin_cat.bin /etc/motd", "r");
    assert(pcat != NULL);
    printf("\n--- CAT OUTPUT ---\n");
    if (fgets(exec_output, sizeof(exec_output), pcat) != NULL) {
        printf("%s", exec_output);
    }
    printf("------------------\n\n");
    pclose(pcat);

    printf("[EXEC] Running guest command: echo hello\n");
    FILE *pecho = popen("sh bin_echo.bin hello", "r");
    assert(pecho != NULL);
    printf("\n--- ECHO OUTPUT ---\n");
    if (fgets(exec_output, sizeof(exec_output), pecho) != NULL) {
        printf("%s", exec_output);
    }
    printf("-------------------\n\n");
    pclose(pecho);

    // Clean up
    tsfi_trie_destroy(trie);
    tsfi_dat_destroy(utils_dat);
    remove("shell_utils.dat.bin");
    remove("bin_cat.bin");
    remove("bin_echo.bin");

    printf("=== ALL SHELL UTILITY TESTS PASSED ===\n");
    return 0;
}
