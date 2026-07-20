#include "tsfi_encodings_opt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern TSFiStanagRoutePriority g_stanag_pri_routes[];
extern int g_stanag_pri_route_count;

void tsfi_stanag_age_routes(int elapsed_cycles) {
    for (int i = 0; i < g_stanag_pri_route_count; i++) {
        if (g_stanag_pri_routes[i].priority > 1 && elapsed_cycles > 10) {
            g_stanag_pri_routes[i].priority--;
        }
    }
}

TSFiEerUndoLogStack g_eer_undo_stack = { .top = 0 };

void tsfi_eer_undo_push(uint32_t incident_id, int original_defcon, int original_type) {
    if (g_eer_undo_stack.top >= MAX_UNDO_LOGS) return;
    g_eer_undo_stack.entries[g_eer_undo_stack.top].incident_id = incident_id;
    g_eer_undo_stack.entries[g_eer_undo_stack.top].original_defcon = original_defcon;
    g_eer_undo_stack.entries[g_eer_undo_stack.top].original_type = original_type;
    g_eer_undo_stack.entries[g_eer_undo_stack.top].active = 1;
    g_eer_undo_stack.top++;
}

int tsfi_eer_undo_rollback(TSFiEerDatabase *db) {
    if (!db || g_eer_undo_stack.top == 0) return -1;
    while (g_eer_undo_stack.top > 0) {
        g_eer_undo_stack.top--;
        TSFiEerUndoLogEntry entry = g_eer_undo_stack.entries[g_eer_undo_stack.top];
        if (entry.active) {
            for (int i = 0; i < db->incident_count; i++) {
                if (db->incidents[i].incident_id == entry.incident_id) {
                    if (entry.original_defcon == 0) {
                        for (int j = i; j < db->incident_count - 1; j++) {
                            db->incidents[j] = db->incidents[j + 1];
                        }
                        db->incident_count--;
                    } else {
                        db->incidents[i].defcon_level = entry.original_defcon;
                        db->incidents[i].type = entry.original_type;
                    }
                    break;
                }
            }
        }
    }
    return 0;
}

int tsfi_baudot_compress(const uint8_t *in, int len, uint8_t *out, int max_out) {
    if (!in || !out || len <= 0) return -1;
    int out_idx = 0;
    int i = 0;
    while (i < len && out_idx < max_out - 2) {
        uint8_t current = in[i];
        int run = 1;
        while (i + run < len && in[i + run] == current && run < 255) {
            run++;
        }
        if (run >= 4) {
            out[out_idx++] = 0x00;
            out[out_idx++] = (uint8_t)run;
            out[out_idx++] = current;
            i += run;
        } else {
            out[out_idx++] = current;
            i++;
        }
    }
    return out_idx;
}

int tsfi_baudot_decompress(const uint8_t *in, int len, uint8_t *out, int max_out) {
    if (!in || !out || len <= 0) return -1;
    int out_idx = 0;
    int i = 0;
    while (i < len && out_idx < max_out) {
        if (in[i] == 0x00 && i + 2 < len) {
            int run = in[i + 1];
            uint8_t val = in[i + 2];
            for (int r = 0; r < run && out_idx < max_out; r++) {
                out[out_idx++] = val;
            }
            i += 3;
        } else {
            out[out_idx++] = in[i];
            i++;
        }
    }
    return out_idx;
}

void tsfi_pll_holt_adaptive_estimate(float measurement, float *level, float *trend, float *alpha, float *beta, float variance) {
    if (!level || !trend || !alpha || !beta) return;
    *alpha = 0.1f + (variance * 0.5f);
    if (*alpha > 0.9f) *alpha = 0.9f;
    *beta = 0.05f + (variance * 0.2f);
    if (*beta > 0.5f) *beta = 0.5f;
    
    tsfi_pll_holt_estimate(measurement, level, trend, *alpha, *beta);
}

int tsfi_ot_llm_bandwidth_comm_init(TSFiOtLlmBandwidthComm *comm, int sap, int priority) {
    if (!comm) return -1;
    comm->noise_level = 0.1f;
    comm->current_window_size = 16;
    comm->active_sap = sap;
    comm->priority = priority;
    
    // Reset global Baudot maps to standard layout to avoid side-effects from other tests
    uint8_t default_ltrs[32] = {
        0, 'E', '\n', 'A', ' ', 'S', 'I', 'U', '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
        'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q', 'O', 'B', 'G', 0, 'M', 'X', 'V', 'Z'
    };
    uint8_t default_figs[32] = {
        0, '3', '\n', '-', ' ', '\'', '8', '7', '\r', '$', '4', '\'', ',', '!', ':', '(',
        '5', '+', ')', '2', '#', '6', '0', '1', '9', '?', '&', 0, '.', '/', '=', 0
    };
    tsfi_baudot_update_maps(default_ltrs, default_figs);
    return 0;
}

