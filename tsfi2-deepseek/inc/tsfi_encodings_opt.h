#ifndef TSFI_ENCODINGS_OPT_H
#define TSFI_ENCODINGS_OPT_H

#include "tsfi_encodings.h"

// --- Moved Functions Prototypes ---
int tsfi_ot_llm_bandwidth_comm_init(TSFiOtLlmBandwidthComm *comm, int sap, int priority);
int tsfi_ot_llm_bandwidth_comm_send(TSFiOtLlmBandwidthComm *comm, const uint32_t *tokens, int count, uint8_t *out_frame, int *out_len);
int tsfi_ot_llm_bandwidth_comm_recv(TSFiOtLlmBandwidthComm *comm, const uint8_t *frame, int len, uint32_t *tokens_out, int *count_out);
int tsfi_eer_bridge_ot_llm_comm_acab(TSFiEerDatabase *db, const char *dat_bin_path);
void tsfi_eer_undo_push(uint32_t incident_id, int original_defcon, int original_type);
int tsfi_eer_undo_rollback(TSFiEerDatabase *db);
int tsfi_baudot_compress(const uint8_t *in, int len, uint8_t *out, int max_out);
int tsfi_baudot_decompress(const uint8_t *in, int len, uint8_t *out, int max_out);
void tsfi_pll_holt_adaptive_estimate(float measurement, float *level, float *trend, float *alpha, float *beta, float variance);
void tsfi_stanag_age_routes(int elapsed_cycles);

// --- New Optimization/Capabilities Prototypes ---

// 1. SIMD-Accelerated/Compiler-Vectorized LRC Encoding
void tsfi_encode_lrc_simd(const uint8_t *in, int len, uint8_t *out);

// 2. Probability-Weighted Choice Point Pruning for LogOS
int tsfi_logos_prune_choice_points(uint32_t *choice_points, float *weights, int count, float threshold);

// 3. Coaxial Leakage Telemetry Loop integration with Holt adaptive tuning
void tsfi_coax_telemetry_loop(float coax_z_measured, float *alpha, float *beta, float *level, float *trend, float variance);

// 4. Bi-Directional Shift-Free EDO-22 Encodings
int tsfi_encode_edo22_shiftfree(const char *in, uint8_t *out, int max_len);
int tsfi_decode_edo22_shiftfree(const uint8_t *in, int len, char *out, int max_len);

#endif // TSFI_ENCODINGS_OPT_H
