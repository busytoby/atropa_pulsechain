#ifndef TSFI_VAESEN_FPGA_MISTRAL_EXT_H
#define TSFI_VAESEN_FPGA_MISTRAL_EXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_vaesen_device_slice.h"

#define TSFI_TOOL_OP_NONE         0x00
#define TSFI_TOOL_OP_LIGHT_ITEM   0x01
#define TSFI_TOOL_OP_STRIKE_BELL  0x02
#define TSFI_TOOL_OP_OFFER_GIFT   0x03
#define TSFI_TOOL_OP_SEAL_RITUAL  0x04

typedef struct __attribute__((packed)) {
    uint8_t  op_code;             /* TSFI_TOOL_OP_* */
    uint16_t target_6502_addr;    /* Memory address in Zero-Page or I/O */
    uint8_t  value;               /* Value to write */
    char     description[256];    /* Diagnostic action string (Rule 11 >= 256 bytes) */
} TsfiHardwareToolCall;

typedef struct {
    float   temperature;          /* [0.0..1.0] derived from Fervour, Fear, Dogma */
    float   top_p;                /* [0.5..1.0] cumulative probability cutoff */
    uint8_t jitter_variance_milli;/* [0..1000] milli-units of acoustic jitter */
} TsfiAcousticEntropyConfig;

typedef struct {
    uint8_t draft_count;          /* K=4 candidate tokens */
    uint16_t draft_tokens[4];
    uint8_t verified_count;
    bool    all_accepted;
} TsfiSpeculativeDraftResult;

/* Mistral Extensions API */
TsfiAcousticEntropyConfig tsfi_vaesen_compute_acoustic_entropy(const TsfiVaesenDeviceSliceBin *slice);

int tsfi_vaesen_dispatch_hardware_tool(
    const TsfiVaesenDeviceSliceBin *slice,
    uint8_t requested_op,
    uint16_t target_addr,
    uint8_t value,
    TsfiHardwareToolCall *out_call
);

int tsfi_vaesen_speculative_verify_draft(
    const TsfiVaesenDeviceSliceBin *slice,
    const uint16_t *draft_tokens,
    uint8_t draft_count,
    TsfiSpeculativeDraftResult *out_result
);

#endif /* TSFI_VAESEN_FPGA_MISTRAL_EXT_H */