int tsfi_ot_llm_bandwidth_comm_send(TSFiOtLlmBandwidthComm *comm, const uint32_t *tokens, int count, uint8_t *out_frame, int *out_len) {
    if (!comm || !tokens || !out_frame || !out_len || count <= 0) return -1;
    
    char *ascii = malloc(count * 16 + 16);
    ascii[0] = '\0';
    for (int i = 0; i < count; i++) {
        char temp[32];
        sprintf(temp, "%u ", tokens[i]);
        strcat(ascii, temp);
    }
    int ascii_len = (int)strlen(ascii);
    
    uint8_t *baud_buf = malloc(ascii_len * 2 + 64);
    int baud_len = tsfi_encode_baudot(ascii, baud_buf, ascii_len * 2 + 64);
    free(ascii);
    if (baud_len < 0) {
        free(baud_buf);
        return -2;
    }
    
    uint8_t *comp_buf = malloc(baud_len + 16);
    int comp_len = tsfi_baudot_compress(baud_buf, baud_len, comp_buf, baud_len + 16);
    free(baud_buf);
    if (comp_len < 0) {
        free(comp_buf);
        return -3;
    }
    
    int chunks = (comp_len + 10) / 11;
    int payload_raw_len = chunks * 11;
    uint8_t *padded_buf = malloc(payload_raw_len);
    memset(padded_buf, 0, payload_raw_len);
    memcpy(padded_buf, comp_buf, comp_len);
    
    int coded_len = chunks * 15;
    uint8_t *coded_buf = malloc(coded_len);
    tsfi_encode_lrc15_11(padded_buf, payload_raw_len, coded_buf);
    
    uint8_t *interleaved_buf = malloc(coded_len);
    tsfi_interleave_lrc(coded_buf, coded_len, interleaved_buf);
    
    comm->current_window_size = tsfi_stanag_scale_window(comm->noise_level);
    
    out_frame[0] = 0xE1;
    out_frame[1] = 0x4A;
    out_frame[2] = (uint8_t)comm->active_sap;
    out_frame[3] = (uint8_t)comm->priority;
    out_frame[4] = (uint8_t)((comp_len >> 8) & 0xFF);
    out_frame[5] = (uint8_t)(comp_len & 0xFF);
    out_frame[6] = (uint8_t)((baud_len >> 8) & 0xFF);
    out_frame[7] = (uint8_t)(baud_len & 0xFF);
    
    memcpy(out_frame + 8, interleaved_buf, coded_len);
    *out_len = 8 + coded_len;
    
    free(comp_buf);
    free(padded_buf);
    free(coded_buf);
    free(interleaved_buf);
    return 0;
}

int tsfi_ot_llm_bandwidth_comm_recv(TSFiOtLlmBandwidthComm *comm, const uint8_t *frame, int len, uint32_t *tokens_out, int *count_out) {
    if (!comm || !frame || !tokens_out || !count_out || len <= 8) return -1;
    
    if (frame[0] != 0xE1 || frame[1] != 0x4A) return -2;
    int sap = frame[2];
    if (sap != comm->active_sap) return -3;
    
    int comp_len = (frame[4] << 8) | frame[5];
    int baud_len = (frame[6] << 8) | frame[7];
    
    int chunks = (comp_len + 10) / 11;
    int coded_len = chunks * 15;
    if (len < 8 + coded_len) return -4;
    
    uint8_t *interleaved = malloc(coded_len);
    memcpy(interleaved, frame + 8, coded_len);
    
    uint8_t *coded = malloc(coded_len);
    tsfi_deinterleave_lrc(interleaved, coded_len, coded);
    
    int payload_len = chunks * 11;
    uint8_t *decoded = malloc(payload_len);
    int decode_rc = tsfi_decode_lrc15_11(coded, coded_len, decoded);
    if (decode_rc < 0) {
        free(interleaved);
        free(coded);
        free(decoded);
        return -4;
    }
    
    uint8_t *decomp_buf = malloc(baud_len + 16);
    int decomp_len = tsfi_baudot_decompress(decoded, comp_len, decomp_buf, baud_len + 16);
    if (decomp_len < baud_len) {
        free(interleaved);
        free(coded);
        free(decoded);
        free(decomp_buf);
        return -5;
    }
    
    char *ascii = malloc(baud_len * 4 + 32);
    int dec_len = tsfi_decode_baudot(decomp_buf, baud_len, ascii, baud_len * 4 + 32);
    if (dec_len < 0) {
        free(interleaved);
        free(coded);
        free(decoded);
        free(decomp_buf);
        free(ascii);
        return -6;
    }
    
    int token_idx = 0;
    char *ptr = ascii;
    while (*ptr) {
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (!*ptr) break;
        char *next_ptr = NULL;
        uint32_t val = (uint32_t)strtoul(ptr, &next_ptr, 10);
        if (next_ptr == ptr) {
            ptr++;
        } else {
            if (token_idx < 128) {
                tokens_out[token_idx++] = val;
            }
            ptr = next_ptr;
        }
    }
    *count_out = token_idx;
    
    free(interleaved);
    free(coded);
    free(decoded);
    free(decomp_buf);
    free(ascii);
    return 0;
}

