#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/file.h>
#include <openssl/sha.h>
#include "tsfi_zmm_rpc.h"
#include "lau_yul_thunk.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "tsfi_computel_blue_box.h"
    #define GRID_SIZE 80
#define MAX_BGP_PEERS 32
#define MAX_GGUF_ROUTES 64
#define MAX_VALIDATOR_BIDS 16
#include <dirent.h>
#include <sys/stat.h>
#include "tsfi_mainframe_decnet.h"


static const float mf_freqs_f1[12] = {700.0f, 700.0f, 900.0f, 700.0f, 900.0f, 1100.0f, 700.0f, 900.0f, 1100.0f, 1300.0f, 1100.0f, 1500.0f};
static const float mf_freqs_f2[12] = {900.0f, 1100.0f, 1100.0f, 1300.0f, 1300.0f, 1300.0f, 1500.0f, 1500.0f, 1500.0f, 1500.0f, 1700.0f, 1700.0f};
static const char mf_char_map[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'K', 'S'};
static double sf_phase = 0.0;
static double mf_phase_f1 = 0.0;
static double mf_phase_f2 = 0.0;
static double red_box_phase_f1 = 0.0;
static double red_box_phase_f2 = 0.0;
void generate_sf_seizure(float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return;
    for (int i = 0; i < num_samples; i++) {
        buffer[i] = (float)sin(sf_phase);
        sf_phase += 2.0 * M_PI * 2600.0 / SAMPLE_RATE;
        if (sf_phase > 2.0 * M_PI) sf_phase -= 2.0 * M_PI;
    }
}

