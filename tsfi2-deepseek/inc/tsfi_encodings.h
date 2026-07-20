#ifndef TSFI_ENCODINGS_H
#define TSFI_ENCODINGS_H

#include <stdint.h>
#include <stdbool.h>

#include "tsfi_oregon_trail.h"
#include "tsfi_eer_emergency.h"

// Encodings API
int tsfi_encode_ascii(const char *in, uint8_t *out, int max_len);
int tsfi_decode_ascii(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_ebcdic(const char *in, uint8_t *out, int max_len);
int tsfi_decode_ebcdic(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_morse(const char *in, char *out, int max_len);
int tsfi_decode_morse(const char *in, char *out, int max_len);

int tsfi_encode_hollerith(const char *in, uint16_t *out, int max_len);
int tsfi_decode_hollerith(const uint16_t *in, int len, char *out, int max_len);

int tsfi_encode_baudot(const char *in, uint8_t *out, int max_len);
int tsfi_decode_baudot(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_bcdic(const char *in, uint8_t *out, int max_len);
int tsfi_decode_bcdic(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_fieldata(const char *in, uint8_t *out, int max_len);
int tsfi_decode_fieldata(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_murray(const char *in, uint8_t *out, int max_len);
int tsfi_decode_murray(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_radix50(const char *in, uint16_t *out, int max_len);
int tsfi_decode_radix50(const uint16_t *in, int len, char *out, int max_len);

#include "tsfi_ot_accumulator.h"

int tsfi_ot_baud_llm_dat(const char *dat_bin_path);
int tsfi_eer_bridge_ot_acab(TSFiEerDatabase *db, const char *dat_bin_path);

int tsfi_ot_accum_baud_llm_dat(const char *dat_bin_path);
int tsfi_eer_bridge_ot_accum_acab(TSFiEerDatabase *db, const char *dat_bin_path);

int tsfi_ot_crypto_baud_llm_dat(const char *dat_bin_path, int choice_b, const char *msg0, const char *msg1);
int tsfi_eer_bridge_ot_crypto_acab(TSFiEerDatabase *db, const char *dat_bin_path);

int tsfi_ot_crypto_stanag_baud_llm_dat(const char *dat_bin_path, int choice_b, const char *msg0, const char *msg1);
int tsfi_eer_bridge_ot_crypto_stanag_acab(TSFiEerDatabase *db, const char *dat_bin_path);

int tsfi_ot_optical_baud_llm_dat(const char *dat_bin_path);
int tsfi_eer_bridge_ot_optical_acab(TSFiEerDatabase *db, const char *dat_bin_path);

int tsfi_generic_ot_baud_llm_dat(const char *dat_bin_path, const char *payload_text);
int tsfi_eer_bridge_generic_ot_acab(TSFiEerDatabase *db, const char *dat_bin_path, int (*parse_callback)(const char *decrypted, uint32_t *incident_id, int *type, int *defcon));

// --- Improvements APIs ---
void tsfi_encode_hamming74(const uint8_t *in, int len, uint8_t *out);
void tsfi_decode_hamming74(const uint8_t *in, int coded_len, uint8_t *out);

int tsfi_punch_tone_wheel(uint16_t *wheel, int max_cols, int col, char c);

int tsfi_eer_datalog_specialization(TSFiEerDatabase *db, const char *incident_type, const char *severity);

void tsfi_pll_vulkan_project(float error_voltage, float phase_diff, float *ndc_x, float *ndc_y);

// --- Extended Improvements APIs ---
void tsfi_encode_hamming84(const uint8_t *in, int len, uint8_t *out);
int tsfi_decode_hamming84(const uint8_t *in, int coded_len, uint8_t *out);

int tsfi_ot_accumulator_spsc_push(TSFiOTAccumulator *acc, const char *coord, float weight);

int tsfi_eer_datalog_cascade(TSFiEerDatabase *db, const char *entity, const char *rule1, const char *rule2);

void tsfi_pll_pi_tune(float error_voltage, float last_integral, float kp, float ki, float dt, float *output_voltage, float *next_integral);

// --- Third-Generation Improvements APIs ---
int tsfi_baudot_update_maps(const uint8_t *new_ltrs, const uint8_t *new_figs);

void tsfi_encode_bch15_7(const uint8_t *in, int len, uint16_t *out);
int tsfi_decode_bch15_7(const uint16_t *in, int len, uint8_t *out);

void tsfi_pll_pid_agc_tune(float error_voltage, float last_integral, float last_error, float kp, float ki, float kd, float dt, float input_signal_amp, float *output_voltage, float *next_integral, float *next_error, float *agc_gain);

int tsfi_eer_delete_incident(TSFiEerDatabase *db, uint32_t incident_id);

// --- Fourth-Generation Improvements APIs ---
#define MAX_STANAG_ROUTES 8
typedef void (*tsfi_stanag_sap_handler)(TSFiEerDatabase *db, const uint8_t *payload, int len);

typedef struct {
    uint8_t sap;
    tsfi_stanag_sap_handler handler;
} TSFiStanagRoute;

int tsfi_stanag_register_route(uint8_t sap, tsfi_stanag_sap_handler handler);
int tsfi_stanag_route_frame(TSFiEerDatabase *db, uint8_t sap, const uint8_t *payload, int len);

void tsfi_encode_lrc15_11(const uint8_t *in, int len, uint8_t *out);
int tsfi_decode_lrc15_11(const uint8_t *in, int len, uint8_t *out);

void tsfi_pll_ema_estimate(float measurement, float *state, float alpha);

int tsfi_eer_audit_invariants(const TSFiEerDatabase *db);

// --- Fifth-Generation Improvements APIs ---
void tsfi_interleave_lrc(const uint8_t *in, int len, uint8_t *out);
void tsfi_deinterleave_lrc(const uint8_t *in, int len, uint8_t *out);

void tsfi_pll_holt_estimate(float measurement, float *level, float *trend, float alpha, float beta);

typedef struct {
    uint8_t sap;
    tsfi_stanag_sap_handler handler;
    int priority;
} TSFiStanagRoutePriority;

int tsfi_stanag_register_priority_route(uint8_t sap, tsfi_stanag_sap_handler handler, int priority);
int tsfi_stanag_route_priority_frame(TSFiEerDatabase *db, uint8_t sap, const uint8_t *payload, int len);

int tsfi_eer_audit_paths(const TSFiEerDatabase *db);

#endif // TSFI_ENCODINGS_H
