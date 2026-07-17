#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <unistd.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <math.h>
#include "tsfi_zmm_rpc.h"
#include "tsfi_trie_dispatcher.h"
#include "tsfi_tst.h"
#include "tsfi_opt_zmm.h"
#include "tsfi_genetic.h"
#include "tsfi_svdag.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include "tsfi_block_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#define MAX_DILEMMA_LOGS 128
#define MAX_STORE_KEYS 1024
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define SAMPLE_RATE 8000
#define MAX_ARRANGEMENT 64
#define MAX_STEPS 32
#define MAX_REGISTERED_OPERATORS 8



DilemmaLogEntry g_dilemma_logs[MAX_DILEMMA_LOGS];
int g_dilemma_log_count = 0;

StoreEntry g_seq_store[MAX_STORE_KEYS];
int g_seq_store_count = 0;
pthread_mutex_t g_seq_store_mutex = PTHREAD_MUTEX_INITIALIZER;
bool g_seq_store_loaded = false;


void save_seq_store(void) {
    FILE *f = fopen("tmp/mcp_sequential_store.json", "w");
    if (!f) f = fopen("../tmp/mcp_sequential_store.json", "w");
    if (f) {
        fprintf(f, "[\n");
        for (int i = 0; i < g_seq_store_count; i++) {
            fprintf(f, "  {\"key\": \"%s\", \"value\": \"%s\", \"timestamp\": %lu}%s\n",
                    g_seq_store[i].key, g_seq_store[i].value, (unsigned long)g_seq_store[i].timestamp,
                    (i == g_seq_store_count - 1) ? "" : ",");
        }
        fprintf(f, "]\n");
        fclose(f);
    }
}


void load_seq_store(void) {
    FILE *f = fopen("tmp/mcp_sequential_store.json", "r");
    if (!f) f = fopen("../tmp/mcp_sequential_store.json", "r");
    if (!f) {
        g_seq_store_loaded = true;
        return;
    }
    static char buf[1024 * 1024];
    size_t n = fread(buf, 1, sizeof(buf)-1, f);
    buf[n] = '\0';
    fclose(f);
    
    char *ptr = buf;
    g_seq_store_count = 0;
    while ((ptr = strstr(ptr, "{\"key\":"))) {
        if (g_seq_store_count >= MAX_STORE_KEYS) break;
        StoreEntry *e = &g_seq_store[g_seq_store_count];
        char val[4096] = {0};
        char key[128] = {0};
        unsigned long ts = 0;
        if (extract_json_string(ptr, "\"key\"", key, sizeof(key)) &&
            extract_json_string(ptr, "\"value\"", val, sizeof(val))) {
            char *ts_ptr = strstr(ptr, "\"timestamp\":");
            if (ts_ptr) {
                ts = strtoul(ts_ptr + 12, NULL, 10);
            }
            strcpy(e->key, key);
            strcpy(e->value, val);
            e->timestamp = ts;
            g_seq_store_count++;
        }
        ptr += 7;
    }
    g_seq_store_loaded = true;
}


// Helper to extract a string parameter from simple JSON
int extract_json_string(const char *json, const char *key, char *out, size_t max) {
    char *k = strstr(json, key);
    if (!k) return 0;
    char *start = strchr(k, ':');
    if (!start) return 0;
    start = strchr(start, '"');
    if (!start) return 0;
    start++;
    size_t i = 0;
    while (*start && i < max - 1) {
        if (*start == '"' && (i == 0 || *(start-1) != '\\')) break;
        out[i++] = *start++;
    }
    out[i] = 0;
    return 1;
}


int extract_json_int(const char *json, const char *key, int default_val) {
    char *k = strstr(json, key);
    if (!k) return default_val;
    char *start = strchr(k, ':');
    if (!start) return default_val;
    return atoi(start + 1);
}


size_t decode_hex(const char *hex, uint8_t *out, size_t max) {
    size_t len = strlen(hex);
    size_t decoded = 0;
    for (size_t i = 0; i < len && decoded < max; i += 2) {
        unsigned int val;
        sscanf(hex + i, "%02x", &val);
        out[decoded++] = (uint8_t)val;
    }
    return decoded;
}




RpcSynthMounts g_rpc_mounts = { false, false, false, false, false };
pa_simple *g_rpc_pulse_stream = NULL;