bool generate_mf_tone(char digit, float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return false;

    int idx = -1;
    for (int i = 0; i < 12; i++) {
        if (mf_char_map[i] == digit) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return false;

    float f1 = mf_freqs_f1[idx];
    float f2 = mf_freqs_f2[idx];

    for (int i = 0; i < num_samples; i++) {
        buffer[i] = (float)((sin(mf_phase_f1) + sin(mf_phase_f2)) * 0.5);
        mf_phase_f1 += 2.0 * M_PI * f1 / SAMPLE_RATE;
        mf_phase_f2 += 2.0 * M_PI * f2 / SAMPLE_RATE;
        if (mf_phase_f1 > 2.0 * M_PI) mf_phase_f1 -= 2.0 * M_PI;
        if (mf_phase_f2 > 2.0 * M_PI) mf_phase_f2 -= 2.0 * M_PI;
    }

    return true;
}

int generate_red_box_coin_tone(int denomination, float *buffer, int max_samples) {
    if (!buffer || max_samples <= 0) return 0;

    int beeps = 0;
    if (denomination == 5) beeps = 1;
    else if (denomination == 10) beeps = 2;
    else if (denomination == 25) beeps = 5;
    else return 0;

    int beep_samples = (int)(0.066f * SAMPLE_RATE); // 66 ms beep
    int pause_samples = (int)(0.066f * SAMPLE_RATE); // 66 ms pause
    int total_samples = beeps * (beep_samples + pause_samples);

    if (total_samples > max_samples) return 0;

    int offset = 0;
    for (int b = 0; b < beeps; b++) {
        // Generate Beep
        for (int i = 0; i < beep_samples; i++) {
            float decay = (float)exp(-3.0 * (double)i / beep_samples);
            buffer[offset + i] = (float)((sin(red_box_phase_f1) + sin(red_box_phase_f2)) * 0.5 * decay);
            
            // Emulate transient gong chirp in first 40 samples
            if (i < 40) {
                buffer[offset + i] += (float)(((double)rand() / RAND_MAX) * 0.15);
            }

            red_box_phase_f1 += 2.0 * M_PI * 1700.0 / SAMPLE_RATE;
            red_box_phase_f2 += 2.0 * M_PI * 2200.0 / SAMPLE_RATE;
            if (red_box_phase_f1 > 2.0 * M_PI) red_box_phase_f1 -= 2.0 * M_PI;
            if (red_box_phase_f2 > 2.0 * M_PI) red_box_phase_f2 -= 2.0 * M_PI;
        }
        offset += beep_samples;

        // Generate Pause
        for (int i = 0; i < pause_samples; i++) {
            buffer[offset + i] = 0.0f;
        }
        offset += pause_samples;
    }

    return offset;
}

static bool goertzel_detect(const float *samples, int num_samples, float target_freq) {
    float s_prev = 0.0f;
    float s_prev2 = 0.0f;
    float normalized_freq = target_freq / SAMPLE_RATE;
    float coeff = 2.0f * (float)cos(2.0 * M_PI * normalized_freq);
    for (int i = 0; i < num_samples; i++) {
        float s = samples[i] + coeff * s_prev - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }
    float power = s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2;
    return (power > (float)num_samples * 0.1f);
}

bool blue_box_validate_slug(const float *samples, int num_samples) {
    if (!samples || num_samples < 200) return false;
    
    // 1. Transient check (Variance in the first 40 samples to detect impact chirp)
    float sum = 0.0f;
    for (int i = 0; i < 40; i++) sum += samples[i];
    float mean = sum / 40.0f;
    float variance = 0.0f;
    for (int i = 0; i < 40; i++) {
        float diff = samples[i] - mean;
        variance += diff * diff;
    }
    variance /= 40.0f;
    if (variance < 0.001f) return false; // Fail if flat/electronic sine wave

    // 2. Exponential Decay check (amplitude comparison)
    float amp_start = 0.0f;
    float amp_end = 0.0f;
    int half = num_samples / 2;
    for (int i = 0; i < half; i++) {
        amp_start += (float)fabs(samples[i]);
        amp_end += (float)fabs(samples[i + half]);
    }
    if (amp_start == 0.0f) return false;
    float ratio = amp_end / amp_start;
    
    // Metallic damping ratio must be between 0.1 and 0.85
    if (ratio < 0.1f || ratio > 0.85f) {
        return false;
    }
    return true;
}

bool blue_box_detect_and_accumulate(const float *samples, int num_samples) {
    if (!samples || num_samples <= 0) return false;
    
    // Skip SF 2600Hz lines from slug validation; only validate coin tones
    if (goertzel_detect(samples, num_samples, 2600.0f)) {
        blue_box_accumulate_state(2600);
        return true;
    }
    
    if (num_samples >= 200) {
        if (!blue_box_validate_slug(samples, num_samples)) {
            // Flag fraud by zeroing active trunk mask
            current_block_state.active_trunk_mask = 0;
            return false;
        }
    }

    float targets[6] = {700.0f, 900.0f, 1100.0f, 1300.0f, 1500.0f, 1700.0f};
    uint64_t found_mask = 0;
    for (int i = 0; i < 6; i++) {
        if (goertzel_detect(samples, num_samples, targets[i])) {
            found_mask |= (1ULL << i);
            blue_box_accumulate_state((uint64_t)targets[i]);
        }
    }
    return found_mask != 0;
}

bool blue_box_generate_tone(uint32_t freq1, uint32_t freq2, float *samples_out, size_t count) {
    if (!samples_out || count == 0) return false;
    
    // Write frequencies to WinchesterMQ VM registers via generateTone thunk selector
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
    uint8_t tone_cd[68] = {0x0f, 0xf1, 0x10, 0x00};
    // Pack freq1 and freq2 as big-endian 32-byte arguments
    tone_cd[35] = (uint8_t)(freq1 & 0xFF);
    tone_cd[34] = (uint8_t)((freq1 >> 8) & 0xFF);
    tone_cd[33] = (uint8_t)((freq1 >> 16) & 0xFF);
    tone_cd[32] = (uint8_t)((freq1 >> 24) & 0xFF);
    tone_cd[67] = (uint8_t)(freq2 & 0xFF);
    tone_cd[66] = (uint8_t)((freq2 >> 8) & 0xFF);
    tone_cd[65] = (uint8_t)((freq2 >> 16) & 0xFF);
    tone_cd[64] = (uint8_t)((freq2 >> 24) & 0xFF);
    
    uint8_t tone_ret[32];
    size_t tone_ret_len = 32;
    lau_yul_thunk_execute("WinchesterMQ", tone_cd, 68, tone_ret, &tone_ret_len);
    
    // Synthesize dual sine wave audio samples
    double fs = 8000.0; // Standard telephony sampling rate (8 kHz)
    for (size_t n = 0; n < count; n++) {
        double t = (double)n / fs;
        samples_out[n] = 0.5 * (sin(2.0 * M_PI * freq1 * t) + sin(2.0 * M_PI * freq2 * t));
    }
    return true;
}

bool blue_box_evaluate_visual_coverage(const float *x_coords, const float *y_coords, size_t count, float *coverage_out, float *symmetry_out) {
    if (!x_coords || !y_coords || count == 0 || !coverage_out || !symmetry_out) return false;
    
    #define GRID_SIZE 80
    uint8_t grid[GRID_SIZE][GRID_SIZE];
    memset(grid, 0, sizeof(grid));
    
    size_t active_pixels = 0;
    for (size_t i = 0; i < count; i++) {
        // Normalize coordinates from [-1.0, 1.0] to [0, GRID_SIZE-1]
        int px = (int)((x_coords[i] + 1.0f) * 0.5f * (GRID_SIZE - 1));
        int py = (int)((y_coords[i] + 1.0f) * 0.5f * (GRID_SIZE - 1));
        if (px >= 0 && px < GRID_SIZE && py >= 0 && py < GRID_SIZE) {
            if (grid[py][px] == 0) {
                grid[py][px] = 1;
                active_pixels++;
            }
        }
    }
    
    *coverage_out = (float)active_pixels / (GRID_SIZE * GRID_SIZE);
    
    // Evaluate symmetry by reflecting along the vertical Y-axis reflection plane
    size_t sym_hits = 0;
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE / 2; x++) {
            if (grid[y][x] == grid[y][GRID_SIZE - 1 - x]) {
                sym_hits++;
            }
        }
    }
    *symmetry_out = (float)sym_hits / (GRID_SIZE * (GRID_SIZE / 2));
    return true;
}

