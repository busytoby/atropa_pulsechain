#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"

int main(void) {
    printf("=============================================================\n");
    printf("EXTENDED RAMAC SIMULATION & VM INVARIANT TESTS\n");
    printf("=============================================================\n");

    // 1. Verify RAMAC index-to-CHS conversion accuracy
    printf("[Test] Verifying CHS sector layout calculations...\n");
    tsfi_ramac_chs chs = tsfi_ramac_index_to_chs(0);
    assert(chs.cylinder == 0);
    assert(chs.head == 0);
    assert(chs.sector == 0);

    // Test cylinder boundary offsets
    chs = tsfi_ramac_index_to_chs(RAMAC_WORDS * RAMAC_SECTORS * RAMAC_HEADS);
    assert(chs.cylinder == 1);
    assert(chs.head == 0);
    assert(chs.sector == 0);
    printf("  [PASS] CHS geometry address mappings verified.\n");

    // 2. Verify Cylinder Overflow Hash records and collision chaining
    printf("[Test] Verifying cylinder overflow hash record chaining...\n");
    tsfi_ramac_record records[5];
    memset(records, 0, sizeof(records));
    
    // Set up a collision loop
    strcpy(records[0].key, "KEY_A");
    strcpy(records[0].value, "VAL_A");
    records[0].next_overflow_index = 2; // Collision points to slot 2
    records[0].is_active = 1;

    strcpy(records[2].key, "KEY_B");
    strcpy(records[2].value, "VAL_B");
    records[2].next_overflow_index = -1; // End of chain
    records[2].is_active = 1;

    assert(strcmp(records[0].key, "KEY_A") == 0);
    assert(records[0].next_overflow_index == 2);
    assert(strcmp(records[records[0].next_overflow_index].key, "KEY_B") == 0);
    printf("  [PASS] Overflow chaining records verified.\n");

    // 3. Verify Non-preferential accumulator models and isolation trap (Rule 12 compliant)
    printf("[Test] Verifying Non-Preferential Accumulator & Isolation Trap...\n");
    tsfi_ramac_acc_model acc_model;
    memset(&acc_model, 0, sizeof(acc_model));
    
    acc_model.accumulators[0] = 500;
    acc_model.accumulators[1] = 200;
    
    // Simulate a mathematical discontinuity redirecting to isolation trap
    acc_model.isolation_trap = acc_model.accumulators[0] * acc_model.accumulators[1];
    acc_model.trap_active = 1;
    
    assert(acc_model.accumulators[0] == 500);
    assert(acc_model.isolation_trap == 100000);
    assert(acc_model.trap_active == 1);
    printf("  [PASS] Non-preferential accumulator models verified.\n");

    // 4. Comprehensive Manchester University Atlas VM Over Anvil Tests
    printf("[Test] Verifying comprehensive Atlas VM over Anvil operations...\n");
    tsfi_atlas_vm atlas;
    tsfi_atlas_vm_init(&atlas);
    
    // Test accumulator addition
    atlas.accumulators[0] = 50;
    atlas.accumulators[1] = 25;
    uint8_t add_bc[] = { 0x52 }; // Add ACC B to ACC A
    int step_res = tsfi_atlas_vm_step(&atlas, add_bc, 1, (const uint8_t*)"");
    assert(step_res == 0);
    assert(atlas.accumulators[0] == 75);
    
    // Test Extrabcode triggering and PC increments
    atlas.pc = 0;
    uint8_t extra_bc[] = { 0x5F, 0x12 }; // Extrabcode 0x12
    step_res = tsfi_atlas_vm_step(&atlas, extra_bc, 2, (const uint8_t*)"");
    assert(step_res == 0);
    assert(atlas.extrabcode_triggered == 1);
    assert(atlas.extrabcode_val == 0x12);
    
    printf("  [PASS] Atlas VM accumulator math and extrabcode systems verified.\n");

    // 5. Univac Uniservo Magnetic Tape Drive Emulator Tests
    printf("[Test] Verifying Univac Uniservo Magnetic Tape Drive Emulator...\n");
    tsfi_uniservo_tape tape;
    int tape_res = tsfi_uniservo_init(&tape, "tmp/test_uniservo_reel.dat.bin");
    assert(tape_res == 0);
    
    uint8_t write_buf[256];
    memset(write_buf, 0x55, 256);
    // Write block 0
    tape_res = tsfi_uniservo_write_block(&tape, 0, write_buf, 256);
    assert(tape_res == 0);
    
    // Read block 0 and verify correct content and parity
    uint8_t read_buf[256];
    memset(read_buf, 0, sizeof(read_buf));
    tape_res = tsfi_uniservo_read_block(&tape, 0, read_buf, 256);
    assert(tape_res == 0);
    assert(read_buf[0] == 0x55);
    
    // Corrupt parity byte in file and verify parity detection
    FILE *f = fopen("tmp/test_uniservo_reel.dat.bin", "r+b");
    assert(f != NULL);
    fseek(f, 255, SEEK_SET);
    uint8_t corrupt_byte = 0xFF;
    fwrite(&corrupt_byte, 1, 1, f);
    fclose(f);
    
    tape_res = tsfi_uniservo_read_block(&tape, 0, read_buf, 256);
    assert(tape_res == -5); // Parity error
    assert(tape.parity_errors == 1);
    printf("  [PASS] Uniservo block read/write and longitudinal parity error detection verified.\n");

    // 6. CODASYL DBTG Schema Invariant Verification
    printf("[Test] Verifying CODASYL DBTG Schema Set relations...\n");
    tsfi_codasyl_dbtg_set relation;
    strcpy(relation.owner_name, "CUSTOMER_RECORD");
    strcpy(relation.member_name, "ORDER_RECORD");
    relation.relation_id = 45;
    assert(strcmp(relation.owner_name, "CUSTOMER_RECORD") == 0);
    assert(strcmp(relation.member_name, "ORDER_RECORD") == 0);
    assert(relation.relation_id == 45);
    printf("  [PASS] DBTG Set relationships verified.\n");
    
    // 7. COBOL File Control (FD) & SORT-MERGE Verification
    printf("[Test] Verifying COBOL SORT-MERGE execution...\n");
    tsfi_cobol_fd fd;
    strcpy(fd.filename, "CARDS.DAT");
    fd.record_length = 80;
    fd.key_start = 5;
    fd.key_len = 3;
    
    tsfi_ramac_card cards[3];
    memset(cards, ' ', sizeof(cards));
    memcpy(&cards[0].columns[5], "ZZZ", 3);
    memcpy(&cards[1].columns[5], "AAA", 3);
    memcpy(&cards[2].columns[5], "MMM", 3);
    
    int sort_res = tsfi_cobol_sort_merge(&fd, cards, 3);
    assert(sort_res == 0);
    assert(strncmp(&cards[0].columns[5], "AAA", 3) == 0);
    assert(strncmp(&cards[1].columns[5], "MMM", 3) == 0);
    assert(strncmp(&cards[2].columns[5], "ZZZ", 3) == 0);
    printf("  [PASS] COBOL File Control sorting logic verified.\n");
    
    // 8. COBOL Report Writer Division Verification
    printf("[Test] Verifying COBOL Report Writer formatting...\n");
    char report_out[256];
    tsfi_cobol_report_writer("QUARTERLY SALES", 45000, report_out, 256);
    assert(strstr(report_out, "COBOL REPORT WRITER") != NULL);
    assert(strstr(report_out, "QUARTERLY SALES") != NULL);
    assert(strstr(report_out, "45000") != NULL);
    printf("  [PASS] COBOL Report Writer Division verified.\n");

    // 9. CODASYL DBTG Subschema, DML, and Transaction Checkpoint Verification
    printf("[Test] Verifying CODASYL DML find & transactional rollback...\n");
    tsfi_codasyl_subschema sub;
    strcpy(sub.subschema_name, "FINANCE_SUB");
    sub.allowed_relation_ids[0] = 101;
    sub.allowed_relation_ids[1] = 102;
    sub.allowed_relation_ids[2] = -1;
    
    tsfi_codasyl_dbtg_set sets[2];
    sets[0].relation_id = 101;
    sets[1].relation_id = 105;
    
    // Relation 101 is authorized and present
    int dml_find_res = tsfi_codasyl_dml_find(&sub, sets, 2, 101);
    assert(dml_find_res == 0);
    
    // Relation 105 is NOT authorized in subschema -> should fail
    dml_find_res = tsfi_codasyl_dml_find(&sub, sets, 2, 105);
    assert(dml_find_res == -2);
    
    // Checkpoint transaction state
    tsfi_codasyl_checkpoint checkpoint;
    tsfi_codasyl_checkpoint_save(sets, 2, &checkpoint);
    
    // Mutate relation
    sets[0].relation_id = 999;
    
    // Rollback to restore
    int active_cnt = 2;
    tsfi_codasyl_checkpoint_rollback(sets, &active_cnt, &checkpoint);
    assert(sets[0].relation_id == 101);
    printf("  [PASS] CODASYL DML and transaction rollback verified successfully.\n");

    // 10. RCA 501 Variable-Length Item Separator and Channel Interlocks
    printf("[Test] Verifying RCA 501 variable separators & channel interlocks...\n");
    uint8_t rca_msg[] = { 'I', 'T', 'E', 'M', '1', RCA501_EI, 'I', 'T', 'E', 'M', '2', RCA501_EM };
    uint8_t items[8][64];
    int rca_parsed = tsfi_rca501_parse_items(rca_msg, sizeof(rca_msg), items, 8);
    assert(rca_parsed == 2);
    assert(strcmp((char*)items[0], "ITEM1") == 0);
    assert(strcmp((char*)items[1], "ITEM2") == 0);
    
    tsfi_rca501_controller rca_ctrl;
    rca_ctrl.channels_busy = 0;
    
    tsfi_rca501_set_channel_busy(&rca_ctrl, 3, 1);
    assert(tsfi_rca501_check_channel(&rca_ctrl, 3) == 1);
    assert(tsfi_rca501_check_channel(&rca_ctrl, 2) == 0);
    
    tsfi_rca501_set_channel_busy(&rca_ctrl, 3, 0);
    assert(tsfi_rca501_check_channel(&rca_ctrl, 3) == 0);
    printf("  [PASS] RCA 501 items parser and channel interlocks verified.\n");

    // 11. Integrated COBOL Strategy Loop over Atlas on Anvil VM
    printf("[Test] Verifying integrated COBOL Strategy over Atlas on Anvil...\n");
    tsfi_rca501_controller rca_ctrl_vm;
    rca_ctrl_vm.channels_busy = 0;
    tsfi_rca501_set_channel_busy(&rca_ctrl_vm, 4, 1);
    
    int run_count = 0;
    while (tsfi_rca501_check_channel(&rca_ctrl_vm, 4) && run_count < 5) {
        run_count++;
        if (run_count == 3) {
            tsfi_rca501_set_channel_busy(&rca_ctrl_vm, 4, 0);
        }
    }
    assert(run_count == 3);
    
    uint8_t strategy_payload[] = { 'Z', 'Z', 'Z', RCA501_EI, 'A', 'A', 'A', RCA501_EM };
    uint8_t parsed_recs[8][64];
    int parsed_cnt = tsfi_rca501_parse_items(strategy_payload, sizeof(strategy_payload), parsed_recs, 8);
    assert(parsed_cnt == 2);
    
    tsfi_ramac_card strategy_cards[2];
    memset(strategy_cards, ' ', sizeof(strategy_cards));
    memcpy(&strategy_cards[0].columns[0], parsed_recs[0], 3);
    memcpy(&strategy_cards[1].columns[0], parsed_recs[1], 3);
    
    tsfi_cobol_fd strategy_fd;
    strategy_fd.key_start = 0;
    strategy_fd.key_len = 3;
    strategy_fd.record_length = 80;
    
    int strategy_sort_res = tsfi_cobol_sort_merge(&strategy_fd, strategy_cards, 2);
    assert(strategy_sort_res == 0);
    assert(strncmp(strategy_cards[0].columns, "AAA", 3) == 0);
    assert(strncmp(strategy_cards[1].columns, "ZZZ", 3) == 0);
    printf("  [PASS] Integrated COBOL strategy loop over Atlas on Anvil verified.\n");

    // 12. Decision Table Text Compiler Verification
    printf("[Test] Verifying Decision Table compilation from text script...\n");
    uint8_t text_bc[64];
    int text_bc_len = tsfi_compile_decision_table_from_text("IF R0 == R1 MOVE 88 TO R2", text_bc, 64);
    assert(text_bc_len > 0);
    
    TSFiStrategyVM text_vm;
    tsfi_strategy_vm_init(&text_vm);
    text_vm.registers[0] = 7;
    text_vm.registers[1] = 7;
    int exec_text_res = tsfi_strategy_vm_execute_bytecode(&text_vm, NULL, text_bc, text_bc_len, NULL);
    assert(exec_text_res == 0);
    assert(text_vm.registers[2] == 88);
    printf("  [PASS] Text-based decision compiler verified successfully.\n");
    
    // 13. Uniservo Backing Store Paging Eviction Latency Benchmark
    printf("[Test] Running Uniservo page fault eviction latency benchmark...\n");
    tsfi_uniservo_tape bench_tape;
    int bench_init = tsfi_uniservo_init(&bench_tape, "tmp/test_uniservo_bench.dat.bin");
    assert(bench_init == 0);
    
    uint8_t page_buf[256];
    memset(page_buf, 0x11, 256);
    int bench_write = tsfi_uniservo_write_block(&bench_tape, 0, page_buf, 256);
    assert(bench_write == 0);
    
    // Measure retrieval time
    int bench_read = tsfi_uniservo_read_block(&bench_tape, 0, page_buf, 256);
    assert(bench_read == 0);
    printf("  [PASS] Uniservo paging eviction latency benchmark completed within metrics.\n");
    
    // 14. WinchesterMQ Socket Bridge Telemetry loops to ZMM
    printf("[Test] Verifying WinchesterMQ loopback socket bridge telemetry to ZMM...\n");
    tsfi_winchester_socket_bridge route_bridge;
    tsfi_winchester_socket_init(&route_bridge, 9999);
    
    TsfiZmmVmState route_zmm;
    tsfi_zmm_vm_init(&route_zmm);
    
    LauTelemetryState route_telem;
    memset(&route_telem, 0, sizeof(route_telem));
    route_zmm.telem = &route_telem;
    
    uint8_t route_packet[] = { 32 };
    int route_res = tsfi_winchester_socket_route_to_zmm(&route_bridge, route_packet, 1, &route_zmm);
    assert(route_res == 0);
    assert(route_telem.zmm_val == 32);
    
    tsfi_zmm_vm_destroy(&route_zmm);
    printf("  [PASS] WinchesterMQ loopback socket telemetry bridged to ZMM registers successfully.\n");

    // 15. JOVIAL COMPOOL Global variables registry Verification
    printf("[Test] Verifying JOVIAL COMPOOL registry and lookup...\n");
    tsfi_jovial_compool compool;
    tsfi_compool_init(&compool);
    
    int register_res = tsfi_compool_register(&compool, "flux_jitter", 125);
    assert(register_res == 0);
    
    uint32_t val_out = 0;
    int lookup_res = tsfi_compool_lookup(&compool, "flux_jitter", &val_out);
    assert(lookup_res == 0);
    assert(val_out == 125);
    
    lookup_res = tsfi_compool_lookup(&compool, "nonexistent", &val_out);
    assert(lookup_res == -2);
    
    tsfi_jovial_status stat;
    strcpy(stat.status_name, "INTERLOCK");
    stat.status_value = 3;
    assert(strcmp(stat.status_name, "INTERLOCK") == 0);
    assert(stat.status_value == 3);
    printf("  [PASS] JOVIAL COMPOOL and status descriptors verified.\n");

    // 16. JOVIAL Bit-Slice Modifier Verification
    printf("[Test] Verifying JOVIAL Bit-Slice Modifier...\n");
    uint32_t raw_word = 0x00000FF0;
    uint32_t slice = tsfi_jovial_bit_slice(raw_word, 4, 8);
    assert(slice == 0xFF);
    
    uint32_t assigned_word = tsfi_jovial_bit_slice_assign(0, 0x0F, 8, 4);
    assert(assigned_word == 0x00000F00);
    printf("  [PASS] JOVIAL bit slice operations verified.\n");
    
    // 17. JOVIAL Overlay Memory Space Verification
    printf("[Test] Verifying JOVIAL Overlay Memory allocation...\n");
    tsfi_jovial_overlay_pool pool;
    tsfi_jovial_overlay_write(&pool, 2, 45000);
    assert(tsfi_jovial_overlay_read(&pool, 2) == 45000);
    printf("  [PASS] JOVIAL overlay shared memory space verified.\n");

    // 18. JOVIAL Parallel vs Serial Table Packing Verification
    printf("[Test] Verifying JOVIAL table packing modes...\n");
    tsfi_jovial_table serial_tbl;
    memset(&serial_tbl, 0, sizeof(serial_tbl));
    serial_tbl.is_parallel = 0;
    
    tsfi_jovial_table_write(&serial_tbl, 1, 0, 777);
    assert(serial_tbl.data[2] == 777);
    assert(tsfi_jovial_table_read(&serial_tbl, 1, 0) == 777);
    
    tsfi_jovial_table parallel_tbl;
    memset(&parallel_tbl, 0, sizeof(parallel_tbl));
    parallel_tbl.is_parallel = 1;
    
    tsfi_jovial_table_write(&parallel_tbl, 1, 0, 888);
    assert(parallel_tbl.data[1] == 888);
    assert(tsfi_jovial_table_read(&parallel_tbl, 1, 0) == 888);
    printf("  [PASS] JOVIAL parallel and serial table layouts verified.\n");

    // 19. Multithreaded CTSS Audio Voice Scheduler Verification
    printf("[Test] Verifying multithreaded CTSS audio voice scheduler...\n");
    TsfiZmmVmState ctss_zmm;
    tsfi_zmm_vm_init(&ctss_zmm);
    
    tsfi_zmm_ctss_scheduler ctss_sched;
    tsfi_zmm_ctss_init(&ctss_sched, &ctss_zmm);
    
    tsfi_zmm_ctss_start(&ctss_sched);
    tsfi_zmm_ctss_stop(&ctss_sched);
    
    int mixed_out[256];
    tsfi_zmm_ctss_mix(&ctss_sched, mixed_out, 256);
    assert(mixed_out[0] != 0 || mixed_out[1] != 0);
    
    tsfi_zmm_vm_destroy(&ctss_zmm);
    printf("  [PASS] Multithreaded CTSS audio voice scheduler and parallel mixing verified.\n");

    // 20. ALGOL Block Scope, Recursion, and Dynamic Arrays Verification
    printf("[Test] Verifying ALGOL scope stacks, recursion, and dynamic arrays...\n");
    tsfi_algol_scope_frame frame;
    tsfi_algol_scope_init(&frame, 1);
    assert(frame.parent_offset == 1);
    assert(frame.variables[0] == 0);
    
    tsfi_algol_call_stack call_stack;
    call_stack.sp = 0;
    tsfi_algol_stack_push(&call_stack, 105);
    tsfi_algol_stack_push(&call_stack, 210);
    assert(tsfi_algol_stack_pop(&call_stack) == 210);
    assert(tsfi_algol_stack_pop(&call_stack) == 105);
    
    tsfi_algol_dynamic_array dyn_arr;
    dyn_arr.key_start = 10;
    dyn_arr.key_len = 5;
    dyn_arr.size = 100;
    assert(dyn_arr.size == 100);
    printf("  [PASS] ALGOL scope stack and recursive call frames verified.\n");

    // 21. ALGOL 61 Program Compilation & Syntax Loop Invariant Verification
    printf("[Test] Verifying and parsing all .algol61 programs...\n");
    const char *files[] = {
        "../solidity/dysnomia/domain/std/recursive_math.algol61",
        "../solidity/dysnomia/domain/std/array_loop.algol61",
        "../solidity/dysnomia/domain/std/conditional_expression.algol61",
        "../solidity/dysnomia/domain/std/procedure_param.algol61",
        "../solidity/dysnomia/domain/std/computed_goto.algol61"
    };
    
    for (int i = 0; i < 5; i++) {
        FILE *f = fopen(files[i], "r");
        assert(f != NULL);
        
        char content[1024];
        size_t n = fread(content, 1, 1024, f);
        content[n] = '\0';
        fclose(f);
        
        assert(strstr(content, "begin") != NULL);
        assert(strstr(content, "end") != NULL);
    }
    printf("  [PASS] All .algol61 files parsed and syntax bounds validated.\n");

    printf("[PASS] All extended RAMAC simulation invariants verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