double rpc_note_to_frequency(const char *note) {
    if (strcmp(note, "C2") == 0) return 65.41;
    if (strcmp(note, "Bb1") == 0) return 58.27;
    if (strcmp(note, "Ab1") == 0) return 51.91;
    if (strcmp(note, "C3") == 0) return 130.81;
    if (strcmp(note, "Eb3") == 0) return 155.56;
    if (strcmp(note, "G3") == 0) return 196.00;
    if (strcmp(note, "F3") == 0) return 174.61;
    if (strcmp(note, "Ab3") == 0) return 207.65;
    if (strcmp(note, "Bb3") == 0) return 233.08;
    if (strcmp(note, "C4") == 0) return 261.63;
    if (strcmp(note, "Eb4") == 0) return 311.13;
    if (strcmp(note, "F4") == 0) return 349.23;
    if (strcmp(note, "G4") == 0) return 392.00;
    if (strcmp(note, "Ab4") == 0) return 415.30;
    if (strcmp(note, "Bb4") == 0) return 466.16;
    if (strcmp(note, "Ab0") == 0) return 25.96;
    if (strcmp(note, "Bb0") == 0) return 29.14;
    if (strcmp(note, "C1") == 0) return 32.70;
    return 0.0;
}



void rpc_play_polyphonic_step(double f_lead, double f_bass, double f_growl, double growl_gain, double growl_mod,
                           bool has_kick, bool has_snare, double duration) {
    // CCITT in-band tone control check:
    // If a 2600 Hz tone is received, perform line control reset (disconnect / unmount)
    if ((f_lead >= 2595.0 && f_lead <= 2605.0) ||
        (f_bass >= 2595.0 && f_bass <= 2605.0) ||
        (f_growl >= 2595.0 && f_growl <= 2605.0)) {
        printf("[CCITT SIGNAL] 2600 Hz disconnect signal detected. Collapsing session.\n");
        g_rpc_mounts.lead_mounted = false;
        g_rpc_mounts.bass_mounted = false;
        g_rpc_mounts.growl_mounted = false;
        g_rpc_mounts.drums_mounted = false;
    }

    uint32_t total_samples = (uint32_t)(SAMPLE_RATE * duration);
    uint8_t *buffer = malloc(total_samples);
    if (!buffer) return;

    double kick_phase = 0.0;

    for (uint32_t i = 0; i < total_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        double mix = 0.0;

        if (f_lead > 0.0 && g_rpc_mounts.lead_mounted) {
            double lead_saw = 0.15 * (2.0 * (t * f_lead - floor(t * f_lead)) - 1.0);
            double lead_sq = 0.08 * (sin(2.0 * M_PI * f_lead * t) > 0.0 ? 1.0 : -1.0);
            double lead_env = exp(-6.0 * t);
            mix += (lead_saw + lead_sq) * lead_env * 0.5;
        }

        if (f_bass > 0.0 && g_rpc_mounts.bass_mounted) {
            double bass_tri = 0.3 * (2.0 * fabs(2.0 * (t * f_bass - floor(t * f_bass + 0.5))) - 1.0);
            double bass_env = exp(-4.0 * t);
            mix += bass_tri * bass_env * 0.7;
        }

        if (f_growl > 0.0 && growl_gain > 0.0 && g_rpc_mounts.growl_mounted) {
            double wobble = sin(2.0 * M_PI * growl_mod * t);
            double growl_sig = 0.6 * sin(2.0 * M_PI * f_growl * t + 3.5 * wobble);
            double growl_env = exp(-1.2 * t) * (1.0 - exp(-35.0 * t)) * (1.0 + 0.4 * wobble);
            mix += growl_sig * growl_env * (growl_gain * 2.2) * 0.95;
        }

        if (has_kick && g_rpc_mounts.drums_mounted) {
            double kick_freq = 120.0 * exp(-35.0 * t) + 40.0;
            kick_phase += 2.0 * M_PI * kick_freq / SAMPLE_RATE;
            double kick_sig = 0.6 * sin(kick_phase) * exp(-8.0 * t);
            mix += kick_sig * 0.8;
        }

        if (has_snare && g_rpc_mounts.drums_mounted) {
            double rand_noise = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
            double snare_noise = rand_noise * 0.15 * exp(-12.0 * t);
            double snare_body = 0.35 * sin(2.0 * M_PI * 180.0 * t) * exp(-18.0 * t);
            mix += (snare_noise + snare_body) * 0.45 * 0.7;
        }

        double master = tanh(mix * 0.8);
        double val = 127.0 + 120.0 * master;
        if (val < 0.0) val = 0.0;
        if (val > 255.0) val = 255.0;
        buffer[i] = (uint8_t)val;
    }

    if (g_rpc_pulse_stream) {
        int error;
        pa_simple_write(g_rpc_pulse_stream, buffer, total_samples, &error);
    } else {
        usleep((useconds_t)(duration * 1000000.0));
    }
    free(buffer);
}


int g_consecutive_verification_failures = 0;
bool g_is_session_locked_out = false;
int g_telemetry_23_log_count = 0;