static uint32_t g_mf_state = 0; // 0 = idle, 1 = dialing, 2 = routed
static char g_mf_buffer[32] = {0};
bool blue_box_dial_mf_digit(char digit) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    if (digit == 'K') { // Keypulse (KP) initiates dialing
        g_mf_state = 1;
        memset(g_mf_buffer, 0, sizeof(g_mf_buffer));
        lau_yul_thunk_sstore(0xF130, 1);
        lau_yul_thunk_sstore(0xF131, 0);
        printf("[MF STATE] KP tone received. Resetting digit dialing buffer.\n");
        return true;
    }
    if (g_mf_state == 1) {
        if (digit >= '0' && digit <= '9') {
            size_t len = strlen(g_mf_buffer);
            if (len < sizeof(g_mf_buffer) - 1) {
                g_mf_buffer[len] = digit;
                // Accumulate hash in VM storage
                uint64_t hash = 0;
                for (size_t i = 0; g_mf_buffer[i]; i++) {
                    hash = hash * 10 + (g_mf_buffer[i] - '0');
                }
                lau_yul_thunk_sstore(0xF131, hash);
                printf("[MF STATE] Digit '%c' appended. Buffer: %s (VM Hash: %lu)\n", digit, g_mf_buffer, hash);
            }
            return true;
        }
        if (digit == 'S') { // Start (ST) terminates and routes
            g_mf_state = 2;
            lau_yul_thunk_sstore(0xF130, 2);
            printf("[MF STATE] ST tone received. Dialing complete. Routing trunk call to: %s\n", g_mf_buffer);
            return true;
        }
    }
    return false;
}

