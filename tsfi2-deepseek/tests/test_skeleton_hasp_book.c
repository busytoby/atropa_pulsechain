#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xplos_kernel_internal.h"
#include "tsfi_mainframe_computerworld.h"

extern bool tsfi_xplos_shell_cbt_jcl(const char *cmd);
extern bool tsfi_xplos_shell_cbt_jes(const char *cmd);
extern bool tsfi_xplos_shell_book(const char *cmd);
extern void tsfi_cbt_pds_format_dir_block(uint8_t *dir_block, const char *member_name, uint32_t offset, uint32_t size);

int main(void) {
    printf("=== RUNNING SKELETON-HASP-BOOK END-TO-END PROOFS ===\n");

    // 1. Initialize virtual disk VFS and JCL spool systems
    static XplosVirtualDisk vfs;
    memset(&vfs, 0, sizeof(XplosVirtualDisk));
    vfs.count = 0;

    // 2. SKELETON: Simulate expanding a JCL job from template
    printf("  -> Phase 1: Expanding SKELETON JCL job...\n");
    // We register the job under the CBT JCL command tables
    bool jcl_ok = tsfi_xplos_shell_cbt_jcl("jclrun JOB_PROD");
    assert(jcl_ok == true);

    // 3. HASP: Route expanded job via spool loops
    printf("  -> Phase 2: Routing job through HASP spool scheduler...\n");
    bool jes_ok = tsfi_xplos_shell_cbt_jes("cbthaspspoollogsstatus");
    assert(jes_ok == true);

    // 4. BOOK: Load dataset members dynamically into VSAM
    printf("  -> Phase 3: Loading PDS members via BOOK loader to VSAM...\n");
    remove("VTOC.dat.bin");
    tsfi_cw_vsam_ksds ksds;
    int open_rc = tsfi_cw_vsam_open(&ksds, "VTOC.dat.bin");
    assert(open_rc == 0);

    // Write a mock book record representing PDS catalog data
    uint8_t mock_data[64] = "PGM=TEST_PROD,RECFM=FB,LRECL=80";
    int write_rc = tsfi_cw_vsam_write(&ksds, "M1", mock_data, 32);
    if (write_rc != 0) {
        printf("  -> tsfi_cw_vsam_write failed with RC=%d (filepath='%s')\n", write_rc, ksds.filepath);
    }
    assert(write_rc == 0);

    // 5. Read back verify the record from VSAM (WinchesterMQ loopback route)
    uint8_t read_buf[64] = {0};
    int read_len = 0;
    int read_rc = tsfi_cw_vsam_read(&ksds, "M1", read_buf, sizeof(read_buf), &read_len);
    assert(read_rc == 0);
    assert(strncmp((char *)read_buf, "PGM=TEST_PROD", 13) == 0);

    // 6. Test Class-Based Queue Scheduling and Dispatching
    printf("  -> Phase 4: Verifying Class-Based Queue scheduling...\n");
    extern CbtSpoolJob cbt_job_table[10];
    extern int g_hasp_job_priority[10];

    // Clear jobs to start with clean test state
    for (int i = 0; i < 10; i++) {
        cbt_job_table[i].active = false;
    }

    // Register Job B with Class B, Priority 10
    strcpy(cbt_job_table[3].job_id, "JOB003");
    strcpy(cbt_job_table[3].job_name, "JOB_CL_B");
    strcpy(cbt_job_table[3].status, "READY");
    cbt_job_table[3].class_char = 'B';
    cbt_job_table[3].active = true;
    g_hasp_job_priority[3] = 10;

    // Register Job A with Class A, Priority 50 (Higher priority but different class)
    strcpy(cbt_job_table[4].job_id, "JOB004");
    strcpy(cbt_job_table[4].job_name, "JOB_CL_A");
    strcpy(cbt_job_table[4].status, "READY");
    cbt_job_table[4].class_char = 'A';
    cbt_job_table[4].active = true;
    g_hasp_job_priority[4] = 50;

    // Dispatch Class B specifically
    bool dispatch_b_ok = tsfi_xplos_shell_cbt_jes("cbthasp dispatch B");
    assert(dispatch_b_ok == true);

    // Verify Job B completed, while Job A remains READY
    assert(strcmp(cbt_job_table[3].status, "COMPLETED") == 0);
    assert(strcmp(cbt_job_table[4].status, "READY") == 0);

    // Now dispatch Class A
    bool dispatch_a_ok = tsfi_xplos_shell_cbt_jes("cbthasp dispatch A");
    assert(dispatch_a_ok == true);
    assert(strcmp(cbt_job_table[4].status, "COMPLETED") == 0);

    printf("  -> Class-based queue scheduling verified successfully.\n");

    // 7. Test HOLD and RELEASE operations
    printf("  -> Phase 5: Verifying HOLD and RELEASE spool operations...\n");
    // Register Job H with Class A, Priority 100, placed in READY status
    strcpy(cbt_job_table[5].job_id, "JOB005");
    strcpy(cbt_job_table[5].job_name, "JOB_HELD");
    strcpy(cbt_job_table[5].status, "READY");
    cbt_job_table[5].class_char = 'A';
    cbt_job_table[5].active = true;
    g_hasp_job_priority[5] = 100;

    // Put it on hold
    bool hold_ok = tsfi_xplos_shell_cbt_jes("cbthasp hold JOB005");
    assert(hold_ok == true);
    assert(strcmp(cbt_job_table[5].status, "HELD") == 0);

    // Dispatch Class A - should skip JOB_HELD since it is HELD
    bool dispatch_held_ok = tsfi_xplos_shell_cbt_jes("cbthasp dispatch A");
    assert(dispatch_held_ok == true);
    assert(strcmp(cbt_job_table[5].status, "HELD") == 0); // Still held

    // Release the job to READY status
    bool release_ok = tsfi_xplos_shell_cbt_jes("cbthasp release JOB005");
    assert(release_ok == true);
    // Since cbthasp release triggers hasp_dispatch_highest_priority, it should execute the released job and set it to COMPLETED
    assert(strcmp(cbt_job_table[5].status, "COMPLETED") == 0);

    // 8. Verify Priority Aging
    printf("  -> Phase 6: Verifying Priority Aging execution...\n");
    // Register Job P with Priority 5
    strcpy(cbt_job_table[6].job_id, "JOB006");
    strcpy(cbt_job_table[6].job_name, "JOB_AGE");
    strcpy(cbt_job_table[6].status, "READY");
    cbt_job_table[6].class_char = 'A';
    cbt_job_table[6].active = true;
    g_hasp_job_priority[6] = 5;

    // Run dispatch on different class B to trigger aging tick (READY jobs priorities increment)
    tsfi_xplos_shell_cbt_jes("cbthasp dispatch B");
    assert(g_hasp_job_priority[6] == 6); // Incremented by 1

    // 9. Verify SMFDUMP dynamic readings from VSAM
    printf("  -> Phase 7: Verifying SMFDUMP database logs retrieval...\n");
    remove("SMF.dat.bin"); // Start with clean SMF file
    // Dispatch JOB_AGE to trigger writing its SMF record
    tsfi_xplos_shell_cbt_jes("cbthasp dispatch A");
    assert(strcmp(cbt_job_table[6].status, "COMPLETED") == 0);

    // Run smfdump command which reads from SMF.dat.bin VSAM database
    bool smfdump_ok = tsfi_xplos_shell_cbt_jes("smfdump");
    assert(smfdump_ok == true);

    // 10. Verify Dynamic PROCLIB catalog PDS queries
    printf("  -> Phase 8: Verifying dynamic PROCLIB catalog queries via VSAM...\n");
    remove("PROCLIB.dat.bin");
    tsfi_cw_vsam_ksds proclib_ksds;
    int open_proclib_rc = tsfi_cw_vsam_open(&proclib_ksds, "PROCLIB.dat.bin");
    assert(open_proclib_rc == 0);

    // Write a mock JCL job into PROCLIB under key "MJ1"
    uint8_t mock_jcl_cards[256] = 
        "//MJ1 JOB 'CBT TEST',CLASS=A\n"
        "//STEP1 EXEC PGM=IEBCOPY\n";
    int write_proclib_rc = tsfi_cw_vsam_write(&proclib_ksds, "MJ1", mock_jcl_cards, strlen((char *)mock_jcl_cards));
    assert(write_proclib_rc == 0);

    // Write JOB_CICS_SEC JCL skeleton
    uint8_t sec_skeleton_cards[256] = 
        "//MJ7 JOB 'CICS AUDIT'\n"
        "//STEP1 EXEC PGM=DUMMYPGM\n";
    int write_sec_rc = tsfi_cw_vsam_write(&proclib_ksds, "MJ7", sec_skeleton_cards, strlen((char *)sec_skeleton_cards));
    assert(write_sec_rc == 0);

    // Write JOB_PDS_COMP JCL skeleton
    uint8_t comp_skeleton_cards[256] = 
        "//MJ8 JOB 'PDS COMPRESS'\n"
        "//STEP1 EXEC PGM=DUMMYPGM\n";
    int write_comp_rc = tsfi_cw_vsam_write(&proclib_ksds, "MJ8", comp_skeleton_cards, strlen((char *)comp_skeleton_cards));
    assert(write_comp_rc == 0);

    // Write Yul VM execution skeleton MJ9
    uint8_t yulvm_skeleton_cards[256] = 
        "//MJ9 JOB 'YUL VM RUN'\n"
        "//STEP1 EXEC PGM=YULVM\n";
    int write_yulvm_rc = tsfi_cw_vsam_write(&proclib_ksds, "MJ9", yulvm_skeleton_cards, strlen((char *)yulvm_skeleton_cards));
    assert(write_yulvm_rc == 0);

    // Execute the job, which should dynamically load it from the PROCLIB KSDS database
    bool mockjob_run_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ1");
    assert(mockjob_run_ok == true);

    // 11. Verify WinchesterMQ and ABI integration via BOOK and VSAM bypasses
    printf("  -> Phase 9: Verifying WinchesterMQ and ABI registry bypass limits...\n");
    extern uint32_t global_wmq_bypass_count;
    uint32_t start_wmq_bypasses = global_wmq_bypass_count;
    assert(start_wmq_bypasses > 0); // Check that we had active bypass events during JCL execution

    // Perform an explicit write to verify ABI registers
    uint8_t mock_abi_data[64] = "ABI_VERIFY_SAMPLE";
    int abi_write_rc = tsfi_cw_vsam_write(&proclib_ksds, "AB1", mock_abi_data, 17);
    assert(abi_write_rc == 0);
    assert(global_wmq_bypass_count > start_wmq_bypasses); // Verified that the write routed through WMQ

    printf("  -> WinchesterMQ bypass events counted: %u loops.\n", global_wmq_bypass_count);

    // 12. Verify Multi-PDS search path and SET overrides
    printf("  -> Phase 10: Verifying Multi-PDS concatenation and SET overrides...\n");
    remove("USERLIB.dat.bin");
    tsfi_cw_vsam_ksds userlib_ksds;
    int open_userlib_rc = tsfi_cw_vsam_open(&userlib_ksds, "USERLIB.dat.bin");
    assert(open_userlib_rc == 0);

    // Write a mock JCL job into USERLIB under key "MJ2" which uses variable overrides
    uint8_t mock_userlib_cards[256] = 
        "//MJ2 JOB 'CBT TEST 2'\n"
        "//SET MYPGM=IEFBR14\n"
        "//STEP1 EXEC PGM=&MYPGM\n";
    int write_userlib_rc = tsfi_cw_vsam_write(&userlib_ksds, "MJ2", mock_userlib_cards, strlen((char *)mock_userlib_cards));
    assert(write_userlib_rc == 0);

    // Execute the job: JCL runner should find it in USERLIB and process the SET symbols
    bool mockjob2_run_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ2");
    assert(mockjob2_run_ok == true);

    // 13. Verify Coaxial Instream Data Routing (SYSTSIN DD * to TSOTMP)
    printf("  -> Phase 11: Verifying coaxial instream data routing...\n");
    remove("USERLIB.dat.bin");
    int open_userlib_rc2 = tsfi_cw_vsam_open(&userlib_ksds, "USERLIB.dat.bin");
    assert(open_userlib_rc2 == 0);

    // Write a mock JCL job into USERLIB under key "MJ3" which uses instream data cards
    uint8_t mock_instream_cards[512] = 
        "//MJ3 JOB 'CBT COAX'\n"
        "//SYSTSIN DD *\n"
        "cbtrexx vput SYSVAR 953467954114363\n"
        "/*\n"
        "//STEP1 EXEC PGM=TSOTMP\n";
    int write_userlib_rc2 = tsfi_cw_vsam_write(&userlib_ksds, "MJ3", mock_instream_cards, strlen((char *)mock_instream_cards));
    assert(write_userlib_rc2 == 0);

    // Execute the job: it should route the command coaxially and execute cbtrexx
    bool mockjob3_run_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ3");
    assert(mockjob3_run_ok == true);

    // 14. Verify Binary PDS Directory Block Formatting
    printf("  -> Phase 12: Verifying binary PDS directory block formatting...\n");
    uint8_t dir_block_buf[256];
    tsfi_cbt_pds_format_dir_block(dir_block_buf, "MJ3", 1024, 256);
    assert(dir_block_buf[1] == 18);
    assert(strncmp((char *)&dir_block_buf[2], "MJ3     ", 8) == 0);
    assert(dir_block_buf[12] == 0x00);

    // 15. Verify Step-level COND parameter auditing
    printf("  -> Phase 13: Verifying step-level COND parameter auditing...\n");
    remove("USERLIB.dat.bin");
    int open_userlib_rc3 = tsfi_cw_vsam_open(&userlib_ksds, "USERLIB.dat.bin");
    assert(open_userlib_rc3 == 0);

    // Write a mock JCL job into USERLIB under key "MJ4"
    uint8_t mock_cond_cards[512] = 
        "//MJ4 JOB 'CBT COND'\n"
        "//ST1 EXEC PGM=IEFBR14\n"
        "//ST2 EXEC PGM=IEBCOPY,COND=(0,EQ,ST1)\n";
    int write_userlib_rc3 = tsfi_cw_vsam_write(&userlib_ksds, "MJ4", mock_cond_cards, strlen((char *)mock_cond_cards));
    assert(write_userlib_rc3 == 0);

    bool mockjob4_run_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ4");
    assert(mockjob4_run_ok == true);

    // 16. Verify RAUPGM execution to mount RAU and check registers
    printf("  -> Phase 14: Verifying RAUPGM execution and RAU registers...\n");
    remove("USERLIB.dat.bin");
    int open_userlib_rc4 = tsfi_cw_vsam_open(&userlib_ksds, "USERLIB.dat.bin");
    assert(open_userlib_rc4 == 0);

    // Write JCL job MJ5 that runs RAUPGM
    uint8_t mock_rau_cards[256] = 
        "//MJ5 JOB 'CBT RAU'\n"
        "//STEP1 EXEC PGM=RAUPGM\n";
    int write_userlib_rc4 = tsfi_cw_vsam_write(&userlib_ksds, "MJ5", mock_rau_cards, strlen((char *)mock_rau_cards));
    assert(write_userlib_rc4 == 0);

    bool mockjob5_run_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ5");
    assert(mockjob5_run_ok == true);

    // 17. Verify SMF Archive to Virtual Tape Catalog
    printf("  -> Phase 15: Verifying SMF Archive to Virtual Tape Catalog...\n");
    remove("TAPE.SMF.dat.bin");
    remove("SMF.dat.bin");
    tsfi_cw_vsam_ksds smf_ksds;
    int open_smf_rc = tsfi_cw_vsam_open(&smf_ksds, "SMF.dat.bin");
    assert(open_smf_rc == 0);
    uint8_t mock_smf_rec[128] = "JOB_NAME=MJ5,PGM=RAUPGM,CPU=0.015,RC=0000";
    int write_smf_rc = tsfi_cw_vsam_write(&smf_ksds, "MJ5", mock_smf_rec, strlen((char *)mock_smf_rec));
    assert(write_smf_rc == 0);

    // Trigger archive command
    bool archive_ok = tsfi_xplos_shell_cbt_jes("smfarchive");
    assert(archive_ok == true);

    // Read back and verify from TAPE.SMF.dat.bin
    tsfi_cw_vsam_ksds tape_ksds;
    int open_tape_rc = tsfi_cw_vsam_open(&tape_ksds, "TAPE.SMF.dat.bin");
    assert(open_tape_rc == 0);
    uint8_t read_tape_buf[128] = {0};
    int read_tape_len = 0;
    int read_tape_rc = tsfi_cw_vsam_read(&tape_ksds, "MJ5", read_tape_buf, sizeof(read_tape_buf), &read_tape_len);
    assert(read_tape_rc == 0);
    assert(read_tape_len > 0);
    assert(strncmp((char *)read_tape_buf, "JOB_NAME=MJ5", 12) == 0);

    // 18. Verify SCSIPGM execution for WinchesterMQ bridge & DisplacementShader vertex math
    printf("  -> Phase 16: Verifying SCSIPGM execution for WinchesterMQ and DisplacementShader...\n");
    remove("USERLIB.dat.bin");
    int open_userlib_rc5 = tsfi_cw_vsam_open(&userlib_ksds, "USERLIB.dat.bin");
    assert(open_userlib_rc5 == 0);

    // Write JCL job MJ6 that runs SCSIPGM
    uint8_t mock_scsi_cards[256] = 
        "//MJ6 JOB 'CBT SCSI'\n"
        "//STEP1 EXEC PGM=SCSIPGM\n";
    int write_userlib_rc5 = tsfi_cw_vsam_write(&userlib_ksds, "MJ6", mock_scsi_cards, strlen((char *)mock_scsi_cards));
    assert(write_userlib_rc5 == 0);

    bool mockjob6_run_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ6");
    assert(mockjob6_run_ok == true);

    // 19. Verify CICS Teddy Bear Participant Verification and Hogan registration
    printf("  -> Phase 17: Verifying CICS Teddy Bear Participant Verification and Hogan registration...\n");
    extern bool tsfi_xplos_shell_cbt_cics(const char *cmd);
    bool register_ok = tsfi_xplos_shell_cbt_cics("cbtcicsregister TeddyBearSkelCharacter");
    assert(register_ok == true);

    // 20. Verify CICS Transaction Security Auditor compliance logs
    printf("  -> Phase 18: Verifying CICS Transaction Security Auditor compliance logs...\n");
    bool audit_ok = tsfi_xplos_shell_cbt_cics("cbtcicsaudit TeddyBearSkelCharacter");
    assert(audit_ok == true);

    // 21. Verify PDS Library Compression utility
    printf("  -> Phase 19: Verifying PDS Library Compression utility...\n");
    extern XplosVirtualDisk g_vfs;
    tsfi_xplos_create_file(&g_vfs, "USERLIB.dat.bin", 10240);
    extern bool tsfi_xplos_shell_book(const char *cmd);
    bool compress_ok = tsfi_xplos_shell_book("cbtcompress USERLIB");
    assert(compress_ok == true);

    // 22. Deploy and expand additional skeletons
    printf("  -> Phase 20: Deploying and expanding additional JCL skeletons...\n");
    bool run_sec_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ7");
    assert(run_sec_ok == true);
    bool run_comp_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ8");
    assert(run_comp_ok == true);

    // 23. Verify Yul VM execution upon TSQ/TDQ DAT rails
    printf("  -> Phase 21: Verifying Yul VM execution upon TSQ/TDQ DAT rails...\n");
    bool run_yulvm_ok = tsfi_xplos_shell_cbt_jcl("jclrun MJ9");
    assert(run_yulvm_ok == true);

    // 24. Verify CICS Transaction Performance Profiler
    printf("  -> Phase 22: Verifying CICS Transaction Performance Profiler...\n");
    bool profile_ok = tsfi_xplos_shell_cbt_cics("cbtcicsprofile");
    assert(profile_ok == true);

    // 25. Verify Dysnomia Contract Deployments (LAU, Username, Chat submission)
    printf("  -> Phase 23: Deploying Dysnomia contracts & verifying LAU, username, and chats...\n");
    
    // Deploy all Dysnomia core contracts in sequence
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write VMREQ dynamic_0x0000000000000000000000000000000000000001") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write SHAFactory dynamic_0x0000000000000000000000000000000000000002") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write SHIOFactory dynamic_0x0000000000000000000000000000000000000003") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write YI dynamic_0x0000000000000000000000000000000000000004") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write ZHENG dynamic_0x0000000000000000000000000000000000000005") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write ZHOU dynamic_0x0000000000000000000000000000000000000006") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write YAU dynamic_0x0000000000000000000000000000000000000007") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write YANG dynamic_0x0000000000000000000000000000000000000008") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write SIU dynamic_0x0000000000000000000000000000000000000009") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write VOID dynamic_0x000000000000000000000000000000000000000A") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write LAUFactory dynamic_0x000000000000000000000000000000000000000B") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write CHO dynamic_0x000000000000000000000000000000000000000C") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write HECKE dynamic_0x000000000000000000000000000000000000000D") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write MAP dynamic_0x000000000000000000000000000000000000000E") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write QI dynamic_0x000000000000000000000000000000000000000F") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write MAI dynamic_0x0000000000000000000000000000000000000010") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write XIA dynamic_0x0000000000000000000000000000000000000011") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write XIE dynamic_0x0000000000000000000000000000000000000012") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write CHAN dynamic_0x0000000000000000000000000000000000000013") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write SEI dynamic_0x0000000000000000000000000000000000000014") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write CHOA dynamic_0x0000000000000000000000000000000000000015") == true);
    assert(tsfi_xplos_shell_cbt_cics("cbtcicsts write CHEON dynamic_0x0000000000000000000000000000000000000016") == true);

    // Deploy test LAU
    bool tsq_lau_ok = tsfi_xplos_shell_cbt_cics("cbtcicsts write TestLAU dynamic_0x0000000000000000000000000000000000000099");
    assert(tsq_lau_ok == true);
    
    // Set username
    bool tsq_user_ok = tsfi_xplos_shell_cbt_cics("cbtcicsts write USER_NAME HoganBear");
    assert(tsq_user_ok == true);
    
    // Submit chat message to TDQ
    bool tdq_chat_ok = tsfi_xplos_shell_cbt_cics("cbtcicstd write CHAT Auncient Wavelet deployed successfully under HASP!");
    assert(tdq_chat_ok == true);

    printf("[DEPLOYMENT CHECK] Querying active credentials from 2-3 Tree rails...\n");
    bool tsq_read_user = tsfi_xplos_shell_cbt_cics("cbtcicsts read USER_NAME");
    assert(tsq_read_user == true);

    printf("  -> End-to-end data pipeline integrity verified successfully.\n");
    printf("\n=== SKELETON HASP BOOK PROOFS PASSED ===\n");
    return 0;
}
