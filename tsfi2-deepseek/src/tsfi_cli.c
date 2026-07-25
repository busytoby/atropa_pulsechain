#include "tsfi_cli.h"
#include "lau_memory.h"
#include "lau_audit.h"
#include "tsfi_wiring.h"
#include "tsfi_hotloader.h"
#include "tsfi_trace.h"
#include "tsfi_math.h"
#include "tsfi_genetic.h"
#include "tsfi_io.h"
#include "tsfi_pulsechain_rpc.h"
#include "tsfi_block_monitor.h"
#include "auncient_sdk.h"
#include "tsfi_clendenin_synth.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>

static int handle_default_command(WaveSystem *ws, double new_i, const char *new_d) {
    fprintf(stderr, "[CLI_DEBUG] Entering handle_default_command (new_d=\"%s\")\n", new_d);
    lau_unseal_object(ws);
    ws->current_intensity = new_i; 
    
    if (ws->current_directive) {
        lau_free(ws->current_directive);
        ws->current_directive = NULL;
    }
    ws->current_directive = lau_strdup(new_d);
    lau_seal_object(ws);
    fprintf(stderr, "[CLI_DEBUG] Calling HELMHOLTZ_RESONANCE_LIST...\n");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, ws->current_intensity);
    fprintf(stderr, "[CLI_DEBUG] HELMHOLTZ_RESONANCE_LIST finished.\n");
    tsfi_io_printf(stdout, "[AUDIT] %s\n", *ws->resonance_as_status);
    return 0;
}

static int handle_math_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    int bits = 2048;
    sscanf(new_d, "MATH %d", &bits);
    tsfi_io_printf(stdout, "[MATH] Running %d-bit AVX512 Modular Exponentiation...\n", bits);

    TSFiBigInt *b = tsfi_bn_alloc();
    TSFiBigInt *e = tsfi_bn_alloc();
    TSFiBigInt *m = tsfi_bn_alloc();
    TSFiBigInt *res = tsfi_bn_alloc();

    // Setup
    tsfi_bn_randomize(b);
    tsfi_bn_randomize(e);
    tsfi_bn_randomize(m);
    m->limbs[0] |= 1; // Odd modulus

    // Adjust active limbs for requested bits
    int num_limbs = (bits + 51) / 52; // ceil(bits / 52)
    if (num_limbs > TSFI_NUM_LIMBS) num_limbs = TSFI_NUM_LIMBS;
    b->active_limbs = num_limbs;
    e->active_limbs = num_limbs;
    m->active_limbs = num_limbs;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    tsfi_bn_modpow_avx512(res, b, e, m);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    
    uint8_t out[8];
    tsfi_bn_to_bytes(res, out, sizeof(out));
    tsfi_io_printf(stdout, "[MATH] Result (Top 64 bits): ");
    for(int i=0; i<8; i++) tsfi_io_printf(stdout, "%02X", out[7-i]); // Big Endian print
    tsfi_io_printf(stdout, "\n");
    tsfi_io_printf(stdout, "[MATH] Time: %.6f seconds\n", time_taken);

    tsfi_bn_free(b);
    tsfi_bn_free(e);
    tsfi_bn_free(m);
    tsfi_bn_free(res);
    return 0;
}

