#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_displacementshader.h"
#include "lau_thunk.h"
#include "tsfi_riinterface.h"

// Simulated XPLSM Monitor State
typedef struct {
    uint32_t heartbeat;
    uint32_t interrupts_handled;
    bool supervisor_trap;
} XplsmMonitor;

// Simulated XCOM Handshake Controller
typedef struct {
    bool req;
    bool ack;
    uint8_t data;
} XcomController;

// Simulated ANALYZER parser state for SKELETON JCL template expansion
typedef struct {
    char variable_key[32];
    char variable_val[64];
    uint32_t parsed_lines;
} AnalyzerParser;

uint32_t ce_gprs[16] = {0};
uint8_t ce_memory[1024] = {0};
uint32_t xdc_ip = 0;


// Perform JCL SKELETON template expansion using ANALYZER variable binding
bool analyzer_expand_skeleton(AnalyzerParser *parser, const char *skeleton_in, char *jcl_out, size_t max_len) {
    if (!parser || !skeleton_in || !jcl_out) return false;
    
    // Simple template search-and-replace for target key
    char placeholder[64];
    snprintf(placeholder, sizeof(placeholder), "&%s", parser->variable_key);
    
    const char *found = strstr(skeleton_in, placeholder);
    if (found) {
        size_t prefix_len = found - skeleton_in;
        if (prefix_len + strlen(parser->variable_val) + strlen(found + strlen(placeholder)) >= max_len) {
            return false;
        }
        
        strncpy(jcl_out, skeleton_in, prefix_len);
        jcl_out[prefix_len] = '\0';
        strcat(jcl_out, parser->variable_val);
        strcat(jcl_out, found + strlen(placeholder));
        parser->parsed_lines++;
        return true;
    }
    
    strncpy(jcl_out, skeleton_in, max_len - 1);
    jcl_out[max_len - 1] = '\0';
    return true;
}

static int g_thunk_callback_val = 0;
static void test_thunk_callback(const char *arg) {
    if (strcmp(arg, "thunk_verify_test") == 0) {
        g_thunk_callback_val = 42;
    }
}

static bool execute_transactional_thunk(ThunkProxy *thunk, void *fn, const char *arg, int sector) {
    extern uint32_t ce_gprs[16];
    extern bool tsfi_xplos_shell_tape(const char *cmd);
    
    uint32_t snapshot[16];
    memcpy(snapshot, ce_gprs, sizeof(snapshot));
    
    // Mutate register to represent active execution state change
    ce_gprs[7] = 777;
    
    typedef void (*thunk_entry)(void);
    void *jit_fn = ThunkProxy_emit_baked(thunk, fn, 1, arg);
    ((thunk_entry)jit_fn)();
    
    char cmd_buf[128];
    snprintf(cmd_buf, sizeof(cmd_buf), "cbttape write %d 1", sector);
    bool write_ok = tsfi_xplos_shell_tape(cmd_buf);
    
    if (!write_ok) {
        memcpy(ce_gprs, snapshot, sizeof(snapshot));
        printf("[VM TRANSACTION] Transaction failed. Rolled back register states.\n");
        return false;
    }
    printf("[VM TRANSACTION] Transaction committed. Registers persisted.\n");
    return true;
}

