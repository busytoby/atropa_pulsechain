#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_winchester_bridge.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_dat.h"
#include "tsfi_trie.h"

int main(void) {
    printf("=============================================================\n");
    printf("EXTENDED RAMAC SIMULATION & VM INVARIANT TESTS - PART 2\n");
    printf("=============================================================\n");

    // We need mock structures or local initializations where needed.
    // Let's initialize trie and dat dependencies for some tests.
    tsfi_dat_t *dat_mq = tsfi_dat_create();
    tsfi_trie_t *trie_root_mq = tsfi_trie_create();

    // 71. COBOL Sub-schema Data Division Mapper Verification
    printf("[Test] Verifying COBOL Sub-schema mapper...\n");
    tsfi_subschema_map ss_map;
    tsfi_subschema_init(&ss_map, "LEDGER-SS", "ENTRY-REC", "LEDGER-SET");
    ss_map.field_offsets[0] = 0;
    ss_map.field_offsets[1] = 4;
    ss_map.field_count = 2;
    
    uint8_t record_data[8] = { 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x03, 0xE8 };
    int registers[8] = {0};
    int map_res = tsfi_subschema_map_data(&ss_map, record_data, registers);
    assert(map_res == 0);
    assert(registers[0] == 5);
    assert(registers[1] == 1000);
    printf("  [PASS] COBOL Sub-schema record data byte mapping verified.\n");

    // 72. DBTG Database Currency Indicators Verification
    printf("[Test] Verifying DBTG database currency tracking...\n");
    tsfi_dbtg_currency dbtg_cur;
    tsfi_dbtg_currency_init(&dbtg_cur);
    assert(dbtg_cur.current_run_unit == -1);
    
    tsfi_dbtg_currency_update(&dbtg_cur, 1001, 2, 3);
    assert(dbtg_cur.current_run_unit == 1001);
    assert(dbtg_cur.current_record_type[2] == 1001);
    assert(dbtg_cur.current_set_type[3] == 1001);
    printf("  [PASS] DBTG Run-unit, Record-Type, and Set-Type currency tracking verified.\n");

    // 73. DBTG Area (Realm) Control Registry Verification
    printf("[Test] Verifying DBTG Area (Realm) Control locks...\n");
    tsfi_dbtg_realm_registry realm_reg;
    tsfi_dbtg_realm_init(&realm_reg);
    
    int realm_reg_ok = tsfi_dbtg_realm_register(&realm_reg, "LEDGER-AREA");
    assert(realm_reg_ok == 0);
    assert(realm_reg.area_count == 1);
    
    int open_ok = tsfi_dbtg_realm_open(&realm_reg, "LEDGER-AREA", DBTG_LOCK_EXCLUSIVE_UPDATE);
    assert(open_ok == 0);
    assert(realm_reg.areas[0].is_open == 1);
    assert(realm_reg.areas[0].lock_mode == DBTG_LOCK_EXCLUSIVE_UPDATE);
    
    int close_ok = tsfi_dbtg_realm_close(&realm_reg, "LEDGER-AREA");
    assert(close_ok == 0);
    assert(realm_reg.areas[0].is_open == 0);
    assert(realm_reg.areas[0].lock_mode == DBTG_LOCK_NONE);
    printf("  [PASS] DBTG Area Realm registration and locking verified.\n");

    // 74. ZMM VM Programmer Stepper Verification
    printf("[Test] Verifying ZMM VM Programmer Stepper...\n");
    TsfiZmmVmState zmm_state;
    tsfi_zmm_vm_init(&zmm_state);
    
    union {
        uint32_t words[8];
        float floats[8];
    } bytecode_block;
    memset(&bytecode_block, 0, sizeof(bytecode_block));
    bytecode_block.words[0] = ZMM_OP_WLOAD;
    bytecode_block.words[1] = 1;
    bytecode_block.floats[2] = 42.0f;
    bytecode_block.words[3] = ZMM_OP_WSTORE;
    bytecode_block.words[4] = 0;
    bytecode_block.words[5] = 0;
    bytecode_block.words[6] = ZMM_OP_END;
    
    zmm_state.program_counter = 0;
    int step1 = tsfi_zmm_vm_step_block(&zmm_state, bytecode_block.words);
    assert(step1 == 3);
    assert(zmm_state.program_counter == 3);
    
    int step2 = tsfi_zmm_vm_step_block(&zmm_state, bytecode_block.words);
    assert(step2 == 6);
    
    int step3 = tsfi_zmm_vm_step_block(&zmm_state, bytecode_block.words);
    assert(step3 == -1);
    
    tsfi_zmm_vm_destroy(&zmm_state);
    printf("  [PASS] ZMM VM Programmer Stepper instruction step cycles verified.\n");

    // 75. CODASYL DB-EXCEPTION Declarative Verification
    printf("[Test] Verifying CODASYL DB-EXCEPTION declaratives...\n");
    tsfi_dbtg_realm_registry exception_realm_reg;
    tsfi_dbtg_realm_init(&exception_realm_reg);
    tsfi_dbtg_realm_register(&exception_realm_reg, "MUTABLE-AREA");
    
    tsfi_dbtg_realm_open(&exception_realm_reg, "MUTABLE-AREA", DBTG_LOCK_RETRIEVAL);
    
    tsfi_dbtg_exception_context db_exc_ctx;
    tsfi_dbtg_exception_init(&db_exc_ctx);
    
    int exc_check_val = tsfi_dbtg_validate_action(&db_exc_ctx, &exception_realm_reg, "MUTABLE-AREA", 1);
    assert(exc_check_val == -3);
    assert(db_exc_ctx.db_status == DB_STATUS_LOCK_VIOLATION);
    assert(db_exc_ctx.exception_triggered == 1);
    assert(strcmp(db_exc_ctx.failing_realm, "MUTABLE-AREA") == 0);
    
    exc_check_val = tsfi_dbtg_validate_action(&db_exc_ctx, &exception_realm_reg, "MUTABLE-AREA", 0);
    assert(exc_check_val == 0);
    assert(db_exc_ctx.db_status == DB_STATUS_OK);
    assert(db_exc_ctx.exception_triggered == 0);
    printf("  [PASS] DB-EXCEPTION declarative lock and status validation verified.\n");

    // 76. DBTG Set Membership Invariants Verification
    printf("[Test] Verifying DBTG Set Membership constraints...\n");
    tsfi_dbtg_set_membership mandatory_mem;
    tsfi_dbtg_set_membership_init(&mandatory_mem, "LEDGER-SET", DBTG_INSERT_AUTOMATIC, DBTG_RETENTION_MANDATORY);
    assert(mandatory_mem.insertion_mode == DBTG_INSERT_AUTOMATIC);
    assert(mandatory_mem.retention_mode == DBTG_RETENTION_MANDATORY);
    
    int db_stat = 0;
    int disc_res = tsfi_dbtg_validate_disconnect(&mandatory_mem, &db_stat);
    assert(disc_res == -2);
    assert(db_stat == DB_STATUS_MEMBER_ERR);
    
    tsfi_dbtg_set_membership optional_mem;
    tsfi_dbtg_set_membership_init(&optional_mem, "TEMP-SET", DBTG_INSERT_MANUAL, DBTG_RETENTION_OPTIONAL);
    disc_res = tsfi_dbtg_validate_disconnect(&optional_mem, &db_stat);
    assert(disc_res == 0);
    assert(db_stat == DB_STATUS_OK);
    printf("  [PASS] DBTG Set Membership mandatory retention bounds verified.\n");

    // 77. DBTG Set Directional Navigation Verification
    printf("[Test] Verifying DBTG Set Directional Navigation...\n");
    int navigate_count = 5;
    assert(tsfi_dbtg_navigate_set(2, navigate_count, DBTG_NAV_FIRST) == 0);
    assert(tsfi_dbtg_navigate_set(2, navigate_count, DBTG_NAV_LAST) == 4);
    assert(tsfi_dbtg_navigate_set(2, navigate_count, DBTG_NAV_NEXT) == 3);
    assert(tsfi_dbtg_navigate_set(0, navigate_count, DBTG_NAV_PRIOR) == 4);
    printf("  [PASS] DBTG FIRST, LAST, NEXT, and PRIOR offset mappings verified.\n");

    // 78. DBTG Set Sort Order Invariants Verification
    printf("[Test] Verifying DBTG Set Sort Order invariants...\n");
    tsfi_dbtg_sorted_record sorted_records[4];
    memset(sorted_records, 0, sizeof(sorted_records));
    
    int s_count = 0;
    s_count = tsfi_dbtg_insert_sorted(sorted_records, s_count, 4, 101, 50);
    assert(s_count == 1);
    
    s_count = tsfi_dbtg_insert_sorted(sorted_records, s_count, 4, 102, 20);
    assert(s_count == 2);
    assert(sorted_records[0].record_id == 102);
    assert(sorted_records[1].record_id == 101);
    
    s_count = tsfi_dbtg_insert_sorted(sorted_records, s_count, 4, 103, 35);
    assert(s_count == 3);
    assert(sorted_records[0].record_id == 102);
    assert(sorted_records[1].record_id == 103);
    assert(sorted_records[2].record_id == 101);
    printf("  [PASS] DBTG Set Sorted insertion key alignment verified.\n");

    // 79. DBTG CALC Location Mode Verification
    printf("[Test] Verifying DBTG CALC location hash mappings...\n");
    tsfi_dbtg_calc_address addr1, addr2;
    memset(&addr1, 0, sizeof(addr1));
    memset(&addr2, 0, sizeof(addr2));
    
    tsfi_dbtg_resolve_calc("RECORD_KEY_A", 100, 16, &addr1);
    tsfi_dbtg_resolve_calc("RECORD_KEY_B", 100, 16, &addr2);
    
    assert(addr1.target_page < 100);
    assert(addr1.target_slot < 16);
    assert(addr2.target_page < 100);
    assert(addr2.target_slot < 16);
    assert(addr1.target_page != addr2.target_page || addr1.target_slot != addr2.target_slot);
    printf("  [PASS] DBTG CALC location hashing target page and slot resolver verified.\n");

    // 80. DBTG DML Command Execution Tracker Verification
    printf("[Test] Verifying DBTG DML Command Tracker...\n");
    tsfi_dbtg_dml_tracker dml_track;
    tsfi_dbtg_dml_tracker_init(&dml_track);
    
    assert(tsfi_dbtg_execute_dml(&dml_track, DBTG_VERB_STORE) == 0);
    assert(tsfi_dbtg_execute_dml(&dml_track, DBTG_VERB_GET) == 0);
    assert(tsfi_dbtg_execute_dml(&dml_track, DBTG_VERB_GET) == 0);
    assert(tsfi_dbtg_execute_dml(&dml_track, DBTG_VERB_MODIFY) == 0);
    assert(tsfi_dbtg_execute_dml(&dml_track, DBTG_VERB_ERASE) == 0);
    assert(tsfi_dbtg_execute_dml(&dml_track, DBTG_VERB_CONNECT) == 0);
    assert(tsfi_dbtg_execute_dml(&dml_track, DBTG_VERB_DISCONNECT) == 0);
    
    assert(dml_track.store_count == 1);
    assert(dml_track.get_count == 2);
    assert(dml_track.modify_count == 1);
    assert(dml_track.erase_count == 1);
    assert(dml_track.connect_count == 1);
    assert(dml_track.disconnect_count == 1);
    printf("  [PASS] DBTG DML verb call volume metrics tracking verified.\n");

    // 81. ZMM VM Strategy DBTG integration Verification
    printf("[Test] Verifying ZMM VM Strategy DBTG integration...\n");
    TSFiStrategyVM strat_vm;
    tsfi_strategy_vm_init(&strat_vm);
    
    tsfi_dbtg_currency int_cur;
    tsfi_dbtg_currency_init(&int_cur);
    int_cur.current_run_unit = 2048;
    
    tsfi_dbtg_realm_registry int_realm;
    tsfi_dbtg_realm_init(&int_realm);
    tsfi_dbtg_realm_register(&int_realm, "MAIN-REALM");
    tsfi_dbtg_realm_open(&int_realm, "MAIN-REALM", DBTG_LOCK_EXCLUSIVE_UPDATE);
    
    tsfi_strategy_vm_bind_dbtg(&strat_vm, &int_cur, &int_realm);
    assert(strat_vm.registers[0] == 2048);
    assert(strat_vm.registers[1] == DBTG_LOCK_EXCLUSIVE_UPDATE);
    printf("  [PASS] DBTG currency and locks bound to Strategy registers verified.\n");

    // 82. WinchesterMQ DBTG Exception Register Mapping Verification
    printf("[Test] Verifying WinchesterMQ hardware mapping for DBTG exceptions...\n");
    TSFiSynthPerfEngine *perf_engine_exc = tsfi_synth_perf_create(dat_mq, trie_root_mq);
    TSFiWinchesterBridge *bridge_exc = tsfi_winchester_bridge_create(perf_engine_exc);
    assert(bridge_exc != NULL);
    
    tsfi_winchester_bridge_map_dbtg_exception(bridge_exc, DB_STATUS_LOCK_VIOLATION);
    assert((bridge_exc->registers.status_reg >> 16) == DB_STATUS_LOCK_VIOLATION);
    
    tsfi_winchester_bridge_destroy(bridge_exc);
    tsfi_synth_perf_destroy(perf_engine_exc);
    printf("  [PASS] WinchesterMQ hardware register mapping for DBTG exceptions verified.\n");

    // 83. Strategy Syntactic Compiler Bindings Verification
    printf("[Test] Verifying Strategy Syntactic Compiler Bindings...\n");
    uint8_t compiled_buf[256];
    int compiled_len = 0;
    int compile_res = tsfi_strategy_compile_script(
        "PROCEDURE DIVISION. MOVE DBTG-CRU TO R2. MOVE DBTG-REALM-LOCK TO R3.",
        compiled_buf, sizeof(compiled_buf), &compiled_len
    );
    assert(compile_res == 0);
    assert(compiled_len > 0);
    
    TSFiStrategyVM bound_strat_vm;
    tsfi_strategy_vm_init(&bound_strat_vm);
    bound_strat_vm.registers[0] = 5005;
    bound_strat_vm.registers[1] = 4;
    
    int exec_res = tsfi_strategy_vm_execute_bytecode(&bound_strat_vm, NULL, compiled_buf, compiled_len, NULL);
    assert(exec_res == 0);
    assert(bound_strat_vm.registers[2] == 5005);
    assert(bound_strat_vm.registers[3] == 4);
    printf("  [PASS] DBTG-CRU and DBTG-REALM-LOCK keywords successfully compiled to bytecode.\n");

    // 84. S/370 Dynamic Address Translation (DAT) Verification
    printf("[Test] Verifying S/370 Dynamic Address Translation (DAT)...\n");
    tsfi_s370_vs_dat dat_s370;
    tsfi_s370_vs_dat_init(&dat_s370);
    dat_s370.segment_table[3] = 0x1000;
    dat_s370.page_tables[3][5] = 0x50000;
    
    uint32_t phys_addr = 0;
    int trans_res = tsfi_s370_vs_dat_translate(&dat_s370, (3 << 12) | (5 << 8) | 12, &phys_addr);
    assert(trans_res == 0);
    assert(phys_addr == 0x5000C);
    printf("  [PASS] S/370 Virtual-to-Real dynamic address mapping verified.\n");

    // 85. Relational Tuple-Space to CODASYL Mapper Verification
    printf("[Test] Verifying Relational Tuple-Space mapping...\n");
    tsfi_relational_tuple relational_t;
    tsfi_relational_tuple_init(&relational_t, "MUTABLE-AREA", "USER_KEY_123");
    
    int rel_db_stat = 0;
    int rel_map_res = tsfi_relational_map_to_codasyl(&relational_t, &exception_realm_reg, &rel_db_stat);
    assert(rel_map_res == 0);
    assert(rel_db_stat == DB_STATUS_OK);
    printf("  [PASS] Relational query projection matrix mapper verified.\n");

    // 86. Structured Programming Go-To-Less Static Analyzer Verification
    printf("[Test] Verifying Structured Programming static analyzer...\n");
    tsfi_structured_analysis_report struct_report;
    tsfi_structured_analyze_script("LOOPSTART: ADD R1 TO R2. GOTO LOOPSTART.", &struct_report);
    assert(struct_report.goto_count == 1);
    assert(struct_report.backward_jmp_detected == 1);
    printf("  [PASS] Go-To-Less structured static analysis checker verified.\n");

    // 87. Mainframe VTAM Access Subsystem Verification
    printf("[Test] Verifying VTAM access logical unit sessions...\n");
    tsfi_vtam_session lu_session;
    tsfi_vtam_session_init(&lu_session, "LTERM01");
    assert(lu_session.session_state == VTAM_STATE_INIT);
    
    int bind_res = tsfi_vtam_session_handshake(&lu_session, VTAM_EV_BIND);
    assert(bind_res == 0);
    assert(lu_session.session_state == VTAM_STATE_NEGOTIATE);
    
    int sdt_res = tsfi_vtam_session_handshake(&lu_session, VTAM_EV_SDT);
    assert(sdt_res == 0);
    assert(lu_session.session_state == VTAM_STATE_ACTIVE);
    
    int vtam_send_res = tsfi_vtam_session_send(&lu_session, "VTAM_PAYLOAD_TEST", 17);
    assert(vtam_send_res == 17);
    assert(lu_session.bytes_transmitted == 17);
    printf("  [PASS] VTAM LU-LU session binds and data traffic verified.\n");

    // 88. VM/370 Conversational Monitor System (CMS) Verification
    printf("[Test] Verifying VM/370 CMS commands...\n");
    tsfi_cms_session cms_sess;
    tsfi_cms_session_init(&cms_sess);
    assert(cms_sess.is_booted == 0);
    
    assert(tsfi_cms_execute_command(&cms_sess, "ACCESS 191 A") == -2);
    
    assert(tsfi_cms_execute_command(&cms_sess, "IPL CMS") == 0);
    assert(cms_sess.is_booted == 1);
    
    assert(tsfi_cms_execute_command(&cms_sess, "ACCESS 191 A") == 0);
    assert(cms_sess.disk_mounted_191 == 1);
    
    assert(tsfi_cms_execute_command(&cms_sess, "LISTFILE") == 0);
    printf("  [PASS] VM/370 CMS boot, access, and file catalog commands verified.\n");

    // 89. VM/370 Control Program (CP) Hypervisor Verification
    printf("[Test] Verifying VM/370 CP console commands...\n");
    tsfi_cp_session cp_sess;
    tsfi_cp_session_init(&cp_sess);
    assert(cp_sess.virtual_storage_kb == 4096);
    assert(cp_sess.spool_class == ' ');
    
    assert(tsfi_cp_execute_command(&cp_sess, "DEFINE STORAGE 16M") == 0);
    assert(cp_sess.virtual_storage_kb == 16384);
    
    assert(tsfi_cp_execute_command(&cp_sess, "SPOOL PUNCH CLASS A") == 0);
    assert(cp_sess.spool_class == 'A');
    
    assert(tsfi_cp_execute_command(&cp_sess, "QUERY VIRTUAL") == 0);
    printf("  [PASS] VM/370 CP storage resizing and virtual device spooling verified.\n");

    // 90. VM/370 CP Virtual Reader & Spool Queue Verification
    printf("[Test] Verifying VM/370 CP Virtual Reader Spool Queue...\n");
    tsfi_cp_spool_queue spool_q;
    tsfi_cp_spool_queue_init(&spool_q);
    assert(spool_q.count == 0);
    
    assert(tsfi_cp_spool_push(&spool_q, "CARD RECORD 1") == 0);
    assert(tsfi_cp_spool_push(&spool_q, "CARD RECORD 2") == 0);
    assert(spool_q.count == 2);
    
    char card_buf[80];
    assert(tsfi_cp_spool_pop(&spool_q, card_buf) == 0);
    assert(strcmp(card_buf, "CARD RECORD 1") == 0);
    assert(tsfi_cp_spool_pop(&spool_q, card_buf) == 0);
    assert(strcmp(card_buf, "CARD RECORD 2") == 0);
    assert(spool_q.count == 0);
    printf("  [PASS] VM/370 card queue spool push and pop verified.\n");

    // 91. VM/370 IUCV Inter-User Communication Verification
    printf("[Test] Verifying VM/370 IUCV broker messages...\n");
    tsfi_iucv_broker broker;
    tsfi_iucv_broker_init(&broker);
    
    int path_id = tsfi_iucv_connect(&broker, "USERVM1", "USERVM2");
    assert(path_id >= 0);
    assert(broker.paths[path_id].status == IUCV_PATH_ACTIVE);
    
    int send_ok = tsfi_iucv_send(&broker, path_id, "DIRECT_IUCV_MSG_1");
    assert(send_ok == 0);
    assert(broker.paths[path_id].message_pending == 1);
    
    char recv_buf[64];
    int recv_ok = tsfi_iucv_receive(&broker, path_id, recv_buf);
    assert(recv_ok == 0);
    assert(strcmp(recv_buf, "DIRECT_IUCV_MSG_1") == 0);
    assert(broker.paths[path_id].message_pending == 0);
    printf("  [PASS] VM/370 IUCV program-to-program messages routed successfully.\n");

    // 92. VM/370 CP Console Spool Logger Verification
    printf("[Test] Verifying VM/370 CP Virtual Console Spool Logger...\n");
    tsfi_cp_console_spool c_spool;
    tsfi_cp_console_spool_init(&c_spool);
    assert(c_spool.is_spooling == 0);
    
    tsfi_cp_console_spool_start(&c_spool);
    assert(c_spool.is_spooling == 1);
    
    tsfi_cp_console_spool_write(&c_spool, "MSG TO CONSOLE: IPL CMS\n");
    tsfi_cp_console_spool_write(&c_spool, "MSG TO CONSOLE: ACCESS 191 A\n");
    assert(c_spool.log_len > 0);
    assert(strstr(c_spool.log_buffer, "IPL CMS") != NULL);
    
    tsfi_cp_console_spool_stop(&c_spool);
    assert(c_spool.is_spooling == 0);
    printf("  [PASS] VM/370 interactive console spool logging verified.\n");

    // 93. VM/370 CP Scheduler Q1 & Q2 CPU Queues Verification
    printf("[Test] Verifying VM/370 CP Scheduler task states...\n");
    tsfi_cp_scheduler scheduler;
    tsfi_cp_scheduler_init(&scheduler);
    assert(scheduler.task_count == 0);
    
    assert(tsfi_cp_scheduler_register(&scheduler, "VM_USER_A", VM_QUEUE_Q1) == 0);
    assert(tsfi_cp_scheduler_register(&scheduler, "VM_USER_B", VM_QUEUE_Q1) == 0);
    assert(scheduler.task_count == 2);
    
    assert(tsfi_cp_scheduler_dispatch(&scheduler, 0, 20) == 0);
    assert(scheduler.tasks[0].queue_type == VM_QUEUE_Q1);
    
    assert(tsfi_cp_scheduler_dispatch(&scheduler, 1, 80) == 0);
    assert(scheduler.tasks[1].queue_type == VM_QUEUE_Q2);
    printf("  [PASS] VM/370 multi-level queue CPU time allocation scheduler verified.\n");

    // 94. VM/370 CP Directory Privilege Controller Verification
    printf("[Test] Verifying VM/370 CP Directory user privileges...\n");
    tsfi_cp_directory directory;
    tsfi_cp_directory_init(&directory);
    
    assert(tsfi_cp_directory_add(&directory, "ADMIN", 'A', 65536) == 0);
    assert(tsfi_cp_directory_add(&directory, "OPERATOR", 'G', 16384) == 0);
    
    assert(tsfi_cp_directory_check(&directory, "ADMIN", 'A') == 0);
    
    assert(tsfi_cp_directory_check(&directory, "OPERATOR", 'A') == -2);
    
    assert(tsfi_cp_directory_check(&directory, "GUEST", 'G') == -1);
    printf("  [PASS] VM/370 hypervisor directory user privileges verified.\n");

    // 95. VM/370 CP Device Attachment Manager Verification
    printf("[Test] Verifying VM/370 CP device attachments...\n");
    tsfi_cp_attachment_manager attach_mgr;
    tsfi_cp_attachment_init(&attach_mgr);
    assert(attach_mgr.device_count == 0);
    
    // Register physical address
    assert(tsfi_cp_attachment_register(&attach_mgr, 0x191) == 0);
    assert(attach_mgr.device_count == 1);
    
    // Attach physical 0x191 to user VM1 as virtual 0x091
    assert(tsfi_cp_attach(&attach_mgr, 0x191, "VM1", 0x091) == 0);
    assert(attach_mgr.devices[0].is_attached == 1);
    
    // Try to attach again
    assert(tsfi_cp_attach(&attach_mgr, 0x191, "VM2", 0x092) == -2);
    
    // Detach with wrong owner
    assert(tsfi_cp_detach(&attach_mgr, 0x091, "VM2") == -2);
    
    // Detach successfully
    assert(tsfi_cp_detach(&attach_mgr, 0x091, "VM1") == 0);
    assert(attach_mgr.devices[0].is_attached == 0);
    printf("  [PASS] VM/370 CP device attachment and detach release verified.\n");

    // 96. VM/370 CP SMSG Queue Processor Verification
    printf("[Test] Verifying VM/370 CP Special Messages (SMSG)...\n");
    tsfi_cp_smsg_receiver service_vm;
    tsfi_cp_smsg_init(&service_vm, "SPOOLER");
    assert(service_vm.smsg_enabled == 1);
    assert(service_vm.count == 0);
    
    // Send message to SPOOLER VM
    assert(tsfi_cp_smsg_send(&service_vm, "PRINT FILE 101") == 0);
    assert(service_vm.count == 1);
    
    // Receive message
    char smsg_buf[64];
    assert(tsfi_cp_smsg_receive(&service_vm, smsg_buf) == 0);
    assert(strcmp(smsg_buf, "PRINT FILE 101") == 0);
    assert(service_vm.count == 0);
    
    // Disable receipt and verify send fails
    service_vm.smsg_enabled = 0;
    assert(tsfi_cp_smsg_send(&service_vm, "PRINT FILE 102") == -1);
    printf("  [PASS] VM/370 CP special message instant routing queue verified.\n");

    // 97. VM/370 CP Virtual CPU Controller Verification
    printf("[Test] Verifying VM/370 CP Virtual CPU (VCPU) controller...\n");
    tsfi_cp_vcpu vcpu;
    tsfi_cp_vcpu_init(&vcpu);
    assert(vcpu.state == VCPU_STOPPED);
    
    // Start processor execution
    assert(tsfi_cp_vcpu_control(&vcpu, "START") == 0);
    assert(vcpu.state == VCPU_RUNNING);
    
    // Simulate some work address offset update
    vcpu.psw_instruction_address = 0x00FF8000;
    
    // Stop processor execution
    assert(tsfi_cp_vcpu_control(&vcpu, "STOP") == 0);
    assert(vcpu.state == VCPU_STOPPED);
    assert(vcpu.psw_instruction_address == 0x00FF8000);
    
    // Reset CPU registers
    assert(tsfi_cp_vcpu_control(&vcpu, "RESET") == 0);
    assert(vcpu.state == VCPU_STOPPED);
    assert(vcpu.psw_instruction_address == 0x00000000);
    printf("  [PASS] VM/370 CP virtual CPU execution state transitions verified.\n");

    // 98. VM/370 CP Spool Printer Carriage Controller Verification
    printf("[Test] Verifying VM/370 CP Spool Printer Carriage...\n");
    tsfi_cp_spool_printer printer;
    tsfi_cp_printer_init(&printer);
    assert(printer.page_count == 1);
    assert(printer.line_count == 0);
    
    // Write double-spaced line
    assert(tsfi_cp_printer_write_record(&printer, "0DOUBLE SPACE LINE") == 0);
    assert(printer.line_count == 2);
    assert(printer.last_skip_count == 2);
    
    // Write page eject
    assert(tsfi_cp_printer_write_record(&printer, "1NEW PAGE HEADER") == 0);
    assert(printer.page_count == 2);
    assert(printer.line_count == 0);
    printf("  [PASS] VM/370 CP spooled print carriage control vertical skips verified.\n");

    // 99. VM/370 CP Spool Class Filter Verification
    printf("[Test] Verifying VM/370 CP Spool Class filtering...\n");
    tsfi_cp_spool_queue_v2 filter_q;
    tsfi_cp_spool_queue_v2_init(&filter_q, 'A');
    
    // Push different classes
    assert(tsfi_cp_spool_push_v2(&filter_q, "CARD A1", 'A') == 0);
    assert(tsfi_cp_spool_push_v2(&filter_q, "CARD B1", 'B') == 0);
    assert(tsfi_cp_spool_push_v2(&filter_q, "CARD A2", 'A') == 0);
    assert(filter_q.count == 3);
    
    // Pop: first should yield A1
    char pop_buf[80];
    assert(tsfi_cp_spool_pop_v2(&filter_q, pop_buf) == 0);
    assert(strcmp(pop_buf, "CARD A1") == 0);
    
    // Pop: next should yield A2 (skipping B1 because queue is filtered to Class A)
    assert(tsfi_cp_spool_pop_v2(&filter_q, pop_buf) == 0);
    assert(strcmp(pop_buf, "CARD A2") == 0);
    
    // Try to pop again: no Class A cards left
    assert(tsfi_cp_spool_pop_v2(&filter_q, pop_buf) == -2);
    
    // Set filter to '*' and pop B1
    filter_q.active_reader_class = '*';
    assert(tsfi_cp_spool_pop_v2(&filter_q, pop_buf) == 0);
    assert(strcmp(pop_buf, "CARD B1") == 0);
    printf("  [PASS] VM/370 CP spool file reader class filters verified.\n");

    // 100. VM/370 CP Spool File Hold Manager Verification
    printf("[Test] Verifying VM/370 CP Spool File Hold / Release...\n");
    tsfi_cp_spool_queue_v3 hold_q;
    tsfi_cp_spool_queue_v3_init(&hold_q);
    
    // Push spool files
    assert(tsfi_cp_spool_push_v3(&hold_q, "CARD DATA 1", 101) == 0);
    assert(tsfi_cp_spool_push_v3(&hold_q, "CARD DATA 2", 102) == 0);
    assert(hold_q.count == 2);
    
    // Set file 101 to HOLD
    assert(tsfi_cp_spool_hold(&hold_q, 101, 1) == 0);
    
    // Pop: should bypass held file 101 and return card 2 (file 102)
    char hold_buf[80];
    assert(tsfi_cp_spool_pop_v3(&hold_q, hold_buf) == 0);
    assert(strcmp(hold_buf, "CARD DATA 2") == 0);
    assert(hold_q.count == 1);
    
    // Pop again: should fail with -2 because remaining file 101 is held
    assert(tsfi_cp_spool_pop_v3(&hold_q, hold_buf) == -2);
    
    // Release file 101 and pop successfully
    assert(tsfi_cp_spool_hold(&hold_q, 101, 0) == 0);
    assert(tsfi_cp_spool_pop_v3(&hold_q, hold_buf) == 0);
    assert(strcmp(hold_buf, "CARD DATA 1") == 0);
    assert(hold_q.count == 0);
    printf("  [PASS] VM/370 CP spool file hold and release states verified.\n");

    // 101. VM/370 CP Spool Purging Manager Verification
    printf("[Test] Verifying VM/370 CP Spool Purging...\n");
    tsfi_cp_spool_queue_v3 rdr_q;
    tsfi_cp_spool_queue_v3_init(&rdr_q);
    assert(tsfi_cp_spool_push_v3(&rdr_q, "PURGE DATA", 201) == 0);
    assert(rdr_q.count == 1);
    
    tsfi_cp_spool_printer prt_device;
    tsfi_cp_printer_init(&prt_device);
    assert(tsfi_cp_printer_write_record(&prt_device, "0LINE TO BE PURGED") == 0);
    assert(prt_device.line_count == 2);
    
    tsfi_cp_purge_stats purge_stats;
    tsfi_cp_purge_stats_init(&purge_stats);
    
    // Execute Purge Reader
    assert(tsfi_cp_execute_purge(&purge_stats, &rdr_q, &prt_device, "PURGE RDR") == 0);
    assert(rdr_q.count == 0);
    assert(purge_stats.reader_count == 1);
    
    // Execute Purge All
    assert(tsfi_cp_execute_purge(&purge_stats, &rdr_q, &prt_device, "PURGE ALL") == 0);
    assert(prt_device.line_count == 0);
    assert(purge_stats.printer_count == 2);
    assert(purge_stats.total_purged == 3);
    printf("  [PASS] VM/370 CP spool queue purging and memory resets verified.\n");

    // 102. VM/370 CP Resource Query Manager Verification
    printf("[Test] Verifying VM/370 CP Resource Queries...\n");
    tsfi_cp_query_manager query_mgr;
    tsfi_cp_query_init(&query_mgr);
    
    char q_buf[128];
    assert(tsfi_cp_query_execute(&query_mgr, "QUERY TIME", q_buf, sizeof(q_buf)) == 0);
    assert(strstr(q_buf, "CONNECT TIME") != NULL);
    
    assert(tsfi_cp_query_execute(&query_mgr, "QUERY NAMES", q_buf, sizeof(q_buf)) == 0);
    assert(strstr(q_buf, "ACTIVE USERS") != NULL);
    
    assert(tsfi_cp_query_execute(&query_mgr, "INVALID QUERY", q_buf, sizeof(q_buf)) == -1);
    printf("  [PASS] VM/370 CP console queries parsed and resolved successfully.\n");

    // 103. VM/370 CP MSG Console Message Router Verification
    printf("[Test] Verifying VM/370 CP MSG routing...\n");
    tsfi_cp_directory dir_inst;
    tsfi_cp_directory_init(&dir_inst);
    assert(tsfi_cp_directory_add(&dir_inst, "USER1", 'G', 8192) == 0);
    assert(tsfi_cp_directory_add(&dir_inst, "USER2", 'G', 8192) == 0);
    
    char msg_term[128];
    // Route from USER1 to USER2
    assert(tsfi_cp_msg_send(&dir_inst, "USER1", "USER2", "HELLO USER2", msg_term, sizeof(msg_term)) == 0);
    assert(strcmp(msg_term, "MSG FROM USER1: HELLO USER2") == 0);
    
    // Route to missing user
    assert(tsfi_cp_msg_send(&dir_inst, "USER1", "USER3", "HELLO USER3", msg_term, sizeof(msg_term)) == -1);
    assert(strstr(msg_term, "NOT LOGGED ON") != NULL);
    printf("  [PASS] VM/370 CP instant terminal text message routing verified.\n");

    // 104. VM/370 CP WARNING Broadcast Manager Verification
    printf("[Test] Verifying VM/370 CP WARNING broadcasts...\n");
    tsfi_cp_directory dir_brd;
    tsfi_cp_directory_init(&dir_brd);
    assert(tsfi_cp_directory_add(&dir_brd, "ADMIN", 'A', 65536) == 0);
    assert(tsfi_cp_directory_add(&dir_brd, "USER1", 'G', 8192) == 0);
    
    char warn_terms[8][128];
    int brd_count = 0;
    
    // Broadcast from ADMIN (Class A)
    assert(tsfi_cp_warning_broadcast(&dir_brd, "ADMIN", "SHUTDOWN IN 5 MIN", warn_terms, &brd_count) == 0);
    assert(brd_count == 2);
    assert(strcmp(warn_terms[0], "WARN: SHUTDOWN IN 5 MIN") == 0);
    assert(strcmp(warn_terms[1], "WARN: SHUTDOWN IN 5 MIN") == 0);
    
    // Attempt broadcast from USER1 (Class G - non-admin)
    assert(tsfi_cp_warning_broadcast(&dir_brd, "USER1", "SHUTDOWN IN 5 MIN", warn_terms, &brd_count) == -2);
    printf("  [PASS] VM/370 CP console warning broadcast permissions verified.\n");

    // 105. VM/370 CP Virtual Terminal Sleep Manager Verification
    printf("[Test] Verifying VM/370 CP Terminal Sleep states...\n");
    tsfi_cp_terminal_sleep sleep_state;
    tsfi_cp_sleep_init(&sleep_state);
    assert(sleep_state.is_sleeping == 0);
    
    // Start terminal sleep
    assert(tsfi_cp_sleep_start(&sleep_state, 10) == 0);
    assert(sleep_state.is_sleeping == 1);
    assert(sleep_state.remaining_seconds == 10);
    
    // Tick timer down
    assert(tsfi_cp_sleep_tick(&sleep_state) == 9);
    
    // Interrupt sleep
    assert(tsfi_cp_sleep_interrupt(&sleep_state) == 0);
    assert(sleep_state.is_sleeping == 0);
    assert(sleep_state.remaining_seconds == 0);
    printf("  [PASS] VM/370 CP virtual terminal sleep countdown and interrupt verified.\n");

    // 106. VM/370 CP Disconnect/Connect Session Monitor Verification
    printf("[Test] Verifying VM/370 CP Disconnect session monitor...\n");
    tsfi_cp_active_session session;
    tsfi_cp_active_session_init(&session, "VM_USER_A");
    assert(session.is_connected == 1);
    assert(session.background_cycles_run == 0);
    
    // Disconnect terminal session
    assert(tsfi_cp_active_session_disconnect(&session) == 0);
    assert(session.is_connected == 0);
    
    // Background scheduler cycle dispatching
    assert(tsfi_cp_active_session_dispatch(&session, 500) == 500);
    assert(session.background_cycles_run == 500);
    
    // Connect terminal back
    assert(tsfi_cp_active_session_connect(&session) == 0);
    assert(session.is_connected == 1);
    printf("  [PASS] VM/370 CP session disconnect/reconnect and dispatch routing verified.\n");

    // 107. VM/370 CP Terminal Options Configurator Verification
    printf("[Test] Verifying VM/370 CP Terminal Options...\n");
    tsfi_cp_terminal_options t_opts;
    tsfi_cp_term_opts_init(&t_opts);
    
    char processed[64];
    // Test character delete backspacing
    tsfi_cp_term_opts_process(&t_opts, "CMS@ COMMAND", processed, sizeof(processed));
    assert(strcmp(processed, "CM COMMAND") == 0);
    
    // Test line delete resetting
    tsfi_cp_term_opts_process(&t_opts, "CMS COMMAND#NEW COMMAND", processed, sizeof(processed));
    assert(strcmp(processed, "NEW COMMAND") == 0);
    
    // Disable CHARDEL option and verify '@' is treated literally
    assert(tsfi_cp_term_opts_config(&t_opts, "CHARDEL", 0) == 0);
    tsfi_cp_term_opts_process(&t_opts, "CMS@ COMMAND", processed, sizeof(processed));
    assert(strcmp(processed, "CMS@ COMMAND") == 0);
    printf("  [PASS] VM/370 CP terminal CHARDEL/LINEDEL option filters verified.\n");

    // 108. VM/370 CP Minidisk Link Manager Verification
    printf("[Test] Verifying VM/370 CP Minidisk Linkages...\n");
    tsfi_cp_link_manager link_mgr;
    tsfi_cp_link_init(&link_mgr);
    
    // Register disks
    assert(tsfi_cp_link_register(&link_mgr, "MAINT", 191, "MAINT_WRITE") == 0);
    assert(link_mgr.disk_count == 1);
    
    // RR link (should succeed without password)
    assert(tsfi_cp_link_execute(&link_mgr, "USER1", "MAINT", 191, 291, "RR", NULL) == 0);
    assert(link_mgr.links[0].has_write_access == 0);
    
    // MR link password mismatch
    assert(tsfi_cp_link_execute(&link_mgr, "USER1", "MAINT", 191, 292, "MR", "WRONG_PWD") == -2);
    
    // MR link correct password
    assert(tsfi_cp_link_execute(&link_mgr, "USER1", "MAINT", 191, 292, "MR", "MAINT_WRITE") == 0);
    assert(link_mgr.links[1].has_write_access == 1);
    printf("  [PASS] VM/370 CP minidisk sharing linkages and permissions verified.\n");

    // 109. VM/370 CP QUERY DISK Command Verification
    printf("[Test] Verifying VM/370 CP QUERY DISK Command...\n");
    char disk_report[256];
    
    // Verify query output with registered links
    assert(tsfi_cp_query_disk(&link_mgr, disk_report, sizeof(disk_report)) == 0);
    assert(strstr(disk_report, "OWNER=MAINT") != NULL);
    assert(strstr(disk_report, "ACCESS=RW") != NULL);
    assert(strstr(disk_report, "ACCESS=RO") != NULL);
    
    // Verify empty link manager reports correct warning
    tsfi_cp_link_manager empty_mgr;
    tsfi_cp_link_init(&empty_mgr);
    assert(tsfi_cp_query_disk(&empty_mgr, disk_report, sizeof(disk_report)) == 0);
    assert(strcmp(disk_report, "NO ACTIVE MINIDISKS") == 0);
    printf("  [PASS] VM/370 CP minidisk status queries parsed and listed successfully.\n");

    // 110. VM/370 CP Dynamic Device Definer Verification
    printf("[Test] Verifying VM/370 CP Dynamic Device Definitions...\n");
    tsfi_cp_device_table dev_tbl;
    tsfi_cp_device_table_init(&dev_tbl);
    assert(dev_tbl.count == 0);
    
    // Define virtual devices
    assert(tsfi_cp_device_define(&dev_tbl, "GRAF", 0x090) == 0);
    assert(tsfi_cp_device_define(&dev_tbl, "LINE", 0x091) == 0);
    assert(dev_tbl.count == 2);
    
    // Define invalid device type
    assert(tsfi_cp_device_define(&dev_tbl, "PRT", 0x092) == -2);
    
    // Define duplicate address
    assert(tsfi_cp_device_define(&dev_tbl, "GRAF", 0x090) == -3);
    printf("  [PASS] VM/370 CP dynamic device definition tables and duplicates verified.\n");

    // 111. VM/370 CP QUERY VIRTUAL Command Verification
    printf("[Test] Verifying VM/370 CP QUERY VIRTUAL Command...\n");
    char dev_report[256];
    
    // Verify query output with registered devices
    assert(tsfi_cp_query_virtual(&dev_tbl, dev_report, sizeof(dev_report)) == 0);
    assert(strstr(dev_report, "090: TYPE=GRAF") != NULL);
    assert(strstr(dev_report, "091: TYPE=LINE") != NULL);
    
    // Verify empty device table reports correct warning
    tsfi_cp_device_table empty_tbl;
    tsfi_cp_device_table_init(&empty_tbl);
    assert(tsfi_cp_query_virtual(&empty_tbl, dev_report, sizeof(dev_report)) == 0);
    assert(strcmp(dev_report, "NO VIRTUAL DEVICES") == 0);
    printf("  [PASS] VM/370 CP virtual device status queries listed successfully.\n");

    // 112. VM/370 CP LOGOUT Session Cleanup Verification
    printf("[Test] Verifying VM/370 CP Session Logout cleanup...\n");
    tsfi_cp_terminal_sleep sleep_logout;
    tsfi_cp_sleep_init(&sleep_logout);
    assert(tsfi_cp_sleep_start(&sleep_logout, 5) == 0);
    
    tsfi_cp_logout_execute(&dev_tbl, &link_mgr, &sleep_logout);
    assert(dev_tbl.count == 0);
    assert(link_mgr.link_count == 0);
    assert(sleep_logout.is_sleeping == 0);
    assert(sleep_logout.remaining_seconds == 0);
    printf("  [PASS] VM/370 CP session logout resource releases verified.\n");

    // 113. VM/370 CP Spool Card Punch Queue Verification
    printf("[Test] Verifying VM/370 CP Card Punch spooler...\n");
    tsfi_cp_punch_spooler punch_spl;
    tsfi_cp_punch_spooler_init(&punch_spl);
    assert(punch_spl.card_count == 0);
    assert(punch_spl.is_held == 0);
    
    assert(tsfi_cp_punch_write(&punch_spl, "DATA RECORD 1") == 0);
    assert(tsfi_cp_punch_write(&punch_spl, "DATA RECORD 2") == 0);
    assert(punch_spl.card_count == 2);
    
    assert(tsfi_cp_punch_set_hold(&punch_spl, 1) == 0);
    int flushed_count = 0;
    assert(tsfi_cp_punch_flush(&punch_spl, &flushed_count) == -2);
    
    assert(tsfi_cp_punch_set_hold(&punch_spl, 0) == 0);
    assert(tsfi_cp_punch_flush(&punch_spl, &flushed_count) == 0);
    assert(flushed_count == 2);
    assert(punch_spl.card_count == 0);
    printf("  [PASS] VM/370 CP card punch spooler write/hold/flush states verified.\n");

    // 114. CMS JCL FILEDEF Dataset Binder Verification
    printf("[Test] Verifying CMS JCL FILEDEF Dataset Binder...\n");
    tsfi_cms_filedef_table fd_tbl;
    tsfi_cms_filedef_init(&fd_tbl);
    assert(fd_tbl.count == 0);
    
    assert(tsfi_cms_filedef_bind(&fd_tbl, "INFILE", "MY.INPUT.DATA", "DISK") == 0);
    assert(tsfi_cms_filedef_bind(&fd_tbl, "OUTFILE", "MY.OUTPUT.DATA", "PRINTER") == 0);
    assert(fd_tbl.count == 2);
    
    char resolved_dsname[64];
    assert(tsfi_cms_filedef_resolve(&fd_tbl, "INFILE", resolved_dsname, sizeof(resolved_dsname)) == 0);
    assert(strcmp(resolved_dsname, "MY.INPUT.DATA") == 0);
    
    assert(tsfi_cms_filedef_bind(&fd_tbl, "INFILE", "NEW.INPUT.DATA", "DISK") == 0);
    assert(tsfi_cms_filedef_resolve(&fd_tbl, "INFILE", resolved_dsname, sizeof(resolved_dsname)) == 0);
    assert(strcmp(resolved_dsname, "NEW.INPUT.DATA") == 0);
    assert(fd_tbl.count == 2);
    printf("  [PASS] CMS FILEDEF logical-to-physical device bindings verified.\n");
    
    // 115. VM/370 RSCS Network Node Spool Linker Verification
    printf("[Test] Verifying VM/370 RSCS network node spool routing...\n");
    tsfi_rscs_manager rscs_mgr;
    tsfi_rscs_init(&rscs_mgr);
    assert(rscs_mgr.node_count == 0);
    
    assert(tsfi_rscs_add_node(&rscs_mgr, "NYCVM") == 0);
    assert(tsfi_rscs_add_node(&rscs_mgr, "CHIVM") == 0);
    assert(rscs_mgr.node_count == 2);
    
    assert(tsfi_rscs_route_spool(&rscs_mgr, "NYCVM", 1001) == 0);
    assert(rscs_mgr.nodes[0].routed_files == 1);
    
    assert(tsfi_rscs_deactivate_node(&rscs_mgr, "NYCVM") == 0);
    assert(tsfi_rscs_route_spool(&rscs_mgr, "NYCVM", 1002) == -2);
    
    assert(tsfi_rscs_route_spool(&rscs_mgr, "SFVM", 1003) == -1);
    printf("  [PASS] VM/370 RSCS spool packet routing loops verified.\n");

    // 116. VM/370 CP Forms Control Buffer (FCB) Spool Filter Verification
    printf("[Test] Verifying VM/370 CP Forms Control Buffer (FCB) Filter...\n");
    tsfi_cp_fcb fcb_state;
    tsfi_cp_fcb_init(&fcb_state);
    assert(strcmp(fcb_state.fcb_name, "DEFAULT") == 0);
    assert(fcb_state.page_length_lines == 66);
    
    assert(tsfi_cp_fcb_load(&fcb_state, "STD8", 88) == 0);
    assert(strcmp(fcb_state.fcb_name, "STD8") == 0);
    assert(fcb_state.page_length_lines == 88);
    
    assert(tsfi_cp_fcb_set_channel(&fcb_state, 1, 5) == 0);
    assert(tsfi_cp_fcb_set_channel(&fcb_state, 12, 80) == 0);
    assert(fcb_state.channel_stops[0] == 5);
    assert(fcb_state.channel_stops[11] == 80);
    
    assert(tsfi_cp_fcb_set_channel(&fcb_state, 13, 10) == -1);
    assert(tsfi_cp_fcb_set_channel(&fcb_state, 2, 90) == -1);
    printf("  [PASS] VM/370 CP forms control buffer page boundaries verified.\n");

    // 117. VM/370 CP Spool Class Router Verification
    printf("[Test] Verifying VM/370 CP Spool Class Router...\n");
    tsfi_cp_spool_class_router class_router;
    tsfi_cp_spool_router_init(&class_router);
    assert(class_router.count == 0);
    
    assert(tsfi_cp_spool_router_set(&class_router, "00E", 'A') == 0);
    assert(tsfi_cp_spool_router_set(&class_router, "00D", '*') == 0);
    assert(class_router.count == 2);
    
    assert(tsfi_cp_spool_router_set(&class_router, "00C", '1') == -2);
    
    assert(tsfi_cp_spool_router_match(&class_router, "00E", 'A') == 1);
    assert(tsfi_cp_spool_router_match(&class_router, "00E", 'B') == 0);
    assert(tsfi_cp_spool_router_match(&class_router, "00D", 'B') == 1);
    assert(tsfi_cp_spool_router_match(&class_router, "00C", 'B') == 1);
    printf("  [PASS] VM/370 CP spool device class filtering patterns verified.\n");

    // 118. VM/370 CP Spool Transfer Manager Verification
    printf("[Test] Verifying VM/370 CP Spool Transfer Manager...\n");
    tsfi_cp_spool_queue_v3 src_q, dst_q;
    tsfi_cp_spool_queue_v3_init(&src_q);
    tsfi_cp_spool_queue_v3_init(&dst_q);
    
    assert(tsfi_cp_spool_push_v3(&src_q, "CARD DATA 1", 101) == 0);
    assert(tsfi_cp_spool_push_v3(&src_q, "CARD DATA 2", 102) == 0);
    assert(tsfi_cp_spool_push_v3(&src_q, "CARD DATA 3", 103) == 0);
    assert(src_q.count == 3);
    
    assert(tsfi_cp_spool_transfer(&src_q, &dst_q, 102) == 0);
    assert(src_q.count == 2);
    assert(dst_q.count == 1);
    assert(dst_q.queue[0].file_id == 102);
    
    assert(tsfi_cp_spool_transfer(&src_q, &dst_q, -1) == 0);
    assert(src_q.count == 0);
    assert(dst_q.count == 3);
    printf("  [PASS] VM/370 CP spool transfer queue routing and compaction verified.\n");

    // 119. VM/370 Release 3 Discontiguous Shared Segment (DCSS) Manager Verification
    printf("[Test] Verifying VM/370 Release 3 Shared Segments...\n");
    tsfi_cp_dcss_manager cmd_dcss_mgr;
    tsfi_cp_dcss_init(&cmd_dcss_mgr);
    assert(cmd_dcss_mgr.count == 0);
    
    assert(tsfi_cp_dcss_register(&cmd_dcss_mgr, "CMSVSAM", 0x100000, 0x1FFFFF) == 0);
    assert(tsfi_cp_dcss_register(&cmd_dcss_mgr, "CMSBAM", 0x200000, 0x27FFFF) == 0);
    assert(cmd_dcss_mgr.count == 2);
    
    int seg_idx = tsfi_cp_dcss_diagnose_find(&cmd_dcss_mgr, "CMSVSAM");
    assert(seg_idx == 0);
    assert(cmd_dcss_mgr.segments[seg_idx].is_loaded == 0);
    
    assert(tsfi_cp_dcss_diagnose_load(&cmd_dcss_mgr, "CMSVSAM") == 0);
    assert(cmd_dcss_mgr.segments[seg_idx].is_loaded == 1);
    
    assert(tsfi_cp_dcss_diagnose_purge(&cmd_dcss_mgr, "CMSVSAM") == 0);
    assert(cmd_dcss_mgr.segments[seg_idx].is_loaded == 0);
    printf("  [PASS] VM/370 Release 3 discontiguous shared segment dynamic overlays verified.\n");
    
    // 120. CMS VSAM Indexed Access Simulator Verification
    printf("[Test] Verifying CMS VSAM Indexed Access Simulator...\n");
    tsfi_vsam_file vsam;
    tsfi_vsam_init(&vsam);
    assert(vsam.is_opened == 0);
    
    assert(tsfi_vsam_put(&vsam, "KEY01", "VAL01") == -1);
    
    assert(tsfi_vsam_open(&vsam) == 0);
    assert(tsfi_vsam_put(&vsam, "KEY02", "VAL02") == 0);
    assert(tsfi_vsam_put(&vsam, "KEY01", "VAL01") == 0);
    assert(vsam.count == 2);
    assert(strcmp(vsam.records[0].key, "KEY01") == 0);
    assert(strcmp(vsam.records[1].key, "KEY02") == 0);
    
    char vsam_out[64];
    assert(tsfi_vsam_get(&vsam, "KEY01", vsam_out, sizeof(vsam_out)) == 0);
    assert(strcmp(vsam_out, "VAL01") == 0);
    
    assert(tsfi_vsam_put(&vsam, "KEY01", "VAL01_NEW") == 0);
    assert(tsfi_vsam_get(&vsam, "KEY01", vsam_out, sizeof(vsam_out)) == 0);
    assert(strcmp(vsam_out, "VAL01_NEW") == 0);
    
    assert(tsfi_vsam_close(&vsam) == 0);
    printf("  [PASS] CMS VSAM indexed KSDS queries and sorting invariants verified.\n");

    // 121. VM/370 Release 3 Virtual Machine Assist (VMA) Verification
    printf("[Test] Verifying VM/370 Release 3 Virtual Machine Assist (VMA)...\n");
    tsfi_cp_vma_controller vma_ctrl;
    tsfi_cp_vma_init(&vma_ctrl);
    assert(vma_ctrl.assist_enabled == 0);
    assert(vma_ctrl.software_intercepts == 0);
    assert(vma_ctrl.assisted_instructions == 0);
    
    assert(tsfi_cp_vma_execute(&vma_ctrl, "SVC") == 1);
    assert(vma_ctrl.software_intercepts == 1);
    assert(vma_ctrl.assisted_instructions == 0);
    
    assert(tsfi_cp_vma_set(&vma_ctrl, 1) == 0);
    assert(tsfi_cp_vma_execute(&vma_ctrl, "SVC") == 0);
    assert(tsfi_cp_vma_execute(&vma_ctrl, "LPSW") == 0);
    assert(vma_ctrl.software_intercepts == 1);
    assert(vma_ctrl.assisted_instructions == 2);
    
    assert(tsfi_cp_vma_execute(&vma_ctrl, "BALR") == -1);
    printf("  [PASS] VM/370 CP virtual machine assist bypass intercepts verified.\n");

    // 122. VM/370 APL 3270 Copy Screen Printer Formatter Verification
    printf("[Test] Verifying VM/370 APL 3270 Copy Screen Formatter...\n");
    tsfi_cp_apl_screen apl_scr;
    tsfi_cp_apl_screen_init(&apl_scr);
    
    assert(tsfi_cp_apl_screen_write(&apl_scr, 0, 0, "APL WORKSPACE") == 0);
    assert(tsfi_cp_apl_screen_write(&apl_scr, 2, 5, "x \x01 y") == 0);
    
    tsfi_cp_spool_printer printer_dev;
    tsfi_cp_printer_init(&printer_dev);
    
    assert(tsfi_cp_apl_copy_to_printer(&apl_scr, &printer_dev) == 0);
    assert(printer_dev.line_count == 24);
    printf("  [PASS] VM/370 CP APL console screen translation spool writes verified.\n");

    // 123. VM/370 Release 3 CP Scheduler Share Configurator Verification
    printf("[Test] Verifying VM/370 CP Scheduler Share Configurator...\n");
    tsfi_cp_share_table shares;
    tsfi_cp_share_init(&shares);
    assert(shares.count == 0);
    
    assert(tsfi_cp_share_set(&shares, "USER01", 0, 100) == 0);
    assert(tsfi_cp_share_set(&shares, "USER02", 1, 20) == 0);
    assert(shares.count == 2);
    
    assert(tsfi_cp_share_set(&shares, "USER03", 1, 105) == -2);
    assert(tsfi_cp_share_set(&shares, "USER03", 0, 20000) == -3);
    
    int share_slice_ms = 0;
    assert(tsfi_cp_share_calculate_slice(&shares, "USER02", 400, &share_slice_ms) == 0);
    assert(share_slice_ms == 200);
    
    assert(tsfi_cp_share_calculate_slice(&shares, "USER01", 400, &share_slice_ms) == 0);
    assert(share_slice_ms == 250);
    printf("  [PASS] VM/370 CP scheduler share timeslices verified.\n");

    // 124. VM/370 Release 4 IBM 3850 Mass Storage System (MSS) Sharing Linker Verification
    printf("[Test] Verifying VM/370 Release 4 IBM 3850 MSS Sharing...\n");
    tsfi_cp_mss_manager mss;
    tsfi_cp_mss_init(&mss);
    assert(mss.count == 0);
    
    assert(tsfi_cp_mss_register_group(&mss, "MSSGRP01") == 0);
    assert(tsfi_cp_mss_register_group(&mss, "MSSGRP02") == 0);
    assert(mss.count == 2);
    
    char owner[16];
    assert(tsfi_cp_mss_query(&mss, "MSSGRP01", owner, sizeof(owner)) == 0);
    
    assert(tsfi_cp_mss_mount(&mss, "MSSGRP01", "GUESTVM1") == 0);
    assert(tsfi_cp_mss_query(&mss, "MSSGRP01", owner, sizeof(owner)) == 1);
    assert(strcmp(owner, "GUESTVM1") == 0);
    
    assert(tsfi_cp_mss_mount(&mss, "MSSGRP01", "GUESTVM2") == -2);
    
    assert(tsfi_cp_mss_unmount(&mss, "MSSGRP01", "GUESTVM2") == -2);
    assert(tsfi_cp_mss_unmount(&mss, "MSSGRP01", "GUESTVM1") == 0);
    assert(tsfi_cp_mss_query(&mss, "MSSGRP01", owner, sizeof(owner)) == 0);
    printf("  [PASS] VM/370 CP mass storage sharing mount locks verified.\n");

    // 125. VM/370 Release 4 CP IUCV Multi-Path Connector Verification
    printf("[Test] Verifying VM/370 Release 4 CP IUCV Multi-Path Connector...\n");
    tsfi_cp_iucv_multipath iucv_mp;
    tsfi_cp_iucv_mp_init(&iucv_mp);
    assert(iucv_mp.count == 0);
    
    assert(tsfi_cp_iucv_mp_connect(&iucv_mp, 10, "RSCS") == 0);
    assert(tsfi_cp_iucv_mp_connect(&iucv_mp, 20, "CMSVM") == 0);
    assert(iucv_mp.count == 2);
    
    char msg_target[16];
    assert(tsfi_cp_iucv_mp_send(&iucv_mp, 10, "SYSMSG", msg_target) == 0);
    assert(strcmp(msg_target, "RSCS") == 0);
    
    assert(tsfi_cp_iucv_mp_disconnect(&iucv_mp, 10) == 0);
    assert(tsfi_cp_iucv_mp_send(&iucv_mp, 10, "SYSMSG", msg_target) == -2);
    
    assert(tsfi_cp_iucv_mp_send(&iucv_mp, 30, "SYSMSG", msg_target) == -1);
    printf("  [PASS] VM/370 CP IUCV multi-path connection resolution verified.\n");

    // 126. VM/370 Release 5 CMS Command Resolution Engine Verification
    printf("[Test] Verifying VM/370 Release 5 CMS Command Resolution...\n");
    tsfi_cp_nucleus_table nuc_tbl;
    tsfi_cp_nucleus_init(&nuc_tbl);
    assert(tsfi_cp_nucleus_register(&nuc_tbl, "FILEDEF", 0x803000) == 0);
    assert(tsfi_cp_nucleus_register(&nuc_tbl, "LISTFILE", 0x804200) == 0);
    assert(nuc_tbl.count == 2);
    
    assert(tsfi_cp_dcss_register(&cmd_dcss_mgr, "CMSVSAM", 0x100000, 0x1FFFFF) == 0);
    
    char loc[16];
    uint32_t addr = 0;
    
    assert(tsfi_cp_resolve_command(&nuc_tbl, &cmd_dcss_mgr, "FILEDEF", loc, &addr) == 0);
    assert(strcmp(loc, "NUCLEUS") == 0);
    assert(addr == 0x803000);
    
    assert(tsfi_cp_resolve_command(&nuc_tbl, &cmd_dcss_mgr, "CMSVSAM", loc, &addr) == 1);
    assert(strcmp(loc, "DCSS") == 0);
    assert(addr == 0x100000);
    
    assert(tsfi_cp_resolve_command(&nuc_tbl, &cmd_dcss_mgr, "ERASE", loc, &addr) == -1);
    printf("  [PASS] VM/370 CP command resolution hierarchy verified.\n");

    // 127. VM/370 Release 5 CMS Command Parameter List (PLIST) Parser Verification
    printf("[Test] Verifying VM/370 CMS Command Parameter List (PLIST)...\n");
    tsfi_cms_plist plist;
    assert(tsfi_cms_plist_parse(&plist, "filedef in disk a") == 4);
    assert(plist.count == 4);
    assert(plist.sentinel == 0xFFFFFFFFFFFFFFFFULL);
    
    assert(memcmp(plist.words[0], "FILEDEF ", 8) == 0);
    assert(memcmp(plist.words[1], "IN      ", 8) == 0);
    assert(memcmp(plist.words[2], "DISK    ", 8) == 0);
    assert(memcmp(plist.words[3], "A       ", 8) == 0);
    printf("  [PASS] VM/370 CMS parameter list tokenization verified.\n");

    // 128. VM/370 Release 5 CMS Virtual Tape Utility Manager Verification
    printf("[Test] Verifying VM/370 CMS Virtual Tape Utility...\n");
    tsfi_cms_tape_drive virtual_tape;
    tsfi_cms_tape_init(&virtual_tape);
    assert(virtual_tape.block_count == 0);
    assert(virtual_tape.current_pointer == 0);
    
    assert(tsfi_cms_tape_write_record(&virtual_tape, "VOL1") == 0);
    assert(tsfi_cms_tape_write_record(&virtual_tape, "HDR1") == 0);
    assert(tsfi_cms_tape_write_mark(&virtual_tape) == 0);
    assert(tsfi_cms_tape_write_record(&virtual_tape, "DATA RECORD 1") == 0);
    assert(tsfi_cms_tape_write_mark(&virtual_tape) == 0);
    assert(virtual_tape.block_count == 5);
    
    char r_buf[80];
    int is_mark = 0;
    assert(tsfi_cms_tape_read_record(&virtual_tape, r_buf, sizeof(r_buf), &is_mark) == 0);
    assert(strcmp(r_buf, "VOL1") == 0);
    assert(is_mark == 0);
    
    assert(tsfi_cms_tape_skip_file(&virtual_tape, 1) == 0);
    assert(tsfi_cms_tape_read_record(&virtual_tape, r_buf, sizeof(r_buf), &is_mark) == 0);
    assert(strcmp(r_buf, "DATA RECORD 1") == 0);
    
    assert(tsfi_cms_tape_rewind(&virtual_tape) == 0);
    assert(virtual_tape.current_pointer == 0);
    printf("  [PASS] VM/370 CMS virtual tape write, marks, and skip actions verified.\n");

    // 129. z/VM Virtual Switch (VSwitch) Controller Verification
    printf("[Test] Verifying z/VM VSwitch Controller...\n");
    tsfi_zvm_vswitch_manager vsw_mgr;
    tsfi_zvm_vswitch_init(&vsw_mgr);
    
    // Define a VSwitch
    assert(tsfi_zvm_vswitch_define(&vsw_mgr, "VSWITCH1", 0x1000) == 0);
    // Duplicate define should fail
    assert(tsfi_zvm_vswitch_define(&vsw_mgr, "VSWITCH1", 0x1001) == -2);
    
    // Couple guests
    assert(tsfi_zvm_vswitch_couple(&vsw_mgr, "VSWITCH1", "LINUX01", 0x0600) == 0);
    assert(tsfi_zvm_vswitch_couple(&vsw_mgr, "VSWITCH1", "LINUX02", 0x0600) == 0);
    
    // Transmit packets
    assert(tsfi_zvm_vswitch_transmit(&vsw_mgr, "VSWITCH1", 50) == 0);
    assert(tsfi_zvm_vswitch_transmit(&vsw_mgr, "VSWITCH1", 75) == 0);
    
    // Query VSwitch state
    int ports = 0, packets = 0;
    assert(tsfi_zvm_vswitch_query(&vsw_mgr, "VSWITCH1", &ports, &packets) == 0);
    assert(ports == 2);
    assert(packets == 125);
    
    // Couple to unknown VSwitch should fail
    assert(tsfi_zvm_vswitch_couple(&vsw_mgr, "VSWITCH2", "LINUX01", 0x0600) == -1);
    printf("  [PASS] z/VM VSwitch definition, guest coupling, and packet transmission verified.\n");

    // 130. IBM 3031/3032 SIPL Controller Verification
    printf("[Test] Verifying IBM 3031/3032 SIPL Controller...\n");
    tsfi_zvm_sipl_controller sipl;
    tsfi_zvm_sipl_init(&sipl);
    
    // Register processors
    assert(tsfi_zvm_sipl_register_cpu(&sipl, 0) == 0);
    assert(tsfi_zvm_sipl_register_cpu(&sipl, 1) == 0);
    // Duplicate registration should fail
    assert(tsfi_zvm_sipl_register_cpu(&sipl, 0) == -2);
    
    // Send inter-processor signal
    assert(tsfi_zvm_sipl_send(&sipl, 0, 1, 0x000E0001) == 0);
    
    // Receive signal
    uint32_t signal_code = 0;
    assert(tsfi_zvm_sipl_receive(&sipl, 1, &signal_code) == 0);
    assert(signal_code == 0x000E0001);
    
    // Double receive should fail (no interrupts pending)
    assert(tsfi_zvm_sipl_receive(&sipl, 1, &signal_code) == -2);
    
    // Send to invalid processor should fail
    assert(tsfi_zvm_sipl_send(&sipl, 0, 3, 0x000E0001) == -1);
    printf("  [PASS] IBM 3031/3032 CPU registration and SIPL interrupt signalling verified.\n");

    // 131. CODASYL DDL Schema Analyzer Verification
    printf("[Test] Verifying CODASYL DDL Schema Analyzer...\n");
    tsfi_codasyl_schema schema;
    tsfi_codasyl_schema_init(&schema);
    
    // Parse valid blocks
    assert(tsfi_codasyl_schema_parse(&schema, "SCHEMA NAME IS SALESDB.") == 0);
    assert(tsfi_codasyl_schema_parse(&schema, "AREA NAME IS CORP-AREA.") == 0);
    assert(tsfi_codasyl_schema_parse(&schema, "RECORD NAME IS CUSTREC LENGTH IS 128.") == 0);
    assert(tsfi_codasyl_schema_parse(&schema, "RECORD NAME IS ORDERREC LENGTH IS 64.") == 0);
    
    // Parse valid set referring to valid records
    assert(tsfi_codasyl_schema_parse(&schema, "SET NAME IS CUST-ORDER OWNER IS CUSTREC MEMBER IS ORDERREC.") == 0);
    
    char err_buf[128];
    assert(tsfi_codasyl_schema_validate(&schema, err_buf, sizeof(err_buf)) == 0);
    
    // Test parsing a set with an undefined member record
    tsfi_codasyl_schema invalid_schema;
    tsfi_codasyl_schema_init(&invalid_schema);
    assert(tsfi_codasyl_schema_parse(&invalid_schema, "SCHEMA NAME IS BADDB.") == 0);
    assert(tsfi_codasyl_schema_parse(&invalid_schema, "RECORD NAME IS EMPREC LENGTH IS 80.") == 0);
    assert(tsfi_codasyl_schema_parse(&invalid_schema, "SET NAME IS EMP-JOB OWNER IS EMPREC MEMBER IS JOBREC.") == 0);
    
    // Validate should detect undefined member record "JOBREC"
    int val_res = tsfi_codasyl_schema_validate(&invalid_schema, err_buf, sizeof(err_buf));
    assert(val_res == -4);
    assert(strstr(err_buf, "UNDEFINED MEMBER JOBREC") != NULL);
    printf("  [PASS] CODASYL schema parsed records, sets, and caught invalid references successfully.\n");

    // 132. IBM Mainframe-to-Minicomputer FEP Red/Black Audit Channel Linker Verification
    printf("[Test] Verifying FEP Red/Black Audit Channel Linker...\n");
    tsfi_fep_channel fep_chan;
    tsfi_fep_init(&fep_chan, "FEP01");
    
    // Process Red Rail with correct parity
    // 0x03 has 2 set bits (even parity -> parity bit 0)
    assert(tsfi_fep_process_red_rail(&fep_chan, 0x03, 0) == 0);
    assert(fep_chan.audit.red_rail_valid == 1);
    
    // Process Red Rail with bad parity (should fail and increment parity error audit)
    assert(tsfi_fep_process_red_rail(&fep_chan, 0x03, 1) == -2);
    assert(fep_chan.audit.red_rail_valid == 0);
    assert(fep_chan.audit.parity_errors == 1);
    
    // Process Black Rail (valid timing sector)
    assert(tsfi_fep_process_black_rail(&fep_chan, 107) == 0);
    assert(fep_chan.audit.black_rail_valid == 1);
    assert(fep_chan.audit.timing_sector == 107);
    
    // Process Black Rail timing violation
    assert(tsfi_fep_process_black_rail(&fep_chan, 108) == -2);
    assert(fep_chan.audit.black_rail_valid == 0);
    
    // Query audit metrics
    int trans = 0, errs = 0, timing = 0;
    assert(tsfi_fep_query_audit(&fep_chan, &trans, &errs, &timing) == 0);
    assert(trans == 2); // 1 valid red, 1 valid black
    assert(errs == 1);
    assert(timing == 107);
    printf("  [PASS] FEP channel isolated Red/Black rail audit indicators validated.\n");

    // 133. Model Context Protocol (MCP) Client Channel Multiplexer Verification
    printf("[Test] Verifying Model Context Protocol (MCP) Multiplexer...\n");
    tsfi_mcp_multiplexer mcp_mux;
    tsfi_mcp_mux_init(&mcp_mux);
    
    // Register channels
    assert(tsfi_mcp_mux_register(&mcp_mux, 1, "CLIENT01") == 0);
    assert(tsfi_mcp_mux_register(&mcp_mux, 2, "CLIENT02") == 0);
    // Duplicate register should fail
    assert(tsfi_mcp_mux_register(&mcp_mux, 1, "CLIENT01_DUP") == -2);
    
    // Send standard requests (Red Rail)
    assert(tsfi_mcp_mux_send_request(&mcp_mux, 1, "tools/list", 1) == 0);
    assert(tsfi_mcp_mux_send_request(&mcp_mux, 1, "tools/call", 1) == 0);
    
    // Send invalid token request (Black Rail validation failure)
    assert(tsfi_mcp_mux_send_request(&mcp_mux, 1, "tools/call", 0) == -2);
    
    // Query stats
    int requests = 0, violations = 0;
    assert(tsfi_mcp_mux_query(&mcp_mux, 1, &requests, &violations) == 0);
    assert(requests == 2);
    assert(violations == 1);
    
    // Send request on unregistered channel should fail
    assert(tsfi_mcp_mux_send_request(&mcp_mux, 3, "tools/list", 1) == -1);
    printf("  [PASS] MCP client channel registration and request auditing verified.\n");

    // 134. CODASYL DML Execution Engine Verification
    printf("[Test] Verifying CODASYL DML Execution Engine...\n");
    tsfi_codasyl_dml_runtime dml_rt;
    tsfi_dbtg_currency dml_cur;
    tsfi_dbtg_currency_init(&dml_cur);
    
    tsfi_codasyl_dml_init(&dml_rt, &schema, &dml_cur);
    
    int db_status = 0;
    // Execute store statement for valid record
    assert(tsfi_codasyl_dml_execute(&dml_rt, "STORE CUSTREC", &db_status) == 0);
    assert(db_status == 0);
    assert(dml_rt.mock_records_stored == 1);
    assert(dml_cur.current_run_unit == 100);
    
    // Execute find first statement
    assert(tsfi_codasyl_dml_execute(&dml_rt, "FIND FIRST ORDERREC WITHIN CUST-ORDER", &db_status) == 0);
    assert(db_status == 0);
    
    // Execute store on invalid record should return failure and status code
    assert(tsfi_codasyl_dml_execute(&dml_rt, "STORE INVALIDREC", &db_status) == -2);
    assert(db_status == 117);
    printf("  [PASS] CODASYL DML compiler runtime and syntax validation verified.\n");

    // 135. CODASYL 2-3 Tree Distributed Database Controller Verification
    printf("[Test] Verifying CODASYL 2-3 Tree Distributed Database Controller...\n");
    tsfi_zvm_23_tree tree23;
    tsfi_zvm_23_tree_init(&tree23);
    
    // Add nodes with valid .dat.bin database slice file extensions (Rule 13 compliant)
    assert(tsfi_zvm_23_tree_add_node(&tree23, 10, "node10.dat.bin") == 0);
    assert(tsfi_zvm_23_tree_add_node(&tree23, 20, "node20.dat.bin") == 0);
    // Path violating Rule 13 (must end in .dat.bin) should fail
    assert(tsfi_zvm_23_tree_add_node(&tree23, 30, "node30.json") == -3);
    
    // Mount node
    assert(tsfi_zvm_23_tree_mount(&tree23, 10, "192.168.1.50") == 0);
    
    // Execute DML call from mounted IP
    int call_status = 0;
    assert(tsfi_zvm_23_tree_call(&tree23, 10, "192.168.1.50", "STORE CUSTREC", &call_status) == 0);
    assert(call_status == 0);
    
    // Execute DML call from unmounted IP should fail
    assert(tsfi_zvm_23_tree_call(&tree23, 10, "192.168.1.99", "STORE CUSTREC", &call_status) == -3);
    assert(call_status == 101);
    printf("  [PASS] 2-3 tree client mount and call capabilities verified.\n");

    // 136. CODASYL DDL to Relational Schema Translator Verification
    printf("[Test] Verifying CODASYL DDL to Relational Translator...\n");
    char sql_buf[512];
    assert(tsfi_codasyl_to_relational_translate(&schema, sql_buf, sizeof(sql_buf)) == 0);
    assert(strstr(sql_buf, "CREATE TABLE CUSTREC (ID INT PRIMARY KEY, DATA CHAR(128));") != NULL);
    assert(strstr(sql_buf, "ALTER TABLE ORDERREC ADD FOREIGN KEY (OWNER_ID) REFERENCES CUSTREC (ID);") != NULL);
    printf("  [PASS] CODASYL schema successfully translated into relational SQL schemas.\n");

    // 137. CODASYL Schema Administration Audit Trail Manager Verification
    printf("[Test] Verifying CODASYL Schema Administration Audit Trail Manager...\n");
    tsfi_schema_audit_tracker audit_tracker;
    tsfi_schema_audit_init(&audit_tracker);
    assert(audit_tracker.count == 0);
    assert(tsfi_schema_audit_checksum(&audit_tracker) == 2166136261U);
    
    // Log structural edits
    assert(tsfi_schema_audit_log(&audit_tracker, "ADD_RECORD", "EMPREC", 100) == 0);
    assert(audit_tracker.count == 1);
    assert(tsfi_schema_audit_checksum(&audit_tracker) != 2166136261U);
    
    assert(tsfi_schema_audit_log(&audit_tracker, "ADD_SET", "EMP-JOB", 200) == 0);
    assert(audit_tracker.count == 2);
    printf("  [PASS] Schema structural change logs and hash-chained audit trails validated.\n");

    // 138. CODASYL Subschema Authorization and Area Privilege Auditor Verification
    printf("[Test] Verifying Subschema Authorization and Area Privilege Auditor...\n");
    tsfi_subschema_auditor sub_aud;
    tsfi_subschema_audit_init(&sub_aud);
    
    // Add allowed rules
    assert(tsfi_subschema_add_rule(&sub_aud, "SUBSCHEMA01", "CUSTREC", PRIV_READ) == 0);
    assert(tsfi_subschema_add_rule(&sub_aud, "SUBSCHEMA01", "ORDERREC", PRIV_READ | PRIV_WRITE) == 0);
    
    int auth = 0;
    // Check access: SUBSCHEMA01 reads CUSTREC (allowed)
    assert(tsfi_subschema_authorize(&sub_aud, "SUBSCHEMA01", "CUSTREC", PRIV_READ, &auth) == 0);
    assert(auth == 1);
    assert(sub_aud.authorized_attempts == 1);
    
    // Check access: SUBSCHEMA01 writes CUSTREC (denied)
    assert(tsfi_subschema_authorize(&sub_aud, "SUBSCHEMA01", "CUSTREC", PRIV_WRITE, &auth) == 0);
    assert(auth == 0);
    assert(sub_aud.denied_attempts == 1);
    
    // Check access: SUBSCHEMA01 writes ORDERREC (allowed)
    assert(tsfi_subschema_authorize(&sub_aud, "SUBSCHEMA01", "ORDERREC", PRIV_WRITE, &auth) == 0);
    assert(auth == 1);
    assert(sub_aud.authorized_attempts == 2);
    printf("  [PASS] Subschema database privileges and security attempts audited.\n");

    // 139. CODASYL Database Transaction Recovery and Rollback Auditor Verification
    printf("[Test] Verifying Database Transaction Recovery and Rollback Auditor...\n");
    tsfi_db_tx_manager tx_mgr;
    tsfi_db_tx_init(&tx_mgr);
    
    // Inactive checks: update or commit without active transaction should fail
    assert(tsfi_db_tx_update(&tx_mgr, "test") == -1);
    int dummy_status = 0;
    assert(tsfi_db_tx_commit(&tx_mgr, &dummy_status) == -1);
    assert(tsfi_db_tx_rollback(&tx_mgr, NULL, &dummy_status) == -1);
    
    // Robustness checks with NULL pointers
    assert(tsfi_db_tx_begin(NULL, "CUSTREC", "Name: Alice") == -1);
    assert(tsfi_db_tx_begin(&tx_mgr, NULL, "Name: Alice") == -1);
    assert(tsfi_db_tx_update(NULL, "Name: Bob") == -1);
    assert(tsfi_db_tx_update(&tx_mgr, NULL) == -1);
    assert(tsfi_db_tx_commit(NULL, &dummy_status) == -1);
    assert(tsfi_db_tx_commit(&tx_mgr, NULL) == -1);
    assert(tsfi_db_tx_rollback(NULL, NULL, &dummy_status) == -1);
    assert(tsfi_db_tx_rollback(&tx_mgr, NULL, NULL) == -1);
    
    // Begin transaction
    assert(tsfi_db_tx_begin(&tx_mgr, "CUSTREC", "Name: Alice") == 0);
    assert(tx_mgr.is_active == 1);
    
    // Duplicate begin should fail
    assert(tsfi_db_tx_begin(&tx_mgr, "CUSTREC", "Name: Dave") == -1);
    
    // Multi-step updates: Name: Alice -> Name: Bob -> Name: Charlie
    assert(tsfi_db_tx_update(&tx_mgr, "Name: Bob") == 0);
    assert(strcmp(tx_mgr.after_image, "Name: Bob") == 0);
    assert(tsfi_db_tx_update(&tx_mgr, "Name: Charlie") == 0);
    assert(strcmp(tx_mgr.after_image, "Name: Charlie") == 0);
    
    // Abort and Rollback (must restore Name: Alice)
    char restored[128];
    int tx_status = 0;
    assert(tsfi_db_tx_rollback(&tx_mgr, restored, &tx_status) == 0);
    assert(tx_status == 0);
    assert(tx_mgr.is_active == 0);
    assert(strcmp(restored, "Name: Alice") == 0);
    assert(tx_mgr.rollback_count == 1);
    
    // Commit transaction flow
    assert(tsfi_db_tx_begin(&tx_mgr, "CUSTREC", "Name: Bob") == 0);
    assert(tsfi_db_tx_update(&tx_mgr, "Name: Charlie") == 0);
    assert(tsfi_db_tx_commit(&tx_mgr, &tx_status) == 0);
    assert(tx_mgr.commit_count == 1);
    assert(tx_mgr.is_active == 0);
    printf("  [PASS] Database transaction before-image snapshots and rollbacks validated.\n");

    // 140. Two-Phase Commit (2PC) Protocol Coordinator Verification
    printf("[Test] Verifying Two-Phase Commit (2PC) Coordinator...\n");
    tsfi_2pc_coordinator coordinator;
    tsfi_2pc_init(&coordinator);
    assert(coordinator.state == 0);
    
    // Add nodes
    assert(tsfi_2pc_join(&coordinator, 10) == 0);
    assert(tsfi_2pc_join(&coordinator, 20) == 0);
    assert(tsfi_2pc_join(&coordinator, 10) == -2); // Duplicate check
    
    // Prepare phase
    assert(tsfi_2pc_prepare(&coordinator) == 0);
    assert(coordinator.state == 1);
    assert(coordinator.participants[0].prepared == 1);
    assert(coordinator.participants[1].prepared == 1);
    
    // Commit phase
    int status_2pc = 0;
    assert(tsfi_2pc_commit(&coordinator, &status_2pc) == 0);
    assert(coordinator.state == 2);
    
    // Abort/Reset phase
    tsfi_2pc_init(&coordinator);
    assert(tsfi_2pc_join(&coordinator, 10) == 0);
    assert(tsfi_2pc_abort(&coordinator) == 0);
    assert(coordinator.state == 3);
    assert(coordinator.participants[0].prepared == 0);
    printf("  [PASS] Two-Phase Commit multi-node transaction coordination validated.\n");

    // 141. IBM System/38 Single-Level Store and Logical Access Path Manager Verification
    printf("[Test] Verifying IBM System/38 Single-Level Store and Logical Access Path Manager...\n");
    tsfi_s38_store s38_db;
    tsfi_s38_store_init(&s38_db);
    
    // Create physical & logical file objects under Single-Level Store address space
    assert(tsfi_s38_create_object(&s38_db, 0xFF00FF000100ULL, "CUSTPF", 0, NULL, NULL) == 0);
    assert(tsfi_s38_create_object(&s38_db, 0xFF00FF000200ULL, "CUSTLF", 1, "CUSTPF", "CUSTID") == 0);
    // Duplicate address check
    assert(tsfi_s38_create_object(&s38_db, 0xFF00FF000100ULL, "DUPPF", 0, NULL, NULL) == -2);
    
    // Insert unsorted records into physical file
    assert(tsfi_s38_insert_physical(&s38_db, "CUSTPF", "Record B", 200) == 0);
    assert(tsfi_s38_insert_physical(&s38_db, "CUSTPF", "Record A", 100) == 0);
    assert(tsfi_s38_insert_physical(&s38_db, "CUSTPF", "Record C", 300) == 0);
    
    // Query Logical File and verify access path sorts records dynamically by key_val
    int sorted_keys[8];
    int res_keys = tsfi_s38_query_logical_path(&s38_db, "CUSTLF", sorted_keys, 8);
    assert(res_keys == 3);
    assert(sorted_keys[0] == 100);
    assert(sorted_keys[1] == 200);
    assert(sorted_keys[2] == 300);
    printf("  [PASS] Single-Level Store addressing and logical access path indexing verified.\n");

    // 142. IBM 3848 Cryptographic Subsystem Simulation Verification
    printf("[Test] Verifying IBM 3848 Cryptographic Subsystem...\n");
    tsfi_crypto_subsystem crypto;
    tsfi_crypto_init(&crypto);
    assert(crypto.is_key_loaded == 0);
    
    uint8_t plaintext[8] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };
    uint8_t ciphertext[8] = { 0 };
    uint8_t decrypted[8] = { 0 };
    
    // Load master key
    assert(tsfi_crypto_load_master_key(&crypto, 0xDEADBEEFCAFEBABEULL) == 0);
    assert(crypto.is_key_loaded == 1);
    
    // Privilege check (Problem State should fail with -2)
    assert(tsfi_crypto_encrypt(&crypto, plaintext, ciphertext, 0) == -2);
    
    // Encrypt in Supervisor State (should succeed)
    assert(tsfi_crypto_encrypt(&crypto, plaintext, ciphertext, 1) == 0);
    
    // Decrypt in Supervisor State (should succeed and restore plaintext)
    assert(tsfi_crypto_decrypt(&crypto, ciphertext, decrypted, 1) == 0);
    assert(memcmp(plaintext, decrypted, 8) == 0);
    printf("  [PASS] 3848 Cryptographic Feistel encryption and supervisor state locks validated.\n");

    // 144. NBS FIPS PUB 54 Computer Output Microform (COM) Spool Formatter Verification
    printf("[Test] Verifying NBS FIPS PUB 54 Computer Output Microform Spool Formatter...\n");
    tsfi_com_formatter com_fmt;
    tsfi_com_init(&com_fmt);
    assert(com_fmt.current_frame == 1);
    
    uint8_t frame_buf[256];
    uint16_t frame_len = 0;
    
    // Write 5 records to trigger a microfiche frame advance (4 lines limit)
    assert(tsfi_com_format_record(&com_fmt, "RECORD 001", frame_buf, &frame_len) == 0);
    assert(com_fmt.current_frame == 1);
    assert(tsfi_com_format_record(&com_fmt, "RECORD 002", frame_buf, &frame_len) == 0);
    assert(tsfi_com_format_record(&com_fmt, "RECORD 003", frame_buf, &frame_len) == 0);
    assert(tsfi_com_format_record(&com_fmt, "RECORD 004", frame_buf, &frame_len) == 0);
    assert(com_fmt.current_frame == 1);
    
    // 5th record should advance frame count
    assert(tsfi_com_format_record(&com_fmt, "RECORD 005", frame_buf, &frame_len) == 0);
    assert(com_fmt.current_frame == 2);
    
    // Generate COM index frame metadata
    char index_out[512];
    assert(tsfi_com_generate_index_frame(&com_fmt, index_out, sizeof(index_out)) == 0);
    assert(strstr(index_out, "RECORD 001") != NULL);
    assert(strstr(index_out, "RECORD 005") != NULL);
    printf("  [PASS] Computer Output Microform frame mapping and index generation verified.\n");

    // 145. NBS FIPS PUB 48 Personal Identification Token Authenticator Verification
    printf("[Test] Verifying NBS FIPS PUB 48 Personal Identification Token Authenticator...\n");
    tsfi_fips48_authenticator authenticator;
    tsfi_fips48_init(&authenticator);
    
    // Register badge readers
    assert(tsfi_fips48_register_badge(&authenticator, "USER_A", 10001, 1234) == 0);
    assert(tsfi_fips48_register_badge(&authenticator, "USER_B", 10002, 5678) == 0);
    assert(tsfi_fips48_register_badge(&authenticator, "USER_A", 10001, 4321) == -2); // Duplicate ID check
    
    // Authenticate checks
    int auth_status = 0;
    // Successful login
    assert(tsfi_fips48_authenticate(&authenticator, 10001, 1234, &auth_status) == 0);
    assert(auth_status == 1);
    assert(authenticator.successful_attempts == 1);
    
    // Invalid PIN login
    assert(tsfi_fips48_authenticate(&authenticator, 10001, 9999, &auth_status) == -2);
    assert(auth_status == -2);
    assert(authenticator.failed_attempts == 1);
    
    // Non-existent badge login
    assert(tsfi_fips48_authenticate(&authenticator, 99999, 1234, &auth_status) == -2);
    assert(auth_status == -3);
    assert(authenticator.failed_attempts == 2);
    
    // Register LAU token badge and PKI key PIN mapping
    uint32_t lau_token = 0xBCDE1234;
    uint16_t pki_pin = 8888;
    assert(tsfi_fips48_register_lau_badge(&authenticator, "LAU_USER", lau_token, pki_pin) == 0);
    assert(tsfi_fips48_authenticate(&authenticator, lau_token, pki_pin, &auth_status) == 0);
    assert(auth_status == 1);
    assert(authenticator.successful_attempts == 2);
    printf("  [PASS] FIPS 48 personal identification badge readers and PIN locks validated.\n");

    // 146. NBS FIPS PUB 62 Magnetic Tape Subsystem Interface Verification
    printf("[Test] Verifying NBS FIPS PUB 62 Magnetic Tape Subsystem Interface...\n");
    tsfi_fips62_tape tape;
    tsfi_fips62_tape_init(&tape);
    assert(tape.block_position == 0);
    assert(tape.is_bot == 1);
    
    int new_pos = 0;
    // Command tests
    assert(tsfi_fips62_tape_command(&tape, 0x3F, &new_pos) == 0); // FORWARD SPACE BLOCK
    assert(new_pos == 1);
    assert(tape.is_bot == 0);
    
    assert(tsfi_fips62_tape_command(&tape, 0x27, &new_pos) == 0); // BACKSPACE RECORD
    assert(new_pos == 0);
    assert(tape.is_bot == 1);
    
    assert(tsfi_fips62_tape_command(&tape, 0x1F, NULL) == 0); // WRITE TAPE MARK
    assert(tape.tape_mark_detected == 1);
    
    assert(tsfi_fips62_tape_command(&tape, 0x0F, NULL) == 0); // REWIND
    assert(tape.block_position == 0);
    assert(tape.tape_mark_detected == 0);
    
    // DAT integration test
    tsfi_s370_segment_entry seg_table[1];
    seg_table[0].page_table_origin = 0;
    seg_table[0].length = 16;
    seg_table[0].invalid = 0;
    
    tsfi_s370_page_entry page_tables[16];
    memset(page_tables, 0, sizeof(page_tables));
    page_tables[1].page_frame_real_addr = 0x1000;
    page_tables[1].invalid = 0;
    page_tables[1].write_protect = 0;
    
    uint8_t memory_pool[8192];
    memset(memory_pool, 0, sizeof(memory_pool));
    
    uint8_t dummy_tape_block[8] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22 };
    // Virtual address 0x1004 maps to physical/real memory address 0x1004
    assert(tsfi_fips62_tape_read_to_virtual(&tape, 0x1004, seg_table, 1, page_tables, memory_pool, 8192, dummy_tape_block, 8) == 0);
    assert(memcmp(memory_pool + 0x1004, dummy_tape_block, 8) == 0);
    assert(tape.block_position == 1);
    printf("  [PASS] FIPS 62 tape controller commands and virtual DAT address translation verified.\n");

    // 147. NBS FIPS PUB 63 Rotating Mass Storage Subsystem Interface Verification
    printf("[Test] Verifying NBS FIPS PUB 63 Rotating Mass Storage Subsystem Interface...\n");
    tsfi_fips63_disk fips_disk;
    tsfi_fips63_disk_init(&fips_disk);
    assert(fips_disk.current_cylinder == 0);
    assert(fips_disk.current_sector == 0);
    assert(fips_disk.is_ready == 1);
    
    uint8_t status_byte = 0xFF;
    // Seek cylinder test
    assert(tsfi_fips63_disk_command(&fips_disk, 0x07, 10, 0, &status_byte) == 0);
    assert(status_byte == 0x00);
    assert(fips_disk.current_cylinder == 10);
    
    // Seek Cylinder out of bounds test (cylinder >= 45)
    assert(tsfi_fips63_disk_command(&fips_disk, 0x07, 50, 0, &status_byte) == -2);
    assert(status_byte == 0x02); // Seek Check
    
    // Write sector test
    assert(tsfi_fips63_disk_command(&fips_disk, 0x01, 10, 5, &status_byte) == 0);
    assert(status_byte == 0x0C); // Channel End + Device End
    assert(fips_disk.current_sector == 5);
    
    // Recalibrate drive test
    assert(tsfi_fips63_disk_command(&fips_disk, 0x03, 0, 0, &status_byte) == 0);
    assert(fips_disk.current_cylinder == 0);
    assert(fips_disk.current_sector == 0);
    assert(fips_disk.recalibrate_requested == 1);
    printf("  [PASS] FIPS 63 rotating disk commands and status reporting validated.\n");

    // 148. NBS FIPS PUB 68 Minimal BASIC Interpreter Verification
    printf("[Test] Verifying NBS FIPS PUB 68 Minimal BASIC Interpreter...\n");
    tsfi_fips68_basic basic_interpreter;
    tsfi_fips68_basic_init(&basic_interpreter);
    
    const char *basic_source = 
        "10 LET A = 42\n"
        "20 LET B = 100\n"
        "30 PRINT A\n"
        "40 PRINT \"HELLO BASIC\"\n"
        "50 END\n";
        
    char basic_output[256];
    assert(tsfi_fips68_basic_run(&basic_interpreter, basic_source, basic_output, sizeof(basic_output)) == 0);
    assert(strstr(basic_output, "42\n") != NULL);
    assert(strstr(basic_output, "HELLO BASIC\n") != NULL);
    printf("  [PASS] FIPS 68 Minimal BASIC interpreter parser and variables validated.\n");

    // 149. NBS FIPS PUB 69 Numeric Value Parser/Formatter Verification
    printf("[Test] Verifying NBS FIPS PUB 69 Numeric Value Parser/Formatter...\n");
    double parsed_val = 0.0;
    assert(tsfi_fips69_parse_numeric("+123.45", &parsed_val) == 0);
    assert(parsed_val == 123.45);
    
    assert(tsfi_fips69_parse_numeric("INVALID", &parsed_val) == -2);
    
    char formatted_out[64];
    assert(tsfi_fips69_format_numeric(12.34, formatted_out, sizeof(formatted_out)) == 0);
    assert(strcmp(formatted_out, "+12.34") == 0);
    printf("  [PASS] FIPS 69 explicit sign parsing and string representations validated.\n");

    // 150. NBS FIPS PUB 79 Magnetic Tape Label Parser/Validator Verification
    printf("[Test] Verifying NBS FIPS PUB 79 Magnetic Tape Label Parser/Validator...\n");
    uint8_t label_buf[81];
    assert(tsfi_fips79_format_label(label_buf, "LEDGER_001", 9999, 12, "HDR1") == 0);
    
    char file_id_out[17];
    uint32_t serial_out = 0;
    int blocks_out = 0;
    assert(tsfi_fips79_parse_label(label_buf, file_id_out, &serial_out, &blocks_out) == 0);
    assert(strcmp(file_id_out, "LEDGER_001") == 0);
    assert(serial_out == 9999);
    assert(blocks_out == 12);
    
    // Mismatched label check
    assert(tsfi_fips79_parse_label((const uint8_t*)"BAD1LABEL", file_id_out, &serial_out, &blocks_out) == -2);
    printf("  [PASS] FIPS 79 EBCDIC tape header parsing and EOT trailer validation checks verified.\n");

    tsfi_dat_destroy(dat_mq);
    tsfi_trie_destroy(trie_root_mq);

    printf("[PASS] All extended RAMAC simulation invariants - Part 2 verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