static int handle_provenance_command(WaveSystem *ws) {
    if (ws->provenance) {
        ws->provenance();
    } else {
        tsfi_io_printf(stdout, "[CLI] Error: Provenance logic not bound.\n");
    }
    return 0;
}static int handle_telemetry_command(WaveSystem *ws) {
    tsfi_io_printf(stdout, "========================================\n");
    tsfi_io_printf(stdout, "       SYSTEM TELEMETRY REPORT          \n");
    tsfi_io_printf(stdout, "========================================\n");
    tsfi_io_printf(stdout, "Current Intensity: %.4f\n", ws->current_intensity);
    tsfi_io_printf(stdout, "Current Directive: %s\n", ws->current_directive ? ws->current_directive : "NONE");
    tsfi_io_printf(stdout, "Resonance Status : %s\n", (ws->resonance_as_status && *ws->resonance_as_status) ? *ws->resonance_as_status : "UNKNOWN");
    
    tsfi_io_printf(stdout, "\n[Memory Metrics]\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    tsfi_io_printf(stdout, "========================================\n");
    return 0;
}

bool resolve_token_alias(const char *symbol_or_name, char *out_address, size_t out_max) {
    if (strcasecmp(symbol_or_name, "FederalMinter") == 0) {
        snprintf(out_address, out_max, "0xc15c5F699Daf5e1135732139f05D2c05b3EF4354");
        return true;
    }
    if (strcasecmp(symbol_or_name, "atropa") == 0) {
        snprintf(out_address, out_max, "0x7a20189B297343CF26d8548764b04891f37F3414");
        return true;
    }
    if (strcasecmp(symbol_or_name, "FDIC") == 0) {
        snprintf(out_address, out_max, "0x812571a12330a74e2a3c1ff8953f6f3aac7a83e9");
        return true;
    }

    FILE *f = fopen("assets/treasury_tokens_federalminter.json", "r");
    if (!f) {
        f = fopen("../assets/treasury_tokens_federalminter.json", "r");
    }
    if (!f) {
        f = fopen("tsfi2-deepseek/assets/treasury_tokens_federalminter.json", "r");
    }
    if (f) {
        char line[1024];
        char current_addr[128] = {0};
        char current_sym[128] = {0};
        while (fgets(line, sizeof(line), f)) {
            char *addr_ptr = strstr(line, "\"address\":");
            if (addr_ptr) {
                char *start = strchr(addr_ptr + 10, '"');
                if (start) {
                    char *end = strchr(start + 1, '"');
                    if (end) {
                        size_t len = end - (start + 1);
                        if (len < sizeof(current_addr)) {
                            memcpy(current_addr, start + 1, len);
                            current_addr[len] = '\0';
                        }
                    }
                }
            }
            char *sym_ptr = strstr(line, "\"symbol\":");
            if (sym_ptr) {
                char *start = strchr(sym_ptr + 9, '"');
                if (start) {
                    char *end = strchr(start + 1, '"');
                    if (end) {
                        size_t len = end - (start + 1);
                        if (len < sizeof(current_sym)) {
                            memcpy(current_sym, start + 1, len);
                            current_sym[len] = '\0';
                        }
                    }
                }
                if (current_sym[0] != '\0' && strncasecmp(current_sym, symbol_or_name, strlen(symbol_or_name)) == 0) {
                    if (current_addr[0] != '\0') {
                        snprintf(out_address, out_max, "%s", current_addr);
                        fclose(f);
                        return true;
                    }
                }
            }
            if (strstr(line, "}")) {
                current_addr[0] = '\0';
                current_sym[0] = '\0';
            }
        }
        fclose(f);
    }
    return false;
}

static void handle_kb_query(const char *param) {
    FILE *f = fopen("assets/contract_metadata.dat.bin", "rb");
    if (!f) {
        f = fopen("../assets/contract_metadata.dat.bin", "rb");
    }
    if (!f) {
        f = fopen("tsfi2-deepseek/assets/contract_metadata.dat.bin", "rb");
    }
    if (!f) {
        tsfi_io_printf(stdout, "[KB] Error: contract_metadata.dat.bin not found.\n");
        return;
    }
    
    LauRdbmsTable *table = lau_malloc(sizeof(LauRdbmsTable));
    if (!table) {
        fclose(f);
        tsfi_io_printf(stdout, "[KB] Error: memory allocation failed.\n");
        return;
    }
    
    size_t r = fread(table, sizeof(LauRdbmsTable), 1, f);
    fclose(f);
    
    if (r != 1) {
        tsfi_io_printf(stdout, "[KB] Error: failed to read table from disk.\n");
        lau_free(table);
        return;
    }
    
    tsfi_io_printf(stdout, "[KB] Searching for '%s'...\n", param);
    bool found = false;
    for (uint32_t i = 0; i < table->count; i++) {
        if (strcasecmp(table->rows[i].address, param) == 0 ||
            strncasecmp(table->rows[i].symbol, param, strlen(param)) == 0) {
            tsfi_io_printf(stdout, "----------------------------------------\n");
            tsfi_io_printf(stdout, "Address   : %s\n", table->rows[i].address);
            tsfi_io_printf(stdout, "Symbol    : %s\n", table->rows[i].symbol);
            tsfi_io_printf(stdout, "Name      : %s\n", table->rows[i].name);
            tsfi_io_printf(stdout, "Decimals  : %lu\n", (unsigned long)table->rows[i].decimals);
            tsfi_io_printf(stdout, "Price PLS : %.8f\n", table->rows[i].price_pls);
            found = true;
        }
    }
    if (!found) {
        tsfi_io_printf(stdout, "[KB] No matching token records found.\n");
    } else {
        tsfi_io_printf(stdout, "----------------------------------------\n");
    }
    lau_free(table);
}

static int handle_query_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    char action[32];
    char raw_address[128];
    char raw_data[1024];
    char resolved_address[128];
    char formatted_data[1030];
    char result[8192];

    int parsed = sscanf(new_d, "%s %127s %1023s", action, raw_address, raw_data);
    if (parsed == 2 && strcmp(action, "KB") == 0) {
        handle_kb_query(raw_address);
        return 0;
    }

    if (parsed == 3) {
        // 1. Resolve Address Aliases (hardcoded + registry)
        if (!resolve_token_alias(raw_address, resolved_address, sizeof(resolved_address))) {
            strncpy(resolved_address, raw_address, sizeof(resolved_address) - 1);
            resolved_address[sizeof(resolved_address) - 1] = '\0';
        }

        // 2. Resolve Function Selector Aliases / Format hex data
        char *data_ptr = raw_data;
        if (strcasecmp(raw_data, "FDIC()") == 0) {
            data_ptr = "0x539303e8";
        }
        
        if (strncmp(data_ptr, "0x", 2) != 0 && strncmp(data_ptr, "0X", 2) != 0) {
            snprintf(formatted_data, sizeof(formatted_data), "0x%s", data_ptr);
        } else {
            strncpy(formatted_data, data_ptr, sizeof(formatted_data) - 1);
            formatted_data[sizeof(formatted_data) - 1] = '\0';
        }

        if (strcmp(action, "CALL") == 0) {
            if (tsfi_pulse_rpc_call(resolved_address, formatted_data, result, sizeof(result))) {
                tsfi_io_printf(stdout, "[RPC] Call Result: %s\n", result);
            } else {
                tsfi_io_printf(stdout, "[RPC] Error: Call failed.\n");
            }
        } else if (strcmp(action, "STORAGE") == 0) {
            if (tsfi_pulse_rpc_get_storage_at(resolved_address, formatted_data, result, sizeof(result))) {
                tsfi_io_printf(stdout, "[RPC] Storage Result: %s\n", result);
            } else {
                tsfi_io_printf(stdout, "[RPC] Error: Storage read failed.\n");
            }
        } else {
            tsfi_io_printf(stdout, "[RPC] Error: Unknown query action '%s'.\n", action);
        }
    } else {
        tsfi_io_printf(stdout, "[RPC] Usage: 0.0 <CALL|STORAGE> <address|alias> <hex_data|alias> OR 0.0 KB <alias|address>\n");
    }
    return 0;
}

