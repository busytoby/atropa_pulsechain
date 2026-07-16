#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_ring_buffer.h"
#include "../../scripts/libantigravity_extra2.c"

// Hash function to map string keys to 3D SVDAG voxel grid coordinates (0..99)
static int hash_coordinate(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % 100;
}

int main(void) {
    printf("[KG Unification] Unifying in-memory Knowledge Graph with SVDAG loops...\n");
    fflush(stdout);

    // 1. In-Memory Graph Triple
    const char *subject = "owns_token";
    const char *relation = "agent_007";
    const char *object = "0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266";
    
    // 2. Map triple to 3D SVDAG coordinate path: x/y/z
    int x = hash_coordinate(subject);
    int y = hash_coordinate(relation);
    int z = hash_coordinate(object);
    
    char svdag_path[256];
    snprintf(svdag_path, sizeof(svdag_path), "svdag/%d/%d/%d", x, y, z);
    printf("[KG Unification] Voxelized coordinate path: %s\n", svdag_path);
    fflush(stdout);

    // 3. Simulate Yul Syscall Execution using VM
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    // VM script pushes PKI credentials and executes simulated sys_write
    int dummy_hash = 9876;
    int dummy_v = 27;
    int dummy_r = 1111;
    int dummy_s = 2222;
    
    int syscall_script[15] = {
        1, dummy_hash,
        1, dummy_s,
        1, dummy_r,
        1, dummy_v,
        0xac,         // OP_CHECKSIG (authorizes write)
        6
    };
    assert(interop_stack_vm_execute(&vm, syscall_script, 10) == 0);
    assert(vm.stack_len == 1 && vm.stack[0] == 0x07); // Verified sender

    // 4. Solidify Voxel Path to mmap DAT Disk
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    // Compile coordinates directly as logic relation
    char x_str[32], y_str[32], z_str[32];
    snprintf(x_str, sizeof(x_str), "%d", x);
    snprintf(y_str, sizeof(y_str), "%d", y);
    snprintf(z_str, sizeof(z_str), "%d", z);
    
    tsfi_dat *dat = tsfi_dat_compile_relation(trie_root, x_str, y_str, z_str);
    assert(dat != NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long long duration_ns = (end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
    printf("[KG Unification] Solidified to DAT disk in %lld ns.\n", duration_ns);
    fflush(stdout);

    // 5. Query and verify path
    char query_path[256];
    snprintf(query_path, sizeof(query_path), "%d/%d/%d", x, y, z);
    const char *res = tsfi_dat_search(dat, query_path);
    assert(res != NULL && strcmp(res, "RELATION_TRUE") == 0);
    printf("[KG Unification] Verified SVDAG query result: %s\n", res);
    fflush(stdout);

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Knowledge Graph unification with SVDAG loops verified successfully!\n");
    fflush(stdout);
    return 0;
}
