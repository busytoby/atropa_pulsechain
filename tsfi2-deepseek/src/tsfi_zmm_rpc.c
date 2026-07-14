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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAX_DILEMMA_LOGS 128
typedef struct {
    char event[64];
    char source[64];
    char details[256];
    uint64_t timestamp;
} DilemmaLogEntry;

static DilemmaLogEntry g_dilemma_logs[MAX_DILEMMA_LOGS];
static int g_dilemma_log_count = 0;

// Helper to extract a string parameter from simple JSON
static int extract_json_string(const char *json, const char *key, char *out, size_t max) {
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

static int extract_json_int(const char *json, const char *key, int default_val) {
    char *k = strstr(json, key);
    if (!k) return default_val;
    char *start = strchr(k, ':');
    if (!start) return default_val;
    return atoi(start + 1);
}

static size_t decode_hex(const char *hex, uint8_t *out, size_t max) {
    size_t len = strlen(hex);
    size_t decoded = 0;
    for (size_t i = 0; i < len && decoded < max; i += 2) {
        unsigned int val;
        sscanf(hex + i, "%02x", &val);
        out[decoded++] = (uint8_t)val;
    }
    return decoded;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 8000

typedef struct {
    bool lead_mounted;
    bool bass_mounted;
    bool growl_mounted;
    bool drums_mounted;
    bool audio_stream_control_mounted;
} RpcSynthMounts;

static RpcSynthMounts g_rpc_mounts = { false, false, false, false, false };
static pa_simple *g_rpc_pulse_stream = NULL;

static double rpc_note_to_frequency(const char *note) {
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

#define MAX_ARRANGEMENT 64
#define MAX_STEPS 32

typedef struct {
    char pattern_name[16];
    char lead_notes[MAX_STEPS][8];
    char bass_notes[MAX_STEPS][8];
    char growl_notes[MAX_STEPS][8];
    double growl_gain[MAX_STEPS];
    double growl_mod[MAX_STEPS];
    uint8_t drum_kick[MAX_STEPS];
    uint8_t drum_snare[MAX_STEPS];
    int lead_count;
    int bass_count;
    int growl_count;
    int gain_count;
    int mod_count;
    int kick_count;
    int snare_count;
} RpcPatternData;

static void rpc_play_polyphonic_step(double f_lead, double f_bass, double f_growl, double growl_gain, double growl_mod,
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

static int g_consecutive_verification_failures = 0;
static bool g_is_session_locked_out = false;
static int g_telemetry_23_log_count = 0;

// Fortell-inspired advanced memory diagnostics
typedef enum {
    FAULT_NONE = 0,
    FAULT_OPEN,   // Unmapped memory region
    FAULT_SHORT,  // Memory overlapping/cross-talk
    FAULT_GROUND  // Completely zeroed/grounded register state
} FortellMemoryFault;

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

#define MAX_REGISTERED_OPERATORS 8
typedef struct {
    int project;
    int programmer;
    int key_id;
    int acl_level;
    bool is_active;
} ZmmOperatorEntry;

static ZmmOperatorEntry g_operator_registry[MAX_REGISTERED_OPERATORS];
static int g_operator_count = 0;

static int verify_23_tree_traversal_acl(int project, int programmer, int key_id);

static int register_zmm_operator_via_23_tree(int project, int programmer, int key_id) {
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

static int verify_23_tree_traversal_acl(int project, int programmer, int key_id) {
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

static bool rpc_play_bio_arrangement(const char *file_path, const char **out_err) {
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

static void extract_json_method(const char *json, char *method_out, size_t max_len) {
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
    } else if (method_type == 2) { // RESET
        tsfi_zmm_vm_destroy(state);
        tsfi_zmm_vm_init(state);
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Reset OK\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 3) { // INSPECT
        float density = state->manifest ? state->manifest->synapse.mass_density : 0.0f;
        int swap = state->manifest ? state->manifest->synapse.request_kernel_swap : 0;
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"density\": %.4f, \"swap\": %d}, \"id\": %d}\n", density, swap, id);
        return 1;
    } else if (method_type == 4) { // SCRAMBLE
        if (state->manifest && state->manifest->contiguous_rf) {
            tsfi_scramble_wave512(state->manifest->contiguous_rf, 2048);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Scrambled 2KB RF\", \"id\": %d}\n", id);
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"No Manifest\", \"id\": %d}\n", id);
        }
        return 1;
    } else if (method_type == 5) { // ATTACH
        char shm_id[64];
        if (extract_json_string(min_ptr, "\"shm_id\"", shm_id, sizeof(shm_id))) {
            tsfi_zmm_vm_attach_telemetry(state, shm_id);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Attached to %s\", \"id\": %d}\n", shm_id, id);
            return 1;
        }
    } else if (method_type == 6) { // GENETIC.BENCHMARK
        int iterations = 50000;
        char *iter_key = strstr(min_ptr, "\"iterations\"");
        if (iter_key) {
            char *start = strchr(iter_key, ':');
            if (start) iterations = atoi(start + 1);
        }
        if (iterations <= 0) iterations = 50000;
        char report_buf[4096];
        tsfi_genetic_benchmark_run(iterations, report_buf, sizeof(report_buf));
        int n = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"report\": \"");
        char *optr = output_buf + n;
        size_t rem = out_max - n;
        for (char *c = report_buf; *c && rem > 16; c++) {
            if (*c == '\n') { *optr++ = '\\'; *optr++ = 'n'; rem -= 2; }
            else if (*c == '\t') { *optr++ = '\\'; *optr++ = 't'; rem -= 2; }
            else if (*c == '"') { *optr++ = '\\'; *optr++ = '"'; rem -= 2; }
            else if (*c == '\\') { *optr++ = '\\'; *optr++ = '\\'; rem -= 2; }
            else { *optr++ = *c; rem--; }
        }
        snprintf(optr, rem, "\"}, \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 11) { // GENETIC.ESTABLISH_LLM
        // This method bootstraps an internal "Model" structure (YANG) via crossover
        // of a baseline Architecture (Parent A) and a Strategic Input (Parent B).
        
        char arch[1024] = "WLOAD W0, 0.5\nWLOAD W1, 0.1\nWADD W2, W0, W1\nWSTORE W2, 0";
        char strategy[1024] = "WLOAD W0, 0.8\nWLOAD W1, 0.2\nWMUL W2, W0, W1\nWSTORE W2, 0";
        
        extract_json_string(min_ptr, "\"architecture\"", arch, sizeof(arch));
        extract_json_string(min_ptr, "\"strategy\"", strategy, sizeof(strategy));

        // Setup Parents
        GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
        ParentA->type = GENETIC_TYPE_YI; ParentA->dys_ptr = allocYI();
        memset(ParentA->dys_ptr, 0, sizeof(struct YI));
        ((struct YI*)ParentA->dys_ptr)->Xi = tsfi_bn_alloc();
        tsfi_bn_from_bytes(((struct YI*)ParentA->dys_ptr)->Xi, (const uint8_t*)arch, strlen(arch));

        GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
        ParentB->type = GENETIC_TYPE_YI; ParentB->dys_ptr = allocYI();
        memset(ParentB->dys_ptr, 0, sizeof(struct YI));
        ((struct YI*)ParentB->dys_ptr)->Xi = tsfi_bn_alloc();
        tsfi_bn_from_bytes(((struct YI*)ParentB->dys_ptr)->Xi, (const uint8_t*)strategy, strlen(strategy));

        // Perform Crossover In-Place (Zero-Copy)
        GeneticNode* InternalLLM = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
        InternalLLM->type = GENETIC_TYPE_YI; InternalLLM->dys_ptr = allocYI();
        memset(InternalLLM->dys_ptr, 0, sizeof(struct YI));
        Fourier_UniversalCrossover_InPlace(ParentA, ParentB, InternalLLM);

        // Extract resulting bytecode
        char evolved_code[1024] = {0};
        tsfi_bn_to_bytes(((struct YI*)InternalLLM->dys_ptr)->Xi, (uint8_t*)evolved_code, sizeof(evolved_code) - 1);
        
        // Return status and the evolved "Model" bytecode
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"llm_id\": \"%s\", \"bytecode\": \"%s\"}, \"id\": %d}\n", 
                 InternalLLM->generation_id, evolved_code, id);

        // Cleanup
        freeYI((struct YI*)ParentA->dys_ptr); lau_free(ParentA);
        freeYI((struct YI*)ParentB->dys_ptr); lau_free(ParentB);
        if (InternalLLM->type == GENETIC_TYPE_YI) freeYI((struct YI*)InternalLLM->dys_ptr);
        else freeYANG((struct YANG*)InternalLLM->dys_ptr);
        lau_free(InternalLLM);
        return 1;
        
    } else if (method_type == 19) { // GENETIC.AUTONOMOUS_OPTIMIZE
        // Autonomous optimization script wrapper
        char target_file[1024] = "tests/ballet_elancer.c";
        extract_json_string(min_ptr, "\"target\"", target_file, sizeof(target_file));
        
        char script_path[256];
        snprintf(script_path, sizeof(script_path), "/tmp/tsfi_auto_opt_%d.py", getpid());
        
        FILE *py = fopen(script_path, "w");
        if (py) {
            fprintf(py, 
                "import subprocess, re, sys\n"
                "target = '%s'\n"
                "def run_bench():\n"
                "    r = subprocess.run(['./bin/' + target.split('/')[-1].replace('.c', '')], capture_output=True, text=True)\n"
                "    m = re.search(r'Throughput: ([\\d\\.]+) GFLOPS', r.stdout)\n"
                "    return float(m.group(1)) if m else 0.0\n"
                "best = run_bench()\n"
                "with open(target, 'r') as f: content = f.read()\n"
                "m = re.search(r'__m512 v_c = _mm512_set1_ps\\(([\\d\\.]+)f\\);', content)\n"
                "if m:\n"
                "    cur = float(m.group(1))\n"
                "    for step in [0.0001, -0.0001]:\n"
                "        new_val = cur + step\n"
                "        new_content = re.sub(r'__m512 v_c = _mm512_set1_ps\\([\\d\\.]+f\\);', f'__m512 v_c = _mm512_set1_ps({new_val:.4f}f);', content)\n"
                "        with open(target, 'w') as f: f.write(new_content)\n"
                "        subprocess.run(['make', 'bin/' + target.split('/')[-1].replace('.c', '')], capture_output=True)\n"
                "        g = run_bench()\n"
                "        if g > best:\n"
                "            best = g\n"
                "            content = new_content\n"
                "            break\n"
                "        else:\n"
                "            with open(target, 'w') as f: f.write(content)\n"
                "            subprocess.run(['make', 'bin/' + target.split('/')[-1].replace('.c', '')], capture_output=True)\n"
                "print(best)\n", target_file);
            fclose(py);
            
            char cmd[512];
            snprintf(cmd, sizeof(cmd), "python3 %s", script_path);
            FILE *cmd_out = popen(cmd, "r");
            float best_gflops = 0.0f;
            if (cmd_out) {
                if (fscanf(cmd_out, "%f", &best_gflops) != 1) {
                    best_gflops = 0.0f;
                }
                pclose(cmd_out);
            }
            unlink(script_path);
            
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"status\": \"optimized\", \"target\": \"%s\", \"best_gflops\": %.2f}, \"id\": %d}\n", target_file, best_gflops, id);
        } else {
             snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32000, \"message\": \"Failed to create optimization script\"}, \"id\": %d}\n", id);
        }
        return 1;
    
    } else if (method_type == 12) { // MATH.MOTZKIN
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

    } else if (method_type == 10) { // SHELL.READ_FILE
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
    } else if (method_type == 20) { // manifold.swap_asset
        int slot = extract_json_int(min_ptr, "\"slot\"", -1);
        int type = extract_json_int(min_ptr, "\"type\"", 0);
        char addr_str[64];
        if (slot >= 0 && slot < 32 && extract_json_string(min_ptr, "\"address\"", addr_str, sizeof(addr_str))) {
            uintptr_t addr = (uintptr_t)strtoull(addr_str, NULL, 0);
            if (state->manifest) {
                state->manifest->slots[slot].type = (ZmmType)type;
                state->manifest->slots[slot].data_ptr = (void*)addr;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Swapped slot %d to %p\", \"id\": %d}\n", slot, (void*)addr, id);
                return 1;
            }
        }
    } else if (method_type == 21) { // manifold.set_kernel
        char name[64];
        if (extract_json_string(min_ptr, "\"name\"", name, sizeof(name)) && state->manifest) {
            void (*k)(void*, ZmmSynapse*) = NULL;
            if (strcmp(name, "blue_teddy_bear") == 0) k = tsfi_kernel_blue_teddy_bear;
            else if (strcmp(name, "low_density") == 0) k = state->manifest->kernel_low_density;
            else if (strcmp(name, "high_density") == 0) k = state->manifest->kernel_high_density;
            
            if (k) {
                state->manifest->micro_kernel = k;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Kernel set to %s\", \"id\": %d}\n", name, id);
                return 1;
            }
        }
    } else if (method_type == 22) { // manifold.set_active_mask
        int mask = extract_json_int(min_ptr, "\"mask\"", 0);
        if (state->manifest) {
            state->manifest->active_mask = (uint32_t)mask;
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Active mask set to 0x%08X\", \"id\": %d}\n", state->manifest->active_mask, id);
            return 1;
        }
    } else if (method_type == 23) { // manifold.set_secret
        int slot = extract_json_int(min_ptr, "\"slot\"", 0);
        char val_str[64];
        if (extract_json_string(min_ptr, "\"value\"", val_str, sizeof(val_str)) && state->manifest) {
            float val = strtof(val_str, NULL);
            if (slot >= 0 && slot < 32 && state->manifest->slots[slot].data_ptr) {
                // If it's a TeddyBear, we can inject into spectral_shift
                if (state->manifest->slots[slot].type == ZMM_TYPE_MASS) {
                    TeddyBear *b = (TeddyBear*)state->manifest->slots[slot].data_ptr;
                    b->spectral_shift = val;
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Secret injected into slot %d: %f\", \"id\": %d}\n", slot, val, id);
                    return 1;
                }
            }
        }
    } else if (method_type == 24) { // manifold.dispatch
        if (state->manifest) {
            tsfi_dispatch_zmm_dynamic(state->manifest);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"density\": %f}, \"id\": %d}\n", state->manifest->synapse.mass_density, id);
            return 1;
        }
    } else if (method_type == 25) { // manifold.inspect_slots
        if (state->manifest) {
            int head = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"slots\": [");
            char *optr = output_buf + head;
            size_t rem = out_max - head - 64;
            for (int i = 0; i < 32; i++) {
                int n = snprintf(optr, rem, "{\"slot\": %d, \"type\": %d, \"address\": \"%p\"}%s", 
                                 i, state->manifest->slots[i].type, state->manifest->slots[i].data_ptr, (i < 31 ? ", " : ""));
                optr += n; rem -= n;
                if (rem < 128) break;
            }
            snprintf(optr, rem, "]}, \"id\": %d}\n", id);
            return 1;
        }
    } else if (method_type == 26) { // manifold.upload_asset
        char hex[1024 * 64]; // Max 64KB upload for now
        if (extract_json_string(min_ptr, "\"data\"", hex, sizeof(hex))) {
            size_t bytes_needed = strlen(hex) / 2;
            void *ptr = lau_malloc_wired(bytes_needed);
            if (ptr) {
                decode_hex(hex, (uint8_t*)ptr, bytes_needed);
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"address\": \"%p\", \"size\": %zu}, \"id\": %d}\n", ptr, bytes_needed, id);
                return 1;
            }
        }
    } else if (method_type == 41) { // manifold.load_dna_llm
        char path[1024];
        if (extract_json_string(min_ptr, "\"path\"", path, sizeof(path))) {
            // Check if file exists
            if (access(path, F_OK) == 0) {
                if (state->manifest) {
                    state->manifest->slots[31].type = ZMM_TYPE_MASS;
                    state->manifest->slots[31].data_ptr = (void*)0x55555555; // Placeholder
                    state->manifest->active_mask |= (1U << 31);
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"status\": \"DNA Model Mounted to ZMM31\", \"address\": \"0x55555555\", \"size\": 1024}, \"id\": %d}\n", id);
                } else {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"No ZMM active manifest\", \"id\": %d}\n", id);
                }
                return 1;
            }
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"DNA File not found\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 42) { // manifold.query_llm
        char prompt[2048];
        if (extract_json_string(min_ptr, "\"prompt\"", prompt, sizeof(prompt))) {
            if (state->manifest && (state->manifest->active_mask & (1U << 31))) {
                uint64_t tx_id = ++state->llm_tx_counter;
                int slot = tx_id % 16;
                state->llm_tx_status[slot] = 1;
                snprintf(state->llm_tx_results[slot], 4096, "for '%s'", prompt); // Temp store prompt
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"receipt\": %lu, \"status\": \"pending\"}, \"id\": %d}\n", tx_id, id);
                return 1;
            }
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"DNA model not mounted or invalid prompt\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 43) { // manifold.get_receipt
        char id_str[64];
        extract_json_string(min_ptr, "\"receipt\"", id_str, sizeof(id_str));
        char *r_ptr = strstr(min_ptr, "\"receipt\"");
        if (r_ptr) {
            r_ptr += 9;
            while (*r_ptr == ' ' || *r_ptr == ':') r_ptr++;
            uint64_t tx_id = strtoull(r_ptr, NULL, 10);
            if (tx_id > 0 && tx_id <= state->llm_tx_counter) {
                int slot = tx_id % 16;
                if (state->llm_tx_status[slot] == 2) {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"receipt\": %lu, \"status\": \"done\", \"response\": \"%s\"}, \"id\": %d}\n", tx_id, state->llm_tx_results[slot], id);
                    state->llm_tx_status[slot] = 0; 
                    return 1;
                } else if (state->llm_tx_status[slot] == 1) {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"receipt\": %lu, \"status\": \"pending\"}, \"id\": %d}\n", tx_id, id);
                    return 1;
                }
            }
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Receipt not found\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 44) { // manifold.mount_instrument
        char target[64] = {0};
        if (extract_json_string(min_ptr, "\"target\"", target, sizeof(target))) {
            if (strcmp(target, "lead") == 0) {
                g_rpc_mounts.lead_mounted = true;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted lead\", \"id\": %d}\n", id);
            } else if (strcmp(target, "bass") == 0) {
                g_rpc_mounts.bass_mounted = true;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted bass\", \"id\": %d}\n", id);
            } else if (strcmp(target, "growl") == 0) {
                g_rpc_mounts.growl_mounted = true;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted growl\", \"id\": %d}\n", id);
            } else if (strcmp(target, "drums") == 0) {
                g_rpc_mounts.drums_mounted = true;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted drums\", \"id\": %d}\n", id);
            } else if (strcmp(target, "audio_stream_control") == 0) {
                g_rpc_mounts.audio_stream_control_mounted = true;
                printf("[PDP-11 MOUNT] Device audio_stream_control attached to channel input stream.\n");
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted audio_stream_control\", \"id\": %d}\n", id);
            } else {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Unknown instrument target\", \"id\": %d}\n", id);
            }
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Missing target parameter\", \"id\": %d}\n", id);
        }
        return 1;
    } else if (method_type == 45) { // manifold.play_bio
        char path[256] = {0};
        int project = extract_json_int(min_ptr, "\"project\"", 0);
        int programmer = extract_json_int(min_ptr, "\"programmer\"", 0);
        int key_id = extract_json_int(min_ptr, "\"key_id\"", 0);

        int acl_level = register_zmm_operator_via_23_tree(project, programmer, key_id);
        if (acl_level == -1) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"REVERT: PEER_LOCKED_OUT_DUE_TO_CONSECUTIVE_FAILURES\", \"id\": %d}\n", id);
            return 1;
        }
        if (acl_level < 1) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"REVERT: ACL_PERMISSION_DENIED\", \"id\": %d}\n", id);
            return 1;
        }

        if (!g_rpc_mounts.lead_mounted && !g_rpc_mounts.bass_mounted && !g_rpc_mounts.growl_mounted && !g_rpc_mounts.drums_mounted) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"REVERT: NO_INSTRUMENTS_MOUNTED_ON_SYNTHESIZER\", \"id\": %d}\n", id);
            return 1;
        }

        if (extract_json_string(min_ptr, "\"path\"", path, sizeof(path))) {
            if (!g_rpc_pulse_stream) {
                pa_sample_spec ss;
                ss.format = PA_SAMPLE_U8;
                ss.rate = SAMPLE_RATE;
                ss.channels = 1;
                int error;
                g_rpc_pulse_stream = pa_simple_new(NULL, "ZMM_MCP_PlayBio", PA_STREAM_PLAYBACK, NULL, "Synthesizer", &ss, NULL, NULL, &error);
                if (!g_rpc_pulse_stream) {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Failed to connect to PulseAudio\", \"id\": %d}\n", id);
                    return 1;
                }
            }

            const char *err = NULL;
            if (rpc_play_bio_arrangement(path, &err)) {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"status\": \"success\"}, \"id\": %d}\n", id);
            } else {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"%s\", \"id\": %d}\n", err, id);
            }

            if (g_rpc_pulse_stream) {
                int error;
                pa_simple_drain(g_rpc_pulse_stream, &error);
                pa_simple_free(g_rpc_pulse_stream);
                g_rpc_pulse_stream = NULL;
            }
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Missing path parameter\", \"id\": %d}\n", id);
        }
        return 1;
    } else if (method_type == 27) { // flow.trigger_choreography
        // Launch the Google Labs Flow unified masterpiece matrix in the background
        int sys_ret = system("nohup python3 tools/tsfi_ipomoea_teddy_matrix.py > /tmp/tsfi_choreography.log 2>&1 &");
        if (sys_ret == 0) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Atropa/999 Sovereign Choreography Matrix Initiated (Background).\", \"id\": %d}\n", id);
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Failed to launch choreography matrix.\", \"id\": %d}\n", id);
        }
        return 1;
    } else if (method_type == 28) { // wave512.dilemma_log
        char event_str[64];
        if (extract_json_string(min_ptr, "\"event\"", event_str, sizeof(event_str))) {
            char src_str[64] = "External";
            char det_str[256] = "";
            extract_json_string(min_ptr, "\"source\"", src_str, sizeof(src_str));
            extract_json_string(min_ptr, "\"details\"", det_str, sizeof(det_str));
            
            if (g_dilemma_log_count < MAX_DILEMMA_LOGS) {
                snprintf(g_dilemma_logs[g_dilemma_log_count].event, 64, "%s", event_str);
                snprintf(g_dilemma_logs[g_dilemma_log_count].source, 64, "%s", src_str);
                snprintf(g_dilemma_logs[g_dilemma_log_count].details, 256, "%s", det_str);
                g_dilemma_logs[g_dilemma_log_count].timestamp = (uint64_t)time(NULL);
                g_dilemma_log_count++;
            } else {
                for (int i = 1; i < MAX_DILEMMA_LOGS; i++) {
                    g_dilemma_logs[i - 1] = g_dilemma_logs[i];
                }
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].event, 64, "%s", event_str);
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].source, 64, "%s", src_str);
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].details, 256, "%s", det_str);
                g_dilemma_logs[MAX_DILEMMA_LOGS - 1].timestamp = (uint64_t)time(NULL);
            }
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Log registered\", \"id\": %d}\n", id);
            return 1;
        } else {
            int head = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": [");
            char *optr = output_buf + head;
            size_t rem = out_max - head - 64;
            for (int i = 0; i < g_dilemma_log_count; i++) {
                int n = snprintf(optr, rem, "{\"event\": \"%s\", \"source\": \"%s\", \"details\": \"%s\", \"timestamp\": %lu}%s", 
                                 g_dilemma_logs[i].event, g_dilemma_logs[i].source, g_dilemma_logs[i].details,
                                 (unsigned long)g_dilemma_logs[i].timestamp, (i < g_dilemma_log_count - 1 ? ", " : ""));
                optr += n; rem -= n;
                if (rem < 128) break;
            }
            snprintf(optr, rem, "], \"id\": %d}\n", id);
            return 1;
        }
    } else if (method_type == 29) { // wave512.telemetry
        extern uint64_t g_thunk_cache_hits;
        extern uint64_t g_thunk_cache_lookups;
        extern uint64_t lau_yul_thunk_sload(uint64_t key);
        extern int tsfi_ouroboros_serialize_pq(char *buf, size_t max_len);
        extern uint64_t tsfi_ouroboros_get_adaptive_tick_rate(void);
        
        typedef struct {
            uint64_t pc;
            uint8_t op;
        } YulTraceStepLocal;
        
        extern YulTraceStepLocal s_yul_trace_history[];
        extern uint32_t s_yul_trace_count;
        extern uint32_t s_yul_trace_head;
        extern pthread_mutex_t s_yul_trace_mutex;
        
        uint64_t head = lau_yul_thunk_sload(0xF300);
        uint64_t tail = lau_yul_thunk_sload(0xF301);
        uint64_t size = lau_yul_thunk_sload(0xF302);
        uint64_t lock = lau_yul_thunk_sload(0xF303);
        
        char pq_buf[1536];
        tsfi_ouroboros_serialize_pq(pq_buf, sizeof(pq_buf));
        
        char trace_buf[1536] = "[";
        char *t_ptr = trace_buf + 1;
        size_t t_rem = sizeof(trace_buf) - 2;
        
        pthread_mutex_lock(&s_yul_trace_mutex);
        uint32_t t_count = s_yul_trace_count;
        uint32_t t_head = s_yul_trace_head;
        pthread_mutex_unlock(&s_yul_trace_mutex);
        
        for (uint32_t i = 0; i < t_count; i++) {
            uint32_t idx = (t_head + 32 - t_count + i) % 32;
            int n = snprintf(t_ptr, t_rem, "{\"pc\": %lu, \"op\": %u}%s",
                             (unsigned long)s_yul_trace_history[idx].pc, s_yul_trace_history[idx].op,
                             (i < t_count - 1 ? ", " : ""));
            t_ptr += n;
            t_rem -= n;
            if (t_rem < 64) break;
        }
        snprintf(t_ptr, t_rem, "]");
        
        uint64_t adaptive_tick = tsfi_ouroboros_get_adaptive_tick_rate();
        
        snprintf(output_buf, out_max, 
                 "{\"jsonrpc\": \"2.0\", \"result\": {\"cache_hits\": %lu, \"cache_lookups\": %lu, "
                 "\"evm_queue\": {\"head\": %lu, \"tail\": %lu, \"size\": %lu, \"lock\": %lu}, "
                 "\"host_heap\": %s, \"yul_trace\": %s, \"adaptive_tick_rate\": %lu}, \"id\": %d}\n", 
                 (unsigned long)g_thunk_cache_hits, (unsigned long)g_thunk_cache_lookups,
                 (unsigned long)head, (unsigned long)tail, (unsigned long)size, (unsigned long)lock,
                 pq_buf, trace_buf, (unsigned long)adaptive_tick, id);
        return 1;
    } else if (method_type == 33) { // wave512.inject_event
        uint32_t priority = (uint32_t)extract_json_int(min_ptr, "\"priority\"", 10);
        uint32_t type = (uint32_t)extract_json_int(min_ptr, "\"type\"", 1);
        uint64_t timestamp = (uint64_t)extract_json_int(min_ptr, "\"timestamp\"", 0);
        
        char hex_data[128] = "";
        uint8_t data[32] = {0};
        if (extract_json_string(min_ptr, "\"data\"", hex_data, sizeof(hex_data))) {
            char *p = hex_data;
            if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
            for (int i = 0; i < 32 && *p && *(p+1); i++) {
                unsigned int byte;
                sscanf(p, "%2x", &byte);
                data[i] = (uint8_t)byte;
                p += 2;
            }
        }
        
        extern bool tsfi_ouroboros_push_event(uint32_t priority, uint32_t type, uint64_t timestamp, const uint8_t *data);
        bool push_ok = tsfi_ouroboros_push_event(priority, type, timestamp, data);
        
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"success\": %s}, \"id\": %d}\n", 
                 push_ok ? "true" : "false", id);
        return 1;
    } else if (method_type == 30) { // input.mouse_move
        int x = extract_json_int(min_ptr, "\"x\"", 0);
        int y = extract_json_int(min_ptr, "\"y\"", 0);
        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF004) {
            state->reu_ram[0xF000] = (uint8_t)(x & 0xFF);
            state->reu_ram[0xF001] = (uint8_t)(y & 0xFF);
            state->reu_ram[0xF003] = (uint8_t)((x >> 8) & 0xFF);
            state->reu_ram[0xF004] = (uint8_t)((y >> 8) & 0xFF);
        }
        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t command_byte = (0x00 << 6) | 0x01;
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "MM %d %d", x, y);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mouse move OK\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 31) { // input.mouse_button
        int button = extract_json_int(min_ptr, "\"button\"", 272);
        int btn_state = extract_json_int(min_ptr, "\"state\"", 0);
        int x = extract_json_int(min_ptr, "\"x\"", -1);
        int y = extract_json_int(min_ptr, "\"y\"", -1);
        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF004 && x >= 0 && y >= 0) {
            state->reu_ram[0xF000] = (uint8_t)(x & 0xFF);
            state->reu_ram[0xF001] = (uint8_t)(y & 0xFF);
            state->reu_ram[0xF003] = (uint8_t)((x >> 8) & 0xFF);
            state->reu_ram[0xF004] = (uint8_t)((y >> 8) & 0xFF);
        }
        uint8_t btn_idx = 0;
        if (button == 273) btn_idx = 1;
        if (button == 274) btn_idx = 2;
        uint8_t command_byte = (0x01 << 6) | (btn_state ? 0x04 : 0x00) | (btn_idx & 0x03);

        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "%s %d", btn_state ? "MD" : "MU", button);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mouse button OK\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 32) { // input.keyboard
        int keycode = extract_json_int(min_ptr, "\"keycode\"", 0);
        int key_state = extract_json_int(min_ptr, "\"state\"", 0);
        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF002) {
            state->reu_ram[0xF002] = (uint8_t)keycode;
        }
        uint8_t command_byte = (0x02 << 6) | (key_state ? 0x20 : 0x00) | (keycode & 0x1F);

        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "%s %d", key_state ? "KD" : "KU", keycode);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Keyboard event OK\", \"id\": %d}\n", id);
    } else if (method_type == 50) { // tariffs_query
        int trunk_id = extract_json_int(min_ptr, "\"trunk_id\"", 800);
        // Calculate simulated tariff rate: base rate 60 + trunk offset * 5
        uint32_t rate = 60;
        if (trunk_id >= 800 && trunk_id <= 831) {
            rate = 60 + (trunk_id - 800) * 5;
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"rate\": %u}, \"id\": %d}\n", rate, id);
        return 1;
    }

    
    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Method not found\", \"id\": %d}\n", id);
    return 1;
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