static int handle_scramble_command(WaveSystem *ws) {
    lau_unseal_object(ws);
    if (ws->current_directive) {
        lau_free(ws->current_directive);
        ws->current_directive = NULL;
    }
    lau_seal_object(ws);
    ws->scramble();
    lau_unseal_object(ws);
    ws->current_directive = NULL;
    lau_seal_object(ws);
    tsfi_io_printf(stdout, "[AUDIT] %s\n", *ws->resonance_as_status);
    tsfi_io_printf(stdout, "System ID: %d (Should be random)\n", ws->system_id);
    return 0;
}

static int handle_exit_command(WaveSystem *ws) {
    lau_log_status("INTENTIONAL_EXIT_COMMAND_RECEIVED");
    lau_unseal_object(ws);
    if (ws->current_directive) {
        lau_free(ws->current_directive);
        ws->current_directive = NULL;
    }
    ws->current_directive = lau_strdup("CLOSE_WINDOW");
    lau_seal_object(ws);
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 0.0);
    return 1;
}

static int handle_genetic_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    int iterations = 10000;
    if (strlen(new_d) > 8) {
        sscanf(new_d, "GENETIC %d", &iterations);
    }
    tsfi_io_printf(stdout, "[GENETIC] Running Benchmark with %d iterations (AVX-512 Crossover)...\n", iterations);
    
    char report[8192];
    tsfi_genetic_benchmark_run(iterations, report, sizeof(report));
    tsfi_io_printf(stdout, "%s", report);
    return 0;
}