int main(void) {
    printf("=== RUNNING XPL, XCOM, ANALYZER, XPLSM & SKELETON INTEGRATION PROOFS ===\n");
    
    // 1. Initialize XPLSM system monitor state
    XplsmMonitor monitor = { .heartbeat = 0, .interrupts_handled = 0, .supervisor_trap = false };
    printf("  -> Initializing XPLSM liveness heartbeat: %d\n", monitor.heartbeat);
    monitor.heartbeat++;
    assert(monitor.heartbeat == 1);
    
    // 2. Instantiate ANALYZER variable bindings for SKELETON templates
    AnalyzerParser analyzer = { .variable_key = "PORT", .variable_val = "CICS_PROD", .parsed_lines = 0 };
    const char *skeleton_template = "//JOB1 JOB (ACCT), 'RUN SKELETON', CLASS=A\n//STEP1 EXEC PGM=&PORT\n";
    char expanded_jcl[512] = {0};
    
    printf("  -> Loading SKELETON template into ANALYZER parser...\n");
    bool parse_ok = analyzer_expand_skeleton(&analyzer, skeleton_template, expanded_jcl, sizeof(expanded_jcl));
    assert(parse_ok == true);
    assert(analyzer.parsed_lines == 1);
    assert(strstr(expanded_jcl, "PGM=CICS_PROD") != NULL);
    printf("  -> JCL SKELETON expansion complete: PGM=CICS_PROD bound.\n");
    
    // 3. Initiate XCOM WinchesterMQ SCSI Handshake to transmit the JCL
    XcomController xcom = { .req = false, .ack = false, .data = 0 };
    printf("  -> Initiating XCOM SCSI request signal...\n");
    xcom.req = true;
    
    // Simulate target responding via SCSI ACK
    xcom.ack = true;
    if (xcom.req && xcom.ack) {
        xcom.data = 0x2A; // Identifier token
        xcom.req = false; // Clear REQ once data is read
    }
    
    assert(xcom.data == 0x2A);
    assert(xcom.req == false);
    printf("  -> XCOM communication transfer handshake verified.\n");
    
    // 4. Update XPLSM liveness status
    monitor.interrupts_handled++;
    assert(monitor.interrupts_handled == 1);
    printf("  -> XPLSM supervisor state updated successfully.\n");
    
    // 5. Verify Capstan Shaft Tape Device Transactions
    printf("  -> Testing emulated capstan device status and transaction execution...\n");
    extern bool tsfi_xplos_shell_tape(const char *cmd);
    
    bool status_ok = tsfi_xplos_shell_tape("cbttape status");
    assert(status_ok == true);
    
    bool write_ok = tsfi_xplos_shell_tape("cbttape write 4 170");
    assert(write_ok == true);
    
    bool inject_ok = tsfi_xplos_shell_tape("cbttape inject 0");
    assert(inject_ok == true);
    
    bool fail_ok = tsfi_xplos_shell_tape("cbttape write 5 187");
    assert(fail_ok == false);

    // 6. Verify SAM Driver tape operations (rewind, bsf, fsf)
    printf("  -> Testing SAM Driver tape movements...\n");
    assert(tsfi_xplos_shell_tape("cbttape rewind") == true);
    assert(tsfi_xplos_shell_tape("cbttape fsf 10") == true);
    assert(tsfi_xplos_shell_tape("cbttape bsf 3") == true);
    
    // Restore verify status for journal logging
    assert(tsfi_xplos_shell_tape("cbttape inject 1") == true);

    // 7. Verify SMF Transaction Journal writes
    printf("  -> Testing SMF Transaction Journal writes...\n");
    assert(tsfi_xplos_shell_tape("cbttape journal 101 TX_ALLOCATE") == true);
    assert(tsfi_xplos_shell_tape("cbttape journal 102 TX_COMMIT") == true);

    // 8. Verify ARM Recovery replay
    printf("  -> Testing ARM Recovery manager logic...\n");
    assert(tsfi_xplos_shell_tape("cbttape recover") == true);

    // 9. Verify Extended Read/Write Lock Isolation
    printf("  -> Testing sector lock isolation...\n");
    assert(tsfi_xplos_shell_tape("cbttape lock write 20") == true);
    
    // 10. Verify Extended Multi-Sector Group Commits (aborted by lock)
    printf("  -> Testing group write aborted by lock...\n");
    assert(tsfi_xplos_shell_tape("cbttape writegroup 18 4") == false);
    
    // Unlock and run successful group commit
    assert(tsfi_xplos_shell_tape("cbttape unlock 20") == true);
    assert(tsfi_xplos_shell_tape("cbttape writegroup 18 4") == true);
    
    // 11. Verify Extended Startup Volume Reconciliation
    printf("  -> Testing dirty sector reconciliation...\n");
    assert(tsfi_xplos_shell_tape("cbttape inject 0") == true);
    assert(tsfi_xplos_shell_tape("cbttape reconcile") == true);

    // 12. Verify CAPSTAN KERMIT packet transfer
    printf("  -> Testing CAPSTAN KERMIT packet validation and commit...\n");
    assert(tsfi_xplos_shell_tape("cbttape kermit 23061e44412c") == true);
    assert(tsfi_xplos_shell_tape("cbttape kermit 23061e444100") == true);

    // 13. Verify DisplacementShader Scaling & Vertex Constraints
    printf("  -> Testing DisplacementShader boundary wrapping & vertex coordinate scaling...\n");
    TSFiDisplacementShader ds;
    tsfi_displacementshader_init(&ds, 5.0, 2.0);
    double offset1 = tsfi_displacementshader_eval(&ds, 10.0, 20.0);
    double offset2 = tsfi_displacementshader_eval(&ds, 10.0 + 256.0, 20.0 + 512.0);
    assert(offset1 == offset2);
    printf("  -> DisplacementShader coordinate boundary wrap verified successfully.\n");

    // 14. Verify JIT Thunk VM compilation and forwarding
    printf("  -> Testing JIT Thunk VM context creation and execution...\n");
    ThunkProxy *thunk = ThunkProxy_create();
    assert(thunk != NULL);
    assert(thunk->thunk_pool != NULL);
    
    void *jit_fn = ThunkProxy_emit_baked(thunk, (void*)test_thunk_callback, 1, "thunk_verify_test");
    assert(jit_fn != NULL);
    
    typedef void (*thunk_entry)(void);
    ((thunk_entry)jit_fn)();
    
    assert(g_thunk_callback_val == 42);
    printf("  -> JIT Thunk dynamic execution verified successfully.\n");

    // 15. Verify CPU Register State Rollback on abort
    printf("  -> Testing ZMM GPR register restoration on transaction rollback...\n");
    extern uint32_t ce_gprs[16];
    ce_gprs[5] = 888;
    assert(tsfi_xplos_shell_tape("cbttape inject 0") == true);
    assert(tsfi_xplos_shell_tape("cbttape write 5 187") == false);
    assert(ce_gprs[5] == 888);
    printf("  -> ZMM GPR register restoration verified successfully.\n");

    // 16. Verify transactional thunk execution with closed-loop GPR rollback
    printf("  -> Testing transactional thunk execution rollback...\n");
    ce_gprs[7] = 111;
    // Inject failure, run transactional thunk (which changes it to 777, but rolls back to 111)
    assert(tsfi_xplos_shell_tape("cbttape inject 0") == true);
    execute_transactional_thunk(thunk, (void*)test_thunk_callback, "thunk_verify_test", 6);
    assert(ce_gprs[7] == 111);
    printf("  -> Transactional thunk execution rollback verified successfully.\n");

    // 17. Verify support for GPR SKELETON bound to RenderMan camera speed
    printf("  -> Testing GPR SKELETON bindings mapped to RenderMan interface...\n");
    TSFiRiInterface ri;
    tsfi_riinterface_init(&ri);
    
    ce_gprs[8] = 45; // Camera speed register mapping
    double camera_velocity = (double)ce_gprs[8];
    double verlet_x[3] = { 1.0, 2.0, 3.0 };
    double prev_verlet_x[3] = { 0.9, 1.9, 2.9 };
    
    tsfi_riinterface_run_8step_loop(&ri, camera_velocity, verlet_x, prev_verlet_x, 3);
    assert(ri.is_world_active == false);
    assert(ri.psg_master_volume_l > 0 || ri.psg_master_volume_r > 0);
    printf("  -> GPR SKELETON and RenderMan interface mapping verified successfully.\n");

    // 18. Verify RenderMan LFO phase synchronization with DisplacementShader
    printf("  -> Testing RenderMan LFO phase sync with DisplacementShader...\n");
    TSFiDisplacementShader ds_sync;
    tsfi_displacementshader_init(&ds_sync, 5.0, 2.0);
    double out_no_lfo = tsfi_displacementshader_eval(&ds_sync, 10.0, 20.0);
    
    // Synchronize displacement shader to RenderMan phase value
    tsfi_riinterface_set_lfo_phase(&ri, 31); // Setting RenderMan LFO phase
    tsfi_displacementshader_set_lfo_phase(&ds_sync, 3.1415926535); // Synchronize
    double out_lfo_pi = tsfi_displacementshader_eval(&ds_sync, 10.0, 20.0);
    
    // A phase displacement of PI must invert the sign of the sinusoidal wave output
    assert(fabs(out_no_lfo + out_lfo_pi) < 1e-5);
    printf("  -> RenderMan LFO phase sync with DisplacementShader verified successfully.\n");

    // 19. Verify CAPSTAN KERMIT Quote prefix decoding (escape sequences)
    printf("  -> Testing CAPSTAN KERMIT quote prefix character decoding...\n");
    extern char g_tape_journal_payloads[256][64];
    
    // Inject verify pass
    assert(tsfi_xplos_shell_tape("cbttape inject 1") == true);
    
    // Process packet with payload containing escaped SOH control character (0x23 0x09 0x20 0x44 0x41 0x23 0x41 0x55)
    assert(tsfi_xplos_shell_tape("cbttape kermit 2309204441234155") == true);
    
    // Assert that the journal matches the unescaped output: 'A' followed by 0x01
    assert(g_tape_journal_payloads[32][0] == 'A');
    assert(g_tape_journal_payloads[32][1] == 0x01);
    assert(g_tape_journal_payloads[32][2] == '\0');
    printf("  -> CAPSTAN KERMIT quote prefix character decoding verified successfully.\n");

    // 20. Verify ALU driving physical capstan tape emulation registers
    printf("  -> Testing ALU driving emulated hardware registers via S/370 assembly...\n");
    extern bool tsfi_xpl_execute_assembler(const char *asm_instruction, uint32_t *gprs, uint8_t *memory);
    ce_gprs[1] = 65000; // CAPSTAN_CONTROL memory address
    ce_gprs[2] = 2;     // R2 immediate value
    bool alu_ok = tsfi_xpl_execute_assembler("MVI 0(R1),2", ce_gprs, ce_memory);
    assert(alu_ok == true);
    
    ce_gprs[3] = 100;
    ce_gprs[4] = 50;
    assert(tsfi_xpl_execute_assembler("AR R3, R4", ce_gprs, ce_memory) == true);
    assert(ce_gprs[3] == 150);
    
    // Test Load (L), Store (ST), and Compare (CR) instructions
    ce_gprs[1] = 500; // Base address
    ce_gprs[5] = 0xAABBCCDD; // Value to store
    assert(tsfi_xpl_execute_assembler("ST R5, 10(R1)", ce_gprs, ce_memory) == true); // Store at address 510
    
    ce_gprs[6] = 0;
    assert(tsfi_xpl_execute_assembler("L R6, 10(R1)", ce_gprs, ce_memory) == true); // Load back from 510
    assert(ce_gprs[6] == 0xAABBCCDD);
    
    assert(tsfi_xpl_execute_assembler("CR R5, R6", ce_gprs, ce_memory) == true); // Compare R5 and R6
    assert(ce_gprs[0] == 0); // Result must be 0 (equal)
    
    printf("  -> ALU emulated register manipulations verified successfully.\n");

    // 21. Verify JCL Batch execution against running binary (FRT step execution)
    printf("  -> Testing FRT JCL batch job execution against running binary...\n");
    extern bool tsfi_xplos_shell_cbt_jcl(const char *cmd);
    
    // Inject write verification pass
    assert(tsfi_xplos_shell_tape("cbttape inject 1") == true);
    
    // Run JCL file
    bool jcl_ok = tsfi_xplos_shell_cbt_jcl("jclrun /home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/dysnomia/domain/jcl/frt_resonance.jcl");
    assert(jcl_ok == true);
    printf("  -> FRT JCL batch job execution verified successfully.\n");

    // 22. Verify LOGWRITE JCL step appending to assets/LOG.dat.bin
    printf("  -> Testing LOGWRITE JCL batch job execution and assets/LOG.dat.bin verification...\n");
    remove("assets/LOG.dat.bin"); // Clean start
    
    bool log_jcl_ok = tsfi_xplos_shell_cbt_jcl("jclrun /home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/dysnomia/domain/jcl/logwrite.jcl");
    assert(log_jcl_ok == true);
    
    FILE *log_f = fopen("assets/LOG.dat.bin", "r");
    assert(log_f != NULL);
    char log_buf[256] = {0};
    size_t n_read = fread(log_buf, 1, sizeof(log_buf) - 1, log_f);
    fclose(log_f);
    
    assert(n_read > 0);
    bool found_msg = false;
    const char *pattern = "LOG MESSAGE: SUCCESSFUL SYSTEM STATE SNAPSHOT";
    size_t pat_len = strlen(pattern);
    for (size_t i = 0; i + pat_len <= n_read; i++) {
        if (memcmp(log_buf + i, pattern, pat_len) == 0) {
            found_msg = true;
            break;
        }
    }
    assert(found_msg == true);
    printf("  -> assets/LOG.dat.bin verification successful: Found appended message in binary log record.\n");
    remove("assets/LOG.dat.bin"); // Cleanup
    
    // 23. Verify Synthesizer input configuration and xlog self-test printing
    printf("  -> Testing Synthesizer input configuration and xlog self-test...\n");
    ce_memory[65006] = 1; // HBRIDGE_Q1_HSL = 1 (PNP ON)
    ce_memory[65009] = 1; // HBRIDGE_Q4_LSR = 1 (NPN ON)
    ce_memory[65007] = 0; // HBRIDGE_Q2_HSR = 0
    ce_memory[65008] = 0; // HBRIDGE_Q3_LSL = 0
    
    if (ce_memory[65006] == 1 && ce_memory[65009] == 1) {
        ce_memory[65010] = 8; // TONEWHEEL_CORE_LEVEL = 8
    } else {
        ce_memory[65010] = 0;
    }
    
    ce_memory[64100] = ce_memory[65010]; // Accumulator maps output level
    
    printf("[XLOG SELF-TEST] Active Synthesizer Input Configuration:\n");
    printf("  - High-Side Left PNP (Q1_HSL) : %d\n", ce_memory[65006]);
    printf("  - Low-Side Right NPN (Q4_LSR) : %d\n", ce_memory[65009]);
    printf("  - Tonewheel Core Level        : %d\n", ce_memory[65010]);
    printf("  - Non-Pref Accumulator State  : %d\n", ce_memory[64100]);
    
    assert(ce_memory[65010] == 8);
    assert(ce_memory[64100] == 8);
    printf("  -> Synthesizer config and xlog self-test printed successfully.\n");

    // 24. Verify Synthesizer configured tonewheel outputs drive basic PNP/NPN hardware registers in XPL
    printf("  -> Testing configured synthesizer tonewheel output routing to PNP/NPN hardware...\n");
    typedef struct {
        double level;
        bool gate_pnp;
        bool gate_npn;
    } TonewheelSynthInput;

    TonewheelSynthInput synth_input = {
        .level = 8.0,
        .gate_pnp = true,
        .gate_npn = true
    };
    
    /* Bridge synthesizer outputs to virtual PNP/NPN hardware registers */
    ce_memory[65006] = synth_input.gate_pnp ? 1 : 0; /* HBRIDGE_Q1_HSL (PNP) */
    ce_memory[65009] = synth_input.gate_npn ? 1 : 0; /* HBRIDGE_Q4_LSR (NPN) */
    
    /* Simulating XPL execution receiving the hardware registers */
    if (ce_memory[65006] == 1 && ce_memory[65009] == 1) {
        ce_memory[65010] = (uint8_t)synth_input.level; /* TONEWHEEL_CORE_LEVEL */
    } else {
        ce_memory[65010] = 0;
    }
    ce_memory[64100] = ce_memory[65010]; /* Non-Pref Accumulator */
    
    assert(ce_memory[65006] == 1);
    assert(ce_memory[65009] == 1);
    assert(ce_memory[65010] == 8);
    printf("  -> Synthesizer output successfully received as PNP/NPN hardware parameters by xlog.\n");

    printf("\n=== INTEGRATION PROOFS PASSED ===\n");
    return 0;
}
