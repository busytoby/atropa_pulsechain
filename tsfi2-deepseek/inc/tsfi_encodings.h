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

// --- Improvements APIs ---
void tsfi_encode_hamming74(const uint8_t *in, int len, uint8_t *out);
void tsfi_decode_hamming74(const uint8_t *in, int coded_len, uint8_t *out);

int tsfi_punch_tone_wheel(uint16_t *wheel, int max_cols, int col, char c);

int tsfi_eer_datalog_specialization(TSFiEerDatabase *db, const char *incident_type, const char *severity);

void tsfi_pll_vulkan_project(float error_voltage, float phase_diff, float *ndc_x, float *ndc_y);

#endif // TSFI_ENCODINGS_H