bool blue_box_synthesize_vowel(char vowel, float *samples_out, size_t count) {
    if (!samples_out || count == 0) return false;
    
    // Formant definitions: F1, F2, F3 frequencies per vowel vocable
    uint32_t f1 = 0, f2 = 0, f3 = 0;
    switch (vowel) {
        case 'A': f1 = 730;  f2 = 1090; f3 = 2440; break;
        case 'E': f1 = 530;  f2 = 1840; f3 = 2480; break;
        case 'I': f1 = 270;  f2 = 2290; f3 = 3010; break;
        case 'O': f1 = 570;  f2 = 840;  f3 = 2410; break;
        case 'U': f1 = 300;  f2 = 870;  f3 = 2240; break;
        default: return false;
    }
    
    double fs = 8000.0;
    for (size_t n = 0; n < count; n++) {
        double t = (double)n / fs;
        // Superpose the three formants to synthesize human vocable
        samples_out[n] = 0.33f * (sin(2.0 * M_PI * f1 * t) + sin(2.0 * M_PI * f2 * t) + sin(2.0 * M_PI * f3 * t));
    }
    return true;
}

bool blue_box_trigger_wink(uint32_t duration_ms) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    bool valid = (duration_ms >= 150 && duration_ms <= 300);
    lau_yul_thunk_sstore(0xF135, valid ? 1 : 0);
    printf("[WINK] Wink pulse processed (duration: %u ms). Valid: %s\n", duration_ms, valid ? "YES" : "NO");
    return valid;
}

bool blue_box_apply_notch_filter(const float *samples_in, float *samples_out, size_t count, bool split_active) {
    if (!samples_in || !samples_out || count == 0) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF136, split_active ? 1 : 0);
    
    if (!split_active) {
        memcpy(samples_out, samples_in, count * sizeof(float));
        return true;
    }
    
    // 2600 Hz notch filter parameters at 8000 Hz sample rate
    double w0 = 2.0 * M_PI * 2600.0 / 8000.0;
    double cos_w0 = cos(w0);
    double r = 0.85;
    
    // Filter memory state (2 delays)
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;
    
    for (size_t n = 0; n < count; n++) {
        float x = samples_in[n];
        float y = (float)(x - 2.0 * cos_w0 * x1 + x2 + 2.0 * r * cos_w0 * y1 - r * r * y2);
        
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        
        samples_out[n] = y;
    }
    
    return true;
}

typedef struct {
    uint32_t peer_ip;
    uint16_t peer_as;
    uint32_t precedence;
    uint32_t latency_ms;
} BGPPeer;

static BGPPeer g_bgp_peers[MAX_BGP_PEERS];
size_t g_bgp_peer_count = 0;
bool blue_box_add_bgp_peer(uint32_t peer_ip, uint16_t peer_as, uint32_t precedence, uint32_t latency_ms) {
    // Check if peer already exists, update if so
    for (size_t i = 0; i < g_bgp_peer_count; i++) {
        if (g_bgp_peers[i].peer_ip == peer_ip) {
            g_bgp_peers[i].peer_as = peer_as;
            g_bgp_peers[i].precedence = precedence;
            g_bgp_peers[i].latency_ms = latency_ms;
            printf("[BGP PEER] Updated peer IP: 0x%X (AS: %u, Precedence: %u, Latency: %u ms)\n",
                   peer_ip, peer_as, precedence, latency_ms);
            return true;
        }
    }
    if (g_bgp_peer_count >= MAX_BGP_PEERS) return false;
    g_bgp_peers[g_bgp_peer_count].peer_ip = peer_ip;
    g_bgp_peers[g_bgp_peer_count].peer_as = peer_as;
    g_bgp_peers[g_bgp_peer_count].precedence = precedence;
    g_bgp_peers[g_bgp_peer_count].latency_ms = latency_ms;
    g_bgp_peer_count++;
    printf("[BGP PEER] Added new peer IP: 0x%X (AS: %u, Precedence: %u, Latency: %u ms)\n",
           peer_ip, peer_as, precedence, latency_ms);
    return true;
}