// Fortell-inspired advanced memory diagnostics
static __attribute__((unused)) FortellMemoryFault detect_fortell_memory_fault(void *ptr, size_t size) {
    if (!ptr) return FAULT_OPEN;
    uint8_t *bptr = (uint8_t *)ptr;
    bool is_grounded = true;
    for (size_t i = 0; i < size; i++) {
        if (bptr[i] != 0) {
            is_grounded = false;
            break;
        }
    }
    if (is_grounded) return FAULT_GROUND;
    return FAULT_NONE;
}


ZmmOperatorEntry g_operator_registry[MAX_REGISTERED_OPERATORS];
int g_operator_count = 0;


int register_zmm_operator_via_23_tree(int project, int programmer, int key_id) {
    // Check if already registered
    for (int i = 0; i < g_operator_count; i++) {
        if (g_operator_registry[i].project == project &&
            g_operator_registry[i].programmer == programmer &&
            g_operator_registry[i].key_id == key_id) {
            return g_operator_registry[i].acl_level;
        }
    }

    // Requires validation through the 2-3 tree verification logic
    int acl_level = verify_23_tree_traversal_acl(project, programmer, key_id);
    if (acl_level == -1) {
        return -1; // Locked out
    }
    if (acl_level <= 0) {
        printf("[REGISTRY] Operator registration rejected: 2-3 tree validation failed.\n");
        return 0; // Rejected
    }

    if (g_operator_count < MAX_REGISTERED_OPERATORS) {
        ZmmOperatorEntry *op = &g_operator_registry[g_operator_count++];
        op->project = project;
        op->programmer = programmer;
        op->key_id = key_id;
        op->acl_level = acl_level;
        op->is_active = true;
        printf("[REGISTRY] Unified 2-3 Tree Registration Success: PPN [%d,%d] Key %d registered at ACL %d.\n",
               project, programmer, key_id, acl_level);
        return acl_level; // Success
    }
    return 0;
}


int verify_23_tree_traversal_acl(int project, int programmer, int key_id) {
    if (g_is_session_locked_out) {
        printf("[LOCKOUT] Access blocked: Session locked out due to consecutive failures.\n");
        return -1; // Locked out code
    }

    // BigModExp-based simulated validation signature check (derived from MotzkinPrime)
    // Valid key path requires project/programmer alignment to be non-zero
    uint64_t base = 3;
    uint64_t secret = key_id;
    uint64_t prime = 953467954114363ULL; // MotzkinPrime
    
    uint64_t signature = (base + secret + project + programmer) % prime;
    bool signature_valid = (signature != 0 && key_id != 0);

    // Logging to unified telemetry
    if (g_telemetry_23_log_count < 10) {
        g_telemetry_23_log_count++;
        printf("[2-3 Telemetry Log] Traversal Step - Key: %d, PPN: [%d,%d] - Signature Valid: %s\n",
               key_id, project, programmer, signature_valid ? "TRUE" : "FALSE");
    }

    if (!signature_valid) {
        g_consecutive_verification_failures++;
        if (g_consecutive_verification_failures >= 3) {
            g_is_session_locked_out = true;
            printf("[LOCKOUT] Lockout triggered: 3 consecutive verification failures reached.\n");
        }
        return 0; // Denied
    }

    g_consecutive_verification_failures = 0; // Reset on success

    if (key_id == 11) {
        return 2; // ADMIN
    }
    int path_val = (project + programmer) % 3;
    if (path_val == 0) return 0; // ANONYMOUS / Denied
    if (path_val == 1) return 1; // USER
    return 2; // ADMIN
}


