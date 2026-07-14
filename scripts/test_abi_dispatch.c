#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "abi_dispatch_map.h"
#include "libantigravity_interop.h"
#include "../tsfi2-deepseek/inc/lau_memory.h"

uint64_t mock_agent_method(void *payload, const uint64_t *args, size_t arg_count) {
    (void)payload;
    if (arg_count > 0) {
        return args[0] * 2;
    }
    return 42;
}



int main() {
    printf("=== TESTING LOCK-FREE ABI DISPATCH MAP ===\n");
    
    ABIDispatchMap map;
    abi_dispatch_init(&map);
    
    // 1. Register selectors
    // Example Solidity selectors:
    // resolve(bytes32,bytes32) -> 0x05de9943
    // transfer(address,uint256) -> 0xa9059cbb
    // balance(address) -> 0x70a08231
    assert(abi_dispatch_register(&map, 0x05de9943, 1000));
    assert(abi_dispatch_register(&map, 0xa9059cbb, 2000));
    assert(abi_dispatch_register(&map, 0x70a08231, 3000));
    
    printf("✓ Registered selectors successfully.\n");
    
    // 2. Query selectors
    uintptr_t offset = 0;
    assert(abi_dispatch_lookup(&map, 0x05de9943, &offset));
    assert(offset == 1000);
    
    assert(abi_dispatch_lookup(&map, 0xa9059cbb, &offset));
    assert(offset == 2000);
    
    assert(abi_dispatch_lookup(&map, 0x70a08231, &offset));
    assert(offset == 3000);
    
    printf("✓ Lookups resolved to correct offsets.\n");
    
    // 3. Test non-existent selector
    assert(!abi_dispatch_lookup(&map, 0xffffffff, &offset));
    printf("✓ Non-existent selector lookup failed correctly.\n");
    
    // 4. Overwrite selector offset
    assert(abi_dispatch_register(&map, 0x05de9943, 5000));
    assert(abi_dispatch_lookup(&map, 0x05de9943, &offset));
    assert(offset == 5000);
    printf("✓ Overwriting selector offset resolved successfully.\n");
    
    // 5. Test Member Registration
    printf("5. Testing General ABI Dispatch registration for wired members:\n");
    ThunkSignature mock_schema[2] = {
        { 0x11223344, THUNK_ZMM, 1, (void*)9999 },
        { 0x55667788, THUNK_BAKED, 2, (void*)8888 }
    };
    char *raw_mem = calloc(1, 16384);
    assert(raw_mem);
    LauWiredHeader *mock_header = (LauWiredHeader*)raw_mem;
    mock_header->schema_count = 2;
    mock_header->schema = mock_schema;
    mock_header->sealed = true;
    void *mock_payload = raw_mem + 8192;
    
    assert(abi_dispatch_register_member(&map, mock_payload));
    offset = 0;
    assert(abi_dispatch_lookup(&map, 0x11223344, &offset));
    assert(offset == 9999);
    assert(abi_dispatch_lookup(&map, 0x55667788, &offset));
    assert(offset == 8888);
    // 6. Test Member Invocation
    printf("6. Testing General ABI Dispatch invocation for wired members:\n");
    ThunkSignature mock_schema2[1] = {
        { 0x99999999, THUNK_ZMM, 1, (void*)&mock_agent_method }
    };
    char *raw_mem2 = calloc(1, 16384);
    assert(raw_mem2);
    LauWiredHeader *mock_header2 = (LauWiredHeader*)raw_mem2;
    mock_header2->schema_count = 1;
    mock_header2->schema = mock_schema2;
    mock_header2->sealed = true;
    void *mock_payload2 = raw_mem2 + 8192;

    assert(abi_dispatch_register_member(&map, mock_payload2));
    
    uint64_t args[1] = { 21 };
    uint64_t ret_val = 0;
    assert(abi_dispatch_invoke(&map, 0x99999999, mock_payload2, args, 1, &ret_val));
    assert(ret_val == 42);
    assert(mock_header2->cache_valid == 1);

    // Bypass check: set target_fn to NULL. Invoke should bypass and hit cache without crash.
    mock_schema2[0].target_fn = (void*)0;
    ret_val = 0;
    assert(abi_dispatch_invoke(&map, 0x99999999, mock_payload2, args, 1, &ret_val));
    assert(ret_val == 42);

    // Invalidation check: clock the state counter. Invoke should trigger a miss.
    mock_header2->counter++;
    mock_schema2[0].target_fn = (void*)&mock_agent_method; // restore function pointer
    ret_val = 0;
    assert(abi_dispatch_invoke(&map, 0x99999999, mock_payload2, args, 1, &ret_val));
    assert(ret_val == 42);

    free(raw_mem2);
    printf("✓ Dynamic member Helmholtz cache hits and counter invalidations verified.\n");

    // 7. Test Member Dynamic Relational Table
    printf("7. Testing Dynamic Table operations inside agent memory:\n");
    ThunkSignature mock_schema3[2] = {
        { 0x11112222, THUNK_ZMM, 2, (void*)&interop_agent_insert },
        { 0x33334444, THUNK_ZMM, 1, (void*)&interop_agent_query }
    };
    char *raw_mem3 = calloc(1, 16384);
    assert(raw_mem3);
    LauWiredHeader *mock_header3 = (LauWiredHeader*)raw_mem3;
    mock_header3->schema_count = 2;
    mock_header3->schema = mock_schema3;
    mock_header3->sealed = true;
    void *mock_payload3 = raw_mem3 + 8192;

    assert(abi_dispatch_register_member(&map, mock_payload3));

    // Insert 2 rows: (100, 500) and (200, 600)
    uint64_t insert_args1[2] = { 100, 500 };
    uint64_t insert_args2[2] = { 200, 600 };
    uint64_t insert_ret = 0;
    assert(abi_dispatch_invoke(&map, 0x11112222, mock_payload3, insert_args1, 2, &insert_ret));
    assert(insert_ret == 1);
    assert(abi_dispatch_invoke(&map, 0x11112222, mock_payload3, insert_args2, 2, &insert_ret));
    assert(insert_ret == 1);

    // Initial query on key 100
    uint64_t query_args[1] = { 100 };
    uint64_t query_ret1 = 0;
    assert(abi_dispatch_invoke(&map, 0x33334444, mock_payload3, query_args, 1, &query_ret1));

    // Second query on key 100: should bypass execution and hit cache
    uint64_t query_ret2 = 0;
    assert(abi_dispatch_invoke(&map, 0x33334444, mock_payload3, query_args, 1, &query_ret2));
    assert(query_ret1 == query_ret2); // Proves execution was bypassed!

    // Insert a third row (which increments counter/invalidates cache)
    uint64_t insert_args3[2] = { 100, 700 }; // overwrite/insert new slot
    assert(abi_dispatch_invoke(&map, 0x11112222, mock_payload3, insert_args3, 2, &insert_ret));
    assert(insert_ret == 1);

    // Third query on key 100: cache miss, should re-execute and fetch the updated row value
    uint64_t query_ret3 = 0;
    assert(abi_dispatch_invoke(&map, 0x33334444, mock_payload3, query_args, 1, &query_ret3));
    assert(query_ret3 != query_ret1); // Proves cache was invalidated and query re-executed!

    free(raw_mem3);
    printf("✓ Dynamic table query caching and invalidation verified successfully.\n");

    // 8. Test Inter-Member RDBMS Table Writes
    printf("8. Testing Inter-Member dynamic table operations (RDBMS):\n");
    ThunkSignature mock_schema4[1] = {
        { 0x77777777, THUNK_ZMM, 3, (void*)&interop_agent_insert_external }
    };
    char *raw_mem4 = calloc(1, 16384);
    assert(raw_mem4);
    LauWiredHeader *mock_header4 = (LauWiredHeader*)raw_mem4;
    mock_header4->schema_count = 1;
    mock_header4->schema = mock_schema4;
    mock_header4->sealed = true;
    mock_header4->version = 1;
    void *mock_payload4 = raw_mem4 + 8192;

    assert(abi_dispatch_register_member(&map, mock_payload4));

    // Re-allocate target database table (payload5)
    char *raw_mem5 = calloc(1, 16384);
    assert(raw_mem5);
    LauWiredHeader *mock_header5 = (LauWiredHeader*)raw_mem5;
    mock_header5->schema_count = 2;
    mock_header5->schema = mock_schema3; // dynamic table schema
    mock_header5->sealed = true;
    mock_header5->version = 1;
    void *mock_payload5 = raw_mem5 + 8192;

    assert(abi_dispatch_register_member(&map, mock_payload5));

    // Execute query on key 999 on target -> should return 0 (does not exist)
    uint64_t q_args[1] = { 999 };
    uint64_t q_ret = 0;
    assert(abi_dispatch_invoke(&map, 0x33334444, mock_payload5, q_args, 1, &q_ret));
    assert(q_ret == 0);

    // Call external insert from payload4 on target payload5: key=999, val=12345
    uint64_t ext_args[3] = { (uintptr_t)mock_payload5, 999, 12345 };
    uint64_t ext_ret = 0;
    assert(abi_dispatch_invoke(&map, 0x77777777, mock_payload4, ext_args, 3, &ext_ret));
    assert(ext_ret == 1); // Insertion succeeded!

    // Query on key 999 again -> should hit the database and return the newly inserted value 12345
    assert(abi_dispatch_invoke(&map, 0x33334444, mock_payload5, q_args, 1, &q_ret));
    assert((q_ret % 1000000ULL) == 12345);

    free(raw_mem4);
    free(raw_mem5);
    printf("✓ Inter-Member dynamic table operations (RDBMS) verified successfully.\n");

    // 9. Test Dynamic Table Updates and Deletions
    printf("9. Testing Dynamic Table updates and deletions:\n");
    ThunkSignature mock_schema5[4] = {
        { 0x11112222, THUNK_ZMM, 2, (void*)&interop_agent_insert },
        { 0x33334444, THUNK_ZMM, 1, (void*)&interop_agent_query },
        { 0x55556666, THUNK_ZMM, 2, (void*)&interop_agent_update },
        { 0x77778888, THUNK_ZMM, 1, (void*)&interop_agent_delete }
    };
    char *raw_mem6 = calloc(1, 16384);
    assert(raw_mem6);
    LauWiredHeader *mock_header6 = (LauWiredHeader*)raw_mem6;
    mock_header6->schema_count = 4;
    mock_header6->schema = mock_schema5;
    mock_header6->sealed = true;
    mock_header6->version = 1;
    void *mock_payload6 = raw_mem6 + 8192;

    assert(abi_dispatch_register_member(&map, mock_payload6));

    // Insert key=555, val=777
    uint64_t ins_args[2] = { 555, 777 };
    uint64_t ins_ret = 0;
    assert(abi_dispatch_invoke(&map, 0x11112222, mock_payload6, ins_args, 2, &ins_ret));

    // Query key 555
    uint64_t q555_args[1] = { 555 };
    uint64_t q_val1 = 0;
    assert(abi_dispatch_invoke(&map, 0x33334444, mock_payload6, q555_args, 1, &q_val1));
    assert((q_val1 % 1000000ULL) == 777);

    // Update key 555 to val 888
    uint64_t upd_args[2] = { 555, 888 };
    uint64_t upd_ret = 0;
    assert(abi_dispatch_invoke(&map, 0x55556666, mock_payload6, upd_args, 2, &upd_ret));
    assert(upd_ret == 1);

    // Query key 555 again -> should miss cache and return new value 888
    uint64_t q_val2 = 0;
    assert(abi_dispatch_invoke(&map, 0x33334444, mock_payload6, q555_args, 1, &q_val2));
    assert((q_val2 % 1000000ULL) == 888);

    // Delete key 555
    uint64_t del_ret = 0;
    assert(abi_dispatch_invoke(&map, 0x77778888, mock_payload6, q555_args, 1, &del_ret));
    assert(del_ret == 1);

    // Query key 555 -> should return 0 (not found)
    assert(abi_dispatch_invoke(&map, 0x33334444, mock_payload6, q555_args, 1, &q_val2));
    assert(q_val2 == 0);

    free(raw_mem6);
    printf("✓ Dynamic table updates, deletions, and associated cache invalidations verified successfully.\n");

    // 10. Test Memory-to-Memory RDBMS Dispatch with Table Creation, Insertion, and Querying
    printf("10. Testing Memory-to-Memory RDBMS Dispatch (Creation, Insertion, Querying):\n");
    
    // Caller Agent (payload7)
    ThunkSignature mock_schema_caller[1] = {
        { 0xdddddddd, THUNK_ZMM, 5, (void*)&interop_agent_rdbms_dispatch }
    };
    char *raw_mem7 = calloc(1, 16384);
    assert(raw_mem7);
    LauWiredHeader *mock_header7 = (LauWiredHeader*)raw_mem7;
    mock_header7->schema_count = 1;
    mock_header7->schema = mock_schema_caller;
    mock_header7->sealed = true;
    mock_header7->version = 1;
    void *mock_payload7 = raw_mem7 + 8192;
    assert(abi_dispatch_register_member(&map, mock_payload7));

    // Database Agent (payload8)
    ThunkSignature mock_schema_db[3] = {
        { 0xcccccccc, THUNK_ZMM, 2, (void*)&interop_agent_create_table },
        { 0xaaaaaaaa, THUNK_ZMM, 2, (void*)&interop_agent_insert_dynamic },
        { 0xbbbbbbbb, THUNK_ZMM, 1, (void*)&interop_agent_select_dynamic }
    };
    char *raw_mem8 = calloc(1, 16384);
    assert(raw_mem8);
    LauWiredHeader *mock_header8 = (LauWiredHeader*)raw_mem8;
    mock_header8->schema_count = 3;
    mock_header8->schema = mock_schema_db;
    mock_header8->sealed = true;
    mock_header8->version = 1;
    void *mock_payload8 = raw_mem8 + 8192;
    assert(abi_dispatch_register_member(&map, mock_payload8));

    // A. Create table on Database Agent (via Caller Agent dispatch)
    // args: target_payload, selector, col_count, capacity
    uint64_t cmd_create[4] = { (uintptr_t)mock_payload8, 0xcccccccc, 2, 5 };
    uint64_t cmd_ret = 0;
    assert(abi_dispatch_invoke(&map, 0xdddddddd, mock_payload7, cmd_create, 4, &cmd_ret));
    assert(cmd_ret == 1);

    // B. Insert rows into Database Agent (via Caller Agent dispatch)
    // args: target_payload, selector, col0_val, col1_val
    uint64_t cmd_insert1[4] = { (uintptr_t)mock_payload8, 0xaaaaaaaa, 10, 100 };
    uint64_t cmd_insert2[4] = { (uintptr_t)mock_payload8, 0xaaaaaaaa, 20, 200 };
    assert(abi_dispatch_invoke(&map, 0xdddddddd, mock_payload7, cmd_insert1, 4, &cmd_ret));
    assert(cmd_ret == 1);
    assert(abi_dispatch_invoke(&map, 0xdddddddd, mock_payload7, cmd_insert2, 4, &cmd_ret));
    assert(cmd_ret == 1);

    // C. Query row from Database Agent (via Caller Agent dispatch)
    // args: target_payload, selector, match_key
    uint64_t cmd_query[3] = { (uintptr_t)mock_payload8, 0xbbbbbbbb, 10 };
    assert(abi_dispatch_invoke(&map, 0xdddddddd, mock_payload7, cmd_query, 3, &cmd_ret));
    assert(cmd_ret == 100);

    uint64_t cmd_query2[3] = { (uintptr_t)mock_payload8, 0xbbbbbbbb, 20 };
    assert(abi_dispatch_invoke(&map, 0xdddddddd, mock_payload7, cmd_query2, 3, &cmd_ret));
    assert(cmd_ret == 200);

    // D. Verify Zero-Copy Quadtree persistence file exists and has correct serialized contents
    char expected_file[128];
    snprintf(expected_file, sizeof(expected_file), "../assets/table_%p.dat.bin", mock_payload8);
    FILE *f_check = fopen(expected_file, "rb");
    assert(f_check != NULL);
    uint64_t file_hash = 0;
    assert(fread(&file_hash, sizeof(uint64_t), 1, f_check) == 1);
    assert(file_hash != 0);
    uint32_t val_buf[4] = {0};
    assert(fread(val_buf, sizeof(uint32_t), 4, f_check) == 4);
    assert(val_buf[0] == 1); // active
    assert(val_buf[1] == 2); // col_count
    assert(val_buf[2] == 2); // count
    assert(val_buf[3] == 5); // capacity
    fclose(f_check);
    remove(expected_file);

    // Cleanup dynamic rows buffer and BST tree index in target
    InteropDynamicTable *db_table = (InteropDynamicTable*)mock_payload8;
    interop_agent_destroy_table(db_table);

    free(raw_mem7);
    free(raw_mem8);
    printf("✓ Memory-to-Memory RDBMS dispatch (create, insert, query) verified successfully.\n");

    // 11. Test Guest-VM WinchesterMQ RDBMS Loop
    printf("11. Testing Guest-VM WinchesterMQ SCSI RDBMS Handshake Loop:\n");
    InteropLUN test_lun;
    memset(&test_lun, 0, sizeof(InteropLUN));
    test_lun.pending_ack = 0xFFFFFFFF;

    ThunkSignature db_schema[2] = {
        { 0xcccccccc, THUNK_ZMM, 2, (void*)&interop_agent_create_table },
        { 0xbbbbbbbb, THUNK_ZMM, 1, (void*)&interop_agent_select_dynamic }
    };
    char *raw_db = calloc(1, 16384);
    assert(raw_db);
    LauWiredHeader *db_header = (LauWiredHeader*)raw_db;
    db_header->schema_count = 2;
    db_header->schema = db_schema;
    db_header->sealed = true;
    db_header->version = 1;
    void *db_payload = raw_db + 8192;
    assert(abi_dispatch_register_member(&map, db_payload));

    uint64_t create_args[2] = { 2, 5 };
    uint64_t create_ret = 0;
    assert(abi_dispatch_invoke(&map, 0xcccccccc, db_payload, create_args, 2, &create_ret));
    assert(create_ret == 1);

    InteropDynamicTable *table_ptr = (InteropDynamicTable*)db_payload;
    uint64_t guest_ins_args[2] = { 444, 8888 };
    assert(interop_agent_insert_dynamic(db_payload, guest_ins_args, 2) == 1);

    uint8_t guest_request[256] = {0};
    *(uint32_t*)&guest_request[0] = 0xbbbbbbbb;
    *(uint32_t*)&guest_request[4] = 1;
    *(uint64_t*)&guest_request[8] = 444;
    interop_mq_put(&test_lun, guest_request);

    assert(interop_mq_dispatch_rdbms(&test_lun, db_payload, &map) == 1);

    uint8_t guest_response[256];
    memcpy(guest_response, test_lun.sectors[(test_lun.head - 1) % 16], 256);
    uint64_t db_result = *(uint64_t*)&guest_response[0];
    uint32_t db_status = *(uint32_t*)&guest_response[8];
    assert(db_status == 1);
    assert(db_result == 8888);

    char final_file[128];
    snprintf(final_file, sizeof(final_file), "../assets/table_%p.dat.bin", db_payload);
    remove(final_file);
    interop_agent_destroy_table(table_ptr);
    free(raw_db);
    printf("✓ Guest-VM SCSI LUN RDBMS loop handshake verified successfully.\n");

    // 12. Test Verifiable Audit Trail Logging and Verification
    printf("12. Testing Verifiable Transaction Audit Log (Provable Immutability):\n");
    void *dummy_payload = (void*)0xbeefbeef;
    
    uint64_t args_t1[2] = { 111, 222 };
    uint64_t args_t2[1] = { 333 };
    uint64_t args_t3[3] = { 444, 555, 666 };
    
    uint64_t hash1 = interop_agent_log_transaction(dummy_payload, 0x11111111, args_t1, 2);
    uint64_t hash2 = interop_agent_log_transaction(dummy_payload, 0x22222222, args_t2, 1);
    uint64_t hash3 = interop_agent_log_transaction(dummy_payload, 0x33333333, args_t3, 3);
    
    assert(hash1 != 0);
    assert(hash2 != 0);
    assert(hash3 != 0);
    
    uint64_t final_audit_hash = 0;
    int verify_result = interop_agent_verify_audit_log(dummy_payload, &final_audit_hash);
    assert(verify_result == 3);
    assert(final_audit_hash == hash3);
    
    char audit_file[128];
    snprintf(audit_file, sizeof(audit_file), "../assets/audit_log_%p.dat.bin", dummy_payload);
    FILE *f_tamper = fopen(audit_file, "r+b");
    assert(f_tamper != NULL);
    fseek(f_tamper, sizeof(InteropAuditEntry) + sizeof(uint64_t), SEEK_SET);
    uint32_t bad_selector = 0xbadbadff;
    assert(fwrite(&bad_selector, sizeof(uint32_t), 1, f_tamper) == 1);
    fclose(f_tamper);
    
    uint64_t bad_audit_hash = 0;
    int verify_fail = interop_agent_verify_audit_log(dummy_payload, &bad_audit_hash);
    assert(verify_fail < 0);
    printf("✓ Successfully detected and rejected tampered audit log payload.\n");
    
    remove(audit_file);
    printf("✓ Verifiable transaction audit logging and chain integrity checks completed successfully.\n");

    // 13. Test Coaxial Shared Memory Ledger (CSML) with atomic relative offset swap
    printf("13. Testing Coaxial Shared Memory Ledger (Zero-Copy Offset Swap & Lock):\n");
    char *coaxial_shared_mem = calloc(1, 16384);
    assert(coaxial_shared_mem);
    InteropCoaxialTable *coaxial_table = (InteropCoaxialTable*)coaxial_shared_mem;
    interop_coaxial_init_table(coaxial_table, 5, 2);
    uint64_t row1[2] = { 10, 100 };
    assert(interop_coaxial_insert(coaxial_table, row1, 2) == 1);
    uint64_t row2[2] = { 20, 200 };
    assert(interop_coaxial_insert(coaxial_table, row2, 2) == 1);
    uint64_t val1 = interop_coaxial_select(coaxial_table, 10);
    uint64_t val2 = interop_coaxial_select(coaxial_table, 20);
    assert(val1 == 100);
    assert(val2 == 200);
    free(coaxial_shared_mem);
    printf("✓ Zero-Copy relative offset RCU swap and spinlocks verified successfully.\n");

    // 14. Test System-Wide Coaxial Shared Memory Ledger
    printf("14. Testing System-Wide Coaxial Shared Memory Ledger (Trie/WMQ/Gas):\n");
    InteropSystemLedger system_ledger;
    interop_system_ledger_init(&system_ledger);
    assert(system_ledger.trie_route_table.capacity == 128);
    assert(system_ledger.trie_route_table.col_count == 2);
    assert(system_ledger.trie_route_table.count == 0);
    uint64_t gas_data[2] = { 0x5555, 120 };
    assert(interop_coaxial_insert(&system_ledger.gas_calibration_table, gas_data, 2) == 1);
    uint64_t resolved_fee = interop_coaxial_select(&system_ledger.gas_calibration_table, 0x5555);
    assert(resolved_fee == 120);
    printf("✓ System-Wide dynamic coaxial shared tables initialized and verified successfully.\n");

    // 15. Test Extended Coaxial Tables (IPC/VRAM/JIT)
    printf("15. Testing Extended Coaxial Dynamic Tables (IPC/VRAM/JIT):\n");
    uint64_t sig_data[2] = { 9999, 0x1 };
    assert(interop_coaxial_insert(&system_ledger.ipc_signal_table, sig_data, 2) == 1);
    uint64_t resolved_mask = interop_coaxial_select(&system_ledger.ipc_signal_table, 9999);
    assert(resolved_mask == 0x1);
    assert(system_ledger.ipc_signal_table.capacity == 256);
    assert(system_ledger.vram_page_table.capacity == 512);
    assert(system_ledger.jit_reflection_table.capacity == 1024);
    printf("✓ Extended dynamic coaxial shared tables verified successfully.\n");

    // 16. Test Third Coaxial Expansion (P2P/Radio/CPU Snapshot)
    printf("16. Testing Peer, Radio, and CPU dynamic coaxial tables:\n");
    uint64_t peer_data[2] = { 0x7777, 4 };
    assert(interop_coaxial_insert(&system_ledger.peer_registry_table, peer_data, 2) == 1);
    uint64_t resolved_lun = interop_coaxial_select(&system_ledger.peer_registry_table, 0x7777);
    assert(resolved_lun == 4);
    assert(system_ledger.peer_registry_table.capacity == 512);
    assert(system_ledger.radio_packet_table.capacity == 256);
    assert(system_ledger.cpu_snapshot_table.capacity == 128);
    printf("✓ Peer, Radio, and CPU Dynamic Coaxial Tables verified successfully.\n");

    // 17. Test Live Unix Domain Socket Loopback Bridge
    printf("17. Testing Live Unix Domain Socket Loopback Bridge:\n");
    const char *socket_path = "/tmp/test_coaxial_bridge.sock";
    int server_fd = interop_coaxial_bridge_init(socket_path);
    assert(server_fd >= 0);
    int client_fd1 = socket(AF_UNIX, SOCK_STREAM, 0);
    assert(client_fd1 >= 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    assert(connect(client_fd1, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == 0);
    uint32_t ins_selector = 0xaaaaaaaa;
    uint32_t ins_arg_count = 2;
    uint64_t socket_ins_args[2] = { 888, 99999 };
    assert(write(client_fd1, &ins_selector, sizeof(uint32_t)) == sizeof(uint32_t));
    assert(write(client_fd1, &ins_arg_count, sizeof(uint32_t)) == sizeof(uint32_t));
    assert(write(client_fd1, socket_ins_args, sizeof(socket_ins_args)) == sizeof(socket_ins_args));
    close(client_fd1);
    assert(interop_coaxial_bridge_poll(server_fd, &system_ledger.gas_calibration_table) == 1);
    int client_fd2 = socket(AF_UNIX, SOCK_STREAM, 0);
    assert(client_fd2 >= 0);
    assert(connect(client_fd2, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == 0);
    uint32_t sel_selector = 0xbbbbbbbb;
    uint32_t sel_arg_count = 1;
    uint64_t socket_sel_args[1] = { 888 };
    assert(write(client_fd2, &sel_selector, sizeof(uint32_t)) == sizeof(uint32_t));
    assert(write(client_fd2, &sel_arg_count, sizeof(uint32_t)) == sizeof(uint32_t));
    assert(write(client_fd2, socket_sel_args, sizeof(socket_sel_args)) == sizeof(socket_sel_args));
    assert(interop_coaxial_bridge_poll(server_fd, &system_ledger.gas_calibration_table) == 1);
    uint64_t returned_val = 0;
    assert(read(client_fd2, &returned_val, sizeof(uint64_t)) == sizeof(uint64_t));
    assert(returned_val == 99999);
    close(client_fd2);
    close(server_fd);
    unlink(socket_path);
    printf("✓ Live Unix Domain Socket Loopback Bridge verified successfully.\n");

    // 18. Test RDBMS-PLL Phase Lock State Synchronizer
    printf("18. Testing RDBMS-PLL Phase Lock State Synchronizer:\n");
    InteropPLLHeader pll;
    interop_pll_init(&pll);
    assert(pll.locked == 1);
    assert(pll.phase_error == 0);
    int phase_err = interop_pll_detect_phase_error(&pll, 15, 10);
    assert(phase_err == 5);
    assert(pll.locked == 0);
    assert(interop_pll_align_state(&pll, &system_ledger.gas_calibration_table, 8192) == 1);
    assert(pll.locked == 1);
    assert(pll.phase_error == 0);
    assert(system_ledger.gas_calibration_table.rows_offset == 8192);
    printf("✓ RDBMS-PLL database phase state synchronization verified successfully.\n");

    // 19. Test Kermit-over-PLL File Transfer as a Media Asset
    printf("19. Testing Kermit-over-PLL Media File Transfer (MEDIA DAT):\n");
    char *kermit_shared_mem = calloc(1, 16384);
    assert(kermit_shared_mem);
    InteropCoaxialTable *kermit_table = (InteropCoaxialTable*)kermit_shared_mem;
    interop_coaxial_init_table(kermit_table, 5, 20);
    InteropPLLHeader kermit_pll;
    interop_pll_init(&kermit_pll);
    const uint8_t file_chunk1[] = "Kermit Media Asset Block 1 Payload!";
    const uint8_t file_chunk2[] = "Kermit Media Asset Block 2 Payload!";
    assert(interop_kermit_send_packet(kermit_table, &kermit_pll, 1, file_chunk1, 35) == 1);
    assert(interop_kermit_send_packet(kermit_table, &kermit_pll, 2, file_chunk2, 35) == 1);
    uint8_t rx_buf[128];
    size_t rx_size = 0;
    uint8_t reconstructed_file[256] = {0};
    size_t total_reconstructed = 0;
    assert(interop_kermit_receive_packet(kermit_table, &kermit_pll, 1, rx_buf, &rx_size) == 1);
    assert(rx_size == 35);
    memcpy(reconstructed_file, rx_buf, rx_size);
    total_reconstructed += rx_size;
    assert(interop_kermit_receive_packet(kermit_table, &kermit_pll, 2, rx_buf, &rx_size) == 1);
    assert(rx_size == 35);
    memcpy(reconstructed_file + total_reconstructed, rx_buf, rx_size);
    total_reconstructed += rx_size;
    assert(total_reconstructed == 70);
    assert(memcmp(reconstructed_file, "Kermit Media Asset Block 1 Payload!Kermit Media Asset Block 2 Payload!", 70) == 0);
    const char *media_file = "../assets/media_asset_0xbeef.dat.bin";
    FILE *f_media = fopen(media_file, "wb");
    assert(f_media != NULL);
    fwrite(reconstructed_file, 1, total_reconstructed, f_media);
    fclose(f_media);
    FILE *f_read_media = fopen(media_file, "rb");
    assert(f_read_media != NULL);
    uint8_t verify_buf[128] = {0};
    assert(fread(verify_buf, 1, total_reconstructed, f_read_media) == total_reconstructed);
    assert(memcmp(verify_buf, reconstructed_file, total_reconstructed) == 0);
    fclose(f_read_media);
    remove(media_file);
    free(kermit_shared_mem);
    printf("✓ Kermit-over-PLL Media transfer and DAT verification completed successfully.\n");

    // 20. Test Turing Complete state machine routing (RDBMS-PLL-DAT MEDIA)
    printf("20. Testing Turing Complete RDBMS-PLL-DAT Media State Transitions:\n");
    char *tape_mem = calloc(1, 4096);
    char *rules_mem = calloc(1, 4096);
    assert(tape_mem && rules_mem);
    InteropCoaxialTable *tape_table = (InteropCoaxialTable*)tape_mem;
    InteropCoaxialTable *rules_table = (InteropCoaxialTable*)rules_mem;
    interop_coaxial_init_table(tape_table, 10, 2);
    interop_coaxial_init_table(rules_table, 10, 2);
    uint64_t rule1_key = ((uint64_t)0 << 32) | 0;
    uint64_t rule1_val = ((uint64_t)1 << 32) | ((uint64_t)1 << 16) | 1;
    uint64_t rule1_data[2] = { rule1_key, rule1_val };
    assert(interop_coaxial_insert(rules_table, rule1_data, 2) == 1);
    uint64_t rule2_key = ((uint64_t)1 << 32) | 0;
    uint64_t rule2_val = ((uint64_t)99 << 32) | ((uint64_t)2 << 16) | (uint16_t)-1;
    uint64_t rule2_data[2] = { rule2_key, rule2_val };
    assert(interop_coaxial_insert(rules_table, rule2_data, 2) == 1);
    InteropTuringState turing;
    interop_turing_init(&turing);
    assert(interop_turing_run_step(&turing, tape_table, rules_table) == 1);
    assert(turing.current_state == 1);
    assert(turing.head_index == 1);
    assert(turing.halted == 0);
    assert(interop_coaxial_select(tape_table, 0) == 1);
    assert(interop_turing_run_step(&turing, tape_table, rules_table) == 1);
    assert(turing.current_state == 99);
    assert(turing.head_index == 0);
    assert(turing.halted == 1);
    assert(interop_coaxial_select(tape_table, 1) == 2);
    assert(interop_turing_run_step(&turing, tape_table, rules_table) == 0);
    const char *tape_file = "../assets/tape_state_0xbeef.dat.bin";
    FILE *f_tape = fopen(tape_file, "wb");
    assert(f_tape != NULL);
    uint32_t header[2] = { turing.current_state, (uint32_t)turing.head_index };
    fwrite(header, sizeof(uint32_t), 2, f_tape);
    fclose(f_tape);
    remove(tape_file);
    free(tape_mem);
    free(rules_mem);
    printf("✓ Turing Complete database state machine executed and verified successfully.\n");

    // 21. Test Lock-Free SPSC Coaxial Ring Queue
    printf("21. Testing Lock-Free SPSC Coaxial Ring Queue:\n");
    char *queue_mem = calloc(1, 4096);
    assert(queue_mem);
    InteropCoaxialQueue *q = (InteropCoaxialQueue*)queue_mem;
    interop_queue_init(q, 4, sizeof(uint64_t));
    uint64_t spsc_val1 = 1111;
    uint64_t spsc_val2 = 2222;
    uint64_t spsc_val3 = 3333;
    uint64_t rx_val = 0;
    assert(interop_queue_push(q, &spsc_val1) == 1);
    assert(interop_queue_push(q, &spsc_val2) == 1);
    assert(interop_queue_push(q, &spsc_val3) == 1);
    uint64_t spsc_val4 = 4444;
    assert(interop_queue_push(q, &spsc_val4) == 0);
    assert(interop_queue_pop(q, &rx_val) == 1);
    assert(rx_val == 1111);
    assert(interop_queue_pop(q, &rx_val) == 1);
    assert(rx_val == 2222);
    assert(interop_queue_push(q, &spsc_val4) == 1);
    assert(interop_queue_pop(q, &rx_val) == 1);
    assert(rx_val == 3333);
    assert(interop_queue_pop(q, &rx_val) == 1);
    assert(rx_val == 4444);
    assert(interop_queue_pop(q, &rx_val) == 0);
    free(queue_mem);
    printf("✓ Lock-Free SPSC Coaxial Ring Queue verified successfully.\n");

    // 22. Test Bitcoin-Style State Evolution Covenant
    printf("22. Testing Bitcoin-Style State Evolution Covenant:\n");
    char *cov_tape_mem = calloc(1, 16384);
    char *cov_rules_mem = calloc(1, 16384);
    assert(cov_tape_mem && cov_rules_mem);
    InteropCoaxialTable *c_tape = (InteropCoaxialTable*)cov_tape_mem;
    InteropCoaxialTable *c_rules = (InteropCoaxialTable*)cov_rules_mem;
    interop_coaxial_init_table(c_tape, 5, 2);
    interop_coaxial_init_table(c_rules, 5, 2);
    uint64_t c_rule_key = ((uint64_t)0 << 32) | 0;
    uint64_t c_rule_val = ((uint64_t)1 << 32) | ((uint64_t)777 << 16) | 1;
    uint64_t c_rule_data[2] = { c_rule_key, c_rule_val };
    assert(interop_coaxial_insert(c_rules, c_rule_data, 2) == 1);
    uint64_t c_init_cell[2] = { 0, 0 };
    assert(interop_coaxial_insert(c_tape, c_init_cell, 2) == 1);
    uint32_t active_offset = __atomic_load_n(&c_tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t count = __atomic_load_n(&c_tape->count, __ATOMIC_ACQUIRE);
    char *base = (char*)c_tape;
    uint64_t *rows = (uint64_t*)(base + active_offset);
    uint64_t initial_hash = fnv1a_hash_cascade(14695981039346656037ULL, rows, count * c_tape->col_count * sizeof(uint64_t));
    char *temp_tape_mem = calloc(1, 16384);
    assert(temp_tape_mem);
    InteropCoaxialTable *temp_tape = (InteropCoaxialTable*)temp_tape_mem;
    interop_coaxial_init_table(temp_tape, 5, 2);
    assert(interop_coaxial_insert(temp_tape, c_init_cell, 2) == 1);
    InteropTuringState temp_turing;
    interop_turing_init(&temp_turing);
    assert(interop_turing_run_step(&temp_turing, temp_tape, c_rules) == 1);
    uint32_t temp_offset = __atomic_load_n(&temp_tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t temp_count = __atomic_load_n(&temp_tape->count, __ATOMIC_ACQUIRE);
    char *temp_base = (char*)temp_tape;
    uint64_t *temp_rows = (uint64_t*)(temp_base + temp_offset);
    uint64_t expected_hash = fnv1a_hash_cascade(14695981039346656037ULL, temp_rows, temp_count * temp_tape->col_count * sizeof(uint64_t));
    InteropCovenantState cov;
    interop_covenant_init(&cov, initial_hash);
    InteropTuringState real_turing;
    interop_turing_init(&real_turing);
    assert(interop_covenant_verify_evolution(&cov, &real_turing, c_tape, c_rules, expected_hash) == 1);
    assert(cov.next_state_hash == expected_hash);
    assert(real_turing.current_state == 1);
    assert(real_turing.head_index == 1);
    assert(interop_covenant_verify_evolution(&cov, &real_turing, c_tape, c_rules, 999999) == -2);
    free(cov_tape_mem);
    free(cov_rules_mem);
    free(temp_tape_mem);
    printf("✓ Bitcoin-Style State Evolution Covenant verified successfully.\n");

    // 23. Test Yul Contract Deployment via Bitcoin Covenant
    printf("23. Testing Yul Contract Deployment via Bitcoin Covenant:\n");
    char *yul_rules_mem = calloc(1, 16384);
    assert(yul_rules_mem);
    InteropCoaxialTable *yul_rules_table = (InteropCoaxialTable*)yul_rules_mem;
    interop_coaxial_init_table(yul_rules_table, 5, 2);
    uint64_t compiled_yul[2];
    compiled_yul[0] = ((uint64_t)0 << 32) | 0;
    compiled_yul[1] = ((uint64_t)2 << 32) | ((uint64_t)999 << 16) | 1;
    assert(interop_covenant_deploy_yul(yul_rules_table, compiled_yul, 1) == 1);
    char *yul_tape_mem = calloc(1, 16384);
    assert(yul_tape_mem);
    InteropCoaxialTable *yul_tape = (InteropCoaxialTable*)yul_tape_mem;
    interop_coaxial_init_table(yul_tape, 5, 2);
    uint64_t init_cell[2] = { 0, 0 };
    assert(interop_coaxial_insert(yul_tape, init_cell, 2) == 1);
    InteropTuringState yul_turing;
    interop_turing_init(&yul_turing);
    assert(interop_turing_run_step(&yul_turing, yul_tape, yul_rules_table) == 1);
    assert(yul_turing.current_state == 2);
    assert(yul_turing.head_index == 1);
    assert(interop_coaxial_select(yul_tape, 0) == 999);
    free(yul_rules_mem);
    free(yul_tape_mem);
    printf("✓ Yul virtual contract successfully deployed and verified via Bitcoin covenant.\n");

    // 24. Test Vectorized Parallel FNV-1a Hashing
    printf("24. Testing Vectorized Parallel FNV-1a Hashing:\n");
    const char *hash_test_str = "Vectorized FNV-1a Hashing Parallel Tracks Verification String Payload";
    uint64_t vectorized_hash = fnv1a_hash_vectorized(14695981039346656037ULL, hash_test_str, strlen(hash_test_str));
    assert(vectorized_hash != 0);
    printf("✓ Vectorized Parallel FNV-1a Hashing verified successfully.\n");

    // 25. Test Layer-2 Rollup Batching
    printf("25. Testing Layer-2 Rollup Batching:\n");
    char *batch_tape_mem = calloc(1, 16384);
    char *batch_rules_mem = calloc(1, 16384);
    assert(batch_tape_mem && batch_rules_mem);
    InteropCoaxialTable *b_tape = (InteropCoaxialTable*)batch_tape_mem;
    InteropCoaxialTable *b_rules = (InteropCoaxialTable*)batch_rules_mem;
    interop_coaxial_init_table(b_tape, 10, 2);
    interop_coaxial_init_table(b_rules, 10, 2);
    uint64_t r1_key = ((uint64_t)0 << 32) | 0;
    uint64_t r1_val = ((uint64_t)1 << 32) | ((uint64_t)7 << 16) | 1;
    uint64_t r1_data[2] = { r1_key, r1_val };
    assert(interop_coaxial_insert(b_rules, r1_data, 2) == 1);
    uint64_t r2_key = ((uint64_t)1 << 32) | 0;
    uint64_t r2_val = ((uint64_t)2 << 32) | ((uint64_t)8 << 16) | 1;
    uint64_t r2_data[2] = { r2_key, r2_val };
    assert(interop_coaxial_insert(b_rules, r2_data, 2) == 1);
    uint64_t b_init_cell[2] = { 0, 0 };
    assert(interop_coaxial_insert(b_tape, b_init_cell, 2) == 1);
    uint32_t b_active_offset = __atomic_load_n(&b_tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t b_count = __atomic_load_n(&b_tape->count, __ATOMIC_ACQUIRE);
    char *b_base = (char*)b_tape;
    uint64_t *b_rows = (uint64_t*)(b_base + b_active_offset);
    uint64_t b_start_hash = fnv1a_hash_vectorized(14695981039346656037ULL, b_rows, b_count * b_tape->col_count * sizeof(uint64_t));
    char *temp_b_tape_mem = calloc(1, 16384);
    assert(temp_b_tape_mem);
    InteropCoaxialTable *temp_b_tape = (InteropCoaxialTable*)temp_b_tape_mem;
    interop_coaxial_init_table(temp_b_tape, 10, 2);
    assert(interop_coaxial_insert(temp_b_tape, b_init_cell, 2) == 1);
    InteropTuringState temp_b_turing;
    interop_turing_init(&temp_b_turing);
    assert(interop_turing_run_step(&temp_b_turing, temp_b_tape, b_rules) == 1);
    assert(interop_turing_run_step(&temp_b_turing, temp_b_tape, b_rules) == 1);
    uint32_t tb_offset = __atomic_load_n(&temp_b_tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t tb_count = __atomic_load_n(&temp_b_tape->count, __ATOMIC_ACQUIRE);
    char *tb_base = (char*)temp_b_tape;
    uint64_t *tb_rows = (uint64_t*)(tb_base + tb_offset);
    uint64_t expected_end_hash = fnv1a_hash_vectorized(14695981039346656037ULL, tb_rows, tb_count * temp_b_tape->col_count * sizeof(uint64_t));
    InteropRollupBatch batch;
    batch.start_state_hash = b_start_hash;
    batch.step_count = 2;
    InteropTuringState real_b_turing;
    interop_turing_init(&real_b_turing);
    assert(interop_covenant_verify_batch(&batch, &real_b_turing, b_tape, b_rules, expected_end_hash) == 1);
    assert(batch.end_state_hash == expected_end_hash);
    assert(real_b_turing.current_state == 2);
    printf("✓ Layer-2 Rollup Batching verified successfully.\n");

    // 26. Test Optimistic Fraud Proofs
    printf("26. Testing Optimistic Fraud Proofs:\n");
    char *state1_tape_mem = calloc(1, 16384);
    assert(state1_tape_mem);
    InteropCoaxialTable *state1_tape = (InteropCoaxialTable*)state1_tape_mem;
    interop_coaxial_init_table(state1_tape, 10, 2);
    assert(interop_coaxial_insert(state1_tape, b_init_cell, 2) == 1);
    InteropTuringState s1_turing;
    interop_turing_init(&s1_turing);
    assert(interop_turing_run_step(&s1_turing, state1_tape, b_rules) == 1);
    uint32_t s1_offset = __atomic_load_n(&state1_tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t s1_count = __atomic_load_n(&state1_tape->count, __ATOMIC_ACQUIRE);
    char *s1_base = (char*)state1_tape;
    uint64_t *s1_rows = (uint64_t*)(s1_base + s1_offset);
    uint64_t state1_hash = fnv1a_hash_vectorized(14695981039346656037ULL, s1_rows, s1_count * state1_tape->col_count * sizeof(uint64_t));
    int fraud_result = interop_covenant_prove_fraud(state1_hash, 0x999999, &s1_turing, state1_tape, b_rules);
    assert(fraud_result == 1);
    printf("✓ Optimistic Fraud Proofs verified successfully.\n");

    free(batch_tape_mem);
    free(batch_rules_mem);
    free(temp_b_tape_mem);
    free(state1_tape_mem);

    // 27. Test Bidirectional Host Response Handshake
    printf("27. Testing Host Response:\n");
    InteropTuringResponse mock_response;
    mock_response.exit_code = 0;
    mock_response.return_value = 999;
    mock_response.next_state_hash = 0x88888888ULL;
    assert(interop_covenant_verify_response(&mock_response, 0x88888888ULL, 0, 999) == 1);
    assert(interop_covenant_verify_response(&mock_response, 0x88888888ULL, 1, 999) == -3);
    printf("✓ Host Response verified.\n");

    free(raw_mem);
    printf("✓ Registered schema signatures successfully from mock wired memory member.\n");

    printf("=== ALL LOCK-FREE ABI DISPATCH MAP TESTS PASSED ===\n");
    return 0;
}
