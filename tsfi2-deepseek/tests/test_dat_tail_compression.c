#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_dat.h"
#include "tsfi_trie.h"

int main(void) {
    printf("=== TSFi Double-Array Trie Tail Compression Test ===\n");
    printf("[DEBUG] Creating trie...\n");
    fflush(stdout);

    // 1. Create a Trie node and insert keys containing unique suffixes
    tsfi_trie_node *trie = tsfi_trie_create_node('\0');
    printf("[DEBUG] Inserting keys...\n");
    fflush(stdout);
    
    // Add keys with long suffixes starting with "/bin/sysc" and "/bin/init"
    tsfi_trie_insert(trie, "/bin/sysctl", "SYSCTL_EXEC");
    tsfi_trie_insert(trie, "/bin/init", "INIT_EXEC");
    
    // 2. Compile to DAT with tail compression
    tsfi_dat *dat = tsfi_dat_compile(trie);
    assert(dat != NULL);

    // 3. Verify tail compression occurred (tail buffer is populated)
    printf("[DAT_TAIL] Tail pointer: %p\n", (void*)dat->tail);
    printf("[DAT_TAIL] Tail size: %d bytes\n", dat->tail_size);
    if (dat->tail) {
        printf("[DAT_TAIL] Tail contents: ");
        for (int i = 0; i < dat->tail_size; i++) {
            if (dat->tail[i] == '\0') printf("\\0 ");
            else printf("%c", dat->tail[i]);
        }
        printf("\n");
    }
    fflush(stdout);
    assert(dat->tail != NULL);
    assert(dat->tail_size > 0);
    printf("  [PASS] Suffix tail block successfully populated!\n");

    // 4. Query exact matching keys
    const char *res1 = tsfi_dat_search(dat, "/bin/sysctl");
    printf("[DAT_TAIL] Query '/bin/sysctl' -> Resolved: %s\n", res1 ? res1 : "NULL");
    assert(res1 != NULL && strcmp(res1, "SYSCTL_EXEC") == 0);

    const char *res2 = tsfi_dat_search(dat, "/bin/init");
    printf("[DAT_TAIL] Query '/bin/init' -> Resolved: %s\n", res2 ? res2 : "NULL");
    assert(res2 != NULL && strcmp(res2, "INIT_EXEC") == 0);
    printf("  [PASS] Exact matching keys verified via suffix tail lookup!\n");

    // 5. Query non-existent overlapping keys (must return NULL)
    const char *res3 = tsfi_dat_search(dat, "/bin/sysctx");
    printf("[DAT_TAIL] Query '/bin/sysctx' (invalid) -> Resolved: %s\n", res3 ? res3 : "NULL");
    assert(res3 == NULL);

    const char *res4 = tsfi_dat_search(dat, "/bin/in");
    printf("[DAT_TAIL] Query '/bin/in' (incomplete) -> Resolved: %s\n", res4 ? res4 : "NULL");
    assert(res4 == NULL);
    printf("  [PASS] Non-existent and incomplete keys correctly rejected!\n");

    // Clean up
    tsfi_trie_destroy(trie);
    tsfi_dat_destroy(dat);

    printf("=== ALL DAT TAIL COMPRESSION TESTS PASSED ===\n");
    return 0;
}