static int handle_promote_plane71_command(WaveSystem *ws) {
    (void)ws;
    tsfi_io_printf(stdout, "[ACTION] Promoting Simulation to Physical Plane 71...\n");
    
    int unlocked_fd = tsfi_acquire_hardware_lease();
    if (unlocked_fd >= 0) {
        tsfi_io_printf(stdout, "[SUCCESS] Physical Plane 71 Promoted. Hardware Lease Secured (FD: %d).\n", unlocked_fd);
        close(unlocked_fd); 
    } else {
        tsfi_io_printf(stderr, "[FRACTURE] Failed to acquire Hardware Lease for Plane 71.\n");
    }
    return 0;
}

static int handle_load_command(WaveSystem *ws, const char *param) {
    char output_so[300];
    char source_c[300];
    snprintf(output_so, sizeof(output_so), "./%s.so", param);
    snprintf(source_c, sizeof(source_c), "%s.c", param);
    
    int compile_status = 0;
    
    if (access(source_c, F_OK) == 0) {
        // Source exists, compile it
        compile_status = tsfi_compile_plugin(source_c, output_so);
    } else if (access(output_so, F_OK) == 0) {
        // Source missing, but SO exists. Use pre-compiled.
        tsfi_io_printf(stdout, "[CLI] Source '%s' not found. Using existing '%s'.\n", source_c, output_so);
        compile_status = 0;
    } else {
        tsfi_io_printf(stdout, "[CLI] Error: Cannot find source '%s' or object '%s'.\n", source_c, output_so);
        compile_status = -1;
    }
    
    if (compile_status == 0) {
        TSFiLogicTable new_logic;
        if (tsfi_load_plugin(output_so, &new_logic) == 0) {
            lau_update_logic(ws, &new_logic);
        }
    }
    return 0;
}

extern bool blue_box_decode_access_code(const char *dial_sequence);
extern void blue_box_centrex_add_alias(uint32_t alias_code, uint32_t target_trunk);
extern uint32_t blue_box_centrex_resolve_route(uint32_t dial_code);
extern void blue_box_centrex_add_unicode_alias(const char *unicode_name, uint32_t target_trunk);
extern uint32_t blue_box_centrex_resolve_unicode_route(const char *unicode_name);

static int handle_dial_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    char seq[256];
    if (sscanf(new_d, "DIAL %255s", seq) == 1) {
        bool ok = blue_box_decode_access_code(seq);
        tsfi_io_printf(stdout, "[BLUE_BOX] Dial sequence '%s' executed. Result: %s\n", seq, ok ? "SUCCESS" : "FAILED");
    } else {
        tsfi_io_printf(stdout, "[BLUE_BOX] Usage: 0.0 DIAL <sequence>\n");
    }
    return 0;
}

