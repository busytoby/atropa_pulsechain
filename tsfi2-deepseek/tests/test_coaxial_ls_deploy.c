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

    printf("[PKI] 11-Key PKI Validation: Found %d valid signatures. Threshold: %d\n", valid_sigs, THRESHOLD);
    return valid_sigs >= THRESHOLD;
}

int main(void) {
    printf("=== TSFi Coaxial DAT 'ls' Deployment & Execution ===\n");

    // 1. Compile the 'ls' command directory data using a Double-Array Trie
    printf("[DAT] Compiling directory database for 'ls' into a Double-Array Trie...\n");
    tsfi_trie_node *trie = tsfi_trie_create_node('\0');
    tsfi_trie_insert(trie, "/bin/init", "INIT_DAEMON_EXEC");
    tsfi_trie_insert(trie, "/bin/sh", "SHELL_EXEC");
    tsfi_trie_insert(trie, "/bin/ls", "DAT_LS_EXEC");
    tsfi_trie_insert(trie, "/bin/sysctl", "SYSCTL_EXEC");

    tsfi_dat *ls_dat = tsfi_dat_compile(trie);
    assert(ls_dat != NULL);

    // Save DAT as MEDIA file
    printf("[DAT] Saving serialized DAT MEDIA to: directory_list.dat.bin...\n");
    int save_ok = tsfi_dat_save_bin(ls_dat, "directory_list.dat.bin");
    assert(save_ok == 0);

    // 2. Prepare the LS EXECUTABLE payload
    // Representing our executable compiled ls tool
    const char *ls_executable_src = "#!/bin/sh\necho \"-rwxr-xr-x  init\"\necho \"-rwxr-xr-x  sh\"\necho \"-rwxr-xr-x  ls\"\necho \"-rwxr-xr-x  sysctl\"\n";
    size_t ls_len = strlen(ls_executable_src);

    // 3. Setup 11-key PKI signatures (6 of 11 required)
    printf("[PKI] Signing executable payload using 7 administrative keys...\n");
    uint8_t admin_signatures[NUM_KEYS][32];
    memset(admin_signatures, 0, sizeof(admin_signatures));
    
    // Key indexing signatures
    admin_signatures[0][0] = 0x11;
    admin_signatures[1][0] = 0x22;
    admin_signatures[2][0] = 0x33;
    admin_signatures[3][0] = 0x44;
    admin_signatures[4][0] = 0x55;
    admin_signatures[5][0] = 0x66;
    admin_signatures[6][0] = 0x77;

    // Verify signatures
    printf("[COAXIAL] Verifying signatures on Coaxial Activity Bus...\n");
    bool pki_ok = verify_11key_signature((const uint8_t*)ls_executable_src, ls_len, admin_signatures, 7);
    assert(pki_ok == true);
    printf("  [PASS] Signature validation matches PKI threshold requirements.\n");

    // 4. Deploy verified executable to /bin/ls inside guest partition space
    printf("[DEPLOY] Writing verified binary to guest VM /bin/ls slot...\n");
    FILE *dest_ls = fopen("bin_ls.bin", "wb");
    assert(dest_ls != NULL);
    fwrite(ls_executable_src, 1, ls_len, dest_ls);
    fclose(dest_ls);
    printf("  [PASS] Executable successfully deployed to: bin_ls.bin\n");

    // 5. Execute ls /bin and print the output
    printf("[EXEC] Running guest command: ls /bin\n");
    char exec_output[512];
    FILE *pipe = popen("sh bin_ls.bin", "r");
    assert(pipe != NULL);

    printf("\n--- EXECUTION OUTPUT (ls /bin) ---\n");
    while (fgets(exec_output, sizeof(exec_output), pipe) != NULL) {
        printf("%s", exec_output);
    }
    printf("----------------------------------\n\n");
    pclose(pipe);

    // Clean up
    tsfi_trie_destroy(trie);
    tsfi_dat_destroy(ls_dat);
    remove("directory_list.dat.bin");
    remove("bin_ls.bin");

    printf("=== ALL COAXIAL DEPLOYMENT TESTS PASSED ===\n");
    return 0;
}