bool rpc_play_bio_arrangement(const char *file_path, const char **out_err) {
    FILE *f = fopen(file_path, "r");
    if (!f) {
        *out_err = "REVERT: FAILED_TO_OPEN_BIO_ASSET";
        return false;
    }

    char arrangement_list[MAX_ARRANGEMENT][16];
    int arrangement_count = 0;
    
    RpcPatternData patterns[4];
    memset(patterns, 0, sizeof(patterns));
    strcpy(patterns[0].pattern_name, "intro_riff");
    strcpy(patterns[1].pattern_name, "verse");
    strcpy(patterns[2].pattern_name, "pre_chorus");
    strcpy(patterns[3].pattern_name, "chorus");

    char line[256];
    bool in_arrangement = false;
    int current_pattern_idx = -1;
    int parser_state = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "\"arrangement\"")) {
            in_arrangement = true;
            continue;
        }
        if (in_arrangement) {
            if (strstr(line, "]")) {
                in_arrangement = false;
                continue;
            }
            char *p = line;
            while ((p = strchr(p, '"')) != NULL) {
                char *end_ptr = strchr(p + 1, '"');
                if (end_ptr && arrangement_count < MAX_ARRANGEMENT) {
                    size_t len = end_ptr - (p + 1);
                    strncpy(arrangement_list[arrangement_count], p + 1, len);
                    arrangement_list[arrangement_count][len] = '\0';
                    arrangement_count++;
                    p = end_ptr + 1;
                } else {
                    break;
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            char search_str[256];
            snprintf(search_str, sizeof(search_str), "\"%.15s\"", patterns[i].pattern_name);
            if (strstr(line, search_str) && strstr(line, "{")) {
                current_pattern_idx = i;
                parser_state = 0;
            }
        }

        if (current_pattern_idx != -1) {
            if (strstr(line, "\"lead\"")) { parser_state = 1; continue; }
            if (strstr(line, "\"bass\"")) { parser_state = 10; continue; }
            if (strstr(line, "\"sub_growl\"")) { parser_state = 30; continue; }
            if (strstr(line, "\"drums\"")) { parser_state = 40; continue; }
            
            RpcPatternData *pat = &patterns[current_pattern_idx];

            if (parser_state == 1 && strstr(line, "\"sequence\"")) { parser_state = 2; continue; }
            if (parser_state == 2) {
                if (strstr(line, "]")) { parser_state = 0; continue; }
                char *p = line;
                while ((p = strchr(p, '"')) != NULL) {
                    char *end_ptr = strchr(p + 1, '"');
                    if (end_ptr && pat->lead_count < MAX_STEPS) {
                        size_t len = end_ptr - (p + 1);
                        strncpy(pat->lead_notes[pat->lead_count], p + 1, len);
                        pat->lead_notes[pat->lead_count][len] = '\0';
                        pat->lead_count++;
                        p = end_ptr + 1;
                    } else { break; }
                }
            }

            if (parser_state == 10 && strstr(line, "\"sequence\"")) { parser_state = 20; continue; }
            if (parser_state == 20) {
                if (strstr(line, "]")) { parser_state = 0; continue; }
                char *p = line;
                while ((p = strchr(p, '"')) != NULL) {
                    char *end_ptr = strchr(p + 1, '"');
                    if (end_ptr && pat->bass_count < MAX_STEPS) {
                        size_t len = end_ptr - (p + 1);
                        strncpy(pat->bass_notes[pat->bass_count], p + 1, len);
                        pat->bass_notes[pat->bass_count][len] = '\0';
                        pat->bass_count++;
                        p = end_ptr + 1;
                    } else { break; }
                }
            }

            if (parser_state == 30) {
                if (strstr(line, "\"sequence\"")) { parser_state = 31; continue; }
                if (strstr(line, "\"modulation_rate\"")) { parser_state = 33; continue; }
                if (strstr(line, "\"gain\"")) { parser_state = 35; continue; }
            }
            if (parser_state == 31) {
                if (strstr(line, "]")) { parser_state = 30; continue; }
                char *p = line;
                while ((p = strchr(p, '"')) != NULL) {
                    char *end_ptr = strchr(p + 1, '"');
                    if (end_ptr && pat->growl_count < MAX_STEPS) {
                        size_t len = end_ptr - (p + 1);
                        strncpy(pat->growl_notes[pat->growl_count], p + 1, len);
                        pat->growl_notes[pat->growl_count][len] = '\0';
                        pat->growl_count++;
                        p = end_ptr + 1;
                    } else { break; }
                }
            }
            if (parser_state == 33) {
                if (strstr(line, "]")) { parser_state = 30; continue; }
                char *p = line;
                while (p && pat->mod_count < MAX_STEPS) {
                    char *val_ptr = strpbrk(p, "0123456789.");
                    if (val_ptr) {
                        pat->growl_mod[pat->mod_count++] = atof(val_ptr);
                        p = strchr(val_ptr, ',');
                        if (p) p++;
                    } else { break; }
                }
            }
            if (parser_state == 35) {
                if (strstr(line, "]")) { parser_state = 30; continue; }
                char *p = line;
                while (p && pat->gain_count < MAX_STEPS) {
                    char *val_ptr = strpbrk(p, "0123456789.");
                    if (val_ptr) {
                        pat->growl_gain[pat->gain_count++] = atof(val_ptr);
                        p = strchr(val_ptr, ',');
                        if (p) p++;
                    } else { break; }
                }
            }

            if (parser_state == 40) {
                if (strstr(line, "\"kick\"")) { parser_state = 41; continue; }
                if (strstr(line, "\"snare\"")) { parser_state = 43; continue; }
            }
            if (parser_state == 41) {
                if (strstr(line, "]")) { parser_state = 40; continue; }
                char *p = line;
                while (p && pat->kick_count < MAX_STEPS) {
                    char *val_ptr = strpbrk(p, "01");
                    if (val_ptr) {
                        pat->drum_kick[pat->kick_count++] = (uint8_t)atoi(val_ptr);
                        p = val_ptr + 1;
                    } else { break; }
                }
            }
            if (parser_state == 43) {
                if (strstr(line, "]")) { parser_state = 40; continue; }
                char *p = line;
                while (p && pat->snare_count < MAX_STEPS) {
                    char *val_ptr = strpbrk(p, "01");
                    if (val_ptr) {
                        pat->drum_snare[pat->snare_count++] = (uint8_t)atoi(val_ptr);
                        p = val_ptr + 1;
                    } else { break; }
                }
            }
        }
    }
    fclose(f);

    int total_score_steps = 0;
    for (int i = 0; i < arrangement_count; i++) {
        for (int j = 0; j < 4; j++) {
            if (strcmp(arrangement_list[i], patterns[j].pattern_name) == 0) {
                total_score_steps += patterns[j].lead_count;
            }
        }
    }

    double step_duration = 60.0 / 109.6 / 4.0;

    int played_count = 0;
    for (int i = 0; i < arrangement_count; i++) {
        for (int j = 0; j < 4; j++) {
            if (strcmp(arrangement_list[i], patterns[j].pattern_name) == 0) {
                RpcPatternData *pat = &patterns[j];
                for (int s = 0; s < pat->lead_count; s++) {
                    double f_lead = rpc_note_to_frequency(pat->lead_notes[s]);
                    double f_bass = rpc_note_to_frequency(pat->bass_notes[s]);
                    double f_growl = rpc_note_to_frequency(pat->growl_notes[s]);
                    double g_val = pat->growl_gain[s];
                    double m_val = pat->growl_mod[s];
                    bool kick = pat->drum_kick[s] > 0;
                    bool snare = pat->drum_snare[s] > 0;
                    
                    rpc_play_polyphonic_step(f_lead, f_bass, f_growl, g_val, m_val, kick, snare, step_duration);
                    played_count++;
                }
            }
        }
    }
    return true;
}


