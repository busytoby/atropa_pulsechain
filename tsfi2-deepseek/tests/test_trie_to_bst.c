#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"

// Helper to verify BST ordering (inorder traversal should print alphabetically)
static void verify_bst_inorder(tsfi_string_bst_node *node, char *last_key) {
    if (!node) return;
    verify_bst_inorder(node->left, last_key);
    
    // In-order traversal must yield alphabetically ascending keys
    printf("  - BST Node Key: '%s' -> Value: '%s'\n", node->key, node->value);
    if (last_key[0] != '\0') {
        assert(strcmp(node->key, last_key) > 0);
    }
    strcpy(last_key, node->key);
    
    verify_bst_inorder(node->right, last_key);
}

int main(void) {
    printf("=== TSFi Trie to Alphabetical BST Verification ===\n");

    // 1. Initialize and populate Trie
    tsfi_trie_node *trie_root = tsfi_trie_create_node('\0');
    assert(trie_root != NULL);

    printf("[CONVERT] Populating Trie in non-alphabetical order...\n");
    tsfi_trie_insert(trie_root, "orange", "ORANGE_VAL");
    tsfi_trie_insert(trie_root, "apple", "APPLE_VAL");
    tsfi_trie_insert(trie_root, "banana", "BANANA_VAL");
    tsfi_trie_insert(trie_root, "pear", "PEAR_VAL");
    tsfi_trie_insert(trie_root, "peach", "PEACH_VAL");

    // 2. Convert Trie to BST
    printf("[CONVERT] Converting Trie to AVL String BST...\n");
    tsfi_string_bst_node *bst_root = tsfi_trie_to_string_bst(trie_root);
    assert(bst_root != NULL);
    printf("  [PASS] Trie-to-BST conversion generated a valid tree.\n");

    // 3. Verify exact lookups in the converted BST
    printf("[CONVERT] Testing lookups in converted BST...\n");
    const char *v1 = tsfi_string_bst_find(bst_root, "banana");
    assert(v1 != NULL && strcmp(v1, "BANANA_VAL") == 0);
    printf("  [PASS] 'banana' resolved correctly to: %s\n", v1);

    const char *v2 = tsfi_string_bst_find(bst_root, "apple");
    assert(v2 != NULL && strcmp(v2, "APPLE_VAL") == 0);
    printf("  [PASS] 'apple' resolved correctly to: %s\n", v2);

    const char *v_none = tsfi_string_bst_find(bst_root, "grape");
    assert(v_none == NULL);
    printf("  [PASS] Non-existent key 'grape' correctly resolved to NULL.\n");

    // 4. Verify alphabetical sorting order via inorder traversal
    printf("[CONVERT] Verifying alphabetical sorting order...\n");
    char last_key[256] = {0};
    verify_bst_inorder(bst_root, last_key);
    printf("  [PASS] In-order traversal validated alphabetically ascending keys.\n");

    // 5. Test Balanced Construction using Sort & Select
    printf("[CONVERT] Testing balanced Trie-to-BST conversion...\n");
    tsfi_string_bst_node *balanced_root = tsfi_trie_to_string_bst_balanced(trie_root);
    assert(balanced_root != NULL);
    
    // Check balanced height (for 5 elements, max balanced BST height is 3)
    printf("  - Balanced BST Height: %d\n", balanced_root->height);
    assert(balanced_root->height <= 3);
    
    printf("[CONVERT] Verifying balanced BST sorting order...\n");
    char last_key_bal[256] = {0};
    verify_bst_inorder(balanced_root, last_key_bal);
    printf("  [PASS] Balanced BST inorder traversal validated.\n");

    // 6. Cleanup
    tsfi_trie_destroy(trie_root);
    tsfi_string_bst_destroy(bst_root);
    tsfi_string_bst_destroy(balanced_root);

    printf("=== ALL TRIE-TO-BST CONVERSION TESTS PASSED ===\n");
    return 0;
}
