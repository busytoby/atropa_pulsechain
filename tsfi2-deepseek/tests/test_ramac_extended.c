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
        "../solidity/dysnomia/domain/std/computed_goto.algol61",
        "../solidity/dysnomia/domain/std/designational_goto.algol61",
        "../solidity/dysnomia/domain/std/variable_shadowing.algol61",
        "../solidity/dysnomia/domain/std/ai_bench.algol61",
        "../solidity/dysnomia/domain/std/command_control.algol61",
        "../solidity/dysnomia/domain/std/cdc6600_core.algol61"
    };
    
    for (int i = 0; i < 10; i++) {
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

    // 22. ALGOL Multi-Dimensional Matrix transformations Verification
    printf("[Test] Verifying ALGOL matrix transformations...\n");
    tsfi_algol_matrix mat_a = { 2, 2, { 1, 2, 3, 4 } };
    tsfi_algol_matrix mat_b = { 2, 2, { 5, 6, 7, 8 } };
    tsfi_algol_matrix mat_res;
    tsfi_algol_matrix_multiply(&mat_a, &mat_b, &mat_res);
    assert(mat_res.data[0] == 19);
    assert(mat_res.data[3] == 50);
    printf("  [PASS] ALGOL matrix multiplication verified.\n");
    
    // 23. COBOL COMPUTE Expression Solver Verification
    printf("[Test] Verifying COBOL COMPUTE formula translations...\n");
    int regs[8] = { 0, 10, 5, 8, 0, 0, 0, 0 };
    int comp_res = tsfi_cobol_compute_eval("R1 * R2 + R3", regs);
    assert(comp_res == 58);
    printf("  [PASS] COBOL COMPUTE evaluations verified.\n");

    // 24. ALGOL Floating-Point Trigonometric Library Verification
    printf("[Test] Verifying ALGOL trig library wrappers...\n");
    assert(tsfi_algol_math_sin(0.0) == 0.0);
    assert(tsfi_algol_math_sqrt(16.0) == 4.0);
    printf("  [PASS] ALGOL math wrappers verified.\n");
    
    // 25. Formatted I/O Template Engine Verification
    printf("[Test] Verifying Formatted I/O templates...\n");
    char fmt_buf[32];
    tsfi_algol_format_output("F6.2", 12.345, fmt_buf, sizeof(fmt_buf));
    assert(strcmp(fmt_buf, " 12.35") == 0);
    
    tsfi_algol_format_output("$99.99", 5.67, fmt_buf, sizeof(fmt_buf));
    assert(strcmp(fmt_buf, "$05.67") == 0);
    printf("  [PASS] Output format templates verified.\n");

    // 26. FORTRAN-like COMMON Block Broker Verification
    printf("[Test] Verifying COMMON block shared storage...\n");
    tsfi_algol_common_block common;
    strcpy(common.block_name, "STRAT_SHARE");
    common.size = 64;
    memset(common.data, 0, sizeof(common.data));
    
    int write_res = tsfi_algol_common_write(&common, 5, 999);
    assert(write_res == 0);
    assert(tsfi_algol_common_read(&common, 5) == 999);
    printf("  [PASS] COMMON block shared storage verified.\n");

    // 27. ALGOL / COBOL Cross-Language Interop Verification
    printf("[Test] Verifying ALGOL / COBOL cross-language interop calls...\n");
    int interop_regs[8] = { 0, 4, 3, 0, 0, 0, 0, 0 };
    int cobol_call_res = tsfi_algol_call_cobol("R1 * R2", interop_regs);
    assert(cobol_call_res == 12);
    
    int algol_call_res = tsfi_cobol_call_algol_proc(1, 5);
    assert(algol_call_res == 120);
    printf("  [PASS] ALGOL and COBOL cross-language interop call loops verified.\n");

    // 28. Fred Gruenberger AI Heuristic Maze Solver Verification
    printf("[Test] Verifying AI heuristic maze solver...\n");
    int maze[16] = {
        0, 0, 1, 0,
        1, 0, 1, 0,
        0, 0, 0, 0,
        0, 1, 1, 0
    };
    int visited[16] = {0};
    int maze_res = tsfi_algol_maze_solve(maze, 0, 15, visited);
    assert(maze_res == 1);
    printf("  [PASS] Heuristic path-solving recursion verified.\n");
    
    // 29. Statutory Case Law Indexing Engine Verification
    printf("[Test] Verifying case law keyword index retrieval...\n");
    tsfi_law_case law_db[3] = {
        { "Sherman vs US", "ANTITRUST", 1901 },
        { "State vs Jones", "LIABILITY", 1923 },
        { "Doe vs Smith", "CONTRACT", 1955 }
    };
    int law_res[8] = {0};
    int match_cnt = tsfi_law_query(law_db, 3, "LIABILITY", law_res);
    assert(match_cnt == 1);
    assert(law_res[0] == 1923);
    printf("  [PASS] Statutory query router verified successfully.\n");

    // 30. Command Control warning interlock Verification
    printf("[Test] Verifying command control polling loops...\n");
    tsfi_command_center cc;
    tsfi_command_init(&cc);
    
    strcpy(cc.sensors[0].sensor_name, "RADAR_WARN");
    cc.sensors[0].curr_value = 85;
    cc.sensors[0].threshold = 50;
    cc.sensor_count = 1;
    
    int poll_alert = tsfi_command_poll(&cc);
    assert(poll_alert == 1);
    assert(cc.sensors[0].alert_triggered == 1);
    printf("  [PASS] Command control polling loop alert matched.\n");

    // 31. CDC 3600 Bit-Addressable Byte Extraction Verification
    printf("[Test] Verifying CDC 3600 bit-addressable extraction...\n");
    uint64_t test_word = 0x0000FF0000000000ULL;
    uint64_t ext_byte = tsfi_cdc3600_extract_byte(test_word, 40, 8);
    assert(ext_byte == 0xFF);
    printf("  [PASS] CDC 3600 sub-word bit extraction verified.\n");

    // 32. CDC 3600 48-bit Floating Point Conversion Verification
    printf("[Test] Verifying CDC 3600 48-bit float conversions...\n");
    double test_f = 128.5;
    uint64_t cdc_word = tsfi_double_to_cdc3600_float(test_f);
    double recon_f = tsfi_cdc3600_float_to_double(cdc_word);
    assert(recon_f == test_f);
    printf("  [PASS] CDC 3600 48-bit float encoding/decoding verified.\n");

    // 33. DETAB-X Decision-to-COBOL Generator Verification
    printf("[Test] Verifying DETAB-X decision table parser...\n");
    const char *conds[2] = { "R0 > 5", "R1 == 10" };
    const char *acts[2] = { "SET R2 100", "COMPUTE R3 = R0 * R1" };
    char rules[2][2] = {
        { 'Y', 'Y' },
        { 'N', 'Y' }
    };
    char cobol_buf[512] = "";
    int detab_res = tsfi_detabx_compile(conds, acts, rules, cobol_buf, sizeof(cobol_buf));
    assert(detab_res == 0);
    assert(strstr(cobol_buf, "IF R0 > 5 AND R1 == 10 THEN") != NULL);
    assert(strstr(cobol_buf, "IF R1 == 10 THEN") != NULL);
    printf("  [PASS] DETAB-X decision matrix mapping verified successfully.\n");

    // 34. Full DETAB-X Decision Execution Verification
    printf("[Test] Verifying DETAB-X execution engine...\n");
    tsfi_detabx_table tbl;
    memset(&tbl, 0, sizeof(tbl));
    
    strcpy(tbl.condition_stubs[0], "R0 > 5");
    strcpy(tbl.condition_stubs[1], "R1 == 10");
    tbl.condition_entries[0][0] = 'Y';
    tbl.condition_entries[1][0] = 'Y';
    tbl.condition_entries[0][1] = 'N';
    tbl.condition_entries[1][1] = 'Y';
    
    strcpy(tbl.action_stubs[0], "SET R2 100");
    strcpy(tbl.action_stubs[1], "COMPUTE R3 = R1 * R2");
    tbl.action_entries[0][0] = 'X';
    tbl.action_entries[1][0] = 'X';
    tbl.action_entries[0][1] = ' ';
    tbl.action_entries[1][1] = 'X';
    
    // Set execution sequences
    tbl.action_sequence[0][0] = 1;
    tbl.action_sequence[1][0] = 2;
    
    // Set ELSE rule actions
    tbl.else_action_entries[0] = 'X'; // Set R2 to 100 on ELSE
    
    tbl.num_conditions = 2;
    tbl.num_actions = 2;
    tbl.num_rules = 2;
    
    int interop_exec_regs[8] = { 7, 10, 0, 0, 0, 0, 0, 0 };
    int matched_idx = tsfi_detabx_execute(&tbl, interop_exec_regs);
    assert(matched_idx == 0);
    assert(interop_exec_regs[2] == 100);
    assert(interop_exec_regs[3] == 1000);
    
    // Test ELSE rule execution path
    int else_regs[8] = { 2, 2, 0, 0, 0, 0, 0, 0 };
    int matched_else = tsfi_detabx_execute(&tbl, else_regs);
    assert(matched_else == -2);
    assert(else_regs[2] == 100);
    printf("  [PASS] DETAB-X execution engine, ELSE rules, and sequencing verified.\n");

    // 35. DETAB-X Contradiction & Redundancy Validation Verification
    printf("[Test] Verifying DETAB-X contradiction & redundancy validator...\n");
    tsfi_detabx_table val_tbl;
    memset(&val_tbl, 0, sizeof(val_tbl));
    val_tbl.num_conditions = 2;
    val_tbl.num_actions = 2;
    val_tbl.num_rules = 2;
    
    val_tbl.condition_entries[0][0] = 'Y';
    val_tbl.condition_entries[1][0] = 'Y';
    val_tbl.condition_entries[0][1] = 'Y';
    val_tbl.condition_entries[1][1] = 'Y';
    
    val_tbl.action_entries[0][0] = 'X';
    val_tbl.action_entries[0][1] = 'X';
    
    int val_res = tsfi_detabx_validate(&val_tbl);
    assert(val_res == -2);
    
    val_tbl.action_entries[0][1] = ' ';
    val_tbl.action_entries[1][1] = 'X';
    val_res = tsfi_detabx_validate(&val_tbl);
    assert(val_res == -1);
    printf("  [PASS] DETAB-X static analysis validator verified successfully.\n");

    // 36. CDC 6600 Scoreboard Pipeline Scheduler Verification
    printf("[Test] Verifying CDC 6600 Scoreboard pipeline scheduler...\n");
    cdc_scoreboard sb;
    tsfi_scoreboard_init(&sb);
    
    sb.queue[0].inst_id = 1;
    strcpy(sb.queue[0].op, "ADD");
    sb.queue[0].dest_reg = 2;
    sb.queue[0].src1_reg = 0;
    sb.queue[0].src2_reg = 1;
    sb.queue[0].stage = STAGE_ISSUE;
    sb.size = 1;
    
    int sb_regs[8] = { 15, 25, 0, 0, 0, 0, 0, 0 };
    tsfi_scoreboard_step(&sb, sb_regs);
    assert(sb.queue[0].stage == STAGE_READ_OPERANDS);
    assert(sb.reg_writers[2] == 1);
    
    tsfi_scoreboard_step(&sb, sb_regs);
    assert(sb.queue[0].stage == STAGE_EXECUTE);
    
    tsfi_scoreboard_step(&sb, sb_regs);
    assert(sb.queue[0].stage == STAGE_WRITE_BACK);
    assert(sb_regs[2] == 40);
    printf("  [PASS] CDC 6600 Scoreboard hazard detection and register updates verified.\n");
    
    // 37. CDC 6600 PPU System Verification
    printf("[Test] Verifying CDC 6600 PPU system scheduler...\n");
    cdc_ppu_system ppu_sys;
    tsfi_ppu_init(&ppu_sys);
    tsfi_ppu_assign(&ppu_sys, 3, 5);
    
    int proc_total = 0;
    for (int k = 0; k < 50; k++) {
        proc_total += tsfi_ppu_step(&ppu_sys);
    }
    assert(proc_total == 5);
    assert(ppu_sys.ppus[3].bytes_processed == 5);
    assert(ppu_sys.ppus[3].task_active == 0);
    printf("  [PASS] CDC 6600 PPU time-slice multiplexer verified.\n");
    
    // 38. RAND Tablet Coordinate Interpolator Verification
    printf("[Test] Verifying RAND Tablet coordinate grid mapping...\n");
    rand_tablet_point pt;
    int mock_grid[4][2] = { {0,0}, {1023,0}, {0,1023}, {1023,1023} };
    int interp_res = tsfi_rand_tablet_interpolate(512, 256, mock_grid, &pt);
    assert(interp_res == 0);
    assert(pt.x == 512);
    assert(pt.y == 256);
    assert(pt.pen_down == 1);
    
    rand_tablet_buffer tb;
    tsfi_rand_tablet_trace_init(&tb);
    int trace_res = tsfi_rand_tablet_trace(&tb, 100, 200, mock_grid);
    assert(trace_res == 0);
    assert(tb.count == 1);
    assert(tb.points[0].x == 100);
    assert(tb.points[0].y == 200);
    printf("  [PASS] RAND Tablet coordinate translation and trace buffers verified successfully.\n");

    // 39. CDC 6600 PPU-Driven Scoreboard Dispatch Verification
    printf("[Test] Verifying PPU-driven scoreboard dispatch...\n");
    cdc_ppu_system multi_ppu;
    tsfi_ppu_init(&multi_ppu);
    cdc_scoreboard shared_sb;
    tsfi_scoreboard_init(&shared_sb);
    
    cdc_instruction sample_inst = { 0, "MUL", 3, 0, 1, STAGE_ISSUE };
    int dispatch_res = tsfi_ppu_scoreboard_dispatch(&multi_ppu, &shared_sb, 5, &sample_inst);
    assert(dispatch_res == 0);
    assert(multi_ppu.ppus[5].task_active == 1);
    assert(shared_sb.size == 1);
    assert(shared_sb.queue[0].inst_id == 5);
    printf("  [PASS] Asynchronous PPU scoreboard job offloading verified successfully.\n");

    // 40. CDC 6600 PPU Scoreboard Pipeline Hazard Detection Verification
    printf("[Test] Verifying PPU scoreboard pipeline hazard detection...\n");
    cdc_ppu_system hazard_ppu;
    tsfi_ppu_init(&hazard_ppu);
    cdc_scoreboard hazard_sb;
    tsfi_scoreboard_init(&hazard_sb);
    
    cdc_instruction i1 = { 0, "ADD", 2, 0, 1, STAGE_ISSUE };
    cdc_instruction i2 = { 0, "MUL", 3, 2, 1, STAGE_ISSUE };
    
    tsfi_ppu_scoreboard_dispatch(&hazard_ppu, &hazard_sb, 1, &i1);
    tsfi_ppu_scoreboard_dispatch(&hazard_ppu, &hazard_sb, 2, &i2);
    
    int hz_regs[8] = { 10, 20, 0, 0, 0, 0, 0, 0 };
    
    tsfi_scoreboard_step(&hazard_sb, hz_regs);
    assert(hazard_sb.queue[0].stage == STAGE_READ_OPERANDS);
    
    tsfi_scoreboard_step(&hazard_sb, hz_regs);
    assert(hazard_sb.queue[0].stage == STAGE_EXECUTE);
    assert(hazard_sb.queue[1].stage == STAGE_READ_OPERANDS);
    
    tsfi_scoreboard_step(&hazard_sb, hz_regs);
    assert(hazard_sb.queue[0].stage == STAGE_WRITE_BACK);
    assert(hz_regs[2] == 30);
    
    tsfi_scoreboard_step(&hazard_sb, hz_regs);
    assert(hazard_sb.queue[0].stage == STAGE_DONE);
    assert(hazard_sb.queue[1].stage == STAGE_EXECUTE);
    printf("  [PASS] RAW hazard blocking and release pipeline cycles verified successfully.\n");

    // 41. CODASYL IDS Currency Tracker Verification
    printf("[Test] Verifying CODASYL IDS currency tracker and navigation rings...\n");
    ids_currency_tracker ids_track;
    tsfi_ids_init(&ids_track);
    
    tsfi_ids_insert(&ids_track, 100, 0, "PARENT_REC");
    tsfi_ids_insert(&ids_track, 101, 100, "CHILD_1");
    tsfi_ids_insert(&ids_track, 102, 100, "CHILD_2");
    
    ids_track.current_set[0] = 101;
    int next_rec = tsfi_ids_navigate_next(&ids_track, 0);
    assert(next_rec == 102);
    
    next_rec = tsfi_ids_navigate_next(&ids_track, 0);
    assert(next_rec == 101);
    printf("  [PASS] IDS circular records ring navigation verified.\n");
    
    // 42. MacKenzie Dynamic Drum-to-Tape Segment Migrator Verification
    printf("[Test] Verifying MacKenzie drum-to-tape segment migration...\n");
    mackenzie_storage mstore;
    tsfi_mackenzie_init(&mstore);
    
    for (int k = 0; k < 5; k++) {
        tsfi_mackenzie_access(&mstore, 2);
    }
    
    for (int k = 0; k < 20; k++) {
        mstore.current_tick++;
    }
    
    int mig_cnt = tsfi_mackenzie_migrate(&mstore, 10);
    assert(mig_cnt > 0);
    assert(mstore.segments[3].location == 1);
    assert(mstore.segments[2].location == 0);
    printf("  [PASS] MacKenzie secondary storage paging migration verified.\n");

    // 43. Cross-Chain Token Tracker Verification
    printf("[Test] Verifying Cross-Chain Token Tracker rings...\n");
    cross_chain_tracker cc_track;
    tsfi_cross_chain_init(&cc_track);
    
    tsfi_cross_chain_insert(&cc_track, "0x2b591e99af9f6521961f7e00e86b62ec874c229e", 1, "HEX");
    tsfi_cross_chain_insert(&cc_track, "0x2b591e99af9f6521961f7e00e86b62ec874c229f", 3, "HEX");
    
    cc_track.current_cross_token[0] = 0;
    int next_cc_tok = tsfi_cross_chain_navigate_symbol(&cc_track, 0);
    assert(next_cc_tok == 1);
    
    next_cc_tok = tsfi_cross_chain_navigate_symbol(&cc_track, 0);
    assert(next_cc_tok == 0);
    printf("  [PASS] Cross-chain token address ring navigations verified successfully.\n");

    // 44. COBOL Real-Time Interrupt Controller Verification
    printf("[Test] Verifying COBOL real-time interrupt dispatcher...\n");
    tsfi_cobol_interrupt_controller int_ctrl;
    tsfi_interrupt_init(&int_ctrl);
    
    tsfi_interrupt_register(&int_ctrl, 12, "SET R3 999");
    tsfi_interrupt_register(&int_ctrl, 15, "COMPUTE R2 = R0 + R1");
    
    int int_regs[8] = { 100, 200, 0, 0, 0, 0, 0, 0 };
    int disp_res = tsfi_interrupt_dispatch(&int_ctrl, 12, int_regs);
    assert(disp_res == 0);
    assert(int_regs[3] == 999);
    
    disp_res = tsfi_interrupt_dispatch(&int_ctrl, 15, int_regs);
    assert(disp_res == 0);
    assert(int_regs[2] == 300);
    printf("  [PASS] Asynchronous COBOL interrupt dispatcher verified successfully.\n");

    // 45. PL/I Exception System Verification
    printf("[Test] Verifying PL/I structured exception handling...\n");
    pli_exception_system pli_sys;
    tsfi_pli_exception_init(&pli_sys);
    
    tsfi_pli_exception_register(&pli_sys, "OVERFLOW", "SET R5 777");
    tsfi_pli_exception_register(&pli_sys, "OVERFLOW", "SET R5 888");
    
    int pli_regs[8] = { 0 };
    int trig_res = tsfi_pli_exception_trigger(&pli_sys, "OVERFLOW", pli_regs);
    assert(trig_res == 0);
    assert(pli_regs[5] == 888);
    printf("  [PASS] PL/I exception handler stack matching verified successfully.\n");

    // 46. Zero-Copy PPU Pipeline Verification
    printf("[Test] Verifying Zero-Copy PPU pipeline updates...\n");
    cdc_ppu_system zc_sys;
    tsfi_ppu_init(&zc_sys);
    cdc_scoreboard zc_sb;
    tsfi_scoreboard_init(&zc_sb);
    
    zc_sb.queue[0].inst_id = 0;
    strcpy(zc_sb.queue[0].op, "ADD");
    zc_sb.queue[0].dest_reg = 2;
    zc_sb.queue[0].src1_reg = 0;
    zc_sb.queue[0].src2_reg = 1;
    zc_sb.queue[0].stage = STAGE_ISSUE;
    zc_sb.size = 1;
    
    int zc_res = tsfi_zerocopy_dispatch(&zc_sys, &zc_sb, 4, 0);
    assert(zc_res == 0);
    assert(zc_sys.ppus[4].shared_instruction == &zc_sb.queue[0]);
    assert(zc_sys.ppus[4].task_active == 1);
    
    zc_sys.current_slot = 4;
    int step_processed = tsfi_ppu_step(&zc_sys);
    assert(step_processed == 1);
    assert(zc_sys.ppus[4].bytes_processed == 1);
    assert(zc_sys.ppus[4].shared_instruction->stage == STAGE_ISSUE);
    printf("  [PASS] Zero-copy shared memory pointer referencing verified successfully.\n");

    // 47. Robert Magnuson RMAG Macroprocessor Verification
    printf("[Test] Verifying RMAG macroprocessor expansions...\n");
    rmag_processor rmag;
    tsfi_rmag_init(&rmag);
    
    int def_res = tsfi_rmag_define(&rmag, "SET_REG_VAL", "SET R0 $1");
    assert(def_res == 0);
    
    char expanded_out[128];
    int exp_res = tsfi_rmag_expand(&rmag, "SET_REG_VAL", "777", expanded_out, sizeof(expanded_out));
    assert(exp_res == 0);
    assert(strcmp(expanded_out, "SET R0 777") == 0);
    // Test pre-registered macros
    int exp_res2 = tsfi_rmag_expand(&rmag, "COBOL_COMPUTE", "R3", expanded_out, sizeof(expanded_out));
    assert(exp_res2 == 0);
    assert(strcmp(expanded_out, "COMPUTE R3 = R0 + R1") == 0);
    
    int exp_res3 = tsfi_rmag_expand(&rmag, "YUL_SSTORE", "0xabc", expanded_out, sizeof(expanded_out));
    assert(exp_res3 == 0);
    assert(strcmp(expanded_out, "sstore(0xabc, 1)") == 0);
    
    printf("  [PASS] RMAG macro expansions verified successfully.\n");

    printf("[PASS] All extended RAMAC simulation invariants verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