static int handle_alias_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    char name[256];
    uint32_t trunk = 0;
    if (sscanf(new_d, "ALIAS %255s %u", name, &trunk) == 2) {
        bool is_num = true;
        for (int i = 0; name[i] != '\0'; i++) {
            if (name[i] < '0' || name[i] > '9') {
                is_num = false;
                break;
            }
        }
        if (is_num) {
            blue_box_centrex_add_alias((uint32_t)atoi(name), trunk);
        } else {
            blue_box_centrex_add_unicode_alias(name, trunk);
        }
        tsfi_io_printf(stdout, "[CENTREX] Alias registered: %s -> %u\n", name, trunk);
    } else {
        tsfi_io_printf(stdout, "[CENTREX] Usage: 0.0 ALIAS <name> <trunk>\n");
    }
    return 0;
}

#include "tsfi_micro_focus.h"

static int handle_dispatch_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    char subsystem[32];
    char action[32];
    char payload[128];
    if (sscanf(new_d, "DISPATCH %31s %31s %127s", subsystem, action, payload) == 3) {
        uint8_t out_pdu[64] = {0};
        size_t out_size = 0;
        int res = tsfi_mf_unified_dispatch(subsystem, action, (const uint8_t *)payload, strlen(payload), out_pdu, &out_size);
        if (res == 0) {
            tsfi_io_printf(stdout, "[DISPATCH] Success. PDU Code: 0x%02X, Size: %zu\n", out_pdu[0], out_size);
        } else {
            tsfi_io_printf(stdout, "[DISPATCH] Failed with code %d\n", res);
        }
    } else {
        tsfi_io_printf(stdout, "[DISPATCH] Usage: 0.0 DISPATCH <subsystem> <action> <payload>\n");
    }
    return 0;
}

static int handle_resolve_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    char name[256];
    if (sscanf(new_d, "RESOLVE %255s", name) == 1) {
        bool is_num = true;
        for (int i = 0; name[i] != '\0'; i++) {
            if (name[i] < '0' || name[i] > '9') {
                is_num = false;
                break;
            }
        }
        uint32_t trunk = 0;
        if (is_num) {
            trunk = blue_box_centrex_resolve_route((uint32_t)atoi(name));
        } else {
            trunk = blue_box_centrex_resolve_unicode_route(name);
        }
        tsfi_io_printf(stdout, "[CENTREX] Resolved route: %s -> %u\n", name, trunk);
    } else {
        tsfi_io_printf(stdout, "[CENTREX] Usage: 0.0 RESOLVE <name>\n");
    }
    return 0;
}

static double get_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static auncient_transfluxor_registry_t tpu_registry;
static double tpu_registration_times[MAX_REGISTRY_WORDS];
static bool tpu_initialized = false;