bool blue_box_get_bgp_peer(uint32_t peer_ip, uint32_t *precedence_out, uint32_t *latency_out) {
    if (!precedence_out || !latency_out) return false;
    for (size_t i = 0; i < g_bgp_peer_count; i++) {
        if (g_bgp_peers[i].peer_ip == peer_ip) {
            *precedence_out = g_bgp_peers[i].precedence;
            *latency_out = g_bgp_peers[i].latency_ms;
            return true;
        }
    }
    return false;
}

uint32_t blue_box_query_bgp_peers_by_precedence(uint32_t precedence, uint32_t *ips_out, uint32_t max_results) {
    if (!ips_out || max_results == 0) return 0;
    uint32_t count = 0;
    for (size_t i = 0; i < g_bgp_peer_count && count < max_results; i++) {
        if (g_bgp_peers[i].precedence == precedence) {
            ips_out[count++] = g_bgp_peers[i].peer_ip;
        }
    }
    return count;
}

typedef struct {
    char layer_name[64];
    uint32_t peer_ip;
    uint32_t size_bytes;
} GGUFLayerRoute;

static GGUFLayerRoute g_gguf_routes[MAX_GGUF_ROUTES];
static size_t g_gguf_route_count = 0;
bool blue_box_add_gguf_layer_route(const char *layer_name, uint32_t peer_ip, uint32_t size_bytes) {
    if (!layer_name || g_gguf_route_count >= MAX_GGUF_ROUTES) return false;
    for (size_t i = 0; i < g_gguf_route_count; i++) {
        if (strcmp(g_gguf_routes[i].layer_name, layer_name) == 0 && g_gguf_routes[i].peer_ip == peer_ip) {
            g_gguf_routes[i].size_bytes = size_bytes;
            return true;
        }
    }
    strncpy(g_gguf_routes[g_gguf_route_count].layer_name, layer_name, 63);
    g_gguf_routes[g_gguf_route_count].layer_name[63] = '\0';
    g_gguf_routes[g_gguf_route_count].peer_ip = peer_ip;
    g_gguf_routes[g_gguf_route_count].size_bytes = size_bytes;
    g_gguf_route_count++;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF160, g_gguf_route_count);
    return true;
}

uint32_t blue_box_select_gguf_layer_peer(const char *layer_name) {
    if (!layer_name) return 0;
    uint32_t best_peer_ip = 0;
    double best_cost = 1e18;
    
    for (size_t i = 0; i < g_gguf_route_count; i++) {
        if (strcmp(g_gguf_routes[i].layer_name, layer_name) == 0) {
            uint32_t prec = 0, lat = 0;
            if (!blue_box_get_bgp_peer(g_gguf_routes[i].peer_ip, &prec, &lat)) {
                lat = 100;
            }
            double cost = (double)lat + ((double)g_gguf_routes[i].size_bytes / 100000.0);
            if (cost < best_cost) {
                best_cost = cost;
                best_peer_ip = g_gguf_routes[i].peer_ip;
            }
        }
    }
    return best_peer_ip;
}

bool blue_box_simulate_black_box(float resistance_ohms, uint32_t *voltage_out, bool *billing_active_out) {
    if (!voltage_out || !billing_active_out) return false;
    
    // V_loop = 48V * R / (R + 1000)
    float v_loop = 48.0f * resistance_ohms / (resistance_ohms + 1000.0f);
    uint32_t v_rounded = (uint32_t)(v_loop + 0.5f);
    *voltage_out = v_rounded;
    
    // Black Box threshold window: 10V to 12V suppresses answer-supervision (no billing)
    bool billing_active = true;
    if (v_loop >= 10.0f && v_loop <= 12.0f) {
        billing_active = false;
    } else if (v_loop < 10.0f) {
        // Under 10V is considered on-hook / line-dropped
        billing_active = false;
    }
    
    *billing_active_out = billing_active;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF150, v_rounded);
    lau_yul_thunk_sstore(0xF151, billing_active ? 1 : 0);
    
    printf("[BLACK BOX] Clamped resistance: %.1f ohms. Voltage: %u V. Billing active: %s\n",
           resistance_ohms, v_rounded, billing_active ? "YES" : "NO");
    return true;
}

