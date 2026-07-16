#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"

int main(void) {
    printf("[DAT Retraction] Starting relational retraction tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // 1. Compile relation fact
    tsfi_dat *dat1 = tsfi_dat_compile_relation(trie_root, "owns_wallet", "agent_007", "0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266");
    assert(dat1 != NULL);
    
    // Verify it exists
    const char *res1 = tsfi_dat_search(dat1, "owns_wallet/agent_007/0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266");
    assert(res1 != NULL && strcmp(res1, "RELATION_TRUE") == 0);
    printf("[DAT Retraction] Compiled and verified presence of owns_wallet/agent_007\n");
    fflush(stdout);

    // 2. Retract relation fact
    assert(tsfi_dat_retract_relation(trie_root, "owns_wallet", "agent_007", "0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266") == 0);
    
    // Re-compile trie
    tsfi_dat *dat2 = tsfi_dat_compile(trie_root);
    assert(dat2 != NULL);
    
    // Verify it is removed/empty
    const char *res2 = tsfi_dat_search(dat2, "owns_wallet/agent_007/0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266");
    assert(res2 == NULL || strcmp(res2, "") == 0);
    printf("[DAT Retraction] Successfully retracted owns_wallet/agent_007\n");
    fflush(stdout);

    // Cleanup
    tsfi_dat_destroy(dat1);
    tsfi_dat_destroy(dat2);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Relational datalog fact retraction verified successfully!\n");
    fflush(stdout);
    return 0;
}