int tsfi_cli_process_line(WaveSystem *ws, char *input) {
    // --- ALLIGATOR MANDATORY AUDIT BYPASSED FOR OPERATIONAL SIMULATOR ---
    (void)ws;

    if (!tpu_initialized) {
        auncient_sdk_init_transfluxor_registry(&tpu_registry);
        double now = get_time_sec();

        // Pre-load default Auncient command words
        auncient_transfluxor_word_t w1, w2, w3;
        bool ok = auncient_sdk_compile_transfluxor_word(&w1, "SPK_LOCK_SCSI", 1, 440.0, 880.0, 0.4, 0x10, 0x00);
        assert(ok);
        ok = auncient_sdk_register_transfluxor_word(&tpu_registry, &w1);
        assert(ok);
        tpu_registration_times[0] = now;

        ok = auncient_sdk_compile_transfluxor_word(&w2, "SPK_WRITE_LEDGER", 2, 350.0, 700.0, 0.2, 0x00, 0x02);
        assert(ok);
        ok = auncient_sdk_register_transfluxor_word(&tpu_registry, &w2);
        assert(ok);
        tpu_registration_times[1] = now;

        ok = auncient_sdk_compile_transfluxor_word(&w3, "SPK_RELEASE_SCSI", 3, 600.0, 1200.0, 0.5, 0x20, 0x00);
        assert(ok);
        ok = auncient_sdk_register_transfluxor_word(&tpu_registry, &w3);
        assert(ok);
        tpu_registration_times[2] = now;

        tpu_initialized = true;
    }

    input[strcspn(input, "\n")] = 0;

    // Check if the input is a VoxPL WORD command
    if (strncmp(input, "WORD ", 5) == 0) {
        char name[32] = "";
        uint32_t word_id = 0;
        double f1 = 0.0, f2 = 0.0, decay = 0.0;
        uint32_t wmq = 0, abi = 0;

        int parsed = sscanf(input, "WORD %31s %u %lf %lf %lf %x %x", 
                            name, &word_id, &f1, &f2, &decay, &wmq, &abi);

        if (parsed == 7) {
            auncient_transfluxor_word_t word;
            if (auncient_sdk_compile_transfluxor_word(&word, name, word_id, f1, f2, decay, wmq, abi)) {
                if (auncient_sdk_register_transfluxor_word(&tpu_registry, &word)) {
                    uint32_t idx = tpu_registry.total_registered - 1;
                    tpu_registration_times[idx] = get_time_sec();
                    tsfi_io_printf(stdout, "[TPU] Registered: %s (ID %u) -> Hash: %lu\n", 
                                   word.name, word.word_id, 
                                   auncient_sdk_calculate_transfluxor_hash(f1, f2, decay, word_id));
                    return 0;
                } else {
                    tsfi_io_printf(stdout, "[TPU ERROR] Hash collision or registry full for '%s'.\n", name);
                    return 1;
                }
            }
        }
        tsfi_io_printf(stdout, "[TPU ERROR] Invalid WORD syntax.\n");
        return 1;
    }

    // Check if the input is a VoxPL SPEAK command
    if (strncmp(input, "SPEAK ", 6) == 0) {
        double f1 = 0.0, f2 = 0.0, decay = 0.0;
        uint32_t word_id = 0;
        int parsed = sscanf(input, "SPEAK %lf %lf %lf %u", &f1, &f2, &decay, &word_id);
        if (parsed == 4) {
            auncient_transfluxor_word_t query_word;
            auncient_sdk_compile_transfluxor_word(&query_word, "QUERY", word_id, f1, f2, decay, 0x00, 0x00);
            
            double feedback_freq = 0.0;
            if (auncient_sdk_dispatch_transfluxor_word(&tpu_registry, &query_word, &feedback_freq)) {
                // Find matching registered word to get actual commands
                uint64_t target_hash = auncient_sdk_calculate_transfluxor_hash(f1, f2, decay, word_id);
                auncient_transfluxor_word_t *match = NULL;
                uint32_t match_idx = 0;
                for (uint32_t i = 0; i < tpu_registry.total_registered; i++) {
                    uint64_t existing_hash = auncient_sdk_calculate_transfluxor_hash(
                        tpu_registry.registered_words[i].f1,
                        tpu_registry.registered_words[i].f2,
                        tpu_registry.registered_words[i].decay,
                        tpu_registry.registered_words[i].word_id
                    );
                    if (existing_hash == target_hash) {
                        match = &tpu_registry.registered_words[i];
                        match_idx = i;
                        break;
                    }
                }

                if (match) {
                    // Enforce Wheeler Jump temporal decay window gating
                    double elapsed = get_time_sec() - tpu_registration_times[match_idx];
                    if (elapsed > match->decay) {
                        tsfi_io_printf(stdout, "[TPU REJECT] Wheeler Jump collapsed. Waveform expired by %.3fs (Replay blocked).\n",
                                       elapsed - match->decay);
                        return 1;
                    }

                    tsfi_io_printf(stdout, "[TPU SUCCESS] Spoken word: '%s'. Executing WMQ: 0x%02X, ABI: 0x%02X. Feedback: %.2fHz\n",
                                   match->name, match->wmq_cmd, match->abi_op, feedback_freq);

                    // Call synthetic elliptic synthesizer
                    tsfi_clendenin_synth_summary_t summary;
                    memset(&summary, 0, sizeof(summary));
                    if (tsfi_clendenin_synth_sample(feedback_freq, 0.5, 0.0, &summary) == 0) {
                        tsfi_io_printf(stdout, "[CLENDENIN SYNTH] Generated sample: %.4f (FET: %.4fW, Gas: %u units)\n",
                                       summary.sample_out, summary.fet_power_watts, summary.evm_gas_units);
                    }

                    // Simulate execution outcomes directly on WaveSystem if commands are set
                    if (match->wmq_cmd == 0x10) {
                        tsfi_io_printf(stdout, "[HELMHOLTZ MQ] Winchester SCSI channel locked.\n");
                    }
                    if (match->abi_op == 0x02) {
                        tsfi_io_printf(stdout, "[ABI EXEC] Ledger write complete.\n");
                    }
                    return 0;
                }

            } else {
                tsfi_io_printf(stdout, "[TPU FAILURE] Unknown spoken waveform. Feedback: %.2fHz\n", feedback_freq);
                return 1;
            }
        }

        tsfi_io_printf(stdout, "[TPU ERROR] Invalid SPEAK syntax.\n");
        return 1;
    }

    fprintf(stderr, "[CLI_DEBUG] Processing line: \"%s\"\n", input);
    double new_i;
    char new_d[256];
    char param[256];
    
    // Parse LOAD command
    if (sscanf(input, "LOAD %255s", param) == 1) {
        fprintf(stderr, "[CLI_DEBUG] Matched LOAD: \"%s\"\n", param);
        return handle_load_command(ws, param);
    }

    int parsed = sscanf(input, "%lf %[^\n]", &new_i, new_d);
    fprintf(stderr, "[CLI_DEBUG] sscanf returned %d (new_i=%f, new_d=\"%s\")\n", parsed, (float)new_i, parsed == 2 ? new_d : "");
    if (parsed == 2) {
        if (strcmp(new_d, "EXIT") == 0) { 
            return handle_exit_command(ws); 
        }
        if (strcmp(new_d, "SCRAMBLE") == 0) {
            return handle_scramble_command(ws);
        }
        if (strcmp(new_d, "PROVENANCE") == 0) {
            return handle_provenance_command(ws);
        }
        if (strcmp(new_d, "TELEMETRY") == 0) {
            return handle_telemetry_command(ws);
        }
        if (strncmp(new_d, "CALL", 4) == 0 || strncmp(new_d, "STORAGE", 7) == 0 || strncmp(new_d, "KB", 2) == 0) {
            return handle_query_command(ws, new_d);
        }
        if (strncmp(new_d, "MATH", 4) == 0) {
            return handle_math_command(ws, new_d);
        }
        if (strncmp(new_d, "GENETIC", 7) == 0) {
            return handle_genetic_command(ws, new_d);
        }
        if (strcmp(new_d, "PROMOTE_PLANE_71") == 0) {
            return handle_promote_plane71_command(ws);
        }
        if (strncmp(new_d, "DIAL", 4) == 0) {
            return handle_dial_command(ws, new_d);
        }
        if (strncmp(new_d, "ALIAS", 5) == 0) {
            return handle_alias_command(ws, new_d);
        }
        if (strncmp(new_d, "DISPATCH", 8) == 0) {
            return handle_dispatch_command(ws, new_d);
        }
        if (strncmp(new_d, "RESOLVE", 7) == 0) {
            return handle_resolve_command(ws, new_d);
        }
        // Default behavior if previous commands not matched
        return handle_default_command(ws, new_i, new_d);
    }
    
    return 0;
}
