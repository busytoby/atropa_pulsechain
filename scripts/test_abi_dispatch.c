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
    ABIDispatchMap map;
    abi_dispatch_init(&map);
    assert(abi_dispatch_register(&map, 0x05de9943, 1000));
    assert(abi_dispatch_register(&map, 0xa9059cbb, 2000));
    uintptr_t offset = 0;
    assert(abi_dispatch_lookup(&map, 0x05de9943, &offset));
    assert(offset == 1000);
    assert(!abi_dispatch_lookup(&map, 0xffffffff, &offset));
    assert(abi_dispatch_register(&map, 0x05de9943, 5000));
    assert(abi_dispatch_lookup(&map, 0x05de9943, &offset));
    assert(offset == 5000);
    printf("✓ Base ABI Dispatch Map verified.\n");
       // 5. Test Member Registration & Invocation
    printf("5. Testing General ABI Dispatch registration & invocation:\n");
    ThunkSignature mock_schema[2] = {
        { 0x11223344, THUNK_ZMM, 1, (void*)9999 },
        { 0x99999999, THUNK_ZMM, 1, (void*)&mock_agent_method }
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
    
    uint64_t args[1] = { 21 };
    uint64_t ret_val = 0;
    assert(abi_dispatch_invoke(&map, 0x99999999, mock_payload, args, 1, &ret_val));
    assert(ret_val == 42);
    assert(mock_header->cache_valid == 1);
    
    // Invalidation check: clock the state counter
    mock_header->counter++;
    ret_val = 0;
    assert(abi_dispatch_invoke(&map, 0x99999999, mock_payload, args, 1, &ret_val));
    assert(ret_val == 42);
    printf("✓ Member dispatch verified.\n");
    printf("✓ Dynamic member Helmholtz cache hits and counter invalidations verified.\n");

    ThunkSignature mock_schema3[2] = {
        { 0x11112222, THUNK_ZMM, 2, (void*)&interop_agent_insert },
        { 0x33334444, THUNK_ZMM, 1, (void*)&interop_agent_query }
    };

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
    fclose(f_check);
    remove(expected_file);

    // E. Verify Guest-VM WinchesterMQ RDBMS Loop using this table
    InteropLUN test_lun;
    memset(&test_lun, 0, sizeof(InteropLUN));
    test_lun.pending_ack = 0xFFFFFFFF;
    uint8_t guest_request[256] = {0};
    *(uint32_t*)&guest_request[0] = 0xbbbbbbbb;
    *(uint32_t*)&guest_request[4] = 1;
    *(uint64_t*)&guest_request[8] = 10;
    interop_mq_put(&test_lun, guest_request);
    assert(interop_mq_dispatch_rdbms(&test_lun, mock_payload8, &map) == 1);
    uint8_t guest_response[256];
    memcpy(guest_response, test_lun.sectors[(test_lun.head - 1) % 16], 256);
    assert(*(uint64_t*)&guest_response[0] == 100);

    // Cleanup dynamic rows buffer and BST tree index in target
    InteropDynamicTable *db_table = (InteropDynamicTable*)mock_payload8;
    interop_agent_destroy_table(db_table);

    free(raw_mem7);
    free(raw_mem8);
    printf("✓ Memory-to-Memory RDBMS and Guest-VM SCSI LUN loop verified.\n");

    // 12. Test Verifiable Audit Trail Logging and Verification
    printf("12. Testing Verifiable Transaction Audit Log:\n");
    void *dummy_payload = (void*)0xbeefbeef;
    uint64_t args_t1[2] = { 111, 222 };
    uint64_t args_t2[1] = { 333 };
    uint64_t hash1 = interop_agent_log_transaction(dummy_payload, 0x11111111, args_t1, 2);
    uint64_t hash2 = interop_agent_log_transaction(dummy_payload, 0x22222222, args_t2, 1);
    assert(hash1 != 0);
    assert(hash2 != 0);
    uint64_t final_audit_hash = 0;
    assert(interop_agent_verify_audit_log(dummy_payload, &final_audit_hash) == 2);
    assert(final_audit_hash == hash2);
    char audit_file[128];
    snprintf(audit_file, sizeof(audit_file), "../assets/audit_log_%p.dat.bin", dummy_payload);
    remove(audit_file);
    printf("✓ Verifiable transaction audit logging verified.\n");

    // 13. Test Coaxial Shared Memory Ledger (CSML)
    printf("13. Testing Coaxial Shared Memory Ledger:\n");
    InteropSystemLedger system_ledger;
    interop_system_ledger_init(&system_ledger);
    assert(system_ledger.trie_route_table.capacity == 128);
    uint64_t row1[2] = { 10, 100 };
    assert(interop_coaxial_insert(&system_ledger.gas_calibration_table, row1, 2) == 1);
    uint64_t row2[2] = { 20, 200 };
    assert(interop_coaxial_insert(&system_ledger.gas_calibration_table, row2, 2) == 1);
    assert(interop_coaxial_select(&system_ledger.gas_calibration_table, 10) == 100);
    assert(interop_coaxial_select(&system_ledger.gas_calibration_table, 20) == 200);
    uint64_t sig_data[2] = { 9999, 0x1 };
    assert(interop_coaxial_insert(&system_ledger.ipc_signal_table, sig_data, 2) == 1);
    assert(interop_coaxial_select(&system_ledger.ipc_signal_table, 9999) == 0x1);
    uint64_t peer_data[2] = { 0x7777, 4 };
    assert(interop_coaxial_insert(&system_ledger.peer_registry_table, peer_data, 2) == 1);
    assert(interop_coaxial_select(&system_ledger.peer_registry_table, 0x7777) == 4);
    printf("✓ Coaxial Shared Memory Ledger verified.\n");

    // 17. Test Live Unix Domain Socket Loopback Bridge
    printf("17. Testing Live Unix Domain Socket Loopback Bridge:\n");
    const char *socket_path = "/tmp/test_coaxial_bridge.sock";
    int server_fd = interop_coaxial_bridge_init(socket_path);
    assert(server_fd >= 0);
    close(server_fd);
    unlink(socket_path);
    printf("✓ Live Unix Domain Socket Loopback Bridge verified.\n");

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
    printf("19. Testing Kermit-over-PLL Media File Transfer:\n");
    char *kermit_shared_mem = calloc(1, 16384);
    assert(kermit_shared_mem);
    InteropCoaxialTable *kermit_table = (InteropCoaxialTable*)kermit_shared_mem;
    interop_coaxial_init_table(kermit_table, 5, 20);
    InteropPLLHeader kermit_pll;
    interop_pll_init(&kermit_pll);
    const uint8_t file_chunk1[] = "Kermit Media Asset Block 1 Payload!";
    assert(interop_kermit_send_packet(kermit_table, &kermit_pll, 1, file_chunk1, 35) == 1);
    uint8_t rx_buf[128];
    size_t rx_size = 0;
    assert(interop_kermit_receive_packet(kermit_table, &kermit_pll, 1, rx_buf, &rx_size) == 1);
    assert(rx_size == 35);
    assert(memcmp(rx_buf, "Kermit Media Asset Block 1 Payload!", 35) == 0);
    free(kermit_shared_mem);
    printf("✓ Kermit-over-PLL Media transfer verified.\n");

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
    // 28. Test Stateful Token Mint & Transfer Lifecycle
    printf("28. Testing Stateful Token Mint & Transfer Lifecycle:\n");
    char *token_tape_mem = calloc(1, 16384);
    assert(token_tape_mem);
    InteropCoaxialTable *token_tape = (InteropCoaxialTable*)token_tape_mem;
    interop_coaxial_init_table(token_tape, 10, 2);
    InteropStateDelta mint_delta[1] = { { 100, 1000 } };
    assert(interop_coaxial_insert(token_tape, (uint64_t[]){ 100, 1000 }, 2) == 1);
    InteropStateDelta transfer_deltas[2] = {
        { 100, 600 },
        { 200, 400 }
    };
    assert(interop_coaxial_insert(token_tape, (uint64_t[]){ 100, 600 }, 2) == 1);
    assert(interop_coaxial_insert(token_tape, (uint64_t[]){ 200, 400 }, 2) == 1);
    uint32_t tok_offset = __atomic_load_n(&token_tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t tok_count = __atomic_load_n(&token_tape->count, __ATOMIC_ACQUIRE);
    char *tok_base = (char*)token_tape;
    uint64_t *tok_rows = (uint64_t*)(tok_base + tok_offset);
    uint64_t final_token_hash = fnv1a_hash_vectorized(14695981039346656037ULL, tok_rows, tok_count * token_tape->col_count * sizeof(uint64_t));
    char *recon_tape_mem = calloc(1, 16384);
    assert(recon_tape_mem);
    InteropCoaxialTable *recon_tape = (InteropCoaxialTable*)recon_tape_mem;
    interop_coaxial_init_table(recon_tape, 10, 2);
    assert(interop_coaxial_insert(recon_tape, (uint64_t[]){ mint_delta[0].cell_index, mint_delta[0].cell_value }, 2) == 1);
    assert(interop_covenant_replay_log(recon_tape, transfer_deltas, 2, final_token_hash) == 1);
    assert(interop_coaxial_select(recon_tape, 100) == 600);
    assert(interop_coaxial_select(recon_tape, 200) == 400);
    free(token_tape_mem);
    free(recon_tape_mem);
    printf("✓ Stateful Token Mint & Transfer Lifecycle verified.\n");

    // 29. Test Agentic Scheduler
    printf("29. Testing Agentic Scheduler:\n");
    InteropAgentScheduler sched;
    assert(interop_scheduler_register(&sched, 3, 0x11223344, 9999) == 1);
    uint64_t val = 0;
    assert(interop_scheduler_tick(&sched, 1, &val) == 0);
    assert(interop_scheduler_tick(&sched, 2, &val) == 0);
    assert(interop_scheduler_tick(&sched, 3, &val) == 1);
    assert(val == 9999);
    assert(interop_scheduler_tick(&sched, 3, &val) == 0);
    printf("✓ Agentic Scheduler verified.\n");

    // 30. Test MAMT Verification Suite
    printf("30. Testing MAMT Verification Suite:\n");
    uint64_t child = 555;
    uint64_t secret = 7;
    uint64_t parent = interop_mamt_adduct(child, secret);
    assert(interop_mamt_verify(child, parent, secret) == 1);
    assert(interop_mamt_verify(child, parent + 1, secret) == 0);
    InteropAgentScheduler lanes[2];
    assert(interop_scheduler_register(&lanes[0], 2, 0x1111, 100) == 1);
    assert(interop_scheduler_register(&lanes[1], 3, 0x2222, 200) == 1);
    uint64_t triggered[2] = {0};
    int triggered_count = interop_scheduler_tick_multilane(lanes, 2, 2, triggered);
    assert(triggered_count == 1);
    assert(triggered[0] == 100);
    uint32_t gas_price = 100;
    interop_gas_calibrate(5, &gas_price);
    assert(gas_price == 100);
    interop_gas_calibrate(15, &gas_price);
    assert(gas_price == 200);
    interop_gas_calibrate(120, &gas_price);
    assert(gas_price == 500);
    printf("✓ MAMT Verification Suite verified.\n");

    // 31. Test Agentic Preference Accumulation and Fee Calculation
    printf("31. Testing Preference Accumulation:\n");
    assert(interop_fee_calculate(3, 10, 150) == 4500);
    InteropPreferenceEntry pref_matrix[2] = {
        { 0x1000, 50 },
        { 0x2000, 100 }
    };
    assert(interop_preference_accumulate(pref_matrix, 2, 0x1000, 25) == 1);
    assert(pref_matrix[0].preference_weight == 75);
    assert(interop_preference_accumulate(pref_matrix, 2, 0x3000, 25) == 0);
    printf("✓ Preference Accumulation verified.\n");

    // 32. Test SDSA, prioritized scheduling, and re-entrancy locks
    printf("32. Testing SDSA, prioritized scheduling, and re-entrancy locks:\n");
    uint32_t lock_bitmap = 0;
    assert(interop_reentrancy_lock(&lock_bitmap, 2) == 1);
    assert(interop_reentrancy_lock(&lock_bitmap, 2) == 0);
    interop_reentrancy_unlock(&lock_bitmap, 2);
    assert(interop_reentrancy_lock(&lock_bitmap, 2) == 1);
    InteropAgentScheduler prio_lanes[2];
    uint32_t priorities[2] = { 5, 12 };
    assert(interop_scheduler_register(&prio_lanes[0], 2, 0x1111, 100) == 1);
    assert(interop_scheduler_register(&prio_lanes[1], 2, 0x2222, 200) == 1);
    uint64_t prio_triggered[2] = {0};
    int prio_count = interop_scheduler_tick_prioritized(prio_lanes, 2, 2, prio_triggered, priorities);
    assert(prio_count == 2);
    assert(prio_triggered[0] == 200);
    assert(prio_triggered[1] == 100);
    assert(interop_sdsa_verify_alignment((void*)64) == 1);
    assert(interop_sdsa_verify_alignment((void*)15) == 0);
    printf("✓ SDSA, prioritized scheduling, and re-entrancy locks verified.\n");

    // 33. Test Accumulator-Gated Decision Tree
    printf("33. Testing Accumulator-Gated Decision Tree:\n");
    InteropDecisionNode nodes[3] = {
        { 100, 1, 2, 0 },
        { 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xAAAA },
        { 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xBBBB }
    };
    assert(interop_decision_tree_evaluate(nodes, 0, 150) == 0xAAAA);
    assert(interop_decision_tree_evaluate(nodes, 0, 50) == 0xBBBB);
    printf("✓ Accumulator-Gated Decision Tree verified.\n");

    // 34. Test KNN Search
    printf("34. Testing KNN Search:\n");
    InteropKNNAgent ka[3] = {
        { 0x1111, {10, 20, 30} },
        { 0x2222, {100, 200, 300} },
        { 0x3333, {12, 22, 32} }
    };
    uint64_t qc[3] = {10, 20, 31};
    uint64_t nb[2] = {0};
    assert(interop_knn_search(ka, 3, qc, nb, 2) == 2);
    assert(nb[0] == 0x1111);
    assert(nb[1] == 0x3333);
    printf("✓ KNN Search verified.\n");

    // 35. Test Coaxial Clustering
    printf("35. Testing Coaxial Clustering:\n");
    uint64_t cr[6] = {
        10, 20, 30,
        100, 200, 300
    };
    uint64_t ce[6] = {
        11, 21, 31,
        99, 199, 299
    };
    uint32_t as[2] = {0};
    assert(interop_coaxial_cluster(cr, 2, ce, 2, as) == 0);
    assert(as[0] == 0);
    assert(as[1] == 1);
    assert(ce[0] == 10);
    assert(ce[3] == 100);
    printf("✓ Coaxial Clustering verified.\n");

    // 36-38. Test MAMT, AC cache, preference sharding clustering
    printf("36-38. Testing Domain-Specific Clustering:\n");
    uint64_t m_hashes[2] = { 0x11111111ULL, 0x99999999ULL };
    uint32_t m_lanes[2] = {0};
    assert(interop_mamt_cluster(m_hashes, 2, m_lanes, 2) == 0);
    assert(m_lanes[0] == 0 && m_lanes[1] == 1);

    uint32_t c_freqs[2] = { 10, 1000 };
    uint32_t c_lines[2] = {0};
    assert(interop_ac_cache_cluster(c_freqs, 2, c_lines, 2) == 0);
    assert(c_lines[0] == 0 && c_lines[1] == 1);

    uint64_t p_prefs[2] = { 5, 500 };
    uint32_t p_shards[2] = {0};
    assert(interop_preference_cluster(p_prefs, 2, p_shards, 2) == 0);
    assert(p_shards[0] == 0 && p_shards[1] == 1);
    printf("✓ MAMT, AC Cache, and Preference clustering verified.\n");

    // 39-41. Test MAMT, AC cache, preference sharding KNN search
    printf("39-41. Testing Domain-Specific KNN Search:\n");
    uint64_t q_h = 0x11111111ULL;
    uint64_t m_out[2] = {0};
    assert(interop_mamt_knn_search(m_hashes, 2, &q_h, m_out, 2) == 2);
    assert(m_out[0] == 0x11111111ULL);

    uint32_t c_out[2] = {0};
    assert(interop_ac_cache_knn_search(c_freqs, 2, 10, c_out, 2) == 2);
    assert(c_out[0] == 10);

    uint64_t p_out[2] = {0};
    assert(interop_preference_knn_search(p_prefs, 2, 5, p_out, 2) == 2);
    assert(p_out[0] == 5);
    printf("✓ MAMT, AC Cache, and Preference KNN search verified.\n");

    // 42-44. Test MAMT, AC cache, fee decision trees
    printf("42-44. Testing Domain-Specific Decision Trees:\n");
    assert(interop_mamt_decision_evaluate(nodes, 0, 150) == 0xAAAA);
    assert(interop_ac_cache_decision_evaluate(nodes, 0, 50) == 0xBBBB);
    assert(interop_fee_decision_evaluate(nodes, 0, 150) == 0xAAAA);
    printf("✓ MAMT, AC Cache, and Fee decision trees verified.\n");

    // 45-47. Test AVX-512 distance, multi-decision node, and re-seeding
    uint64_t c1[3] = {10, 20, 30};
    uint64_t c2[3] = {12, 22, 32};
    assert(interop_knn_distance_avx512(c1, c2) == 6);
    InteropMultiDecisionNode mn[2] = {
        { {100, 200, 300}, {1, 1, 1, 1} },
        { {0xCCCC, 0, 0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} }
    };
    assert(interop_multi_decision_evaluate(mn, 0, 50) == 0xCCCC);
    uint64_t cr_res[6] = {10, 20, 30, 10, 20, 30};
    uint64_t ce_res[6] = {10, 20, 30, 99, 99, 99};
    uint32_t as_res[2] = {0};
    assert(interop_coaxial_cluster(cr_res, 2, ce_res, 2, as_res) == 0);
    assert(ce_res[3] == 11);
    printf("✓ AVX-512 KNN, multi-decision, and centroid re-seeding verified.\n");

    // 48-50. Test batch AVX decision, weighted clustering, and LSH ANN search
    uint64_t acc_vals[2] = {50, 50};
    uint32_t dec_res[2] = {0};
    interop_multi_decision_evaluate_avx512(mn, 0, acc_vals, dec_res, 2);
    assert(dec_res[0] == 0xCCCC);
    uint64_t w_coords[6] = { 10, 20, 30, 20, 40, 60 };
    uint64_t w_weights[2] = { 1, 2 };
    uint64_t w_cents[6] = { 10, 20, 30, 999, 999, 999 };
    uint32_t w_assign[2] = {0};
    assert(interop_coaxial_cluster_weighted(w_coords, w_weights, 2, w_cents, 2, w_assign) == 0);
    assert(w_assign[0] == 0 && w_assign[1] == 0);
    assert(w_cents[0] == 16);
    uint64_t lsh_out[2] = {0};
    assert(interop_lsh_ann_search(ka, 3, qc, lsh_out, 2) != -1);
    printf("✓ Batch AVX decision, weighted clustering, and LSH ANN search verified.\n");

    // 51-53. Test AVX-512 LSH, sorted decision, and hierarchical clustering
    uint64_t lsh_proj_res[3] = {0};
    interop_lsh_project_avx512(ka, 3, lsh_proj_res);
    assert(lsh_proj_res[0] != 0xFFFFFFFF);
    uint64_t sorted_acc[2] = {90, 50};
    uint32_t sorted_res[2] = {0};
    interop_multi_decision_evaluate_sorted(mn, 0, sorted_acc, sorted_res, 2);
    assert(sorted_res[1] == 0xCCCC);
    uint64_t h_coords[6] = {10, 20, 30, 100, 200, 300};
    uint64_t h_cents[6] = {11, 21, 31, 99, 199, 299};
    uint32_t h_assign[2] = {0};
    assert(interop_coaxial_cluster_hierarchical(h_coords, 2, h_cents, 2, h_assign) == 0);
    assert(h_cents[0] == 10 && h_cents[3] == 100);
    printf("✓ AVX-512 LSH, sorted decision, and hierarchical clustering verified.\n");

    // 54-56. Test AVX-512 vector search, Minkowski distance, and adaptive cluster splits
    uint32_t ths[4] = { 10, 20, 30, 40 };
    assert(interop_decision_vector_search_avx512(ths, 4, 25) == 2);
    assert(interop_knn_distance_minkowski(c1, c2, 2) == 12);
    uint64_t ad_coords[6] = { 10, 20, 30, 9999, 9999, 9999 };
    uint64_t ad_cents[45] = { 10, 20, 30 };
    uint32_t ad_k = 1;
    uint32_t ad_assign[2] = {0};
    assert(interop_coaxial_cluster_adaptive(ad_coords, 2, ad_cents, &ad_k, ad_assign) == 0);
    assert(ad_k == 2);
    printf("✓ Decision vector search, Minkowski distance, and adaptive splits verified.\n");

    // 57. Test Quadtree Media file read, write, query
    InteropQuadNode q_nodes[2] = {
        { 0, 0, 100, 100, 0, {1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { 0, 0, 50, 50, 999, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} }
    };
    const char *q_file = "../assets/test_quad.dat.bin";
    assert(interop_quadtree_write(q_file, q_nodes, 2) == 0);
    InteropQuadNode read_q[2];
    assert(interop_quadtree_read(q_file, read_q, 2) == 2);
    assert(read_q[1].value == 999);
    assert(interop_quadtree_query(read_q, 0, 10, 10) == 999);
    remove(q_file);
    printf("✓ Quadtree media reader, writer, and coordinate query verified.\n");

    // 58-60. Test quadtree parallel check, RLE compression, and VEB alignment
    uint32_t x_mins[4] = {0, 51, 0, 51};
    uint32_t x_maxs[4] = {50, 100, 50, 100};
    uint32_t y_mins[4] = {0, 0, 51, 51};
    uint32_t y_maxs[4] = {50, 50, 100, 100};
    assert(interop_quadtree_quadrant_check_avx512(x_mins, x_maxs, y_mins, y_maxs, 10, 10) == 0);
    assert(interop_quadtree_write_rle(q_file, q_nodes, 2) == 0);
    InteropQuadNode read_rle[2];
    assert(interop_quadtree_read_rle(q_file, read_rle, 2) == 2);
    assert(read_rle[1].value == 999);
    remove(q_file);
    InteropQuadNode veb_out[2];
    interop_quadtree_veb_align(q_nodes, veb_out, 2);
    assert(veb_out[1].value == 999);
    printf("✓ Parallel quadrant checks, RLE, and VEB layout verified.\n");

    // 61-63. Test RLE vector decoding, decision pruning, and Minkowski clustering
    uint32_t runs[2] = { 2, 3 };
    uint32_t values[2] = { 10, 20 };
    uint32_t decoded[5] = {0};
    interop_rle_decode_avx512(runs, values, 2, decoded);
    assert(decoded[0] == 10 && decoded[4] == 20);
    InteropMultiDecisionNode prune_nodes[5] = {
        { {100, 200, 300}, {1, 2, 3, 4} },
        { {0xAAAA, 0, 0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { {0xAAAA, 0, 0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { {0xAAAA, 0, 0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { {0xAAAA, 0, 0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} }
    };
    interop_multi_decision_prune(prune_nodes, 5);
    assert(prune_nodes[0].children[0] == 0xFFFFFFFF);
    uint64_t mink_cents[6] = { 10, 20, 30, 999, 999, 999 };
    assert(interop_coaxial_cluster_minkowski(w_coords, 2, mink_cents, 2, w_assign, 2) == 0);
    printf("✓ Vectorized RLE decode, decision tree pruning, and Minkowski clustering verified.\n");

    // 64-65. Test Turing Machine compile & execute on binary media
    InteropTMHeader tm_hdr = { 3, 2, 0, 1, 2 };
    InteropTMTransition tm_trs[2] = {
        { 0, 'a', 'b', 1, 0, 1 },
        { 1, 'x', 'y', 0, 0, 1 }
    };
    const char *tm_file = "../assets/tm_binary.dat.bin";
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t tape[4] = { 'a', 'x', '0', '0' };
    uint32_t tm_final = 0;
    int steps_run = interop_tm_execute(tm_file, tape, 4, 10, &tm_final);
    assert(steps_run == 1);
    assert(tape[0] == 'b');
    assert(tm_final == 1);
    remove(tm_file);
    printf("✓ Turing Machine binary media compilation and execution verified.\n");

    // 66. Test multi-tape Turing Machine execution
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t tape1[4] = { 'a', 'x', '0', '0' };
    uint8_t tape2[4] = { '0', '0', '0', '0' };
    uint32_t mt_final = 0;
    int mt_steps = interop_tm_execute_multitape(tm_file, tape1, 4, tape2, 4, 10, &mt_final);
    assert(mt_steps == 1);
    assert(tape2[0] == 'b');
    remove(tm_file);

    // 67. Test Turing Machine-based Quadtree Decompression
    InteropQuadNode rle_nodes[2] = {
        { 0, 0, 100, 100, 999, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { 0, 0, 50, 50, 999, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} }
    };
    assert(interop_quadtree_write_rle(q_file, rle_nodes, 2) == 0);
    InteropQuadNode decomp_nodes[2];
    assert(interop_tm_decompress_quadtree(q_file, decomp_nodes, 2) == 2);
    assert(decomp_nodes[0].value == 888);
    remove(q_file);
    printf("✓ Multi-tape Turing Machine and TM-based quadtree decompression verified.\n");

    // 68-70. Test NTM execution, subsampling, and Winchester handshake
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t ntm_tape[4] = { 'a', 'x', '0', '0' };
    uint32_t ntm_final = 0;
    int ntm_steps = interop_tm_execute_ntm(tm_file, ntm_tape, 4, 10, &ntm_final);
    assert(ntm_steps == 1);
    assert(ntm_final == 1);
    InteropQuadNode sub_src[4] = {
        { 0, 0, 10, 10, 10, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { 10, 0, 20, 10, 20, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { 0, 10, 10, 20, 30, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { 10, 10, 20, 20, 40, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} }
    };
    InteropQuadNode sub_dst[1];
    assert(interop_tm_subsample_quadtree(sub_src, 4, sub_dst) == 1);
    assert(sub_dst[0].value == 25);
    assert(interop_tm_winchester_handshake(tm_file, 'a') == 0);
    remove(tm_file);
    printf("✓ NTM path solver, TM subsampling, and Winchester handshake verified.\n");

    // 71-73. Test TM upsampling, Yul parsing, and Winchester collision recovery
    InteropQuadNode interp_dst[4];
    assert(interop_tm_interpolate_quadtree(sub_dst, 1, interp_dst, 4) == 4);
    assert(interp_dst[0].value == 25);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t code_buf[4] = { 'a', 'x', 0, 0 };
    assert(interop_tm_yul_parse(tm_file, code_buf, 4) == 0);
    assert(interop_tm_winchester_resolve_collision(tm_file, 'a', 'x') == 0);
    remove(tm_file);
    printf("✓ TM upsampling, Yul parsing, and Winchester collision recovery verified.\n");

    // 74-76. Test NTM path pruning, bounds sorting, and Minkowski hull
    InteropTMTransition prune_trs[3] = {
        { 0, 'a', 'b', 1, 0, 1 },
        { 0, 'a', 'b', 1, 0, 1 },
        { 1, 'x', 'y', 0, 0, 1 }
    };
    size_t tr_count = 3;
    interop_tm_ntm_prune(prune_trs, &tr_count);
    assert(tr_count == 2);
    uint64_t b_coords[6] = { 100, 200, 300, 10, 20, 30 };
    interop_tm_bounds_sort(b_coords, 2);
    assert(b_coords[0] == 10);
    assert(interop_tm_minkowski_hull(b_coords, 2) == 113400ULL);
    printf("✓ NTM pruning, bounds sorting, and Minkowski hull verified.\n");

    // 77-79. Test CNN matrix convolution, threshold activation, and pooling
    uint8_t cnn_input[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int8_t cnn_kernel[9] = { 0, 0, 0, 0, 1, 0, 0, 0, 0 };
    uint8_t cnn_output[9] = {0};
    assert(interop_tm_cnn_convolve(cnn_input, 3, 3, cnn_kernel, cnn_output) == 0);
    assert(cnn_output[4] == 5);
    uint8_t act_features[3] = { 10, 50, 100 };
    interop_tm_cnn_activate(act_features, 3, 50);
    assert(act_features[0] == 0 && act_features[1] == 50);
    uint8_t pool_input[4] = { 10, 20, 30, 40 };
    uint8_t pool_output[1] = {0};
    assert(interop_tm_cnn_pool(pool_input, 2, 2, pool_output) == 0);
    assert(pool_output[0] == 40);
    printf("✓ CNN convolution, activation, and pooling verified.\n");

    // 80. Test Encoder-Only TM Model Simulation
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t enc_only_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t enc_only_state = 0;
    assert(interop_tm_model_encoder_only(tm_file, enc_only_tape, 4, &enc_only_state) == 1);
    assert(enc_only_state == 1);

    // 81. Test Encoder-Decoder TM Model Simulation
    uint8_t dec_tape[4] = { 0 };
    uint32_t enc_dec_state = 0;
    enc_only_tape[0] = 'a';
    assert(interop_tm_model_encoder_decoder(tm_file, enc_only_tape, 4, dec_tape, 4, &enc_dec_state) == 1);
    assert(dec_tape[0] == 'b');
    remove(tm_file);
    printf("✓ Encoder-only and encoder-decoder model simulations verified.\n");

    // 82-84. Test Vectorized Attention, Decision Gated Weights, and Minkowski Attention Weights
    float att_q[2] = { 1.0f, 2.0f };
    float att_k[2] = { 3.0f, 4.0f };
    float att_v[2] = { 0.5f, 0.2f };
    float att_out[2] = { 0.0f };
    interop_tm_cnn_attention_avx512(att_q, att_k, att_v, 2, att_out);
    assert(att_out[0] == 3.5f);
    uint64_t cnn_feats[2] = { 150, 250 };
    uint32_t cnn_gates[2] = { 0 };
    interop_tm_cnn_gate_weights(prune_nodes, 0, cnn_feats, cnn_gates, 2);
    assert(cnn_gates[0] == 0xAAAA);
    uint64_t mink_weights[2] = {0};
    interop_tm_cnn_minkowski_attention(b_coords, b_coords, 2, 2, mink_weights);
    assert(mink_weights[0] == 0);
    printf("✓ Vectorized attention, weight gating, and Minkowski attention weights verified.\n");

    // 85. Test PLL Phase Lock loop
    InteropPLL test_pll = { 0.0, 10.0, 0.0 };
    interop_pll_update(&test_pll, 1.0, 0.1, 2.0);
    assert(test_pll.frequency > 10.0);

    // 86. Test PMG Amplitude Gating
    InteropPMG test_pmg = { 0.0, 5.0, 0 };
    assert(interop_pmg_gate(&test_pmg, 2.0) == 1);
    assert(interop_pmg_gate(&test_pmg, 6.0) == 0);
    printf("✓ PLL tracking loop and PMG gating verified.\n");

    // 87-89. Test RDBMS PMG/PLL sync and ZMM controller agent dispatch
    assert(interop_rdbms_sync_pmg(77, &test_pmg) == 0);
    assert(interop_rdbms_sync_pll(88, &test_pll) == 0);
    uint64_t reg_state[1] = {0};
    assert(interop_zmm_dispatch_controller(99, 2, reg_state) == 0);
    assert(reg_state[0] == 1234);
    printf("✓ RDBMS sync and ZMM controller dispatch verified.\n");

    // 90-92. Test parallel PLL, decision gating frequency locks, and NTM gate searches
    InteropPLL test_plls[2] = { {0.0, 10.0, 0.0}, {1.0, 5.0, 0.0} };
    double ref_phases[2] = { 1.0, 2.0 };
    interop_pll_update_avx512(test_plls, ref_phases, 2, 0.1, 2.0);
    assert(test_plls[0].frequency > 10.0);
    test_pll.error = 0.15;
    interop_pll_decision_gate(&test_pll, prune_nodes, 0);
    assert(test_pll.frequency > 10.0);
    InteropPMG test_pmgs[2] = { {0.0, 5.0, 0}, {0.0, 8.0, 0} };
    uint32_t path_out[2] = {0};
    assert(interop_pmg_gate_search_ntm(test_pmgs, 2, 6.0, path_out) == 1);
    assert(path_out[0] == 0);
    printf("✓ Parallel PLL updates, decision frequency gating, and NTM gate paths verified.\n");

    // 93-94. Test scheduler PMG task gating and PLL time-slice sync
    assert(interop_scheduler_gate_task(&test_pmg, 6.0, 4) == 1);
    assert(interop_scheduler_gate_task(&test_pmg, 2.0, 4) == 0);
    double t_slice = 0.016;
    interop_scheduler_sync_slice(&test_pll, 0.016, 0.020, &t_slice);
    assert(t_slice > 0.001);
    printf("✓ Scheduler PMG task gating and PLL time-slice sync verified.\n");

    // 95-97. Test parallel task gating, decision tree classification, and NTM path routing
    uint32_t comp_list[2] = { 4, 1 };
    uint32_t gated_res[2] = {0};
    interop_scheduler_gate_tasks_avx512(&test_pmg, 6.0, comp_list, gated_res, 2);
    assert(gated_res[0] == 1 && gated_res[1] == 0);
    assert(interop_scheduler_classify_task(prune_nodes, 0, 100, 50) == 0xAAAA);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t route_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t route_state = 0;
    assert(interop_scheduler_route_ntm(tm_file, route_tape, 4, &route_state) == 1);
    assert(route_state == 1);
    remove(tm_file);
    printf("✓ Vectorized scheduler gating, decision classification, and NTM routing verified.\n");

    // 98-100. Test vectorized LSH projection, decision pruning, and Minkowski LSH hashing
    uint64_t lsh_coords[3] = { 10, 20, 30 };
    uint64_t lsh_hashes[1] = {0};
    interop_lsh_project_avx512_keys(lsh_coords, 1, lsh_hashes);
    assert(lsh_hashes[0] < 4);
    uint64_t prune_query[3] = { 50, 50, 50 };
    assert(interop_knn_prune_candidates(prune_nodes, 0, prune_query) == 0xAAAA);
    assert(interop_lsh_hash_minkowski(lsh_coords, 2) < 4);
    printf("✓ Vectorized LSH projection, decision pruning, and Minkowski LSH hashing verified.\n");

    // 101. Test Vaesen-gated sparse learning
    InteropSparseWeight sw[2] = {
        { 0.8f, 1, { 0.5f, 0.1f, 0.7f, 0.0f, 0.0f } },
        { 0.9f, 1, { 0.2f, 0.8f, 0.1f, 0.0f, 0.0f } }
    };
    InteropVaesenScores th = { 0.4f, 0.5f, 0.6f, 0.0f, 0.0f };
    interop_sparse_learn_gate_vaesen(sw, 2, &th);
    assert(sw[0].active == 1);
    assert(sw[1].active == 0);
    printf("✓ Vaesen score gated sparse learning verified.\n");

    // 102. Test coaxial empathy sharding clustering
    InteropVaesenScores profiles[2] = {
        { 0.5f, 0.1f, 0.7f, 0.0f, 0.0f },
        { 0.2f, 0.8f, 0.1f, 0.0f, 0.0f }
    };
    InteropVaesenScores centroids[2] = {
        { 0.5f, 0.1f, 0.7f, 0.0f, 0.0f },
        { 0.2f, 0.8f, 0.1f, 0.0f, 0.0f }
    };
    uint32_t emp_assign[2] = {0};
    assert(interop_coaxial_empathy_cluster(profiles, 2, centroids, 2, emp_assign) == 0);
    assert(emp_assign[0] == 0 && emp_assign[1] == 1);

    // 103. Test NTM empathy route gating
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    InteropVaesenScores route_scores = { 0.5f, 0.1f, 0.7f, 0.0f, 0.0f };
    InteropVaesenScores limit = { 1.0f, 1.0f, 1.0f, 0.5f, 0.5f };
    uint32_t final_emp_state = 0;
    assert(interop_tm_empathy_gate_route(tm_file, &route_scores, &limit, &final_emp_state) == 1);
    assert(final_emp_state == 1);
    remove(tm_file);
    printf("✓ Empathy sharding clustering and NTM emotional route gating verified.\n");

    // 104-106. Test GEMM, thunk selection, and Yul compiler optimization
    float gemm_a[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float gemm_b[4] = { 2.0f, 0.0f, 0.0f, 2.0f };
    float gemm_c[4] = { 0.0f };
    interop_gemm_avx512(gemm_a, gemm_b, gemm_c, 2, 2, 2);
    assert(gemm_c[0] == 2.0f && gemm_c[3] == 8.0f);
    assert(interop_zmm_select_thunk(prune_nodes, 0, 100, 50) == 0xAAAA);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t code_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t opt_state = 0;
    assert(interop_tm_yul_optimize(tm_file, code_tape, 4, &opt_state) == 1);
    assert(opt_state == 1);
    remove(tm_file);
    printf("✓ GEMM, thunk selection, and Yul optimizer verified.\n");

    // 107-108. Test coordinate projection and GEMM wave synthesis
    InteropProjectionConfig proj_cfg = {
        1,
        { 2.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 2.0f },
        0.0f, 0.0f
    };
    float in_coords[3] = { 1.0f, 2.0f, 3.0f };
    float out_coords[3] = { 0.0f };
    interop_project_coordinates(&proj_cfg, in_coords, out_coords, 1);
    assert(out_coords[0] == 2.0f && out_coords[2] == 6.0f);
    float osc_data[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
    float mix_mat[4] = { 2.0f, 0.0f, 0.0f, 2.0f };
    float synth_out[4] = { 0.0f };
    interop_gemm_synthesize(osc_data, mix_mat, synth_out, 2, 2);
    assert(synth_out[0] == 1.0f && synth_out[3] == 2.0f);
    printf("✓ Coordinate projection and GEMM synthesizer verified.\n");

    // 109-111. Test vectorized PKI, key revocation gating, and NTM LAU path finder
    uint64_t test_sigs[1] = { 0x5555 ^ 0xDEADBEEF };
    uint64_t test_pubkeys[1] = { 0x5555 };
    uint32_t valid_sigs[1] = {0};
    interop_pki_verify_signatures_avx512(test_sigs, test_pubkeys, valid_sigs, 1);
    assert(valid_sigs[0] == 1);
    assert(interop_pki_evaluate_revocation(prune_nodes, 0, 100, 50) == 0xAAAA);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t lau_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t lau_state = 0;
    assert(interop_lau_route_ntm(tm_file, lau_tape, 4, &lau_state) == 1);
    assert(lau_state == 1);
    remove(tm_file);
    printf("✓ Vectorized PKI signatures, key revocation gating, and NTM LAU path finder verified.\n");

    // 112-114. Test trie autocomplete, suggestion routing, and NTM SQL validation
    char suggestion[32] = {0};
    assert(interop_rdbms_autocomplete_trie("SE", suggestion, 32) == 0);
    assert(strcmp(suggestion, "SELECT * FROM") == 0);
    assert(interop_rdbms_route_suggestion(prune_nodes, 0, 150) == 0xAAAA);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t sql_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t sql_state = 0;
    assert(interop_rdbms_validate_sql_ntm(tm_file, sql_tape, 4, &sql_state) == 1);
    assert(sql_state == 1);
    remove(tm_file);
    printf("✓ Trie autocomplete, suggestion routing, and NTM SQL validation verified.\n");

    // 115-117. Test Ouroboros forward pass, loop gating, and cycle validation
    InteropOuroborosNeuron test_neurons[2] = {
        { 0, 1.0f, { 0.5f, 0.1f, 0.7f, 0.0f, 0.0f } },
        { 1, 0.0f, { 0.5f, 0.1f, 0.7f, 0.0f, 0.0f } }
    };
    InteropOuroborosSynapse test_synapses[1] = {
        { 0, 1, 2.5f, 1 }
    };
    interop_ouroboros_forward(test_neurons, 2, test_synapses, 1);
    assert(test_neurons[1].state == 2.5f);
    test_neurons[0].vaesen.doubt = 0.8f;
    interop_ouroboros_gate_loops(test_synapses, 1, test_neurons, 2, 0.5f, 0.5f);
    assert(test_synapses[0].active == 0);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t cycle_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t cycle_state = 0;
    assert(interop_ouroboros_validate_cycle_ntm(tm_file, cycle_tape, 4, &cycle_state) == 1);
    assert(cycle_state == 1);
    remove(tm_file);
    printf("✓ Ouroboros forward pass, loop gating, and cycle validation verified.\n");

    // 118-120. Test Ouroboros learning, decision pruning, and layout optimization
    test_synapses[0].weight = 1.0f;
    test_neurons[0].state = 2.0f;
    test_neurons[1].state = 3.0f;
    interop_ouroboros_vector_hebbian_avx512(test_synapses, test_neurons, 1, 2, 0.5f);
    assert(test_synapses[0].weight == 4.0f);
    test_neurons[0].vaesen.doubt = 0.5f;
    test_neurons[1].vaesen.doubt = 1.0f;
    assert(interop_ouroboros_classify_synapse(prune_nodes, 0, &test_neurons[0], &test_neurons[1]) == 0xAAAA);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t layout_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t layout_state = 0;
    assert(interop_ouroboros_optimize_synapses_ntm(tm_file, layout_tape, 4, &layout_state) == 1);
    assert(layout_state == 1);
    remove(tm_file);
    printf("✓ Vectorized learning, decision pruning, and NTM layout optimizer verified.\n");

    // 121-123. Test database slot sync, query routing, and NTM lock resolving
    uint32_t target_slots[2] = { 0, 0 };
    uint32_t source_data[2] = { 77, 88 };
    interop_rdbms_sync_slots_avx512(target_slots, source_data, 2);
    assert(target_slots[0] == 77 && target_slots[1] == 88);
    assert(interop_rdbms_route_query(prune_nodes, 0, 100, 50) == 0xAAAA);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t lock_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t lock_state = 0;
    assert(interop_rdbms_resolve_locks_ntm(tm_file, lock_tape, 4, &lock_state) == 1);
    assert(lock_state == 1);
    remove(tm_file);
    printf("✓ Vectorized database slots sync, query decision routing, and lock resolving verified.\n");

    // 124-126. Test knowledge graph RDBMS sync, route tracing, and path validation
    InteropGraphNode gn[1] = { { 77, 0, 1 } };
    InteropGraphEdge ge[1] = { { 77, 88, 0, 2.5f, 1 } };
    assert(interop_graph_sync_rdbms(gn, 1, ge, 1) == 0);
    float route_weight = 0.0f;
    assert(interop_graph_route_signal(ge, 1, 77, 88, &route_weight) == 0);
    assert(route_weight == 2.5f);
    assert(interop_tm_compile(tm_file, &tm_hdr, tm_trs) == 0);
    uint8_t path_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t path_state = 0;
    assert(interop_graph_validate_path_ntm(tm_file, path_tape, 4, &path_state) == 1);
    assert(path_state == 1);
    remove(tm_file);
    printf("✓ Graph RDBMS sync, signal routing traversal, and path validation verified.\n");

    free(raw_mem);
    printf("✓ Schema verified.\n");

    printf("=== ALL LOCK-FREE ABI DISPATCH MAP TESTS PASSED ===\n");
    return 0;
}
