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

    // 3.9.9.9.9.9.9.9.9.9.9.8. Muroga Threshold Logic Gate Verification
    printf("[Test] Verifying Saburo Muroga threshold logic gate...\n");
    int th_inputs[3] = {1, 0, 1};
    int th_weights[3] = {2, 3, 1}; // Weighted sum: 1*2 + 0*3 + 1*1 = 3
    int th_output = -1;
    
    int th_ret = tsfi_s370_muroga_threshold_gate(th_inputs, th_weights, 3, 3, &th_output);
    assert(th_ret == 0);
    assert(th_output == 1); // Weighted sum 3 >= threshold 3

    th_ret = tsfi_s370_muroga_threshold_gate(th_inputs, th_weights, 3, 4, &th_output);
    assert(th_ret == 0);
    assert(th_output == 0); // Weighted sum 3 < threshold 4
    printf("  [PASS] Saburo Muroga threshold logic gate verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9. Autonetics Recomp II Word Decoder Verification
    printf("[Test] Verifying Autonetics Recomp II word decoder...\n");
    // Build a 40-bit word with two instructions:
    // Left: Opcode = 0x1E (30), Address = 0x7A0 (1952) -> 20-bit: (30 << 15) | (1952 << 3) = 0xF3D00
    // Right: Opcode = 0x0A (10), Address = 0x120 (288) -> 20-bit: (10 << 15) | (288 << 3) = 0x50900
    // Combined 40-bit: (0xF3D00 << 20) | 0x50900 = 0xF3D0050900
    uint64_t raw_recomp_word = 0xF3D0050900ULL;
    int op1 = 0, addr1 = 0, op2 = 0, addr2 = 0;
    int recomp_dec_ret = tsfi_s370_recomp_ii_decode_word(raw_recomp_word, &op1, &addr1, &op2, &addr2);
    assert(recomp_dec_ret == 0);
    assert(op1 == 30);
    assert(addr1 == 1952);
    assert(op2 == 10);
    assert(addr2 == 288);
    printf("  [PASS] Autonetics Recomp II word decoder verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.2. Autonetics Recomp II Drum Scheduler Verification
    printf("[Test] Verifying Autonetics Recomp II drum scheduling latency optimization...\n");
    int optimal_sector = -1;
    // Current sector = 10, execution cycles = 15.
    // 15 cycles fits in 2 sectors (15 + 7)/8 = 2.
    // Optimal target sector = (10 + 2 + 1) = 13
    int sched_ret = tsfi_s370_recomp_ii_drum_schedule(10, 15, &optimal_sector);
    assert(sched_ret == 0);
    assert(optimal_sector == 13);
    printf("  [PASS] Autonetics Recomp II drum scheduler verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.3. Perforated Paper Tape Synthesizer Verification
    printf("[Test] Verifying perforated paper tape synthesizer sequencer...\n");
    // Trigger tape: Row 0 has bit 0 (Bass) and bit 3 (A4) active (0x09)
    uint8_t dummy_tape[] = {0x09, 0x00, 0x01};
    double synth_audio[1000] = {0.0};
    int synth_ret = tsfi_s370_paper_tape_synthesizer(dummy_tape, 3, 8, synth_audio, 1000, 8000.0);
    assert(synth_ret == 0);
    // At sample 0, the audio should be non-zero since row 0 has active triggers
    printf("  Synthesizer sample 0 value: %.4f\n", synth_audio[0]);
    assert(fabs(synth_audio[0]) < 0.000001); // Initial sin(0) = 0, but sample 1 is non-zero
    printf("  Synthesizer sample 1 value: %.4f\n", synth_audio[1]);
    assert(fabs(synth_audio[1]) > 0.0001); // Trigger active sound wave decay
    printf("  [PASS] Perforated paper tape synthesizer verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.4. MIT Lincoln Laboratory TX-2 SIMD ALU Verification
    printf("[Test] Verifying MIT Lincoln Laboratory TX-2 SIMD ALU...\n");
    uint64_t alu_out = 0;
    // 18-bit SIMD ADD mode: High half: 0x10 + 0x05 = 0x15. Low half: 0x20 + 0x0A = 0x2A
    // Input A: (0x10 << 18) | 0x20 = 0x400020
    // Input B: (0x05 << 18) | 0x0A = 0x14000A
    int alu_ret = tsfi_s370_tx2_simd_alu(0x400020ULL, 0x14000AULL, 18, "ADD", &alu_out);
    assert(alu_ret == 0);
    assert(alu_out == (((0x15ULL) << 18) | 0x2AULL));
    printf("  [PASS] TX-2 SIMD ALU verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.5. MIT Lincoln Laboratory TX-2 Light Pen Tracking Loop Verification
    printf("[Test] Verifying MIT Lincoln Laboratory TX-2 light pen tracking loop...\n");
    double track_x = 10.0;
    double track_y = 10.0;
    // Pen is slightly offset to the right at (10.05, 10.0)
    int tracked_points = tsfi_s370_tx2_light_pen_track(10.05, 10.0, &track_x, &track_y, 0.2);
    assert(tracked_points > 0);
    // The cross should shift towards the pen position
    assert(track_x > 10.0);
    printf("  Centroid relocated to: (%.4f, %.4f)\n", track_x, track_y);
    printf("  [PASS] TX-2 light pen tracking loop verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.6. Ramo-Wooldridge RW-400 Matrix Switch Verification
    printf("[Test] Verifying RW-400 polymorphic matrix switch...\n");
    // Matrix: 3 CPUs, 4 Buffers
    // Requested connections: CPU 0 -> Buffer 2, CPU 1 -> Buffer 0, CPU 2 -> Buffer 3
    int conn[12] = {
        0, 0, 1, 0, // CPU 0
        1, 0, 0, 0, // CPU 1
        0, 0, 0, 1  // CPU 2
    };
    int routes[3] = {-1, -1, -1};
    int sw_ret = tsfi_s370_rw400_matrix_switch(conn, 3, 4, routes);
    assert(sw_ret == 0);
    assert(routes[0] == 2);
    assert(routes[1] == 0);
    assert(routes[2] == 3);

    // Conflict case: CPU 0 and CPU 1 both request Buffer 2
    int conn_conflict[12] = {
        0, 0, 1, 0, // CPU 0 -> Buffer 2
        0, 0, 1, 0, // CPU 1 -> Buffer 2
        0, 0, 0, 1  // CPU 2 -> Buffer 3
    };
    int sw_conflict_ret = tsfi_s370_rw400_matrix_switch(conn_conflict, 3, 4, routes);
    assert(sw_conflict_ret == -1); // Routing failed due to destination conflict
    printf("  [PASS] RW-400 polymorphic matrix switch routing verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.7. UNCOL Universal Computer Oriented Language VM Verification
    printf("[Test] Verifying UNCOL VM execution...\n");
    // Write simple program:
    // Memory[0] = 10, Memory[1] = 5
    // Program: LOAD R0, [0]; LOAD R1, [1]; ADD R2, R0, R1; STORE R2, [2]
    int uncol_mem[4] = {10, 5, 0, 0};
    int uncol_regs[8] = {0};
    tsfi_uncol_instruction uncol_prog[4] = {
        {"LOAD", 0, 0, 0, 0},
        {"LOAD", 1, 0, 0, 1},
        {"ADD", 2, 0, 1, 0},
        {"STORE", 2, 0, 0, 2}
    };
    int vm_ret = tsfi_s370_uncol_vm_exec(uncol_prog, 4, uncol_mem, 4, uncol_regs, 8);
    assert(vm_ret == 0);
    assert(uncol_regs[2] == 15);
    assert(uncol_mem[2] == 15);
    printf("  [PASS] UNCOL VM execution verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.8. Polymorphic WinchesterMQ SCSI Routing Verification
    printf("[Test] Verifying polymorphic WinchesterMQ SCSI routing...\n");
    // Allocate fresh disk to verify SCSI commits
    tsfi_ramac_record *poly_disk = (tsfi_ramac_record*)calloc(RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS, sizeof(tsfi_ramac_record));
    assert(poly_disk != NULL);

    int poly_conn[4] = {
        0, 1, // Initiator 0 -> Target 1
        1, 0  // Initiator 1 -> Target 0
    };
    uint8_t scsi_statuses[2] = {0, 0};
    uint8_t data_regs[2] = {0, 0};
    uint8_t str0[3] = {42, 43, 44};
    uint8_t str1[3] = {88, 89, 90};
    const uint8_t *streams_arr[2] = {str0, str1};
    int stream_lens_arr[2] = {3, 3};
    int poly_routes[2] = {-1, -1};

    int poly_ret = tsfi_s370_polymorphic_winchester_mq_route(poly_conn, 2, 2, scsi_statuses, data_regs,
                                                            streams_arr, stream_lens_arr, poly_disk, poly_routes);
    assert(poly_ret == 0);
    assert(poly_routes[0] == 1); // Initiator 0 routed to Target 1 (Cylinder 10)
    assert(poly_routes[1] == 0); // Initiator 1 routed to Target 0 (Cylinder 0)

    // Verify written data (SCSI keys should be present in target cylinder memory slices)
    double seek_poly = 0.0;
    const char *val_i0 = tsfi_ramac_search_record(poly_disk, "scsi_key_42", 10, &seek_poly);
    assert(val_i0 != NULL);
    assert(strcmp(val_i0, "scsi_val_44") == 0);

    const char *val_i1 = tsfi_ramac_search_record(poly_disk, "scsi_key_88", 0, &seek_poly);
    assert(val_i1 != NULL);
    assert(strcmp(val_i1, "scsi_val_90") == 0);

    free(poly_disk);
    printf("  [PASS] Polymorphic WinchesterMQ SCSI routing verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.9. ZMM Lock Registry Verification
    printf("[Test] Verifying ZMM lock registry...\n");
    tsfi_zmm_lock_registry zmm_locks;
    tsfi_s370_zmm_lock_init(&zmm_locks);

    // Acquire shared lock on cylinder 5 (initiator 1, priority 2)
    int acq1 = tsfi_s370_zmm_lock_acquire(&zmm_locks, 1, 5, 1, 100, 2);
    assert(acq1 == 0);

    // Try to acquire conflicting exclusive lock on cylinder 5 (initiator 2, priority 2) -> denied
    int acq2 = tsfi_s370_zmm_lock_acquire(&zmm_locks, 2, 5, 2, 101, 2);
    assert(acq2 == -2);

    // Try to acquire exclusive lock with high priority (initiator 3, priority 8) -> preempted success
    int acq3 = tsfi_s370_zmm_lock_acquire(&zmm_locks, 3, 5, 2, 102, 8);
    assert(acq3 == 2);
    assert(zmm_locks.cylinder_owners[5] == 3);

    // Release owned lock
    int rel1 = tsfi_s370_zmm_lock_release(&zmm_locks, 3, 5);
    assert(rel1 == 0);
    assert(zmm_locks.locked_cylinders[5] == 0);

    printf("  [PASS] ZMM lock registry verified successfully.\n");

    // 3.9.9.9.9.9.9.9.9.9.9.9.9.9. ZY-IR Yul and ZMM Aware IR Verification
    printf("[Test] Verifying ZY-IR executor...\n");
    uint8_t *yul_memory = (uint8_t*)calloc(256, 1);
    int zyir_regs[8] = {0};
    uint64_t ticks = 0;
    tsfi_zmm_lock_registry zyir_locks;
    tsfi_s370_zmm_lock_init(&zyir_locks);
    
    // Allocate fresh disk for testing
    tsfi_ramac_record *zyir_disk = (tsfi_ramac_record*)calloc(RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS, sizeof(tsfi_ramac_record));

    // Program:
    // registers[0] = 500 (loaded via MLOAD from yul_memory)
    // Acquire Exclusive Lock on cylinder 12 (using register 1 as lock mode 2)
    // Write register 0 value to ZMM cylinder 12
    // Read from ZMM cylinder 12 into register 2
    // Store register 2 to yul_memory address 32
    yul_memory[0] = 0;
    yul_memory[1] = 0;
    yul_memory[2] = 1;
    yul_memory[3] = 244; // 500
    zyir_regs[1] = 2; // Exclusive lock mode

    tsfi_zyir_instruction zyir_prog[5] = {
        {"MLOAD", 0, 0, 0, 0},        // reg[0] = memory[0..3] (500)
        {"ZLOCK", 3, 1, 0, 12},       // reg[3] = ZLOCK(cylinder 12, mode 2)
        {"ZWRITE", 0, 0, 0, 12},      // ZWRITE(cylinder 12, key_9, value_500)
        {"ZREAD", 2, 0, 0, 12},       // reg[2] = ZREAD(cylinder 12)
        {"MSTORE", 0, 2, 0, 32}       // memory[32..35] = reg[2] (500)
    };

    int zyir_ret = tsfi_s370_zyir_exec(zyir_prog, 5, yul_memory, 256, zyir_disk, &zyir_locks, 9, 2, zyir_regs, 8, &ticks);
    assert(zyir_ret == 0);
    assert(zyir_regs[2] == 500);
    assert(yul_memory[35] == 244);
    
    free(zyir_disk);
    free(yul_memory);
    printf("  [PASS] ZY-IR executor verified successfully.\n");

    // Test Scenario 1: ZMM Lock Timeout Eviction Verification
    printf("[Test] Verifying ZMM lock timeout eviction...\n");
    tsfi_zmm_lock_registry time_locks;
    tsfi_s370_zmm_lock_init(&time_locks);
    // Acquire lock on cylinder 15 at tick 100
    int ret_acq = tsfi_s370_zmm_lock_acquire(&time_locks, 1, 15, 2, 100, 1);
    assert(ret_acq == 0);
    // Attempt conflict acquire at tick 150 -> denied
    int ret_fail = tsfi_s370_zmm_lock_acquire(&time_locks, 2, 15, 2, 150, 1);
    assert(ret_fail == -2);
    // Attempt conflict acquire at tick 1200 (> 1000 tick duration) -> success due to stale lock eviction
    int ret_evict = tsfi_s370_zmm_lock_acquire(&time_locks, 2, 15, 2, 1200, 1);
    assert(ret_evict == 0);
    assert(time_locks.cylinder_owners[15] == 2);
    printf("  [PASS] ZMM lock timeout eviction verified successfully.\n");

    // Test Scenario 2: ZY-IR Access Violations (ZWRITE without lock)
    printf("[Test] Verifying ZY-IR access violation handling...\n");
    uint8_t *viol_mem = (uint8_t*)calloc(256, 1);
    int viol_regs[8] = {0};
    uint64_t viol_ticks = 0;
    tsfi_zmm_lock_registry viol_locks;
    tsfi_s370_zmm_lock_init(&viol_locks);
    tsfi_ramac_record *viol_disk = (tsfi_ramac_record*)calloc(RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS, sizeof(tsfi_ramac_record));

    // Try to write without locking cylinder 20
    tsfi_zyir_instruction viol_prog[1] = {
        {"ZWRITE", 0, 0, 0, 20}
    };
    int viol_ret = tsfi_s370_zyir_exec(viol_prog, 1, viol_mem, 256, viol_disk, &viol_locks, 9, 2, viol_regs, 8, &viol_ticks);
    assert(viol_ret == -3); // Returns access violation
    printf("  [PASS] ZY-IR write access violation verified successfully.\n");
    free(viol_disk);
    free(viol_mem);

    // Test Scenario 3: Complex ZY-IR Pipeline (multiple cylinders routing)
    printf("[Test] Verifying complex ZY-IR pipeline routing...\n");
    uint8_t *pipe_mem = (uint8_t*)calloc(256, 1);
    int pipe_regs[8] = {0};
    uint64_t pipe_ticks = 0;
    tsfi_zmm_lock_registry pipe_locks;
    tsfi_s370_zmm_lock_init(&pipe_locks);
    tsfi_ramac_record *pipe_disk = (tsfi_ramac_record*)calloc(RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS, sizeof(tsfi_ramac_record));

    // Operations:
    // Write 77 to cylinder 4 (requires Exclusive Lock)
    // Read from cylinder 4, store in memory offset 64
    // Write 99 to cylinder 5 (requires Exclusive Lock)
    // Read from cylinder 5, store in memory offset 68
    pipe_regs[0] = 77;
    pipe_regs[1] = 99;
    pipe_regs[4] = 2; // Exclusive lock mode

    tsfi_zyir_instruction pipe_prog[10] = {
        {"ZLOCK", 5, 4, 0, 4},   // Lock cyl 4 -> reg[5]
        {"ZWRITE", 0, 0, 0, 4},  // Write reg[0] (77)
        {"ZREAD", 6, 0, 0, 4},   // Read cyl 4 -> reg[6]
        {"MSTORE", 0, 6, 0, 64}, // Store reg[6] to mem[64]
        {"ZRELEASE", 5, 0, 0, 4},// Release cyl 4
        {"ZLOCK", 5, 4, 0, 5},   // Lock cyl 5 -> reg[5]
        {"ZWRITE", 0, 1, 0, 5},  // Write reg[1] (99)
        {"ZREAD", 7, 0, 0, 5},   // Read cyl 5 -> reg[7]
        {"MSTORE", 0, 7, 0, 68}, // Store reg[7] to mem[68]
        {"ZRELEASE", 5, 0, 0, 5} // Release cyl 5
    };

    int pipe_ret = tsfi_s370_zyir_exec(pipe_prog, 10, pipe_mem, 256, pipe_disk, &pipe_locks, 10, 2, pipe_regs, 8, &pipe_ticks);
    assert(pipe_ret == 0);
    assert(pipe_mem[67] == 77);
    assert(pipe_mem[71] == 99);

    free(pipe_disk);
    free(pipe_mem);
    printf("  [PASS] Complex ZY-IR pipeline verified successfully.\n");

    // Test Scenario 4: UNCOL to Yul code compiler
    printf("[Test] Verifying UNCOL to Yul compiler...\n");
    tsfi_uncol_instruction uncol_yul_prog[3] = {
        {"LOAD", 0, 0, 0, 64},
        {"ADD", 2, 0, 0, 0},
        {"STORE", 2, 0, 0, 128}
    };
    char yul_out[256];
    int yul_comp_ret = tsfi_s370_uncol_to_yul(uncol_yul_prog, 3, yul_out, 256);
    assert(yul_comp_ret == 0);
    assert(strstr(yul_out, "let r0 := mload(64)") != NULL);
    assert(strstr(yul_out, "let r2 := add(r0, r0)") != NULL);
    assert(strstr(yul_out, "mstore(128, r2)") != NULL);
    printf("  [PASS] UNCOL to Yul compiler verified successfully.\n");

    // Test Scenario 5: Philco 212 Instruction Decoder & Modification Modes
    printf("[Test] Verifying Philco 212 CPU instruction decoder and modes...\n");
    // Pack two instructions:
    // Left: opcode = 0xA2, index_reg = 3, mod_mode = 1 (post-increment), address = 100
    // left_val = (0xA2 << 16) | (3 << 13) | (1 << 11) | 100 = 0xA26864
    // Right: opcode = 0xB5, index_reg = 2, mod_mode = 2 (post-decrement), address = 200
    // right_val = (0xB5 << 16) | (2 << 13) | (2 << 11) | 200 = 0xB550C8
    uint64_t philco_word = ((uint64_t)0xA26864 << 24) | 0xB550C8;
    tsfi_philco212_instruction inst_l, inst_r;
    int decode_ret = tsfi_s370_philco212_decode(philco_word, &inst_l, &inst_r);
    assert(decode_ret == 0);
    assert(inst_l.opcode == 0xA2);
    assert(inst_l.index_reg == 3);
    assert(inst_l.mod_mode == 1);
    assert(inst_l.address == 100);

    assert(inst_r.opcode == 0xB5);
    assert(inst_r.index_reg == 2);
    assert(inst_r.mod_mode == 2);
    assert(inst_r.address == 200);

    // Verify Address Modification Modes
    int index_regs[8] = {0, 10, 50, 80, 0, 0, 0, 0};
    uint16_t mod_addr_l = 0;
    uint16_t mod_addr_r = 0;

    // Test left: Address = 100, Index Register 3 has value 80, mode 1 (post-increment)
    // Modified address should be 100 + 80 = 180. Post value should increment to 81.
    int mod_ret_l = tsfi_s370_philco212_modify_address(&inst_l, index_regs, 8, &mod_addr_l);
    assert(mod_ret_l == 0);
    assert(mod_addr_l == 180);
    assert(index_regs[3] == 81);

    // Test right: Address = 200, Index Register 2 has value 50, mode 2 (post-decrement)
    // Modified address should be 200 + 50 = 250. Post value should decrement to 49.
    int mod_ret_r = tsfi_s370_philco212_modify_address(&inst_r, index_regs, 8, &mod_addr_r);
    assert(mod_ret_r == 0);
    assert(mod_addr_r == 250);
    assert(index_regs[2] == 49);

    printf("  [PASS] Philco 212 CPU decoder and modes verified successfully.\n");

    // Test Scenario 6: Bendix G-20 Instruction Decoder & Index Registers
    printf("[Test] Verifying Bendix G-20 CPU instruction decoder and memory indexing...\n");
    // Opcode = 25 (0x19), Index Reg = 14 (0x0E), Address = 4096 (0x1000), Flags = 7 (0x07)
    // raw_word = (25 << 26) | (14 << 20) | (4096 << 5) | 7
    // raw_word = 0x64000000 | 0x00E00000 | 0x00020000 | 7 = 0x64E20007
    uint32_t g20_word = 0x64E20007;
    tsfi_bendixg20_instruction g20_inst;
    int g20_dec_ret = tsfi_s370_bendixg20_decode(g20_word, &g20_inst);
    assert(g20_dec_ret == 0);
    assert(g20_inst.opcode == 25);
    assert(g20_inst.index_reg == 14);
    assert(g20_inst.address == 4096);
    assert(g20_inst.flags == 7);

    // Verify Address Resolution using Memory Pool
    int g20_mem_pool[64];
    memset(g20_mem_pool, 0, sizeof(g20_mem_pool));
    // Index Register 14 (Memory Location 14) holds offset value 1024
    g20_mem_pool[14] = 1024;

    uint32_t eff_addr = 0;
    int g20_res_ret = tsfi_s370_bendixg20_resolve_address(&g20_inst, g20_mem_pool, 64, &eff_addr);
    assert(g20_res_ret == 0);
    // Effective address should be 4096 + 1024 = 5120
    assert(eff_addr == 5120);

    printf("  [PASS] Bendix G-20 CPU decoder and memory indexing verified successfully.\n");

    // Test Scenario 7: Librascope LGP-30 Twin-Triode Flip-Flop Physical Simulation
    printf("[Test] Verifying Librascope LGP-30 vacuum tube twin-triode flip flop...\n");
    tsfi_lgp30_flipflop ff;
    tsfi_lgp30_flipflop_init(&ff);

    // Run simulator for 100 ticks with no triggers, it should stabilize to Triode 2 conducting
    double dt = 1.0e-6; // 1 microsecond steps
    for (int i = 0; i < 100; i++) {
        tsfi_lgp30_flipflop_tick(&ff, 0.0, 0.0, dt);
    }
    // Triode 2 plate voltage should be low (conducting), Triode 1 plate voltage high (cutoff)
    assert(ff.triode2_plate_v < 100.0);
    assert(ff.triode1_plate_v > 130.0);

    // Apply trigger SET pulse (e.g. +80V) to turn Triode 1 on and toggle state
    for (int i = 0; i < 200; i++) {
        tsfi_lgp30_flipflop_tick(&ff, 80.0, 0.0, dt);
    }
    // Now state should have toggled: Triode 1 conducting (low plate V), Triode 2 cutoff (high plate V)
    assert(ff.triode1_plate_v < 100.0);
    assert(ff.triode2_plate_v > 130.0);

    // Remove trigger, run 50 ticks to ensure latching (stability)
    for (int i = 0; i < 50; i++) {
        tsfi_lgp30_flipflop_tick(&ff, 0.0, 0.0, dt);
    }
    assert(ff.triode1_plate_v < 100.0);
    assert(ff.triode2_plate_v > 130.0);

    printf("  [PASS] Librascope LGP-30 vacuum tube flip flop physical latching verified successfully.\n");

    // Test Scenario 8: Bendix G-15 DA-1 Digital Differential Analyzer (DDA)
    printf("[Test] Verifying Bendix G-15 DA-1 Digital Differential Analyzer...\n");
    tsfi_bendixg15_dda_integrator dda[2];
    
    // Setup integrator 0: Sine wave generator
    dda[0].y = 0;
    dda[0].r = 0;
    dda[0].limit = 1000;
    dda[0].output_dz = 0;
    dda[0].src_dx_integrator = -1; // time step dx=1
    dda[0].src_dy_integrator = 1;  // dy from Cosine (integrator 1)
    dda[0].dy_invert = 0;

    // Setup integrator 1: Cosine wave generator
    dda[1].y = 1000; // start value
    dda[1].r = 0;
    dda[1].limit = 1000;
    dda[1].output_dz = 0;
    dda[1].src_dx_integrator = -1; // time step dx=1
    dda[1].src_dy_integrator = 0;  // dy from Sine (integrator 0)
    dda[1].dy_invert = 1;          // inverted dy output (negative feedback for oscillation)

    // Execute 60 ticks to run simulation
    for (int i = 0; i < 60; i++) {
        int dda_ret = tsfi_s370_bendixg15_dda_tick(dda, 2);
        assert(dda_ret == 0);
    }
    // Verify that the integrators accumulated values (sine/cosine oscillation values)
    assert(dda[0].y != 0);
    assert(dda[1].y < 1000); // Decayed as it transfers energy to dda[0]

    printf("  [PASS] Bendix G-15 DA-1 DDA co-processor simulation verified successfully.\n");

    // Test Scenario 9: LGP-30 instruction set interpreter strategy runner
    printf("[Test] Verifying Librascope LGP-30 machine strategy runner...\n");
    int lgp_mem[200] = {0};
    int lgp_acc = 0;
    int lgp_pc = 0;

    // Load initial funds: Bring memory location 100
    lgp_mem[0] = (0 << 20) | 100;
    // Subtract cost: Subtract location 101
    lgp_mem[1] = (3 << 20) | 101;
    // Multiply by yield factor: Multiply location 102
    lgp_mem[2] = (4 << 20) | 102;
    // Store result to temp holding location 103
    lgp_mem[3] = (9 << 20) | 103;
    // Stop
    lgp_mem[4] = (11 << 20) | 0;

    // Set memory values
    lgp_mem[100] = 50000; // initial funds
    lgp_mem[101] = 40000; // cost
    lgp_mem[102] = 2;     // yield factor

    int lgp_ret = tsfi_s370_lgp30_interpreter(lgp_mem, 200, &lgp_acc, &lgp_pc, 100);
    assert(lgp_ret > 0);
    assert(lgp_mem[103] == 20000); // (50000 - 40000) * 2 = 20000 holding value
    printf("  [PASS] Librascope LGP-30 machine strategy runner verified successfully.\n");

    // Test Scenario 10: UNCOL to LGP-30 target bytecode compiler
    printf("[Test] Verifying UNCOL to LGP-30 compiler...\n");
    tsfi_uncol_instruction lgp_uncol_prog[3];
    strcpy(lgp_uncol_prog[0].op, "LOAD");
    lgp_uncol_prog[0].address = 50;
    strcpy(lgp_uncol_prog[1].op, "ADD");
    lgp_uncol_prog[1].address = 51;
    strcpy(lgp_uncol_prog[2].op, "STORE");
    lgp_uncol_prog[2].address = 52;

    int compiled_lgp[10] = {0};
    int comp_count = tsfi_s370_uncol_to_lgp30(lgp_uncol_prog, 3, compiled_lgp, 10);
    assert(comp_count == 3);
    assert(compiled_lgp[0] == ((0 << 20) | 50)); // Bring 50
    assert(compiled_lgp[1] == ((2 << 20) | 51)); // Add 51
    assert(compiled_lgp[2] == ((9 << 20) | 52)); // Hold/Store 52
    printf("  [PASS] UNCOL to LGP-30 compiler verified successfully.\n");

    free(disk);

    // Test Scenario 11: IBM 7030 STRETCH bit-addressable memory access
    printf("[Test] Verifying IBM 7030 STRETCH bit-addressable memory...\n");
    uint64_t stretch_mem[4] = {0};
    
    // Write 0xDEADBEEF (32 bits) at bit address 48 (spanning word index 0 and 1)
    int stretch_ret = tsfi_s370_ibm7030_write_bits(stretch_mem, 48, 32, 0xDEADBEEFULL);
    assert(stretch_ret == 0);

    uint64_t stretch_val = 0;
    stretch_ret = tsfi_s370_ibm7030_read_bits(stretch_mem, 48, 32, &stretch_val);
    assert(stretch_ret == 0);
    assert(stretch_val == 0xDEADBEEFULL);

    // Verify raw memory values:
    // Low 16 bits of 0xDEADBEEF (0xBEEF) at memory[0] bits 48..63
    assert((stretch_mem[0] >> 48) == 0xBEEFULL);
    // High 16 bits of 0xDEADBEEF (0xDEAD) at memory[1] bits 0..15
    assert((stretch_mem[1] & 0xFFFFULL) == 0xDEADULL);
    printf("  [PASS] IBM 7030 STRETCH bit-addressable memory verified successfully.\n");

    // Test Scenario 12: IBM 7030 STRETCH Hamming SEC-DED ECC
    printf("[Test] Verifying IBM 7030 STRETCH Hamming SEC-DED ECC...\n");
    uint64_t data_word = 0x0023456789ABCDEFULL;
    uint64_t ecc_word = tsfi_s370_ibm7030_ecc_encode(data_word);

    // 1. Decode clean word
    uint64_t decoded_val = 0;
    int ecc_ret = tsfi_s370_ibm7030_ecc_decode(ecc_word, &decoded_val);
    assert(ecc_ret == 0);
    assert(decoded_val == data_word);

    // 2. Decode with single-bit error (bit position 17)
    uint64_t corrupted_word1 = ecc_word ^ (1ULL << 17);
    ecc_ret = tsfi_s370_ibm7030_ecc_decode(corrupted_word1, &decoded_val);
    assert(ecc_ret == 1); // Corrected
    assert(decoded_val == data_word);

    // 3. Decode with double-bit error (bit positions 17 and 25)
    uint64_t corrupted_word2 = ecc_word ^ (1ULL << 17) ^ (1ULL << 25);
    ecc_ret = tsfi_s370_ibm7030_ecc_decode(corrupted_word2, &decoded_val);
    assert(ecc_ret == 2); // Double error detected

    printf("  [PASS] IBM 7030 STRETCH Hamming SEC-DED ECC verified successfully.\n");

    // Test Scenario 13: EVM-Native Yul SEC-DED ECC Validation
    printf("[Test] Verifying EVM-Native Yul SEC-DED ECC Contract...\n");
    extern bool lau_yul_thunk_init(const char *name, const char *yul_path, uint64_t virtual_address);
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);

    bool yul_init = lau_yul_thunk_init("sec_ded", "../solidity/bin/sec_ded.yul", 0x777);
    assert(yul_init);

    // 1. Test encoding a value using Yul
    uint8_t enc_cd[36] = {0x51, 0xc7, 0x20, 0x54};
    uint64_t test_data = 0x0023456789ABCDEFULL;
    for (int k = 0; k < 8; k++) {
        enc_cd[35 - k] = (test_data >> (k * 8)) & 0xFF;
    }
    uint8_t enc_ret[32];
    size_t enc_ret_len = 32;
    bool exec_ok = lau_yul_thunk_execute("sec_ded", enc_cd, 36, enc_ret, &enc_ret_len);
    assert(exec_ok);

    uint64_t yul_ecc_word = 0;
    for (int k = 0; k < 8; k++) {
        yul_ecc_word |= ((uint64_t)enc_ret[31 - k] << (k * 8));
    }
    uint64_t expected_ecc = tsfi_s370_ibm7030_ecc_encode(test_data);
    assert(yul_ecc_word == expected_ecc);
    printf("  [PASS] Yul encoder output matches C implementation.\n");

    // 2. Test decoding clean word using Yul
    uint8_t dec_cd[36] = {0x90, 0x0f, 0x0a, 0xa0};
    for (int k = 0; k < 8; k++) {
        dec_cd[35 - k] = (yul_ecc_word >> (k * 8)) & 0xFF;
    }
    uint8_t yul_dec_ret[64];
    size_t dec_ret_len = 64;
    exec_ok = lau_yul_thunk_execute("sec_ded", dec_cd, 36, yul_dec_ret, &dec_ret_len);
    assert(exec_ok);

    uint64_t yul_decoded_val = 0;
    uint64_t yul_err_code = 0;
    for (int k = 0; k < 8; k++) {
        yul_decoded_val |= ((uint64_t)yul_dec_ret[31 - k] << (k * 8));
        yul_err_code |= ((uint64_t)yul_dec_ret[63 - k] << (k * 8));
    }
    assert(yul_decoded_val == test_data);
    assert(yul_err_code == 0);
    printf("  [PASS] Yul decoder successfully decoded clean word.\n");

    // 3. Test decoding single-bit error using Yul
    uint64_t corrupted_yul_word1 = yul_ecc_word ^ (1ULL << 17);
    for (int k = 0; k < 8; k++) {
        dec_cd[35 - k] = (corrupted_yul_word1 >> (k * 8)) & 0xFF;
    }
    dec_ret_len = 64;
    exec_ok = lau_yul_thunk_execute("sec_ded", dec_cd, 36, yul_dec_ret, &dec_ret_len);
    assert(exec_ok);

    yul_decoded_val = 0;
    yul_err_code = 0;
    for (int k = 0; k < 8; k++) {
        yul_decoded_val |= ((uint64_t)yul_dec_ret[31 - k] << (k * 8));
        yul_err_code |= ((uint64_t)yul_dec_ret[63 - k] << (k * 8));
    }
    assert(yul_decoded_val == test_data);
    assert(yul_err_code == 1);
    printf("  [PASS] Yul decoder successfully corrected single-bit error.\n");

    // 4. Test decoding double-bit error using Yul
    uint64_t corrupted_yul_word2 = yul_ecc_word ^ (1ULL << 17) ^ (1ULL << 25);
    for (int k = 0; k < 8; k++) {
        dec_cd[35 - k] = (corrupted_yul_word2 >> (k * 8)) & 0xFF;
    }
    dec_ret_len = 64;
    exec_ok = lau_yul_thunk_execute("sec_ded", dec_cd, 36, yul_dec_ret, &dec_ret_len);
    assert(exec_ok);

    yul_err_code = 0;
    for (int k = 0; k < 8; k++) {
        yul_err_code |= ((uint64_t)yul_dec_ret[63 - k] << (k * 8));
    }
    assert(yul_err_code == 2);
    printf("  [PASS] Yul decoder successfully detected double-bit error.\n");

    printf("  [PASS] EVM-Native Yul SEC-DED ECC verified successfully.\n");

    // Test Scenario 14: IBM 7030 STRETCH Index Register Auto-Modification
    printf("[Test] Verifying IBM 7030 STRETCH Index Register Auto-Modification...\n");
    tsfi_ibm7030_index_reg idx_reg = {
        .value = 100,
        .count = 2,
        .limit = 105
    };
    uint64_t indicators = 0;
    uint32_t offset = 0;

    // 1. Modifying within limits
    int idx_res = tsfi_s370_ibm7030_index_modify(&idx_reg, 2, &indicators, &offset);
    assert(idx_res == 0);
    assert(offset == 100);
    assert(idx_reg.value == 102);
    assert(idx_reg.count == 1);
    assert(indicators == 0);

    // 2. Modifying to exceed limit and trigger trap
    idx_res = tsfi_s370_ibm7030_index_modify(&idx_reg, 3, &indicators, &offset);
    assert(idx_res == 1);
    assert(offset == 102);
    assert(idx_reg.value == 105);
    assert(idx_reg.count == 0);
    assert((indicators & (1ULL << 1)) != 0);
    assert((indicators & (1ULL << 2)) != 0);

    printf("  [PASS] IBM 7030 STRETCH Index Register Auto-Modification verified successfully.\n");

    // Test Scenario 15: IBM 7030 STRETCH Look-Ahead Unit Pipelined Queue
    printf("[Test] Verifying IBM 7030 STRETCH Look-Ahead Unit Pipelined Queue...\n");
    tsfi_ibm7030_lau_queue lau_q;
    tsfi_s370_ibm7030_lau_init(&lau_q);

    uint64_t mock_memory[64] = {0};
    mock_memory[10] = 0xDEADBEEF1234ULL;

    // 1. Push look-ahead load of address 10
    int lau_res = tsfi_s370_ibm7030_lau_push_load(&lau_q, 10);
    assert(lau_res == 0);

    // 2. Push look-ahead store to address 10 (triggers RAW forwarding)
    lau_res = tsfi_s370_ibm7030_lau_push_store(&lau_q, 10, 0xFEEDFACE5678ULL);
    assert(lau_res == 0);

    // Verify load entry is forwarded
    assert(lau_q.entries[0].forwarded == 1);
    assert(lau_q.entries[0].value == 0xFEEDFACE5678ULL);

    // 3. Commit look-ahead operations to memory
    int commits = tsfi_s370_ibm7030_lau_commit(&lau_q, mock_memory, 64);
    assert(commits == 2);
    assert(mock_memory[10] == 0xFEEDFACE5678ULL);

    printf("  [PASS] IBM 7030 STRETCH Look-Ahead Unit verified successfully.\n");

    // Test Scenario 16: IBM 7030 STRETCH Variable Field Length (VFL) Arithmetic
    printf("[Test] Verifying IBM 7030 STRETCH VFL Arithmetic...\n");
    uint64_t vfl_memory[4] = {0};

    int write_res = tsfi_s370_ibm7030_write_bits(vfl_memory, 10, 8, 0x55);
    assert(write_res == 0);

    write_res = tsfi_s370_ibm7030_write_bits(vfl_memory, 25, 8, 0xAA);
    assert(write_res == 0);

    uint64_t vfl_sum = 0;
    int vfl_res = tsfi_s370_ibm7030_vfl_add(vfl_memory, 10, 1, 8, 25, 1, 8, &vfl_sum);
    assert(vfl_res == 0);
    assert(vfl_sum == 0xFF);

    printf("  [PASS] IBM 7030 STRETCH VFL Arithmetic verified successfully.\n");

    // Test Scenario 17: IBM 7030 STRETCH Variable Field Length (VFL) Logical Operations
    printf("[Test] Verifying IBM 7030 STRETCH VFL Logic...\n");
    uint64_t vfl_logic_memory[4] = {0};

    int l_res = tsfi_s370_ibm7030_write_bits(vfl_logic_memory, 10, 8, 0x0F);
    assert(l_res == 0);
    l_res = tsfi_s370_ibm7030_write_bits(vfl_logic_memory, 25, 8, 0xF0);
    assert(l_res == 0);

    uint64_t vfl_log_val = 0;
    int log_res = tsfi_s370_ibm7030_vfl_logic(vfl_logic_memory, 10, 1, 8, 25, 1, 8, "XOR", &vfl_log_val);
    assert(log_res == 0);
    assert(vfl_log_val == 0xFF);

    log_res = tsfi_s370_ibm7030_vfl_logic(vfl_logic_memory, 10, 1, 8, 25, 1, 8, "AND", &vfl_log_val);
    assert(log_res == 0);
    assert(vfl_log_val == 0x00);

    printf("  [PASS] IBM 7030 STRETCH VFL Logic verified successfully.\n");

    // Test Scenario 18: Honeywell 800 Traffic Control Multiprogramming Scheduler
    printf("[Test] Verifying Honeywell 800 Traffic Control Multiprogramming Scheduler...\n");
    tsfi_honeywell800_scheduler hw_sched;
    tsfi_s370_honeywell800_init(&hw_sched);

    int hw_memory[64] = {0};
    hw_memory[0] = (0x01 << 24) | 50;
    hw_memory[1] = (0x02 << 24) | 51;
    hw_memory[2] = (0x03 << 24) | 52;
    hw_memory[3] = (0x04 << 24) | 0;

    hw_memory[4] = (0x01 << 24) | 51;
    hw_memory[5] = (0x02 << 24) | 50;
    hw_memory[6] = (0x03 << 24) | 53;
    hw_memory[7] = (0x04 << 24) | 0;

    hw_memory[50] = 100;
    hw_memory[51] = 200;

    hw_sched.threads[0].pc = 0;
    hw_sched.threads[0].is_active = 1;
    hw_sched.threads[1].pc = 4;
    hw_sched.threads[1].is_active = 1;

    int ran_tid = tsfi_s370_honeywell800_tick(&hw_sched, hw_memory, 64);
    assert(ran_tid == 0);
    assert(hw_sched.threads[0].accumulator == 100);

    ran_tid = tsfi_s370_honeywell800_tick(&hw_sched, hw_memory, 64);
    assert(ran_tid == 1);
    assert(hw_sched.threads[1].accumulator == 200);

    ran_tid = tsfi_s370_honeywell800_tick(&hw_sched, hw_memory, 64);
    assert(ran_tid == 0);
    assert(hw_sched.threads[0].accumulator == 300);

    ran_tid = tsfi_s370_honeywell800_tick(&hw_sched, hw_memory, 64);
    assert(ran_tid == 1);
    assert(hw_sched.threads[1].accumulator == 300);

    while (tsfi_s370_honeywell800_tick(&hw_sched, hw_memory, 64) >= 0);

    assert(hw_memory[52] == 300);
    assert(hw_memory[53] == 300);

    printf("  [PASS] Honeywell 800 Traffic Control Multiprogramming Scheduler verified successfully.\n");

    // Test Scenario 19: IBM 7090 Loop Control (TIX/TXI)
    printf("[Test] Verifying IBM 7090 Loop Control (TIX/TXI)...\n");
    uint16_t index_val = 10;
    uint16_t pc_val = 100;

    int branched = tsfi_s370_ibm7090_tix(&index_val, 3, 200, &pc_val);
    assert(branched == 1);
    assert(index_val == 7);
    assert(pc_val == 200);

    branched = tsfi_s370_ibm7090_tix(&index_val, 8, 300, &pc_val);
    assert(branched == 0);
    assert(index_val == 7);
    assert(pc_val == 200);

    branched = tsfi_s370_ibm7090_txi(&index_val, 5, 400, &pc_val);
    assert(branched == 1);
    assert(index_val == 12);
    assert(pc_val == 400);

    printf("  [PASS] IBM 7090 Loop Control verified successfully.\n");

    // Test Scenario 20: CDC 6600 Parallel Scoreboard Scheduler
    printf("[Test] Verifying CDC 6600 Scoreboard...\n");
    tsfi_cdc6600_scoreboard sb;
    tsfi_s370_cdc6600_init(&sb);

    int is_ok = tsfi_s370_cdc6600_issue(&sb, CDC_UNIT_ADD, 1, 10, 20, 0);
    assert(is_ok == 0);
    assert(sb.units[CDC_UNIT_ADD].is_busy == 1);

    is_ok = tsfi_s370_cdc6600_issue(&sb, CDC_UNIT_MULTIPLY, 2, 5, 6, 0);
    assert(is_ok == 0);
    assert(sb.units[CDC_UNIT_MULTIPLY].is_busy == 1);

    tsfi_s370_cdc6600_tick(&sb);
    assert(sb.registers[1] == 0);
    assert(sb.registers[2] == 0);

    tsfi_s370_cdc6600_tick(&sb);
    assert(sb.registers[1] == 30);
    assert(sb.registers[2] == 0);
    assert(sb.units[CDC_UNIT_ADD].is_busy == 0);

    tsfi_s370_cdc6600_tick(&sb);
    tsfi_s370_cdc6600_tick(&sb);
    assert(sb.registers[2] == 30);
    assert(sb.units[CDC_UNIT_MULTIPLY].is_busy == 0);

    printf("  [PASS] CDC 6600 Scoreboard verified successfully.\n");

    // Test Scenario 21: CDC 1604 Subtractive Adder (1's complement)
    printf("[Test] Verifying CDC 1604 Subtractive Adder...\n");
    uint64_t res_add = tsfi_s370_cdc1604_subtractive_add(20, 30, 48);
    assert(res_add == 50);

    res_add = tsfi_s370_cdc1604_subtractive_add(0xFFFFFFFFFFFFULL, 5, 48);
    assert(res_add == 5);

    res_add = tsfi_s370_cdc1604_subtractive_add(0xFFFFFFFFFFFEULL, 2, 48);
    assert(res_add == 1);

    printf("  [PASS] CDC 1604 Subtractive Adder verified successfully.\n");

    // Test Scenario 22: CDC 1604 Memory Address Resolution (Indexing + Indirection)
    printf("[Test] Verifying CDC 1604 Address Resolution...\n");
    int cdc_memory[128] = {0};
    int cdc_index_registers[8] = {0, 10, 20, 30, 40, 50, 60, 70};

    cdc_memory[100] = 50;

    eff_addr = 0;
    int resolve_ok = tsfi_s370_cdc1604_resolve_address(cdc_memory, 128, 80, 2, cdc_index_registers, 0, &eff_addr);
    assert(resolve_ok == 0);
    assert(eff_addr == 100);

    resolve_ok = tsfi_s370_cdc1604_resolve_address(cdc_memory, 128, 80, 2, cdc_index_registers, 1, &eff_addr);
    assert(resolve_ok == 0);
    assert(eff_addr == 50);

    printf("  [PASS] CDC 1604 Address Resolution verified successfully.\n");

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
