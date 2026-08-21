#ifndef TSFI_ZMACHINE_VAESEN_BRIDGE_H
#define TSFI_ZMACHINE_VAESEN_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_vaesen_device_slice.h"

#define TSFI_ZMACHINE_OPCODE_WMQ_INTERACT 0xD4
#define TSFI_ZMACHINE_MAX_VECTOR_LINES    32

typedef struct __attribute__((packed)) {
    uint8_t start_x;
    uint8_t start_y;
    uint8_t end_x;
    uint8_t end_y;
    uint8_t color_idx;
} TsfiZMachineVectorLine;

typedef struct {
    uint8_t phys_trauma;  /* 0=Normal, 1=Exhausted, 2=Battered, 3=Broken */
    uint8_t ment_trauma;  /* 0=Normal, 1=Shaken, 2=Terrified, 3=Panicked */
    char    held_item[32];/* Active inventory item */
    char    player_wmq[48];
} TsfiZMachinePlayerState;

typedef struct {
    uint32_t num_lines;
    TsfiZMachineVectorLine lines[TSFI_ZMACHINE_MAX_VECTOR_LINES];
    char     dialogue_output[256];
    bool     ritual_banishment_success;
} TsfiZMachineInteractionResult;

/* Z-Machine Opcode Handler */
int tsfi_zmachine_execute_wmq_interact(
    const TsfiZMachinePlayerState *player,
    const char *target_vaesen_slice_path,
    TsfiZMachineInteractionResult *out_result
);

#endif /* TSFI_ZMACHINE_VAESEN_BRIDGE_H */