void extract_json_method(const char *json, char *method_out, size_t max_len) {
    method_out[0] = '\0';
    const char *m_key = strstr(json, "\"method\"");
    if (!m_key) return;
    const char *colon = strchr(m_key, ':');
    if (!colon) return;
    const char *start = strchr(colon, '"');
    if (!start) return;
    start++;
    const char *end = strchr(start, '"');
    if (!end) return;
    size_t len = end - start;
    if (len >= max_len) len = max_len - 1;
    strncpy(method_out, start, len);
    method_out[len] = '\0';
}


void tsfi_zmm_rpc_step_async_llm(TsfiZmmVmState *state) {
    if (!state) return;
    for (int i = 0; i < 16; i++) {
        if (state->llm_tx_status[i] == 1) { 
            if (state->manifest && (state->manifest->active_mask & (1U << 31))) {
                char prompt_extract[2048] = {0};
                char *prompt_start = strstr(state->llm_tx_results[i], "for '");
                if (prompt_start) {
                    prompt_start += 5;
                    char *prompt_end = strchr(prompt_start, '\'');
                    if (prompt_end) {
                        strncpy(prompt_extract, prompt_start, prompt_end - prompt_start);
                    }
                }
                
                memset(state->llm_tx_results[i], 0, sizeof(state->llm_tx_results[i]));
                
                char cmd[4096];
                snprintf(cmd, sizeof(cmd), "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/bin/query_local_deepseek /home/mariarahel/src/tsfi2/assets/DeepSeek-Coder-6.7B.gguf \"%s\"", prompt_extract);
                
                FILE *fp = popen(cmd, "r");
                if (fp) {
                    char chunk[1024];
                    while (fgets(chunk, sizeof(chunk), fp) != NULL) {
                        char esc_chunk[2048] = {0};
                        int ei = 0;
                        for(int ci=0; chunk[ci] != 0 && ei < 2046; ci++) {
                            if (chunk[ci] == '\n') {
                                esc_chunk[ei++] = '\\';
                                esc_chunk[ei++] = 'n';
                            } else if (chunk[ci] == '"') {
                                esc_chunk[ei++] = '\\';
                                esc_chunk[ei++] = '"';
                            } else if (chunk[ci] == '\\') {
                                esc_chunk[ei++] = '\\';
                                esc_chunk[ei++] = '\\';
                            } else {
                                esc_chunk[ei++] = chunk[ci];
                            }
                        }
                        strcat(state->llm_tx_results[i], esc_chunk);
                    }
                    pclose(fp);
                } else {
                    strcat(state->llm_tx_results[i], "[FRACTURE] Popen failed.\\n");
                }
                
                state->llm_tx_status[i] = 2; // DONE
            } else {
                state->llm_tx_status[i] = 2;
                snprintf(state->llm_tx_results[i], 4096, "ERROR: DNA Manifold Unmounted.");
            }
            break;
        }
    }
}