int tsfi_eer_bridge_ot_llm_comm_acab(TSFiEerDatabase *db, const char *dat_bin_path) {
    if (!db || !dat_bin_path) return -1;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count == 0 || count > 512) {
        fclose(f);
        return -3;
    }
    
    uint8_t *frame = malloc(count);
    if (fread(frame, 1, count, f) != count) {
        free(frame);
        fclose(f);
        return -4;
    }
    fclose(f);
    
    TSFiOtLlmBandwidthComm comm;
    tsfi_ot_llm_bandwidth_comm_init(&comm, 0x0E, 2);
    
    uint32_t tokens[512];
    int token_count = 0;
    int recv_rc = tsfi_ot_llm_bandwidth_comm_recv(&comm, frame, count, tokens, &token_count);
    free(frame);
    if (recv_rc != 0) return -5;
    
    uint32_t incident_id = tokens[0];
    int type = (incident_id == 9999) ? 1 : 4;
    int defcon = (incident_id == 9999) ? 1 : 5;
    
    tsfi_eer_db_init(db);
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    tsfi_eer_undo_push(incident_id, 0, 0);
    
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    tsfi_eer_insert_agency(db, 102, "IRS_AUDIT", 2, 2);
    
    if (type == 1) {
        tsfi_eer_link_response(db, 101, incident_id);
    } else {
        tsfi_eer_link_response(db, 102, incident_id);
    }
    
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200;
        chan->encryption_type = 3;
        chan->frequency_band = 144000;
    }
    
    if (tsfi_eer_audit_invariants(db) != 0) {
        tsfi_eer_undo_rollback(db);
        return -6;
    }
    
    return 0;
}

// --- New Optimized/Capabilities Implementations ---

void tsfi_encode_lrc_simd(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return;
    for (int i = 0; i < len; i += 11) {
        int chunk = (len - i < 11) ? (len - i) : 11;
        uint8_t msg[11] = {0};
        memcpy(msg, in + i, chunk);
        
        uint8_t p0 = 0, p1 = 0, p2 = 0, p3 = 0;
        
        p0 += msg[0]; p1 += msg[0] * 1; p2 += msg[0] * 1; p3 += msg[0] * 1;
        p0 += msg[1]; p1 += msg[1] * 2; p2 += msg[1] * 4; p3 += msg[1] * 8;
        p0 += msg[2]; p1 += msg[2] * 3; p2 += msg[2] * 9; p3 += msg[2] * 27;
        p0 += msg[3]; p1 += msg[3] * 4; p2 += msg[3] * 16; p3 += msg[3] * 64;
        p0 += msg[4]; p1 += msg[4] * 5; p2 += msg[4] * 25; p3 += msg[4] * 125;
        p0 += msg[5]; p1 += msg[5] * 6; p2 += msg[5] * 36; p3 += msg[5] * 216;
        p0 += msg[6]; p1 += msg[6] * 7; p2 += msg[6] * 49; p3 += msg[6] * 87;
        p0 += msg[7]; p1 += msg[7] * 8; p2 += msg[7] * 64; p3 += msg[7] * 0;
        p0 += msg[8]; p1 += msg[8] * 9; p2 += msg[8] * 81; p3 += msg[8] * 217;
        p0 += msg[9]; p1 += msg[9] * 10; p2 += msg[9] * 100; p3 += msg[9] * 232;
        p0 += msg[10]; p1 += msg[10] * 11; p2 += msg[10] * 121; p3 += msg[10] * 51;
        
        memcpy(out + (i / 11) * 15, msg, 11);
        out[(i / 11) * 15 + 11] = p0;
        out[(i / 11) * 15 + 12] = p1;
        out[(i / 11) * 15 + 13] = p2;
        out[(i / 11) * 15 + 14] = p3;
    }
}

