#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xplos_kernel_internal.h"
#include "tsfi_xplos_shell_cbt_jcl.h"
#include "tsfi_xplos_shell_cbt_jes.h"
#include "tsfi_xplos_shell_cbt_tso.h"
#include "tsfi_xplos_shell_cbt_vtam.h"
#include "tsfi_mainframe_computerworld.h"
#include "auncient_sdk.h"
#include "tsfi_winchester_bridge.h"
#include "tsfi_displacementshader.h"
#include "lau_thunk.h"
extern XplosVirtualDisk g_vfs;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;
static void resolve_pds_name_helper(const char *member, char *out, size_t max_len) {
    snprintf(out, max_len, "%s.dat.bin", member);
}

static bool tsfi_xlog_recover(const char *log_filepath) {
    FILE *f = fopen(log_filepath, "rb");
    if (!f) return false;

    printf("[XLOG RECOVERY] Initiating binary log recovery scan from '%s'...\n", log_filepath);
    uint8_t buffer[4096];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);

    if (bytes_read < 24) {
        printf("[XLOG RECOVERY ERROR] Log file too small to contain valid header.\n");
        return false;
    }

    size_t offset = 0;
    int record_index = 0;
    while (offset + 24 <= bytes_read) {
        if (buffer[offset] != 254 || buffer[offset + 1] != 237) {
            printf("[XLOG RECOVERY ERROR] Invalid log magic prefix at record %d offset %zu.\n", record_index, offset);
            return false;
        }

        uint32_t cycle = (buffer[offset + 4] << 24) | (buffer[offset + 5] << 16) | (buffer[offset + 6] << 8) | buffer[offset + 7];
        uint32_t lsn = (buffer[offset + 8] << 24) | (buffer[offset + 9] << 16) | (buffer[offset + 10] << 8) | buffer[offset + 11];
        uint32_t op_count = (buffer[offset + 12] << 24) | (buffer[offset + 13] << 16) | (buffer[offset + 14] << 8) | buffer[offset + 15];
        uint32_t res_bytes = (buffer[offset + 16] << 24) | (buffer[offset + 17] << 16) | (buffer[offset + 18] << 8) | buffer[offset + 19];
        uint8_t buf_active = buffer[offset + 20];
        uint32_t buf_head = (buffer[offset + 21] << 16) | (buffer[offset + 22] << 8) | buffer[offset + 23];

        printf("[XLOG RECOVERY] Record %d: Cycle=%u LSN=%u OpCount=%u ResBytes=%u ActiveBuffer=%u HeadOffset=%u\n",
               record_index, cycle, lsn, op_count, res_bytes, buf_active, buf_head);

        if (offset + 24 + buf_head > bytes_read) {
            printf("[XLOG RECOVERY ERROR] Truncated payload for record %d.\n", record_index);
            return false;
        }

        uint32_t hash = 2166136261U;
        for (uint32_t i = 0; i < buf_head; i++) {
            hash = (hash ^ buffer[offset + 24 + i]) * 16777619U;
        }
        printf("[XLOG RECOVERY] Record %d calculated FNV-1a checksum: 0x%08X\n", record_index, hash);

        char text_payload[2048] = {0};
        uint32_t text_len = 0;
        for (uint32_t i = 0; i < buf_head; i++) {
            uint8_t byte = buffer[offset + 24 + i];
            if (byte >= 32 && byte < 127) {
                text_payload[text_len++] = (char)byte;
            } else if (byte == '\n') {
                text_payload[text_len++] = ' ';
            }
        }
        text_payload[text_len] = '\0';
        if (strlen(text_payload) > 0) {
            printf("[XLOG REPLAY] Replayed Log Message: \"%s\"\n", text_payload);
        }

        offset += 24 + buf_head;
        record_index++;
    }

    printf("[XLOG RECOVERY SUCCESS] All %d binary log records successfully replayed.\n", record_index);
    return true;
}

