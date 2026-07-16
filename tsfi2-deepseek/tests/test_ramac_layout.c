#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "tsfi_ramac_layout.h"

int main(void) {
    printf("=============================================================\n");
    printf("RAMAC LAYOUT GEOMETRY & SEEK SIMULATION VERIFICATION\n");
    printf("=============================================================\n");

    // 1. CHS Translation Verification
    printf("[Test] Verifying Index <-> CHS translation...\n");
    int test_indices[] = {0, 8, 160, 8000, 123456, 799999};
    for (size_t i = 0; i < sizeof(test_indices)/sizeof(test_indices[0]); i++) {
        int idx = test_indices[i];
        tsfi_ramac_chs chs = tsfi_ramac_index_to_chs(idx);
        int roundtrip = tsfi_ramac_chs_to_index(chs);
        printf("  Index %d -> Cyl %d, Head %d, Sec %d, Word %d -> Roundtrip %d\n",
               idx, chs.cylinder, chs.head, chs.sector, chs.word_offset, roundtrip);
        assert(idx == roundtrip);
    }
    printf("  [PASS] CHS translation roundtrip verified successfully.\n");

    // 2. Seek Latency Verification
    printf("[Test] Verifying RAMAC seek simulation latency...\n");
    double seek_aa = tsfi_ramac_calculate_seek(10, 10);
    printf("  Seek same index (10 -> 10): %.2f us\n", seek_aa);
    assert(seek_aa == 0.0);

    double seek_ab = tsfi_ramac_calculate_seek(0, 8);
    printf("  Seek rotational (0 -> 8): %.2f us\n", seek_ab);
    assert(seek_ab > 0.0);

    double seek_ac = tsfi_ramac_calculate_seek(0, 160);
    printf("  Seek head swap (0 -> 160): %.2f us\n", seek_ac);
    assert(seek_ac >= 800.0); 

    double seek_ad = tsfi_ramac_calculate_seek(0, 8000);
    printf("  Seek cylinder travel (0 -> 8000): %.2f us\n", seek_ad);
    assert(seek_ad >= 1500.0); 

    printf("  [PASS] Seek latency model calculations verified successfully.\n");

    // 3. Cylinder Overflow Hash Verification
    printf("[Test] Verifying RAMAC Cylinder Overflow Hash table...\n");
    int total_slots = RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS;
    tsfi_ramac_record *disk = calloc(total_slots, sizeof(tsfi_ramac_record));
    assert(disk != NULL);

    double total_seek = 0.0;
    int idx1 = tsfi_ramac_insert_record(disk, "customer_101", "active_record", 5, &total_seek);
    printf("  Inserted 'customer_101' at slot index %d (seek: %.2f us)\n", idx1, total_seek);
    assert(idx1 != -1);
    assert(disk[idx1].is_active);

    int primary_idx_999 = tsfi_ramac_hash_key("customer_999", 5);
    disk[primary_idx_999].is_active = 1;
    strcpy(disk[primary_idx_999].key, "colliding_dummy");
    strcpy(disk[primary_idx_999].value, "dummy_val");
    disk[primary_idx_999].next_overflow_index = -1;

    double collision_seek = 0.0;
    int idx2 = tsfi_ramac_insert_record(disk, "customer_999", "colliding_record", 5, &collision_seek);
    printf("  Inserted colliding 'customer_999' at slot index %d (seek: %.2f us)\n", idx2, collision_seek);
    assert(idx2 != -1);
    assert(idx2 != primary_idx_999);
    
    tsfi_ramac_chs chs2 = tsfi_ramac_index_to_chs(idx2);
    printf("  Collision resolved to Cylinder %d, Head %d, Sector %d\n", chs2.cylinder, chs2.head, chs2.sector);
    assert(chs2.cylinder == 5);
    assert(chs2.head >= 45 && chs2.head < 50);

    double search_seek = 0.0;
    const char *val1 = tsfi_ramac_search_record(disk, "customer_101", 5, &search_seek);
    assert(val1 != NULL && strcmp(val1, "active_record") == 0);
    printf("  Search 'customer_101' val: %s (seek: %.2f us)\n", val1, search_seek);

    const char *val2 = tsfi_ramac_search_record(disk, "customer_999", 5, &search_seek);
    assert(val2 != NULL && strcmp(val2, "colliding_record") == 0);
    printf("  Search 'customer_999' val: %s (seek: %.2f us)\n", val2, search_seek);

    printf("  [PASS] Cylinder Overflow Hash verified successfully.\n");

    // 3.5. Read-After-Write Verification & Plugboard Emulator
    printf("[Test] Verifying Read-After-Write verification...\n");
    int verify_status = tsfi_ramac_write_verified(disk, "customer_333", "verified_val", 5);
    assert(verify_status == 0);
    printf("  [PASS] Read-After-Write verification check passed.\n");

    printf("[Test] Verifying RAMAC Plugboard panel router...\n");
    uint8_t src_buf[64] = "RAMAC_CONTROL_PANEL_INPUT_DATA_1956";
    uint8_t dest_buf[64] = {0};
    int routed = tsfi_ramac_plugboard_route("6..18->10..22", src_buf, dest_buf, 64);
    printf("  Plugboard routed %d bytes: Dest = '%s'\n", routed, dest_buf + 10);
    assert(routed == 13);
    assert(strcmp((char*)(dest_buf + 10), "CONTROL_PANEL") == 0);
    printf("  [PASS] Plugboard router layout updates verified successfully.\n");

    // 3.8. Non-preferential Accumulator Model (Rule 12 compliant)
    printf("[Test] Verifying Non-preferential Accumulators and isolation trap...\n");
    tsfi_ramac_acc_model accs;
    tsfi_ramac_acc_init(&accs);
    
    tsfi_ramac_acc_add(&accs, 3, 500);
    assert(accs.accumulators[3] == 500);

    tsfi_ramac_acc_div(&accs, 3, 5);
    assert(accs.accumulators[3] == 100);

    int div_zero_ret = tsfi_ramac_acc_div(&accs, 3, 0);
    assert(div_zero_ret == -1);
    assert(accs.trap_active == 1);
    assert(accs.isolation_trap == 100);
    printf("  [PASS] Mathematical continuity interruption successfully intercepted and isolated.\n");

    // 3.9. Inquiry Station & Parity Checker Verification
    printf("[Test] Verifying IBM 370 Inquiry Station console parser & BCD parity checker...\n");
    char response[256];
    
    int inq_ret1 = tsfi_ramac_inquiry_station(disk, "WRT key_inq val_inq", response, sizeof(response));
    assert(inq_ret1 == 0);
    printf("  Inquiry WRT Response: %s\n", response);
    assert(strstr(response, "WRITE_SUCCESS") != NULL);

    int inq_ret2 = tsfi_ramac_inquiry_station(disk, "QRY key_inq", response, sizeof(response));
    assert(inq_ret2 == 0);
    printf("  Inquiry QRY Response: %s\n", response);
    assert(strstr(response, "VAL: val_inq") != NULL);

    char odd_str[2] = { 65 | 128, 0 }; 
    char even_str[2] = { 65, 0 };      

    assert(tsfi_ramac_check_parity(odd_str) == 1);
    assert(tsfi_ramac_check_parity(even_str) == 0);
    printf("  [PASS] IBM 370 inquiry console and BCD parity validation verified successfully.\n");

    // 3.9.5. ALU Program Execution Loop Verification
    printf("[Test] Verifying IBM 305 plugboard ALU processor core...\n");
    tsfi_ramac_acc_model alu_model;
    tsfi_ramac_acc_init(&alu_model);

    tsfi_ramac_instruction prog[6];
    
    strcpy(prog[0].op, "ADD");
    prog[0].acc_dest = 1;
    prog[0].acc_src = 100;
    prog[0].constant = 1;
    prog[0].label[0] = '\0';

    strcpy(prog[1].op, "ADD");
    prog[1].acc_dest = 2;
    prog[1].acc_src = 50;
    prog[1].constant = 1;
    prog[1].label[0] = '\0';

    strcpy(prog[2].op, "CMP");
    prog[2].acc_dest = 1;
    prog[2].acc_src = 2;
    prog[2].constant = 0;
    prog[2].label[0] = '\0';

    strcpy(prog[3].op, "JEQ");
    prog[3].acc_dest = 0;
    prog[3].acc_src = 0;
    prog[3].constant = 0;
    strcpy(prog[3].label, "LABEL_END");

    strcpy(prog[4].op, "SUB");
    prog[4].acc_dest = 1;
    prog[4].acc_src = 2;
    prog[4].constant = 0;
    prog[4].label[0] = '\0';

    strcpy(prog[5].op, "ADD");
    prog[5].acc_dest = 1;
    prog[5].acc_src = 5;
    prog[5].constant = 1;
    strcpy(prog[5].label, "LABEL_END");

    int run_ret = tsfi_ramac_alu_exec(&alu_model, prog, 6);
    assert(run_ret == 0);
    printf("  ALU completed execution. ACC1 value: %lld\n", (long long)alu_model.accumulators[1]);
    assert(alu_model.accumulators[1] == 55);
    printf("  [PASS] IBM 305 ALU program steps and logic branching verified successfully.\n");

    // 3.9.8. System/370 Dynamic Address Translation (DAT) verification
    printf("[Test] Verifying System/370 Dynamic Address Translation (DAT)...\n");
    tsfi_s370_segment_entry seg_table[2];
    seg_table[0].page_table_origin = 0;
    seg_table[0].length = 16;
    seg_table[0].invalid = 0;

    seg_table[1].page_table_origin = 256;
    seg_table[1].length = 16;
    seg_table[1].invalid = 1;

    tsfi_s370_page_entry page_tables[512];
    memset(page_tables, 0, sizeof(page_tables));
    page_tables[5].page_frame_real_addr = 0x8F000;
    page_tables[5].invalid = 0;
    page_tables[5].write_protect = 1; // page-protection enabled

    page_tables[6].page_frame_real_addr = 0;
    page_tables[6].invalid = 1;

    uint32_t virt_addr = 0x5123;
    uint32_t phys_addr = 0;
    int write_protected = 0;
    int translate_ret1 = tsfi_s370_dat_translate(virt_addr, seg_table, 2, page_tables, &phys_addr, &write_protected);
    assert(translate_ret1 == 0);
    printf("  Virtual address 0x%08X translated to physical address 0x%08X (write protected: %d)\n", virt_addr, phys_addr, write_protected);
    assert(phys_addr == 0x8F123);
    assert(write_protected == 1);

    uint32_t invalid_seg_addr = 0x105123;
    int translate_ret2 = tsfi_s370_dat_translate(invalid_seg_addr, seg_table, 2, page_tables, &phys_addr, &write_protected);
    assert(translate_ret2 == -1);

    uint32_t invalid_page_addr = 0x6123;
    int translate_ret3 = tsfi_s370_dat_translate(invalid_page_addr, seg_table, 2, page_tables, &phys_addr, &write_protected);
    assert(translate_ret3 == -1);

    printf("  [PASS] System/370 Dynamic Address Translation (DAT) paging verified successfully.\n");

    // 3.9.9. System/370 Channel I/O Subsystem Program execution
    printf("[Test] Verifying System/370 Channel Command Word (CCW) I/O program chains...\n");
    uint8_t memory_pool[1024];
    memset(memory_pool, 0, sizeof(memory_pool));

    int target_sec = 120;
    memcpy(memory_pool + 10, &target_sec, 4);

    const char *payload_data = "CCW_TEST_DATA";
    memcpy(memory_pool + 20, payload_data, 13);

    tsfi_s370_ccw ccw_chain[3];
    
    ccw_chain[0].cmd_code = 0x07;
    ccw_chain[0].data_addr = 10;
    ccw_chain[0].flags = 0x02;
    ccw_chain[0].count = 4;

    ccw_chain[1].cmd_code = 0x01;
    ccw_chain[1].data_addr = 20;
    ccw_chain[1].flags = 0x02;
    ccw_chain[1].count = 13;

    ccw_chain[2].cmd_code = 0x02;
    ccw_chain[2].data_addr = 40;
    ccw_chain[2].flags = 0x00;
    ccw_chain[2].count = 13;

    int channel_ret = tsfi_s370_channel_execute(disk, total_slots, ccw_chain, 3, memory_pool, 1024);
    assert(channel_ret == 0);
    printf("  Read-back CCW data at memory offset 40: '%s'\n", (char*)(memory_pool + 40));
    assert(strcmp((char*)(memory_pool + 40), "CCW_TEST_DATA") == 0);
    printf("  [PASS] System/370 Channel I/O Program execution verified successfully.\n");

    // 3.9.9.5. System/370 Storage Key Hardware Protection
    printf("[Test] Verifying System/370 Storage Key block protection...\n");
    tsfi_s370_storage_key block_keys[4];
    memset(block_keys, 0, sizeof(block_keys));
    
    block_keys[1].acc = 5;
    block_keys[1].fetch_protect = 1;
    
    block_keys[2].acc = 6;
    block_keys[2].fetch_protect = 0;

    int key_ret1 = tsfi_s370_check_storage_key(5, 5000, 1, block_keys, 4);
    assert(key_ret1 == 0);
    assert(block_keys[1].referenced == 1);
    assert(block_keys[1].changed == 1);

    int key_ret2 = tsfi_s370_check_storage_key(6, 5000, 1, block_keys, 4);
    assert(key_ret2 == -1);

    int key_ret3 = tsfi_s370_check_storage_key(6, 5000, 0, block_keys, 4);
    assert(key_ret3 == -1);

    int key_ret4 = tsfi_s370_check_storage_key(7, 9000, 0, block_keys, 4);
    assert(key_ret4 == 0);
    assert(block_keys[2].referenced == 1);
    assert(block_keys[2].changed == 0);

    int key_ret5 = tsfi_s370_check_storage_key(0, 5000, 1, block_keys, 4);
    assert(key_ret5 == 0);

    printf("  [PASS] System/370 Storage Key hardware protection verified successfully.\n");

    // 3.9.9.8. System/370 Privilege & Security LAP Check Verification
    printf("[Test] Verifying System/370 Privilege & Security LAP protections...\n");
    tsfi_s370_cpu_state cpu;
    
    cpu.supervisor_state = 0; 
    cpu.lap_enabled = 1;
    cpu.psw_key = 5;
    
    int priv_ret1 = tsfi_s370_validate_instruction(&cpu, "SSK");
    assert(priv_ret1 == -1);

    int priv_ret2 = tsfi_s370_validate_instruction(&cpu, "ADD");
    assert(priv_ret2 == 0);

    cpu.supervisor_state = 1; 
    int priv_ret3 = tsfi_s370_validate_instruction(&cpu, "SSK");
    assert(priv_ret3 == 0);

    cpu.supervisor_state = 0; 
    cpu.lap_enabled = 1;
    cpu.psw_key = 0; 
    int lap_ret1 = tsfi_s370_validate_write(&cpu, 200, 0, block_keys, 4);
    assert(lap_ret1 == -1);

    cpu.supervisor_state = 1; 
    cpu.psw_key = 0;
    int lap_ret2 = tsfi_s370_validate_write(&cpu, 200, 0, block_keys, 4);
    assert(lap_ret2 == 0);

    cpu.supervisor_state = 0; 
    cpu.lap_enabled = 0;      
    cpu.psw_key = 5;          
    block_keys[0].acc = 5;
    int lap_ret3 = tsfi_s370_validate_write(&cpu, 200, 0, block_keys, 4);
    assert(lap_ret3 == 0);

    // Test page-protection flag write blocking
    int page_prot_ret = tsfi_s370_validate_write(&cpu, 200, 1, block_keys, 4);
    assert(page_prot_ret == -1);

    // Test TLB cache hits and invalidation purges
    printf("[Test] Verifying CPU TLB cache hardware lookup acceleration...\n");
    tsfi_s370_tlb_purge(&cpu);
    
    tsfi_s370_segment_entry test_seg[1];
    test_seg[0].page_table_origin = 0;
    test_seg[0].length = 16;
    test_seg[0].invalid = 0;

    tsfi_s370_page_entry test_page[16];
    memset(test_page, 0, sizeof(test_page));
    test_page[3].page_frame_real_addr = 0xAA000;
    test_page[3].invalid = 0;
    test_page[3].write_protect = 1;

    uint32_t tlb_phys = 0;
    int tlb_wp = 0;
    // Miss: should perform table walk and populate TLB entry
    int tlb_ret1 = tsfi_s370_dat_translate_with_tlb(&cpu, 0x003123, test_seg, 1, test_page, &tlb_phys, &tlb_wp);
    assert(tlb_ret1 == 0);
    assert(tlb_phys == 0xAA123);
    assert(tlb_wp == 1);
    assert(cpu.tlb[3].valid == 1);
    assert(cpu.tlb[3].virtual_page == 0x003000);

    // Hit: invalidate page table entries to verify the TLB entry bypasses table walks
    test_page[3].invalid = 1;
    tlb_phys = 0;
    tlb_wp = 0;
    int tlb_ret2 = tsfi_s370_dat_translate_with_tlb(&cpu, 0x003123, test_seg, 1, test_page, &tlb_phys, &tlb_wp);
    assert(tlb_ret2 == 0); // Still hit because TLB entry is cached!
    assert(tlb_phys == 0xAA123);

    // Purge TLB cache: lookup should now perform table walk and fail because table entry is invalid
    tsfi_s370_tlb_purge(&cpu);
    int tlb_ret3 = tsfi_s370_dat_translate_with_tlb(&cpu, 0x003123, test_seg, 1, test_page, &tlb_phys, &tlb_wp);
    assert(tlb_ret3 == -1); // Fails as expected!
    printf("  [PASS] CPU TLB hit, miss, eviction, and invalidate purge cycles verified successfully.\n");

    printf("  [PASS] System/370 Privilege States & LAP write validations verified successfully.\n");

    // 3.9.9.9. System/370 LAU Account PKI Key Authorization
    printf("[Test] Verifying System/370 LAU Account PKI authorization mappings...\n");
    tsfi_lau_account user_account;
    strcpy(user_account.account_address, "0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266");
    memset(user_account.public_key, 0xAB, 32);
    user_account.is_admin_tier = 0;

    const uint8_t message[4] = { 'A', 'U', 'T', 'H' };
    uint8_t sig[32];
    for (int i = 0; i < 32; i++) {
        sig[i] = user_account.public_key[i] ^ (message[i % 4] + i);
    }

    uint8_t psw_key = 0;
    int auth_ret1 = tsfi_s370_authorize_psw_key(&user_account, sig, 32, message, 4, &psw_key);
    assert(auth_ret1 == 0);
    printf("  Regular LAU verified. Authorized PSW Access Key: %d\n", psw_key);
    assert(psw_key == ((0xAB % 15) + 1));

    sig[0] ^= 0xFF;
    int auth_ret2 = tsfi_s370_authorize_psw_key(&user_account, sig, 32, message, 4, &psw_key);
    assert(auth_ret2 == -1);

    tsfi_lau_account admin_account;
    strcpy(admin_account.account_address, "0x70997970C51812dc3A010C7d01b50e0d17dc79C8");
    memset(admin_account.public_key, 0xDE, 32);
    admin_account.is_admin_tier = 1;

    for (int i = 0; i < 32; i++) {
        sig[i] = admin_account.public_key[i] ^ (message[i % 4] + i);
    }
    int auth_ret3 = tsfi_s370_authorize_psw_key(&admin_account, sig, 32, message, 4, &psw_key);
    assert(auth_ret3 == 0);
    printf("  Admin LAU verified. Authorized PSW Access Key: %d (Master Key)\n", psw_key);
    assert(psw_key == 0);

    printf("  [PASS] System/370 LAU Account PKI Key authorizations verified successfully.\n");

    // 3.9.9.9.5. System/370 Program Interruption and PSW Swap Hardware Validation
    printf("[Test] Verifying System/370 Program Interruption & PSW Swap handling...\n");
    uint8_t real_memory[1024];
    memset(real_memory, 0, sizeof(real_memory));

    real_memory[88] = (2 << 4); 
    real_memory[89] = 0x00;     
    real_memory[92] = 0x00;
    real_memory[93] = 0x00;
    real_memory[94] = 0x20;
    real_memory[95] = 0x00;     

    cpu.current_psw.key = 5;
    cpu.current_psw.problem_state = 1;
    cpu.current_psw.instruction_address = 0x5000;
    cpu.psw_key = 5;
    cpu.supervisor_state = 0; 

    int interrupt_ret = tsfi_s370_trigger_program_interrupt(&cpu, 0x0004, real_memory, 1024);
    assert(interrupt_ret == 0);

    assert(real_memory[142] == 0x00);
    assert(real_memory[143] == 0x04);

    assert(real_memory[40] == 0x50); 
    assert(real_memory[41] == 0x01); 
    assert(real_memory[44] == 0x00);
    assert(real_memory[45] == 0x00);
    assert(real_memory[46] == 0x50);
    assert(real_memory[47] == 0x00); 

    assert(cpu.current_psw.key == 2);
    assert(cpu.current_psw.problem_state == 0);
    assert(cpu.current_psw.instruction_address == 0x2000);
    assert(cpu.psw_key == 2);
    assert(cpu.supervisor_state == 1); 

    printf("  [PASS] System/370 Program Interruption & hardware PSW swap verified successfully.\n");

    // 3.9.9.9.8. System/370 COMP-3 Packed Decimal COBOL-style verification
    printf("[Test] Verifying System/370 COMP-3 Packed Decimal arithmetic...\n");
    uint8_t packed_a[32];
    uint8_t packed_b[32];
    uint8_t packed_sum[32];

    int size_a = tsfi_s370_pack("12345", packed_a, 32);
    assert(size_a == 3);
    assert(packed_a[0] == 0x12);
    assert(packed_a[1] == 0x34);
    assert(packed_a[2] == 0x5C);

    char unpacked_a[128];
    int unpack_ret1 = tsfi_s370_unpack(packed_a, size_a, unpacked_a, 128);
    assert(unpack_ret1 == 0);
    assert(strcmp(unpacked_a, "12345") == 0);

    int size_b = tsfi_s370_pack("-1234", packed_b, 32);
    assert(size_b == 3);
    assert(packed_b[0] == 0x01);
    assert(packed_b[1] == 0x23);
    assert(packed_b[2] == 0x4D);

    char unpacked_b[128];
    int unpack_ret2 = tsfi_s370_unpack(packed_b, size_b, unpacked_b, 128);
    assert(unpack_ret2 == 0);
    assert(strcmp(unpacked_b, "-1234") == 0);

    int size_sum = tsfi_s370_packed_add(packed_a, size_a, packed_b, size_b, packed_sum, 32);
    assert(size_sum == 3);
    assert(packed_sum[0] == 0x11);
    assert(packed_sum[1] == 0x11);
    assert(packed_sum[2] == 0x1C);

    char unpacked_sum[128];
    int unpack_ret3 = tsfi_s370_unpack(packed_sum, size_sum, unpacked_sum, 128);
    assert(unpack_ret3 == 0);
    assert(strcmp(unpacked_sum, "11111") == 0);

    printf("  [PASS] System/370 COBOL-style packed BCD arithmetic verified successfully.\n");

    // 3.9.9.9.9. System/370 Supervisor Call (SVC) Hardware Interruption Validation
    printf("[Test] Verifying System/370 Supervisor Call (SVC) gatekeeper interrupt...\n");
    
    real_memory[80] = 0x00; 
    real_memory[81] = 0x00; 
    real_memory[84] = 0x00;
    real_memory[85] = 0x00;
    real_memory[86] = 0x10;
    real_memory[87] = 0x00; 

    cpu.current_psw.key = 8;
    cpu.current_psw.problem_state = 1;
    cpu.current_psw.instruction_address = 0x4000;
    cpu.psw_key = 8;
    cpu.supervisor_state = 0; 

    int svc_ret = tsfi_s370_trigger_svc(&cpu, 204, real_memory, 1024);
    assert(svc_ret == 0);

    assert(real_memory[139] == 204);

    assert(real_memory[32] == 0x80); 
    assert(real_memory[33] == 0x01); 
    assert(real_memory[36] == 0x00);
    assert(real_memory[37] == 0x00);
    assert(real_memory[38] == 0x40);
    assert(real_memory[39] == 0x00); 

    assert(cpu.current_psw.key == 0);
    assert(cpu.current_psw.problem_state == 0);
    assert(cpu.current_psw.instruction_address == 0x1000);
    assert(cpu.psw_key == 0);
    assert(cpu.supervisor_state == 1); 

    printf("  [PASS] System/370 SVC gatekeeper interrupt verified successfully.\n");

    // 3.9.9.9.9.5. Benson-Lehner style Data Reduction Unit Verification
    printf("[Test] Verifying Benson-Lehner Data Reduction Unit...\n");
    uint8_t reduced_packed[32];
    int red_bytes = tsfi_s370_data_reduction_unit(4.25, 2.75, 100.0, reduced_packed, 32);
    assert(red_bytes == 2); // 700 -> 70 0C
    assert(reduced_packed[0] == 0x70);
    assert(reduced_packed[1] == 0x0C);

    char unpacked_red[128];
    int unpack_ret_red = tsfi_s370_unpack(reduced_packed, red_bytes, unpacked_red, 128);
    assert(unpack_ret_red == 0);
    printf("  Reduced coordinate values sum to zoned string: '%s'\n", unpacked_red);
    assert(strcmp(unpacked_red, "700") == 0);
    printf("  [PASS] Benson-Lehner Electroplotter data reduction logic verified successfully.\n");

    // 3.9.9.9.9.8. Quadtree Disk Serialization & COMP-3 Native Mappings
    printf("[Test] Verifying Quadtree index serialization to disk under Rule 13 (.dat.bin)...\n");
    tsfi_quadtree_node write_nodes[2];
    memset(write_nodes, 0, sizeof(write_nodes));

    // Mapped boundary node
    write_nodes[0].boundary_x = 4.25;
    write_nodes[0].boundary_y = 2.75;
    write_nodes[0].boundary_size = 10.0;
    write_nodes[0].is_active = 1;
    // Mapped value in COMP-3 format (700 -> 70 0C)
    write_nodes[0].packed_val[0] = 0x70;
    write_nodes[0].packed_val[1] = 0x0C;
    write_nodes[0].val_len = 2;
    write_nodes[0].children_indices[0] = 1;
    write_nodes[0].children_indices[1] = -1;
    write_nodes[0].children_indices[2] = -1;
    write_nodes[0].children_indices[3] = -1;

    // Leaf node
    write_nodes[1].boundary_x = 2.0;
    write_nodes[1].boundary_y = 1.0;
    write_nodes[1].boundary_size = 5.0;
    write_nodes[1].is_active = 1;
    write_nodes[1].packed_val[0] = 0x30;
    write_nodes[1].packed_val[1] = 0x0C; // 300 -> 30 0C
    write_nodes[1].val_len = 2;
    write_nodes[1].children_indices[0] = -1;
    write_nodes[1].children_indices[1] = -1;
    write_nodes[1].children_indices[2] = -1;
    write_nodes[1].children_indices[3] = -1;

    // Test Rule 13 extension validation: should fail with .json
    int ser_ret_json = tsfi_s370_serialize_quadtree("tmp/test_quadtree.json", write_nodes, 2);
    assert(ser_ret_json == -1);

    // Should pass with .dat.bin
    int ser_ret_ok = tsfi_s370_serialize_quadtree("tmp/test_quadtree.dat.bin", write_nodes, 2);
    assert(ser_ret_ok == 0);

    tsfi_quadtree_node read_nodes[4];
    memset(read_nodes, 0, sizeof(read_nodes));

    int des_ret_json = tsfi_s370_deserialize_quadtree("tmp/test_quadtree.json", read_nodes, 4);
    assert(des_ret_json == -1);

    int des_count = tsfi_s370_deserialize_quadtree("tmp/test_quadtree.dat.bin", read_nodes, 4);
    assert(des_count == 2);

    // Assert read nodes boundaries and COMP-3 data properties
    assert(read_nodes[0].boundary_x == 4.25);
    assert(read_nodes[0].boundary_y == 2.75);
    assert(read_nodes[0].packed_val[0] == 0x70);
    assert(read_nodes[0].packed_val[1] == 0x0C);
    assert(read_nodes[0].val_len == 2);
    assert(read_nodes[0].children_indices[0] == 1);

    assert(read_nodes[1].boundary_x == 2.0);
    assert(read_nodes[1].boundary_y == 1.0);
    assert(read_nodes[1].packed_val[0] == 0x30);
    assert(read_nodes[1].packed_val[1] == 0x0C);

    printf("  [PASS] Quadtree serialization, extension rules, and COMP-3 mappings verified successfully.\n");

    // 3.9.9.9.9.9. Benson-Lehner OSCAR style calibration mapping
    printf("[Test] Verifying Benson-Lehner OSCAR Reader calibration overlays...\n");
    double calibration_curve[] = {0.0, 0.2, 0.5, 0.8, 1.0};
    uint8_t oscar_packed[16];
    // Input 0.35 is halfway between 0.2 (idx 1) and 0.5 (idx 2) -> interp_val = 1.5.
    // Digital scaling factor: 1.5 * (1000 / 4) = 1.5 * 250 = 375.
    int oscar_bytes = tsfi_s370_oscar_reader(0.35, calibration_curve, 5, oscar_packed, 16);
    assert(oscar_bytes == 2); // 375 -> 37 5C
    assert(oscar_packed[0] == 0x37);
    assert(oscar_packed[1] == 0x5C);

    char oscar_unpacked[32];
    int unpack_oscar_ret = tsfi_s370_unpack(oscar_packed, oscar_bytes, oscar_unpacked, sizeof(oscar_unpacked));
    assert(unpack_oscar_ret == 0);
    printf("  OSCAR amplitude 0.35 mapped to digitized value: %s\n", oscar_unpacked);
    assert(strcmp(oscar_unpacked, "375") == 0);
    printf("  [PASS] Benson-Lehner OSCAR calibration reader verified successfully.\n");

    // 3.9.9.9.9.9.5. Alfred M. Freudenthal viscoelastic FET discharge solver
    printf("[Test] Verifying A. M. Freudenthal Viscoelastic FET Discharge solver (Rule 10)...\n");
    double decay_charges[10];
    memset(decay_charges, 0, sizeof(decay_charges));
    int phy_ret = tsfi_s370_fet_discharge_freudenthal(10.0, 0.1, 1.0, 5.0, 0.5, 10, decay_charges);
    assert(phy_ret == 0);
    printf("  Initial simulated FET charge: %.2f V, Step 1: %.2f V, Step 9: %.2f V\n", 10.0, decay_charges[0], decay_charges[9]);
    assert(decay_charges[0] == 9.5);
    assert(decay_charges[9] < 1.0); // Conforms to viscoelastic decay decay patterns
    printf("  [PASS] Alfred M. Freudenthal FET viscoelastic discharge solver verified successfully.\n");

    // 3.9.9.9.9.9.8. Alfred M. Freudenthal fatigue damage accumulation solver
    printf("[Test] Verifying A. M. Freudenthal Gate Oxide Fatigue Damage accumulation...\n");
    double cycle_stresses[] = {4.0, 5.5, 6.0, 3.5};
    double accum_damage = tsfi_s370_fet_gate_fatigue_freudenthal(cycle_stresses, 4, 5.0, 2.0);
    printf("  Accumulated FET gate oxide fatigue damage factor: %.4f\n", accum_damage);
    // (4/5)^2 + (5.5/5)^2 + (6/5)^2 + (3.5/5)^2 = 0.64 + 1.21 + 1.44 + 0.49 = 3.78
    assert(accum_damage > 3.77 && accum_damage < 3.79);
    printf("  [PASS] Alfred M. Freudenthal FET gate fatigue accumulation verified successfully.\n");

    // 3.9.9.9.9.9.9. Alfred M. Freudenthal structural reliability safety solver
    printf("[Test] Verifying A. M. Freudenthal Gate Oxide Reliability Index...\n");
    double beta = 0.0, pf = 0.0;
    int rel_ret = tsfi_s370_fet_reliability_freudenthal(10.0, 1.5, 6.0, 2.0, &beta, &pf);
    assert(rel_ret == 0);
    printf("  Gate breakdown Safety Index beta: %.4f, Probability of failure Pf: %.6f\n", beta, pf);
    // denom = sqrt(1.5^2 + 2.0^2) = sqrt(2.25 + 4.0) = sqrt(6.25) = 2.5
    // beta = (10.0 - 6.0) / 2.5 = 4.0 / 2.5 = 1.6
    assert(beta > 1.59 && beta < 1.61);
    assert(pf > 0.05 && pf < 0.06); // Pf = 0.5 * erfc(1.6 / 1.414) ~ 0.0548
    printf("  [PASS] Alfred M. Freudenthal FET gate reliability index verified successfully.\n");

    // 3.9.9.9.9.9.9.5. Ora C. Roehl Keystone Custodian stochastic portfolio optimizer
    printf("[Test] Verifying Ora C. Roehl Keystone Custodian portfolio strategy...\n");
    double assets[] = {0.08, 0.05}; // 8% and 5% yields
    double weights[] = {0.60, 0.40}; // 60% and 40% allocations
    double exp_ret = 0.0, var = 0.0;
    int port_ret = tsfi_s370_portfolio_strategy_keystone(assets, weights, 2, &exp_ret, &var);
    assert(port_ret == 0);
    printf("  Expected portfolio return: %.4f, Variance: %.6f\n", exp_ret, var);
    // exp_ret = 0.60 * 0.08 + 0.40 * 0.05 = 0.048 + 0.02 = 0.068 (6.8%)
    assert(exp_ret > 0.067 && exp_ret < 0.069);
    // std_0 = 0.08 * 0.25 = 0.02, std_1 = 0.05 * 0.25 = 0.0125
    // var = w0^2 * std0^2 + w1^2 * std1^2 + 2 * w0 * w1 * std0 * std1 * rho
    //     = 0.36 * 0.0004 + 0.16 * 0.00015625 + 2 * 0.6 * 0.4 * 0.02 * 0.0125 * 0.15
    //     = 0.000144 + 0.000025 + 0.48 * 0.00025 * 0.15
    //     = 0.000144 + 0.000025 + 0.000018 = 0.000187
    assert(var > 0.000186 && var < 0.000188);
    printf("  [PASS] Ora C. Roehl portfolio yield strategy verified successfully.\n");

    // 3.9.9.9.9.9.9.8. Luis J. A. Villalon executive techniques decision solver
    printf("[Test] Verifying Luis J. A. Villalon executive techniques decision solver...\n");
    double benefit[] = {1000.0, 1500.0, 800.0};
    double cost[] = {200.0, 300.0, 100.0};
    double risk[] = {0.10, 0.25, 0.05}; // 10%, 25%, 5% risk probabilities
    double optimal_value = 0.0;
    int optimal_idx = -1;
    int dec_ret = tsfi_s370_executive_decision_villalon(3, benefit, cost, risk, &optimal_value, &optimal_idx);
    assert(dec_ret == 0);
    printf("  Optimal Decision Index: %d, Expected Net Value: %.2f\n", optimal_idx, optimal_value);
    // V0 = 1000 - 200 - 1000 * 0.10 = 800 - 100 = 700.0
    // V1 = 1500 - 300 - 1500 * 0.25 = 1200 - 375 = 825.0 (Optimal)
    // V2 = 800 - 100 - 800 * 0.05 = 700 - 40 = 660.0
    assert(optimal_idx == 1);
    assert(optimal_value > 824.9 && optimal_value < 825.1);
    printf("  [PASS] Luis J. A. Villalon executive decision solver verified successfully.\n");

    // 3.9.9.9.9.9.9.9. Maurice Nelles deliberate creativeness mutator
    printf("[Test] Verifying Maurice Nelles deliberate creativeness mutator...\n");
    double params[] = {100.0, 200.0};
    int cr_ret = tsfi_s370_deliberate_creativeness_nelles(params, 2, 42);
    assert(cr_ret == 0);
    printf("  Mutated parameters: [0]=%.2f, [1]=%.2f\n", params[0], params[1]);
    assert(params[0] > 90.26 && params[0] < 90.28);
    assert(params[1] > 185.27 && params[1] < 185.29);
    printf("  [PASS] Maurice Nelles deliberate creativeness mutator verified successfully.\n");

    // 3.9.9.9.9.9.9.9.5. Vladimir Zworykin RCA project scale predictor
    printf("[Test] Verifying Vladimir Zworykin RCA project scale predictor...\n");
    double act_budget = 0.0, act_months = 0.0;
    int zw_ret = tsfi_s370_project_scale_zworykin(100000.0, 18.0, &act_budget, &act_months);
    assert(zw_ret == 0);
    printf("  Zworykin actual budget: %.2f USD, actual months: %.2f months\n", act_budget, act_months);
    assert(act_budget > 49999999.9 && act_budget < 50000000.1);
    assert(act_months > 119.9 && act_months < 120.1);
    printf("  [PASS] Vladimir Zworykin RCA project scale predictor verified successfully.\n");

    // 3.9.9.9.9.9.9.9.8. Z-machine RAMAC storage mapping
    printf("[Test] Verifying Z-machine RAMAC primary/overflow storage mapping...\n");
    
    // Dynamic Memory write: slot 100 falls into Cylinder 0 (Primary Area 0-44)
    tsfi_ramac_chs dynamic_chs = tsfi_ramac_index_to_chs(100);
    printf("  Z-machine Dynamic Memory block 100 maps to: Cyl %d, Head %d, Sector %d\n", 
           dynamic_chs.cylinder, dynamic_chs.head, dynamic_chs.sector);
    assert(dynamic_chs.cylinder >= 0 && dynamic_chs.cylinder <= 44);

    // Static/High Memory write: slot 400000 falls into Cylinder 50 (Overflow Area 45-99)
    tsfi_ramac_chs static_chs = tsfi_ramac_index_to_chs(400000);
    printf("  Z-machine Static/High Memory block 400000 maps to: Cyl %d, Head %d, Sector %d\n", 
           static_chs.cylinder, static_chs.head, static_chs.sector);
    assert(static_chs.cylinder >= 45 && static_chs.cylinder <= 99);

    // Allocate temp disk record to test direct address read/writes
    tsfi_ramac_record *temp_disk = (tsfi_ramac_record*)calloc(RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS, sizeof(tsfi_ramac_record));
    assert(temp_disk != NULL);

    // Write to Z-machine dynamic memory address 3200 (Cylinder 0, Sector 100)
    int wr_ret = tsfi_s370_zmachine_write_byte(temp_disk, 3200, 0x55);
    assert(wr_ret == 0);

    // Read back dynamic memory address 3200
    uint8_t rd_val = 0;
    int rd_ret = tsfi_s370_zmachine_read_byte(temp_disk, 3200, &rd_val);
    assert(rd_ret == 0);
    assert(rd_val == 0x55);

    // Attempt write to Z-machine static memory address 2000000 (Cylinder 62, Sector 62500)
    int wr_fail_ret = tsfi_s370_zmachine_write_byte(temp_disk, 2000000, 0xAA);
    assert(wr_fail_ret == -2); // Write protected segment violation exception

    free(temp_disk);
    
    printf("  [PASS] Z-machine RAMAC primary/overflow mapping and memory access verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9. System/370 DAT RAMAC translation
    printf("[Test] Verifying System/370 DAT RAMAC address translator...\n");
    tsfi_s370_segment_entry test_segs[2];
    test_segs[0].page_table_origin = 0;
    test_segs[0].length = 1;
    test_segs[0].invalid = 0;

    tsfi_s370_page_entry test_ptes[256];
    test_ptes[0].page_frame_real_addr = 0x0008F000;
    test_ptes[0].invalid = 0;
    test_ptes[0].write_protect = 1;

    tsfi_ramac_chs dat_chs;
    int trans_ret = tsfi_s370_dat_ramac_translate(0x00000124, test_segs, 1, test_ptes, &dat_chs);
    assert(trans_ret == 0);
    printf("  DAT Virtual 0x00000124 -> Real RealAddr 0x0008F124 -> RAMAC Cyl %d, Head %d, Sector %d, Word %d\n",
           dat_chs.cylinder, dat_chs.head, dat_chs.sector, dat_chs.word_offset);
    // Real address 0x0008F124 = 586020 bytes
    // Word index = 586020 / 4 = 146505
    // 146505 / (50 * 20 * 8) = 146505 / 8000 = Cylinder 18
    assert(dat_chs.cylinder == 18);
    printf("  [PASS] System/370 DAT RAMAC address translator verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.2. WinchesterMQ SCSI Handshake Loop Verification
    printf("[Test] Verifying WinchesterMQ SCSI handshake loop...\n");
    uint8_t scsi_status = 0;
    uint8_t scsi_data = 0;
    uint8_t test_stream[] = {0x0A, 0x0B, 0x0C};
    uint8_t scsi_out[4] = {0};
    int scsi_ret = tsfi_s370_winchester_mq_handshake(&scsi_status, &scsi_data, test_stream, 3, scsi_out, 4);
    assert(scsi_ret == 3);
    assert(scsi_out[0] == 0x0A && scsi_out[1] == 0x0B && scsi_out[2] == 0x0C);
    printf("  [PASS] WinchesterMQ SCSI handshake verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.4. Polynomial OSCAR reader calibration curve
    printf("[Test] Verifying polynomial OSCAR reader calibration...\n");
    double coeffs[] = {0.1, 0.2, 0.3}; // y = 0.1 + 0.2*x + 0.3*x^2
    uint8_t oscar_out[16] = {0};
    int oscar_ret = tsfi_s370_oscar_reader_polynomial(0.5, coeffs, 3, oscar_out, 16);
    assert(oscar_ret > 0);
    char oscar_zoned[32] = {0};
    int unpack_ret = tsfi_s370_unpack(oscar_out, oscar_ret, oscar_zoned, 32);
    assert(unpack_ret == 0);
    printf("  OSCAR amplitude 0.5 mapped to polynomial digital string: '%s'\n", oscar_zoned);
    // y = 0.1 + 0.1 + 0.075 = 0.275 -> normalized to 275 BCD
    assert(strcmp(oscar_zoned, "275") == 0);
    printf("  [PASS] Polynomial OSCAR reader verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.6. Punched card-to-COMP3 conversion
    printf("[Test] Verifying IBM 380 punched card conversion to COMP-3 BCD...\n");
    tsfi_ramac_card test_card;
    memset(test_card.columns, ' ', 80);
    memcpy(test_card.columns + 10, "1234K", 5); // 'K' is negative zone punch representation for 2
    uint8_t card_packed[16] = {0};
    int card_ret = tsfi_s370_punched_card_to_comp3(&test_card, 10, 14, card_packed, 16);
    assert(card_ret > 0);
    char card_zoned[32] = {0};
    assert(tsfi_s370_unpack(card_packed, card_ret, card_zoned, 32) == 0);
    printf("  Punched card field parsed to zoned: '%s'\n", card_zoned);
    assert(strcmp(card_zoned, "-12342") == 0);
    printf("  [PASS] IBM punched card parser verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.8. WinchesterMQ SCSI stream to RAMAC disk records
    printf("[Test] Verifying SCSI stream to RAMAC record layout updates...\n");
    uint8_t stream_buf[] = {0xF1, 0xF2, 0xF3};
    uint8_t scsi_st = 0;
    uint8_t scsi_dt = 0;
    int stream_slot = tsfi_s370_scsi_stream_to_ramac(disk, &scsi_st, &scsi_dt, stream_buf, 3, 5);
    assert(stream_slot != -1);
    printf("  SCSI stream committed to RAMAC Cylinder 5, sector slot: %d\n", stream_slot);
    printf("  [PASS] SCSI-to-RAMAC streamer verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9. Soft-body analog sensor calibration decay validation (Rule 10 compliant)
    printf("[Test] Verifying soft-body analog sensor decay validation...\n");
    double sensor_decay[10] = {0.0};
    // Initialize analog input amplitude of 0.8, verifying spring-mass system decay curves
    int sensor_ret = tsfi_s370_oscar_soft_body_validate(0.8, 1.0, 5.0, 2.0, sensor_decay, 10);
    assert(sensor_ret == 0);
    printf("  Sensor initial value: 80.00, step 9 decay value: %.4f\n", sensor_decay[9]);
    assert(fabs(sensor_decay[9]) < 80.00);
    printf("  [PASS] Soft-body analog sensor decay validator verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.5. SAGE redundancy monitor failover verification
    printf("[Test] Verifying SAGE AN/FSQ-7 active-passive redundancy controller...\n");
    int active_cpu = 0;
    int sag_ret = tsfi_s370_sage_redundancy_monitor(1, 1, &active_cpu);
    assert(sag_ret == 0);
    assert(active_cpu == 1); // Primary active when CPU A is online
    
    sag_ret = tsfi_s370_sage_redundancy_monitor(0, 1, &active_cpu);
    assert(sag_ret == 0);
    assert(active_cpu == 2); // Failover to CPU B when CPU A is offline

    sag_ret = tsfi_s370_sage_redundancy_monitor(0, 0, &active_cpu);
    assert(sag_ret == 0);
    assert(active_cpu == 0); // System crash state when both are offline
    printf("  [PASS] SAGE AN/FSQ-7 redundancy monitor verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.7. Engelbart & Bourne Index Resolver Verification
    printf("[Test] Verifying Engelbart & Bourne index resolver...\n");
    const char *abstract_text = "This document describes the design of real-time SAGE systems and radar data integration networks.";
    const char *keywords_list[] = {"real-time", "SAGE", "radar", "nonexistent"};
    uint8_t index_packed[16] = {0};
    int index_ret = tsfi_s370_engelbart_index_resolve(abstract_text, keywords_list, 4, index_packed, 16);
    assert(index_ret > 0);
    char index_zoned[32] = {0};
    assert(tsfi_s370_unpack(index_packed, index_ret, index_zoned, 32) == 0);
    printf("  Document matching keyword frequency: '%s'\n", index_zoned);
    assert(strcmp(index_zoned, "3") == 0); // Matches 'real-time', 'SAGE', 'radar'
    printf("  [PASS] Engelbart & Bourne index resolver verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9. Saburo Muroga Parametron majority gate verification
    printf("[Test] Verifying Saburo Muroga parametron majority phase logic gate...\n");
    int param_phase = -1;
    int p_ret = tsfi_s370_muroga_parametron_majority(1, 0, 1, &param_phase);
    assert(p_ret == 0);
    assert(param_phase == 1); // 180 degrees phase state is the majority

    p_ret = tsfi_s370_muroga_parametron_majority(0, 0, 1, &param_phase);
    assert(p_ret == 0);
    assert(param_phase == 0); // 0 degrees phase state is the majority
    printf("  [PASS] Saburo Muroga parametron logic gate verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.5. Dynamic Parametron Circuit Netlist DAG Evaluation
    printf("[Test] Verifying dynamic parametron circuit DAG netlist evaluator...\n");
    // Construct a 2-node circuit representing (Input0 AND Input1)
    // Node 0: AND mode -> inputs sources are Input0 (-3), Input1 (-4), and Constant Bias 0 (-1)
    // Node 1: NOT mode -> Node 0 output, Constant Bias 0, Constant Bias 0, with first input inverted
    tsfi_parametron_node circuit_nodes[2];
    memset(circuit_nodes, 0, sizeof(circuit_nodes));
    
    // Node 0: sources = {Input0, Input1, Bias0} -> acts as AND gate
    circuit_nodes[0].sources[0] = -3; 
    circuit_nodes[0].sources[1] = -4;
    circuit_nodes[0].sources[2] = -1; 
    
    // Node 1: sources = {Node 0, Bias1, Bias0}, with inversion on Node 0 input -> acts as NOT gate on Node 0 output
    circuit_nodes[1].sources[0] = 0; 
    circuit_nodes[1].sources[1] = -2;
    circuit_nodes[1].sources[2] = -1;
    circuit_nodes[1].invert[0] = 1;  // Invert Node 0 input phase

    int test_inputs[] = {1, 1}; // Input0 = 1, Input1 = 1
    int c_eval = tsfi_s370_parametron_circuit_eval(circuit_nodes, 2, test_inputs, 2);
    assert(c_eval == 0);
    assert(circuit_nodes[0].output == 1); // AND output is 1
    assert(circuit_nodes[1].output == 0); // NOT output is 0

    test_inputs[0] = 0; // Input0 = 0, Input1 = 1
    c_eval = tsfi_s370_parametron_circuit_eval(circuit_nodes, 2, test_inputs, 2);
    assert(c_eval == 0);
    assert(circuit_nodes[0].output == 0); // AND output is 0
    assert(circuit_nodes[1].output == 1); // NOT output is 1
    printf("  [PASS] Dynamic parametron circuit DAG netlist evaluator verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.7. Batten / Peek-a-boo card matching verification
    printf("[Test] Verifying optical batten card matching simulator...\n");
    uint32_t card_x[2] = {0x000F000F, 0xF0F0F0F0};
    uint32_t card_y[2] = {0x0000FFFF, 0x0F0F0F0F};
    uint32_t match_result[2] = {0};
    int holes_count = tsfi_s370_peek_a_boo_card_match(card_x, card_y, match_result, 2);
    assert(holes_count > 0);
    // card_x[0] & card_y[0] = 0x0000000F -> has 4 holes
    // card_x[1] & card_y[1] = 0x00000000 -> has 0 holes
    // total matches should be 4
    printf("  Optical match holes count: %d\n", holes_count);
    assert(holes_count == 4);
    printf("  [PASS] Batten / Peek-a-boo card matching verified successfully.\n");

    free(disk);

    // 4. Layout Optimization Verification
    printf("[Test] Verifying layout serialization...\n");
    tsfi_dat mock_dat;
    mock_dat.capacity = 1000;
    mock_dat.base = (int*)calloc(mock_dat.capacity, sizeof(int));
    mock_dat.check = (int*)calloc(mock_dat.capacity, sizeof(int));
    mock_dat.values = NULL;
    mock_dat.tail = NULL;
    mock_dat.tail_size = 0;

    for (int i = 0; i < mock_dat.capacity; i++) {
        mock_dat.base[i] = i * 2;
        mock_dat.check[i] = i;
    }

    int ret = tsfi_ramac_layout_optimize(&mock_dat, "tmp/test_ramac_layout.dat.bin");
    assert(ret == 0);
    printf("  [PASS] Saved RAMAC layout file to tmp/test_ramac_layout.dat.bin\n");

    free(mock_dat.base);
    free(mock_dat.check);

    printf("[PASS] All RAMAC layout and seek simulation components verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
