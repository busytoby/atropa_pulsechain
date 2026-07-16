#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
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
    int lap_ret1 = tsfi_s370_validate_write(&cpu, 200, block_keys, 4);
    assert(lap_ret1 == -1);

    cpu.supervisor_state = 1; 
    cpu.psw_key = 0;
    int lap_ret2 = tsfi_s370_validate_write(&cpu, 200, block_keys, 4);
    assert(lap_ret2 == 0);

    cpu.supervisor_state = 0; 
    cpu.lap_enabled = 0;      
    cpu.psw_key = 5;          
    block_keys[0].acc = 5;
    int lap_ret3 = tsfi_s370_validate_write(&cpu, 200, block_keys, 4);
    assert(lap_ret3 == 0);

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

    // Pack positive zoned string "12345" -> 12 34 5C
    int size_a = tsfi_s370_pack("12345", packed_a, 32);
    assert(size_a == 3);
    assert(packed_a[0] == 0x12);
    assert(packed_a[1] == 0x34);
    assert(packed_a[2] == 0x5C);

    char unpacked_a[128];
    int unpack_ret1 = tsfi_s370_unpack(packed_a, size_a, unpacked_a, 128);
    assert(unpack_ret1 == 0);
    assert(strcmp(unpacked_a, "12345") == 0);

    // Pack negative zoned string "-1234" -> 01 23 4D
    int size_b = tsfi_s370_pack("-1234", packed_b, 32);
    assert(size_b == 3);
    assert(packed_b[0] == 0x01);
    assert(packed_b[1] == 0x23);
    assert(packed_b[2] == 0x4D);

    char unpacked_b[128];
    int unpack_ret2 = tsfi_s370_unpack(packed_b, size_b, unpacked_b, 128);
    assert(unpack_ret2 == 0);
    assert(strcmp(unpacked_b, "-1234") == 0);

    // Add packed decimals: 12345 + (-1234) = 11111 -> 11 11 1C
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