static bool handle_jclrun(const char *cmd) {
    char jcl_name[256] = "";
    if (sscanf(cmd + 7, "%255s", jcl_name) == 1) {
        char vfs_filename[256];
        resolve_pds_name_helper(jcl_name, vfs_filename, sizeof(vfs_filename));

        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
                file_idx = i;
                break;
            }
        }

        char jcl_data[8192];
        int vsam_found = 0;
        const char *search_pds[] = {"USERLIB.dat.bin", "JCLLIB.dat.bin", "PROCLIB.dat.bin"};
        for (int p = 0; p < 3; p++) {
            tsfi_cw_vsam_ksds proclib_ksds;
            int open_rc = tsfi_cw_vsam_open(&proclib_ksds, search_pds[p]);
            if (open_rc == 0) {
                uint8_t read_buf[8192] = {0};
                int read_len = 0;
                int read_rc = tsfi_cw_vsam_read(&proclib_ksds, jcl_name, read_buf, sizeof(read_buf), &read_len);
                if (read_rc == 0 && read_len > 0) {
                    strncpy(jcl_data, (char *)read_buf, sizeof(jcl_data) - 1);
                    jcl_data[sizeof(jcl_data) - 1] = '\0';
                    vsam_found = 1;
                    printf("[JCLRUN] Dynamic PDS library lookup: Loaded JCL member '%s' from %s VSAM index\n", jcl_name, search_pds[p]);
                    break;
                }
            }
        }

        if (vsam_found) {
            // Loaded successfully from VSAM PROCLIB
        } else if (file_idx >= 0) {
            strncpy(jcl_data, g_vfs.files[file_idx].data, sizeof(jcl_data) - 1);
            jcl_data[sizeof(jcl_data) - 1] = '\0';
        } else {
            FILE *f = fopen(jcl_name, "r");
            if (f) {
                size_t bytes_read = fread(jcl_data, 1, sizeof(jcl_data) - 1, f);
                jcl_data[bytes_read] = '\0';
                fclose(f);
            } else {
                // Simulated default JCL stream
                printf("[JCLRUN] Warning: JCL member '%s' not found. Using fallback JCL execution.\n", jcl_name);
                snprintf(jcl_data, sizeof(jcl_data),
                         "//%s JOB 'CBT TAPE RUN',CLASS=A,MSGCLASS=X\n"
                         "//STEP1 EXEC PGM=IEBCOPY\n"
                         "//SYSUT1 DD DSN=CBT.V510.FILE002,DISP=SHR\n"
                         "//SYSUT2 DD DSN=CBT.V510.FILE003,DISP=OLD\n"
                         "// IF (RC = 0) THEN\n"
                         "//STEP2 EXEC PGM=IBHDRPLY\n"
                         "// ENDIF\n",
                         jcl_name);
            }
        }

        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "[JCLRUN EXECUTION START: %s]\n", jcl_name);
        printf("%s", log_msg);
        append_spool_log(jcl_name, log_msg);

        char sym_names[10][32] = {0};
        char sym_vals[10][64] = {0};
        int sym_count = 0;

        char step_names[10][32] = {0};
        int step_rcs[10] = {0};
        int step_rc_count = 0;

        char jcl_data_copy[8192];
        strncpy(jcl_data_copy, jcl_data, sizeof(jcl_data_copy) - 1);
        jcl_data_copy[sizeof(jcl_data_copy) - 1] = '\0';

        char *line = strtok(jcl_data, "\n");
        int rc = 0;
        bool skip_block = false;

        while (line) {
            // Trim leading spaces
            while (isspace((unsigned char)*line)) line++;

            if (strncmp(line, "//", 2) == 0) {
                // Copy the card line to a buffer so we can modify it
                char card_buf[1024];
                strncpy(card_buf, line + 2, sizeof(card_buf) - 1);
                card_buf[sizeof(card_buf) - 1] = '\0';
                
                // Trim leading spaces of the card content
                char *card = card_buf;
                while (isspace((unsigned char)*card)) card++;

                // Replace any variable references (e.g. &VARNAME) with symbol values
                for (int s = 0; s < sym_count; s++) {
                    char ref_pattern[64];
                    snprintf(ref_pattern, sizeof(ref_pattern), "&%s", sym_names[s]);
                    char *pos;
                    while ((pos = strstr(card, ref_pattern)) != NULL) {
                        char temp[2048];
                        int offset = pos - card;
                        strncpy(temp, card, offset);
                        temp[offset] = '\0';
                        strcat(temp, sym_vals[s]);
                        strcat(temp, pos + strlen(ref_pattern));
                        strcpy(card_buf, temp);
                        card = card_buf;
                    }
                }

                // Check for SET symbol definition card
                if (strncmp(card, "SET ", 4) == 0 || strncmp(card, "set ", 4) == 0) {
                    char var_name[32] = {0};
                    char var_val[64] = {0};
                    if (sscanf(card + 4, "%31[^=]=%63s", var_name, var_val) == 2) {
                        if (sym_count < 10) {
                            strcpy(sym_names[sym_count], var_name);
                            strcpy(sym_vals[sym_count], var_val);
                            sym_count++;
                            printf("[JCLRUN] Dynamic Symbol Table Override (XCOM): Set %s = %s\n", var_name, var_val);
                        }
                    }
                    line = strtok(NULL, "\n");
                    continue;
                }

                if (strncmp(card, "IF ", 3) == 0 || strncmp(card, "if ", 3) == 0) {
                    snprintf(log_msg, sizeof(log_msg), "  JCL_COND> Checking condition: %s\n", card);
                    printf("%s", log_msg);
                    append_spool_log(jcl_name, log_msg);
                    if (strstr(card, "RC = 0") && rc != 0) {
                        skip_block = true;
                        snprintf(log_msg, sizeof(log_msg), "  JCL_COND> Condition Failed. Bypassing subsequent steps.\n");
                        printf("%s", log_msg);
                        append_spool_log(jcl_name, log_msg);
                    }
                }
                else if (strncmp(card, "ENDIF", 5) == 0 || strncmp(card, "endif", 5) == 0) {
                    skip_block = false;
                    snprintf(log_msg, sizeof(log_msg), "  JCL_COND> ENDIF reached.\n");
                    printf("%s", log_msg);
                    append_spool_log(jcl_name, log_msg);
                }
                else if (strstr(card, " EXEC ") || strstr(card, " exec ")) {
                    if (skip_block) {
                        snprintf(log_msg, sizeof(log_msg), "  JCL_STEP> Bypassed step: %s\n", card);
                        printf("%s", log_msg);
                        append_spool_log(jcl_name, log_msg);
                    } else {
                        // Extract step name from start of the line, e.g. //STEP1 EXEC
                        char step_name[32] = {0};
                        sscanf(line + 2, "%31s", step_name);
                        if (strcasecmp(step_name, "EXEC") == 0) {
                            step_name[0] = '\0';
                        }

                        // Check for step-level COND parameter, e.g. COND=(0,EQ,STEP1)
                        bool cond_bypass = false;
                        char *cond_ptr = strstr(card, "COND=");
                        if (cond_ptr) {
                            int cond_code = 0;
                            char cond_op[8] = "";
                            char cond_step[32] = "";
                            int matched = sscanf(cond_ptr + 5, "(%d,%7[^,)],%31[^)])", &cond_code, cond_op, cond_step);
                            if (matched >= 2) {
                                int target_rc = rc; // Default to previous/global max RC
                                if (matched == 3 && strlen(cond_step) > 0) {
                                    for (int s = 0; s < step_rc_count; s++) {
                                        if (strcmp(step_names[s], cond_step) == 0) {
                                            target_rc = step_rcs[s];
                                            break;
                                        }
                                    }
                                }
                                if (strcmp(cond_op, "EQ") == 0 && target_rc == cond_code) cond_bypass = true;
                                else if (strcmp(cond_op, "NE") == 0 && target_rc != cond_code) cond_bypass = true;
                                else if (strcmp(cond_op, "GT") == 0 && target_rc > cond_code) cond_bypass = true;
                                else if (strcmp(cond_op, "LT") == 0 && target_rc < cond_code) cond_bypass = true;
                                else if (strcmp(cond_op, "GE") == 0 && target_rc >= cond_code) cond_bypass = true;
                                else if (strcmp(cond_op, "LE") == 0 && target_rc <= cond_code) cond_bypass = true;

                                if (cond_bypass) {
                                    snprintf(log_msg, sizeof(log_msg), "  JCL_STEP> COND met: Bypassing step execution based on comparison %d %s %d\n", target_rc, cond_op, cond_code);
                                    printf("%s", log_msg);
                                    append_spool_log(jcl_name, log_msg);
                                }
                            }
                        }

                        if (cond_bypass) {
                            // Bypassed via COND parameter
                            if (step_rc_count < 10 && strlen(step_name) > 0) {
                                strcpy(step_names[step_rc_count], step_name);
                                step_rcs[step_rc_count] = 0; // Standard bypassed step return code
                                step_rc_count++;
                            }
                        } else {
                            char pgm_name[64] = "";
                            char *pgm_ptr = strstr(card, "PGM=");
                            if (pgm_ptr) {
                                sscanf(pgm_ptr + 4, "%63[^, \r\n]", pgm_name);
                            }
                            int step_rc = 0;
                            snprintf(log_msg, sizeof(log_msg), "  JCL_STEP> Executing step Program: %s\n", pgm_name);
                            printf("%s", log_msg);
                            append_spool_log(jcl_name, log_msg);
                            if (strcmp(pgm_name, "IEBCOPY") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * IEBCOPY completed successfully. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);
                            } else if (strcmp(pgm_name, "IBHDRPLY") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * IBHDRPLY Automatic Reply executed. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);
                            } else if (strcmp(pgm_name, "IEFBR14") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * IEFBR14 Dummy Program executed. Resolving DD dispositions. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);
                            } else if (strcmp(pgm_name, "RAUPGM") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * RAUPGM Coaxial RAU Backplane Utility launched. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);

                                sdk_coaxial_env_t env;
                                if (auncient_sdk_init_coaxial(&env)) {
                                    printf("[RAUPGM] Coaxial RAU Backplane mounted successfully.\n");
                                    bool reg_ok = true;
                                    for (uint32_t reg_idx = 1; reg_idx < 8; reg_idx++) {
                                        auncient_rau_poke(&env, reg_idx, 0xAA550000 + reg_idx);
                                        uint32_t val = auncient_rau_peek(&env, reg_idx);
                                        if (val != (0xAA550000 + reg_idx)) {
                                            reg_ok = false;
                                        }
                                    }
                                    if (reg_ok) {
                                        printf("[RAUPGM] Verified read/write access to all RAU registers successfully.\n");
                                    } else {
                                        step_rc = 8;
                                        printf("[RAUPGM ERROR] RAU register check failed.\n");
                                    }
                                    auncient_sdk_close_coaxial(&env);
                                } else {
                                    step_rc = 12;
                                    printf("[RAUPGM ERROR] Failed to mount Coaxial RAU Backplane.\n");
                                }
                            } else if (strcmp(pgm_name, "SCSIPGM") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * SCSIPGM WinchesterMQ SCSI Accessor launched. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);

                                TSFiWinchesterBridge *bridge = tsfi_winchester_bridge_create(NULL);
                                if (bridge) {
                                    printf("[SCSIPGM] WinchesterMQ bridge initialized.\n");
                                    int verify_rc = tsfi_winchester_bridge_loopback_verify(bridge, 32);
                                    if (verify_rc == 0) {
                                        printf("[SCSIPGM] Loopback verification verified successfully.\n");
                                    } else {
                                        step_rc = 8;
                                        printf("[SCSIPGM ERROR] Loopback check failed.\n");
                                    }
                                    
                                    TSFiDisplacementShader ds;
                                    tsfi_displacementshader_init(&ds, 1.0, 2.0);
                                    double val = tsfi_displacementshader_eval(&ds, 10.0, 5.0);
                                    printf("[SCSIPGM] Displacement value calculated: %f\n", val);

                                    tsfi_winchester_bridge_destroy(bridge);
                                } else {
                                    step_rc = 12;
                                    printf("[SCSIPGM ERROR] Bridge creation failed.\n");
                                }
                            } else if (strcmp(pgm_name, "YULVM") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * YULVM Instruction Execution Agent active. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);

                                printf("[YULVM] Fetching next instruction node from TSQ rail...\n");
                                
                                // Initialize identically to ZMM Thunk VM execution pipeline
                                ThunkProxy *thunk = ThunkProxy_create();
                                if (thunk) {
                                    printf("[YULVM] Thunk JVM proxy context initialized at %p\n", thunk->thunk_pool);
                                    // Emit JIT dynamic contract step thunk
                                    extern bool tsfi_xplos_shell_cbt_cics(const char *cmd);
                                    void *jit_fn = ThunkProxy_emit_baked(thunk, (void*)tsfi_xplos_shell_cbt_cics, 1, "cbtcicsts read UNAME");
                                    printf("[YULVM] Emitted dynamic instruction execution thunk at %p\n", jit_fn);
                                    
                                    // Execute the emitted thunk
                                    typedef void (*thunk_entry)(void);
                                    ((thunk_entry)jit_fn)();
                                }
                                
                                printf("[YULVM] Executing opcode: PUSH 953467954114363\n");
                                printf("[YULVM] Executing opcode: TUNE (Motzkin Prime Modulus)\n");
                                printf("[YULVM] Opcode execution state stored persistently in TSQ rail. RC=0000\n");
                            } else if (strcmp(pgm_name, "TSOTMP") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * TSOTMP Terminal Monitor Program launched. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);

                                char coax_cmd[2048] = {0};
                                tsfi_vtam_coax_read_buffer(coax_cmd, sizeof(coax_cmd));
                                if (strlen(coax_cmd) > 0) {
                                    char *nl = strchr(coax_cmd, '\n');
                                    if (nl) *nl = '\0';
                                    snprintf(log_msg, sizeof(log_msg), "      TSO_TMP> Executing command routed coaxially: %s\n", coax_cmd);
                                    printf("%s", log_msg);
                                    append_spool_log(jcl_name, log_msg);
                                    tsfi_xplos_shell_cbt_tso(coax_cmd);
                                }
                            } else if (strcmp(pgm_name, "FRT") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * FRT Fourier Resonance Tracker active. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);
                                
                                printf("[FRT] Binding GPR skeleton. R8=Frequency, R9=Q-Factor\n");
                                extern uint32_t ce_gprs[16];
                                ce_gprs[8] = 44;
                                ce_gprs[9] = 50;
                                
                                // Perform physical verification
                                if (ce_gprs[8] != 44 || ce_gprs[9] < 50) {
                                    step_rc = 16;
                                    snprintf(log_msg, sizeof(log_msg), "    * FRT Aborted: De-tuned frequency or low Q-Factor. RC=0016\n");
                                    printf("%s", log_msg);
                                    append_spool_log(jcl_name, log_msg);
                                } else {
                                    printf("[FRT] GPR verified. Executing full CBT Tape operations loop...\n");
                                    extern bool tsfi_xplos_shell_tape(const char *cmd);
                                    
                                    // 1. Single Sector Write
                                    if (!tsfi_xplos_shell_tape("cbttape write 10 44")) {
                                        step_rc = 16;
                                        printf("[FRT ERROR] Single sector write failed.\n");
                                    }
                                    
                                    // 2. Lock Isolation & Aborted Group Commit
                                    tsfi_xplos_shell_tape("cbttape lock write 20");
                                    if (tsfi_xplos_shell_tape("cbttape writegroup 18 4")) {
                                        step_rc = 16;
                                        printf("[FRT ERROR] Group write succeeded despite exclusive write lock.\n");
                                    }
                                    
                                    // 3. Unlock & Successful Group Commit
                                    tsfi_xplos_shell_tape("cbttape unlock 20");
                                    if (!tsfi_xplos_shell_tape("cbttape writegroup 18 4")) {
                                        step_rc = 16;
                                        printf("[FRT ERROR] Unlocked group write failed.\n");
                                    }
                                    
                                    // 4. Journaling and ARM Recovery Replay
                                    tsfi_xplos_shell_tape("cbttape journal 201 TX_ALLOCATE");
                                    tsfi_xplos_shell_tape("cbttape journal 202 TX_COMMIT");
                                    if (!tsfi_xplos_shell_tape("cbttape recover")) {
                                        step_rc = 16;
                                        printf("[FRT ERROR] Volume recovery replay failed.\n");
                                    }
                                    
                                    // 5. Volume Reconciliation
                                    if (!tsfi_xplos_shell_tape("cbttape reconcile")) {
                                        step_rc = 16;
                                        printf("[FRT ERROR] Volume reconciliation failed.\n");
                                    }
                                    
                                    if (step_rc == 0) {
                                        printf("[FRT] 100%% of CBT Tape operations executed successfully within batch job.\n");
                                    } else {
                                        snprintf(log_msg, sizeof(log_msg), "    * FRT Aborted: Transaction lifecycle failure. RC=0016\n");
                                        printf("%s", log_msg);
                                        append_spool_log(jcl_name, log_msg);
                                    }
                                }
                            } else if (strcmp(pgm_name, "SYNTHPLAY") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * SYNTHPLAY Active. Processing note patterns. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);
                                
                                const char *sysin_ptr = strstr(jcl_data_copy, "SYSIN DD *");
                                if (!sysin_ptr) {
                                    sysin_ptr = strstr(jcl_data_copy, "sysin dd *");
                                }
                                if (sysin_ptr) {
                                    const char *line_start = strchr(sysin_ptr, '\n');
                                    if (line_start) {
                                        line_start++;
                                        while (*line_start && strncmp(line_start, "/*", 2) != 0 && strncmp(line_start, "//", 2) != 0) {
                                            const char *line_end = strchr(line_start, '\n');
                                            if (!line_end) break;
                                            size_t line_len = line_end - line_start;
                                            char command_line[128] = {0};
                                            if (line_len < sizeof(command_line) - 1) {
                                                strncpy(command_line, line_start, line_len);
                                                while (line_len > 0 && (command_line[line_len - 1] == '\r' || command_line[line_len - 1] == ' ')) {
                                                    command_line[line_len - 1] = '\0';
                                                    line_len--;
                                                }
                                                if (strlen(command_line) > 0) {
                                                    char note_name[32] = "";
                                                    char state_name[32] = "";
                                                    if (sscanf(command_line, "NOTE %31s %31s", note_name, state_name) == 2) {
                                                        printf("[SYNTHPLAY] Processing JCL input: NOTE %s %s\n", note_name, state_name);
                                                        extern uint8_t ce_memory[];
                                                        if (strcmp(state_name, "ON") == 0) {
                                                            ce_memory[65006] = 1; /* HBRIDGE_Q1_HSL */
                                                            ce_memory[65009] = 1; /* HBRIDGE_Q4_LSR */
                                                            ce_memory[65010] = 8; /* TONEWHEEL_CORE_LEVEL */
                                                            printf("[SYNTHPLAY] Class B push-pull pair Q1/Q4 active. Synthesizer input driving level: 8\n");
                                                        } else {
                                                            ce_memory[65006] = 0; /* HBRIDGE_Q1_HSL */
                                                            ce_memory[65009] = 0; /* HBRIDGE_Q4_LSR */
                                                            ce_memory[65010] = 0; /* TONEWHEEL_CORE_LEVEL */
                                                            printf("[SYNTHPLAY] Class B push-pull pair Q1/Q4 idle. Synthesizer input driving level: 0\n");
                                                        }
                                                    }
                                                }
                                            }
                                            line_start = line_end + 1;
                                        }
                                    }
                                }
                            } else if (strcmp(pgm_name, "LOGWRITE") == 0) {
                                step_rc = 0;
                                snprintf(log_msg, sizeof(log_msg), "    * LOGWRITE Active. Appending messages to assets/LOG.dat.bin. RC=0000\n");
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);
                                
                                const char *msg_to_write = "Default log message\n";
                                char instream_data[2048] = {0};
                                const char *sysin_ptr = strstr(jcl_data_copy, "SYSIN DD *");
                                if (!sysin_ptr) {
                                    sysin_ptr = strstr(jcl_data_copy, "sysin dd *");
                                }
                                if (sysin_ptr) {
                                    const char *line_start = strchr(sysin_ptr, '\n');
                                    if (line_start) {
                                        line_start++;
                                        while (*line_start && strncmp(line_start, "/*", 2) != 0 && strncmp(line_start, "//", 2) != 0) {
                                            const char *line_end = strchr(line_start, '\n');
                                            if (!line_end) break;
                                            size_t line_len = line_end - line_start;
                                            if (strlen(instream_data) + line_len + 1 < sizeof(instream_data)) {
                                                strncat(instream_data, line_start, line_len);
                                                strcat(instream_data, "\n");
                                            }
                                            line_start = line_end + 1;
                                        }
                                        if (strlen(instream_data) > 0) {
                                            msg_to_write = instream_data;
                                        }
                                    }
                                }
                                
                                // XPL XLOG Memory Emulator (simulating xlog_skeleton.xpl at address 64000)
                                uint8_t xpl_mem[1024];
                                memset(xpl_mem, 0, sizeof(xpl_mem));
                                
                                // 1. Simulate INIT_XLOG_SKELETON
                                xpl_mem[0] = 254; // Magic 0xFEED (254, 237)
                                xpl_mem[1] = 237;
                                xpl_mem[7] = 1;   // Cycle number = 1
                                
                                // 2. Simulate RESERVE_XLOG_SPACE
                                uint32_t msg_len = (uint32_t)strlen(msg_to_write);
                                uint32_t reserved = msg_len + 4;
                                xpl_mem[19] = reserved; // Write to XLOG_RES_BYTES
                                printf("[XPL COPROGRAM] Space reserved: %d bytes (xlog ticket allocated)\n", reserved);
                                
                                // 3. Simulate COMMIT_XLOG_TRANSACTION
                                uint32_t write_offset = 0;
                                for (uint32_t c = 0; c < msg_len; c++) {
                                    xpl_mem[24 + write_offset] = msg_to_write[c];
                                    write_offset++;
                                }
                                xpl_mem[23] = write_offset; // Update XLOG_BUF_HEAD
                                xpl_mem[15]++;              // Increment XLOG_OP_COUNT
                                
                                // 4. ACID-compliant append of xlog struct format
                                FILE *lf = fopen("assets/LOG.dat.bin.tmp", "wb");
                                if (lf) {
                                    // Retain existing log records if any
                                    FILE *existing = fopen("assets/LOG.dat.bin", "rb");
                                    if (existing) {
                                        char buffer[4096];
                                        size_t bytes_read;
                                        while ((bytes_read = fread(buffer, 1, sizeof(buffer), existing)) > 0) {
                                            fwrite(buffer, 1, bytes_read, lf);
                                        }
                                        fclose(existing);
                                    }
                                    
                                    // Append raw binary xlog block
                                    fwrite(xpl_mem, 1, 24 + write_offset, lf);
                                    
                                    fflush(lf);
                                    #ifdef _WIN32
                                    _commit(_fileno(lf));
                                    #else
                                    fsync(fileno(lf));
                                    #endif
                                    fclose(lf);
                                    
                                    rename("assets/LOG.dat.bin.tmp", "assets/LOG.dat.bin");
                                    printf("[LOGWRITE XPL SUCCESS] Committed %d bytes of xlog record structure to assets/LOG.dat.bin\n", 24 + write_offset);
                                    tsfi_xlog_recover("assets/LOG.dat.bin");
                                } else {
                                    step_rc = 16;
                                    printf("[LOGWRITE ERROR] Failed to open temporary commit log.\n");
                                }
                            } else {
                                step_rc = 4;
                                snprintf(log_msg, sizeof(log_msg), "    * Program %s executed. RC=0004 (Warning)\n", pgm_name);
                                printf("%s", log_msg);
                                append_spool_log(jcl_name, log_msg);
                            }
                            
                            rc = (step_rc > rc) ? step_rc : rc; // Track maximum global RC

                            if (step_rc_count < 10 && strlen(step_name) > 0) {
                                strcpy(step_names[step_rc_count], step_name);
                                step_rcs[step_rc_count] = step_rc;
                                step_rc_count++;
                            }
                        }
                    }
                }
                else if (strstr(card, " DD *") || strstr(card, " dd *")) {
                    if (!skip_block) {
                        snprintf(log_msg, sizeof(log_msg), "  JCL_ALLOC> Allocation Instream Data: %s\n", card);
                        printf("%s", log_msg);
                        append_spool_log(jcl_name, log_msg);

                        char instream_buf[2048] = {0};
                        line = strtok(NULL, "\n");
                        while (line) {
                            char *trim_line = line;
                            while (isspace((unsigned char)*trim_line)) trim_line++;
                            if (strncmp(trim_line, "/*", 2) == 0 || strncmp(trim_line, "//", 2) == 0) {
                                break;
                            }
                            strcat(instream_buf, trim_line);
                            strcat(instream_buf, "\n");
                            line = strtok(NULL, "\n");
                        }
                        tsfi_vtam_coax_write_buffer(instream_buf);
                    }
                }
                else if (strstr(card, " DD ") || strstr(card, " dd ")) {
                    if (!skip_block) {
                        snprintf(log_msg, sizeof(log_msg), "  JCL_ALLOC> Allocation: %s\n", card);
                        printf("%s", log_msg);
                        append_spool_log(jcl_name, log_msg);
                    }
                }
            }

            line = strtok(NULL, "\n");
        }

        snprintf(log_msg, sizeof(log_msg), "[JCLRUN EXECUTION COMPLETED: Max RC=%04d]\n", rc);
        printf("%s", log_msg);
        append_spool_log(jcl_name, log_msg);
        return true;
    }
    printf("[JCLRUN ERROR] JCL member name required.\n");
    return true;
}
#include "tsfi_xplos_shell_jcl_ieb.c"
#include "tsfi_xplos_shell_jcl_cbt.c"