int tsfi_zmm_rpc_dispatch(TsfiZmmVmState *state, const char *json_in, char *output_buf, size_t out_max) {
    static tsfi_tst_node *g_rpc_tst_router = NULL;
    if (!g_rpc_tst_router) {
        g_rpc_tst_router = tsfi_tst_init_rpc_router();
        g_rpc_tst_router = tsfi_tst_insert(g_rpc_tst_router, "input.mouse_move", "30");
        g_rpc_tst_router = tsfi_tst_insert(g_rpc_tst_router, "input.mouse_button", "31");
        g_rpc_tst_router = tsfi_tst_insert(g_rpc_tst_router, "input.keyboard", "32");
        g_rpc_tst_router = tsfi_tst_insert(g_rpc_tst_router, "tariffs_query", "50");
    }

    char method_name[128];
    extract_json_method(json_in, method_name, sizeof(method_name));
    int method_type = tsfi_tst_resolve_rpc(g_rpc_tst_router, method_name);
    if (method_type == 0) return 0;

    char *min_ptr = (char*)json_in;

    int id = 1;
    char *id_ptr = strstr(min_ptr, "\"id\"");
    if (id_ptr) {
        id_ptr = strchr(id_ptr, ':');
        if (id_ptr) id = atoi(id_ptr + 1);
    }

    if (method_type == 6 || method_type == 11 || method_type == 19 || 
        (method_type >= 20 && method_type <= 27) || 
        (method_type >= 41 && method_type <= 45)) {
        extern int tsfi_zmm_rpc_dispatch_manifold(TsfiZmmVmState *state, int method_type, const char *min_ptr, char *output_buf, size_t out_max, int id);
        return tsfi_zmm_rpc_dispatch_manifold(state, method_type, min_ptr, output_buf, out_max, id);
    }
    if (method_type == 28 || method_type == 29 || method_type == 33 || 
        method_type == 34 || (method_type >= 30 && method_type <= 32) || 
        method_type == 50 || method_type == 51 || method_type == 63 || 
        method_type == 64 || (method_type >= 52 && method_type <= 59)) {
        extern int tsfi_zmm_rpc_dispatch_wave(TsfiZmmVmState *state, int method_type, const char *min_ptr, char *output_buf, size_t out_max, int id);
        return tsfi_zmm_rpc_dispatch_wave(state, method_type, min_ptr, output_buf, out_max, id);
    }
    if (method_type == 1) { // RUN
        char *code_key = strstr(min_ptr, "\"code\"");
        if (code_key) {
            char *start = strchr(code_key, ':');
            if (start) start = strchr(start, '"');
            if (start) {
                start++; 
                size_t len = strlen(start);
                char *decoded = (char*)lau_malloc(len + 1);
                char *src = start;
                char *dst = decoded;
                while (*src) {
                    if (*src == '"' && *(src-1) != '\\') break; 
                    if (*src == '\\') {
                        src++;
                        if (*src == 'n') *dst++ = '\n';
                        else if (*src == 't') *dst++ = '\t';
                        else if (*src == '"') *dst++ = '"';
                        else if (*src == '\\') *dst++ = '\\';
                        else *dst++ = *src;
                    } else {
                        *dst++ = *src;
                    }
                    src++;
                }
                *dst = 0;

                state->output_pos = 0;
                tsfi_zmm_vm_exec(state, decoded);
                
                int n = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"output\": \"");
                char *optr = output_buf + n;
                size_t rem = out_max - n;
                
                for (int i=0; i<state->output_pos && rem > 16; i++) {
                    char c = state->output_buffer[i];
                    if (c == '\n') { *optr++ = '\\'; *optr++ = 'n'; rem -= 2; }
                    else if (c == '"') { *optr++ = '\\'; *optr++ = '"'; rem -= 2; }
                    else if (c == '\\') { *optr++ = '\\'; *optr++ = '\\'; rem -= 2; }
                    else { *optr++ = c; rem--; }
                }
                snprintf(optr, rem, "\"}, \"id\": %d}\n", id);
                lau_free(decoded);
                return 1;
            }
        }
    }
    if (method_type == 2) { // RESET
        tsfi_zmm_vm_destroy(state);
        tsfi_zmm_vm_init(state);
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Reset OK\", \"id\": %d}\n", id);
        return 1;
    }
    if (method_type == 3) { // INSPECT
        float density = state->manifest ? state->manifest->synapse.mass_density : 0.0f;
        int swap = state->manifest ? state->manifest->synapse.request_kernel_swap : 0;
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"density\": %.4f, \"swap\": %d}, \"id\": %d}\n", density, swap, id);
        return 1;
    }
    if (method_type == 4) { // SCRAMBLE
        if (state->manifest && state->manifest->contiguous_rf) {
            tsfi_scramble_wave512(state->manifest->contiguous_rf, 2048);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Scrambled 2KB RF\", \"id\": %d}\n", id);
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"No Manifest\", \"id\": %d}\n", id);
        }
        return 1;
    }
    if (method_type == 5) { // ATTACH
        char shm_id[64];
        if (extract_json_string(min_ptr, "\"shm_id\"", shm_id, sizeof(shm_id))) {
            tsfi_zmm_vm_attach_telemetry(state, shm_id);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Attached to %s\", \"id\": %d}\n", shm_id, id);
            return 1;
        }
    }
    if (method_type == 35) { // QUERY_KNOWLEDGE_GRAPH
        char address_hex[128] = {0};
        if (extract_json_string(min_ptr, "\"address\"", address_hex, sizeof(address_hex))) {
            uint64_t virtual_address = 0;
            char temp[17] = {0};
            if (address_hex[0] == '0' && (address_hex[1] == 'x' || address_hex[1] == 'X')) {
                memcpy(temp, address_hex + 2, 16);
            } else {
                memcpy(temp, address_hex, 16);
            }
            virtual_address = strtoull(temp, NULL, 16);
            tsfi_qing_graph_node* graph = tsfi_block_monitor_get_graph();
            CachedContract* contract = tsfi_qing_graph_route_find(graph, TSFI_NET_ZMM, virtual_address);
            if (!contract) {
                contract = tsfi_qing_graph_route_find(graph, TSFI_NET_PULSECHAIN, virtual_address);
            }
            if (contract) {
                char symbol[128] = {0};
                uint32_t count = tsfi_block_monitor_get_discovered_tokens_count();
                for (uint32_t i = 0; i < count; i++) {
                    char t_addr[64];
                    char t_symbol[32];
                    char t_name[128];
                    tsfi_block_monitor_get_discovered_token(i, t_addr, t_symbol, t_name);
                    char t_temp[17] = {0};
                    if (t_addr[0] == '0' && (t_addr[1] == 'x' || t_addr[1] == 'X')) {
                        memcpy(t_temp, t_addr + 2, 16);
                    } else {
                        memcpy(t_temp, t_addr, 16);
                    }
                    uint64_t t_val = strtoull(t_temp, NULL, 16);
                    if (t_val == contract->virtual_address) {
                        snprintf(symbol, sizeof(symbol), "%s", t_symbol);
                        break;
                    }
                }
                if (symbol[0] == '\0') {
                    snprintf(symbol, sizeof(symbol), "%s", contract->name);
                }
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"name\": \"%s\", \"symbol\": \"%s\", \"address\": \"0x%lx\"}, \"id\": %d}\n", contract->name, symbol, contract->virtual_address, id);
            } else {
                uint32_t count = tsfi_block_monitor_get_discovered_tokens_count();
                bool found = false;
                for (uint32_t i = 0; i < count; i++) {
                    char t_addr[64];
                    char t_symbol[32];
                    char t_name[128];
                    tsfi_block_monitor_get_discovered_token(i, t_addr, t_symbol, t_name);
                    char t_temp[17] = {0};
                    if (t_addr[0] == '0' && (t_addr[1] == 'x' || t_addr[1] == 'X')) {
                        memcpy(t_temp, t_addr + 2, 16);
                    } else {
                        memcpy(t_temp, t_addr, 16);
                    }
                    uint64_t t_val = strtoull(t_temp, NULL, 16);
                    if (t_val == virtual_address) {
                        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"name\": \"%s\", \"symbol\": \"%s\", \"address\": \"%s\"}, \"id\": %d}\n", t_name, t_symbol, t_addr, id);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32602, \"message\": \"Contract or block not found in routing graph\"}, \"id\": %d}\n", id);
                }
            }
            return 1;
        }
    }
    if (method_type == 12) { // MATH.MOTZKIN
        // Generate Motzkin string buffer and grep it
        char* gen_buf = malloc(1024 * 1024);
        size_t gen_offset = 0;
        
        uint64_t m_prev2 = 1;
        uint64_t m_prev1 = 1;
        
        auto bool is_prime(uint64_t n) {
            if (n <= 1) return false;
            if (n <= 3) return true;
            if (n % 2 == 0 || n % 3 == 0) return false;
            for (uint64_t i = 5; i * i <= n; i += 6) {
                if (n % i == 0 || n % (i + 2) == 0) return false;
            }
            return true;
        }

        for (uint64_t n = 2; n < 60; n++) {
            uint64_t term1 = (2 * n + 1) * m_prev1;
            uint64_t term2 = (3 * n - 3) * m_prev2;
            uint64_t m_curr = (term1 + term2) / (n + 2);
            
            int bits = 0;
            uint64_t temp = m_curr;
            while (temp > 0) { bits++; temp >>= 1; }
            
            if (is_prime(m_curr)) {
                gen_offset += snprintf(gen_buf + gen_offset, 1024 * 1024 - gen_offset, "M_%lu = %lu (0x%lX) - %d bits [PRIME]\n", n, m_curr, m_curr, bits);
            } else {
                gen_offset += snprintf(gen_buf + gen_offset, 1024 * 1024 - gen_offset, "M_%lu = %lu (0x%lX) - %d bits\n", n, m_curr, m_curr, bits);
            }
            
            m_prev2 = m_prev1;
            m_prev1 = m_curr;
        }
        
        // Execute native tsfi_io_grep to find the 50-bit prime!
        extern size_t tsfi_io_grep(const uint8_t* buffer, size_t buffer_size, const char* pattern, size_t pattern_len, size_t* match_indices, size_t max_matches);
        size_t match_idx[100];
        size_t num_matches = tsfi_io_grep((const uint8_t*)gen_buf, gen_offset, "[PRIME]", 7, match_idx, 100);
        
        char result_str[512] = "No 50-bit prime found";
        for (size_t i = 0; i < num_matches; i++) {
            // Find start of line
            size_t line_start = match_idx[i];
            while (line_start > 0 && gen_buf[line_start-1] != '\n') line_start--;
            
            // If it contains "50 bits"
            if (strstr(gen_buf + line_start, "50 bits") && strstr(gen_buf + line_start, "[PRIME]")) {
                size_t line_end = match_idx[i];
                while (line_end < gen_offset && gen_buf[line_end] != '\n') line_end++;
                
                snprintf(result_str, sizeof(result_str), "%.*s", (int)(line_end - line_start), gen_buf + line_start);
                break;
            }
        }
        
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"%s\", \"id\": %d}\n", result_str, id);
        free(gen_buf);

    }
    if (method_type == 10) { // SHELL.READ_FILE
        char path[512];
        if (extract_json_string(min_ptr, "\"path\"", path, sizeof(path))) {
            int secret = extract_json_int(min_ptr, "\"secret\"", 0);
            int limit = extract_json_int(min_ptr, "\"limit\"", 65536); // Increased default limit
            
            if (limit > (int)(out_max / 4)) limit = out_max / 4; // Allow larger reads but keep space for JSON overhead

            FILE *f = fopen(path, "rb");
            if (!f) {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Cannot open file: %s\", \"id\": %d}\n", path, id);
                return 1;
            }

            fseek(f, 0, SEEK_END);
            long total_size = ftell(f);
            if (secret >= total_size) {
                fclose(f);
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"content\": \"\", \"total\": %ld}, \"id\": %d}\n", total_size, id);
                return 1;
            }

            fseek(f, secret, SEEK_SET);
            char *file_data = (char*)lau_malloc(limit + 1);
            size_t n = fread(file_data, 1, limit, f);
            file_data[n] = 0;
            fclose(f);

            int head = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"total\": %ld, \"content\": \"", total_size);
            char *optr = output_buf + head;
            size_t rem = out_max - head - 64; // Safety margin for JSON closing

            for (size_t i = 0; i < n && rem > 8; i++) {
                unsigned char c = (unsigned char)file_data[i];
                if (c == '\n') { *optr++ = '\\'; *optr++ = 'n'; rem -= 2; }
                else if (c == '\t') { *optr++ = '\\'; *optr++ = 't'; rem -= 2; }
                else if (c == '\r') { *optr++ = '\\'; *optr++ = 'r'; rem -= 2; }
                else if (c == '"') { *optr++ = '\\'; *optr++ = '"'; rem -= 2; }
                else if (c == '\\') { *optr++ = '\\'; *optr++ = '\\'; rem -= 2; }
                else if (c >= 32 && c <= 126) { *optr++ = c; rem--; }
                else {
                    // Robust Hex Escape for binary/non-printable data
                    int written = snprintf(optr, rem, "\\\\x%02x", c);
                    optr += written;
                    rem -= written;
                }
            }
            snprintf(optr, rem, "\"}, \"id\": %d}\n", id);
            lau_free(file_data);
            return 1;
        }
    }
    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Method not found\", \"id\": %d}\n", id);
    return 1;
}
