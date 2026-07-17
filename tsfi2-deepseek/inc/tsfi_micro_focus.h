#ifndef TSFI_MICRO_FOCUS_H
#define TSFI_MICRO_FOCUS_H

#include <stdint.h>
#include <stddef.h>

// 1. COMP-5 Native Binary Format Encoders/Decoders
int tsfi_mf_comp5_encode(int64_t value, uint8_t *buffer, int size_bytes);
int64_t tsfi_mf_comp5_decode(const uint8_t *buffer, int size_bytes, int is_signed);

// 2. ISAM (Indexed Sequential Access Method) Reader/Writer (Supports only .dat.bin extension per Rule 13)
int tsfi_mf_isam_write_record(const char *filepath_dat_bin, uint32_t key, const uint8_t *record_data, int record_len);
int tsfi_mf_isam_read_record(const char *filepath_dat_bin, uint32_t key, uint8_t *record_out, int record_len);

#endif // TSFI_MICRO_FOCUS_H
