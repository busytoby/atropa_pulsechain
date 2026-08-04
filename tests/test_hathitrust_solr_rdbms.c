#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "scripts/abi_dispatch_map.h"
#include "scripts/libantigravity_interop.h"
#include "inc/hathitrust_solr.h"
#include "tsfi2-deepseek/inc/lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

static uint64_t simple_hash(const char *str) {
    uint64_t hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int main(void) {
    printf("[Test] Starting HathiTrust Solr & lau_memory RDBMS integration tests...\n");
    
    // Initialize lock-free ABI Dispatch Map
    ABIDispatchMap map;
    abi_dispatch_init(&map);
    
    // Create Caller Agent (mock memory block)
    ThunkSignature mock_schema_caller[1] = {
        { 0xdddddddd, THUNK_ZMM, 5, (void*)&interop_agent_rdbms_dispatch }
    };
    char *raw_mem_caller = calloc(1, 16384);
    assert(raw_mem_caller != NULL);
    LauWiredHeader *header_caller = (LauWiredHeader*)raw_mem_caller;
    header_caller->schema_count = 1;
    header_caller->schema = mock_schema_caller;
    header_caller->sealed = true;
    header_caller->version = 1;
    void *payload_caller = raw_mem_caller + 8192;
    assert(abi_dispatch_register_member(&map, payload_caller));
    
    // Create Database Agent (mock memory block for lau_memory RDBMS)
    ThunkSignature mock_schema_db[3] = {
        { 0xcccccccc, THUNK_ZMM, 2, (void*)&interop_agent_create_table },
        { 0xaaaaaaaa, THUNK_ZMM, 2, (void*)&interop_agent_insert_dynamic },
        { 0xbbbbbbbb, THUNK_ZMM, 1, (void*)&interop_agent_select_dynamic }
    };
    char *raw_mem_db = calloc(1, 16384);
    assert(raw_mem_db != NULL);
    LauWiredHeader *header_db = (LauWiredHeader*)raw_mem_db;
    header_db->schema_count = 3;
    header_db->schema = mock_schema_db;
    header_db->sealed = true;
    header_db->version = 1;
    void *payload_db = raw_mem_db + 8192;
    assert(abi_dispatch_register_member(&map, payload_db));
    
    // A. Create table on Database Agent (via Caller Agent dispatch)
    // parameters: target_payload, selector, col_count, capacity
    uint64_t cmd_create[4] = { (uintptr_t)payload_db, 0xcccccccc, 2, 10 };
    uint64_t cmd_ret = 0;
    assert(abi_dispatch_invoke(&map, 0xdddddddd, payload_caller, cmd_create, 4, &cmd_ret));
    assert(cmd_ret == 1);
    
    // B. Parse mock Solr records and insert tuples into lau_memory RDBMS
    // Column 0: HTID Hash, Column 1: Publication Date Year
    const char *mock_htid = "uc1.31970011037748";
    uint64_t htid_key = simple_hash(mock_htid);
    uint64_t pub_year = 1930;
    
    uint64_t cmd_insert[4] = { (uintptr_t)payload_db, 0xaaaaaaaa, htid_key, pub_year };
    assert(abi_dispatch_invoke(&map, 0xdddddddd, payload_caller, cmd_insert, 4, &cmd_ret));
    assert(cmd_ret == 1);
    
    // C. Query dynamic row from database agent to verify BST Index and Helmholtz caching
    uint64_t cmd_query[3] = { (uintptr_t)payload_db, 0xbbbbbbbb, htid_key };
    assert(abi_dispatch_invoke(&map, 0xdddddddd, payload_caller, cmd_query, 3, &cmd_ret));
    assert(cmd_ret == pub_year);
    
    // D. Verify Helmholtz cache bypass hit rate speed
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000; i++) {
        // Fast-path bypass using the memoization epoch
        abi_dispatch_invoke(&map, 0xdddddddd, payload_caller, cmd_query, 3, &cmd_ret);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    printf("[Test] Average lookup latency: %.2f ns/query\n", elapsed_ns / 1000.0);
    
    // Cleanup dynamic table
    InteropDynamicTable *table = (InteropDynamicTable*)payload_db;
    interop_agent_destroy_table(table);
    
    free(raw_mem_caller);
    free(raw_mem_db);
    
    printf("[Test] Solr & lau_memory RDBMS tests completed successfully.\n");
    return 0;
}