static uint32_t g_hook_flash_count = 0;
bool blue_box_trigger_hook_flash(uint32_t duration_ms, bool *flash_detected_out, uint32_t *flash_count_out) {
    if (!flash_detected_out || !flash_count_out) return false;
    
    bool detected = (duration_ms >= 500 && duration_ms <= 1000);
    if (detected) {
        g_hook_flash_count++;
    } else if (duration_ms > 1000) {
        g_hook_flash_count = 0; // complete disconnect
    }
    
    *flash_detected_out = detected;
    *flash_count_out = g_hook_flash_count;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF152, detected ? 1 : 0);
    lau_yul_thunk_sstore(0xF153, g_hook_flash_count);
    
    printf("[HOOK FLASH] Pulse processed (duration: %u ms). Detected: %s. Total Flashes: %u\n",
           duration_ms, detected ? "YES" : "NO", g_hook_flash_count);
    return true;
}

bool blue_box_trigger_green_box(uint32_t frequency, uint32_t duration_ms, uint32_t *action_out) {
    if (!action_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t blue_box_get_accumulator(void);
    
    uint32_t action = 0;
    bool valid = (duration_ms >= 400 && duration_ms <= 600);
    
    if (valid) {
        if (frequency == 1700) {
            action = 1; // Collect Coin
            // Vault the coin: reset active accumulator to 0
            extern void blue_box_deplete_session_gas_override(void);
            lau_yul_thunk_sstore(0xF120, 0); // resets threat
        } else if (frequency == 2200) {
            action = 2; // Return Coin
            // Return to customer
        }
    }
    
    *action_out = action;
    lau_yul_thunk_sstore(0xF185, action);
    printf("[GREEN BOX] Tone processed: %u Hz (duration: %u ms). Action: %u\n",
           frequency, duration_ms, action);
    return true;
}

bool blue_box_green_box_to_erc20(uint32_t action, char *payload_out, size_t max_len) {
    if (!payload_out || max_len < 256) return false;
    
    const char *method = "";
    if (action == 1) {
        method = "erc20_collect";
    } else if (action == 2) {
        method = "erc20_refund";
    } else {
        return false;
    }
    
    snprintf(payload_out, max_len,
             "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"params\":{\"amount\":%u,\"sig_hash\":\"%02x%02x%02x%02x\"},\"id\":1}",
             method,
             current_block_state.gas_allowance,
             current_block_state.state_hash[4], current_block_state.state_hash[5],
             current_block_state.state_hash[6], current_block_state.state_hash[7]);
             
    return true;
}

bool blue_box_trigger_green_box_diyat(uint32_t frequency, uint32_t duration_ms, uint32_t *action_out) {
    if (!action_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    uint32_t action = 0;
    bool valid = (duration_ms >= 400 && duration_ms <= 600);
    
    if (valid) {
        if (frequency == 1700) {
            action = 1; // Collect
            uint64_t session_gas = current_block_state.gas_allowance;
            uint64_t vault = lau_yul_thunk_sload(0xF186);
            lau_yul_thunk_sstore(0xF186, vault + session_gas);
            current_block_state.gas_allowance = 0;
            lau_yul_thunk_sstore(0xF199, 0);
        } else if (frequency == 2200) {
            action = 2; // Return
            uint64_t session_gas = current_block_state.gas_allowance;
            uint64_t rate = lau_yul_thunk_sload(0xF196);
            if (rate == 0) rate = 5; // Default 5%
            
            uint64_t fee = (session_gas * rate) / 100;
            uint64_t refund = session_gas - fee;
            
            uint64_t fee_pool = lau_yul_thunk_sload(0xF195);
            uint64_t refund_pool = lau_yul_thunk_sload(0xF187);
            
            lau_yul_thunk_sstore(0xF195, fee_pool + fee);
            lau_yul_thunk_sstore(0xF187, refund_pool + refund);
            
            uint64_t total_gas = lau_yul_thunk_sload(0xF199);
            lau_yul_thunk_sstore(0xF199, total_gas + refund);
            current_block_state.gas_allowance = total_gas + refund;
        }
    }
    
    *action_out = action;
    lau_yul_thunk_sstore(0xF185, action);
    printf("[GREEN BOX DIYAT] Tone: %u Hz. Action: %u. Fee Pool: %lu. Allowance: %u\n",
           frequency, action, lau_yul_thunk_sload(0xF195), current_block_state.gas_allowance);
    return true;
}

bool blue_box_sync_qing_coaxial(uint32_t user_count, uint32_t pilot_freq, uint32_t *freq_lock_out) {
    if (!freq_lock_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    uint64_t mod_index = lau_yul_thunk_sload(0xF18C);
    if (mod_index == 0) mod_index = 100; // Default modulation index
    
    uint64_t freq_lock = pilot_freq * (100 + user_count) / 100;
    *freq_lock_out = (uint32_t)freq_lock;
    
    lau_yul_thunk_sstore(0xF18B, user_count);
    lau_yul_thunk_sstore(0xF125, freq_lock); // Sets target lock deviation
    
    printf("[QING SYNC] Users: %u. Pilot Freq: %u Hz. Locked Freq: %lu Hz. Mod Index: %lu\n",
           user_count, pilot_freq, freq_lock, mod_index);
    return true;
}

bool blue_box_run_green_agent(uint32_t *action_out) {
    if (!action_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    uint64_t wink = lau_yul_thunk_sload(0xF135);
    uint64_t mute = lau_yul_thunk_sload(0xF121);
    uint64_t notch = lau_yul_thunk_sload(0xF136);
    
    uint32_t action = 0;
    if (wink == 1 && mute == 0 && notch == 0) {
        action = 1; // Auto-Collect
    } else {
        action = 2; // Auto-Refund
    }
    
    *action_out = action;
    lau_yul_thunk_sstore(0xF185, action);
    lau_yul_thunk_sstore(0xF191, action == 1 ? 2 : 3); // State: Auto-Collect (2) or Auto-Refund (3)
    
    printf("[GREEN AGENT] Wink: %lu. Mute: %lu. Notch: %lu. Action: %u. State: %lu\n",
           wink, mute, notch, action, lau_yul_thunk_sload(0xF191));
    return true;
}

bool blue_box_save_rdbms_tables(void);
bool blue_box_sync_green_agent_rdbms(uint64_t *hash_out) {
    if (!hash_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    uint64_t mode = lau_yul_thunk_sload(0xF18E);
    uint64_t rate = lau_yul_thunk_sload(0xF196);
    uint64_t last_action = lau_yul_thunk_sload(0xF185);
    
    // Compute relational state verification hash
    uint64_t hash = ((mode * 33 + rate) * 33 + last_action) % MotzkinPrime;
    *hash_out = hash;
    
    lau_yul_thunk_sstore(0xF192, hash);
    printf("[GREEN RDBMS] Sync complete. Mode: %lu. Rate: %lu. Action: %lu. Hash: %lu\n",
           mode, rate, last_action, hash);
           
    // Serialize and save tables to disk
    blue_box_save_rdbms_tables();
    return true;
}

bool blue_box_save_rdbms_tables(void) {
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    uint64_t mode = lau_yul_thunk_sload(0xF1B7);
    return blue_box_write_quadtree_to_disk((uint32_t)mode);
}