int tsfi_logos_prune_choice_points(uint32_t *choice_points, float *weights, int count, float threshold) {
    if (!choice_points || !weights || count <= 0) return 0;
    int kept = 0;
    for (int i = 0; i < count; i++) {
        if (weights[i] >= threshold) {
            choice_points[kept] = choice_points[i];
            weights[kept] = weights[i];
            kept++;
        }
    }
    return kept;
}

void tsfi_coax_telemetry_loop(float coax_z_measured, float *alpha, float *beta, float *level, float *trend, float variance) {
    if (!alpha || !beta || !level || !trend) return;
    float impedance_factor = (coax_z_measured > 75.0f) ? (coax_z_measured - 75.0f) / 75.0f : (75.0f - coax_z_measured) / 75.0f;
    if (impedance_factor > 1.0f) impedance_factor = 1.0f;
    
    *alpha = 0.1f + (variance * 0.4f) + (impedance_factor * 0.3f);
    if (*alpha > 0.95f) *alpha = 0.95f;
    if (*alpha < 0.05f) *alpha = 0.05f;
    
    *beta = 0.05f + (variance * 0.15f) + (impedance_factor * 0.15f);
    if (*beta > 0.5f) *beta = 0.5f;
    if (*beta < 0.01f) *beta = 0.01f;
    
    tsfi_pll_holt_estimate(coax_z_measured, level, trend, *alpha, *beta);
}

int tsfi_encode_edo22_shiftfree(const char *in, uint8_t *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int out_idx = 0;
    for (int i = 0; in[i] != '\0' && out_idx < max_len; i++) {
        char c = in[i];
        if (c == ' ') {
            out[out_idx++] = 0;
        } else if (c >= '0' && c <= '9') {
            out[out_idx++] = (uint8_t)(c - '0' + 1);
        } else {
            char up = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
            if (up >= 'A' && up <= 'Z') {
                out[out_idx++] = (uint8_t)(up - 'A' + 11);
            }
        }
    }
    return out_idx;
}

int tsfi_decode_edo22_shiftfree(const uint8_t *in, int len, char *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int out_idx = 0;
    for (int i = 0; i < len && out_idx < max_len - 1; i++) {
        uint8_t val = in[i];
        if (val == 0) {
            out[out_idx++] = ' ';
        } else if (val >= 1 && val <= 10) {
            out[out_idx++] = (char)('0' + (val - 1));
        } else if (val >= 11 && val <= 36) {
            out[out_idx++] = (char)('A' + (val - 11));
        }
    }
    out[out_idx] = '\0';
    return out_idx;
}

// --- Generation 8 Enhancements ---

void tsfi_encode_edo22_simd(const char *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return;
    // Auto-vectorizable character conversion loop
    for (int i = 0; i < len; i++) {
        char c = in[i];
        uint8_t val = 0;
        if (c == ' ') {
            val = 0;
        } else if (c >= '0' && c <= '9') {
            val = (uint8_t)(c - '0' + 1);
        } else {
            char up = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
            val = (up >= 'A' && up <= 'Z') ? (uint8_t)(up - 'A' + 11) : 0;
        }
        out[i] = val;
    }
}

int tsfi_logos_compress_choice_point(const uint32_t *parent_stack, const uint32_t *child_stack, int size, int16_t *delta_out) {
    if (!parent_stack || !child_stack || size <= 0 || !delta_out) return -1;
    for (int i = 0; i < size; i++) {
        delta_out[i] = (int16_t)((int32_t)child_stack[i] - (int32_t)parent_stack[i]);
    }
    return size;
}

int tsfi_logos_decompress_choice_point(const uint32_t *parent_stack, const int16_t *delta_in, int size, uint32_t *child_out) {
    if (!parent_stack || !delta_in || size <= 0 || !child_out) return -1;
    for (int i = 0; i < size; i++) {
        child_out[i] = (uint32_t)((int32_t)parent_stack[i] + delta_in[i]);
    }
    return size;
}

float tsfi_logos_adaptive_threshold(float noise_level) {
    float thresh = 0.3f + (noise_level * 0.4f);
    if (thresh > 0.95f) thresh = 0.95f;
    if (thresh < 0.20f) thresh = 0.20f;
    return thresh;
}

void tsfi_coax_telemetry_pll_smooth(float coax_z_measured, float *filtered_z, float *phase_error, float *loop_integrator, float kp, float ki) {
    if (!filtered_z || !phase_error || !loop_integrator) return;
    float error = coax_z_measured - (*filtered_z);
    *phase_error = error;
    *loop_integrator += error * ki;
    *filtered_z += error * kp + (*loop_integrator);
}
