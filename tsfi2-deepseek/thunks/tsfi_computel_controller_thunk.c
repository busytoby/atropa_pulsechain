#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tsfi_computel_advanced_kermit.c"

/*
 * Auncient Computel Game Controller Thunk Bridge
 * 
 * Translates raw controller inputs (buttons, axes) into encrypted,
 * non-injectable Kermit frames using ephemeral key states.
 */

typedef struct {
    uint8_t buttons;
    int8_t axis_x;
    int8_t axis_y;
    uint32_t timestamp;
} RawControllerInput;

typedef struct {
    AdvancedKermitState kermit;
    uint32_t expected_packet_idx;
} ControllerThunk;

void init_controller_thunk(ControllerThunk *thunk, uint64_t seed) {
    if (!thunk) return;
    init_advanced_kermit(&thunk->kermit, seed);
    thunk->expected_packet_idx = 0;
}

/* Thunk wrapper: Packages raw input into an encrypted transmission payload */
void thunk_package_input(ControllerThunk *thunk, const RawControllerInput *input, uint8_t *out_payload, uint8_t *out_len) {
    if (!thunk || !input || !out_payload) return;

    // Serialize RawControllerInput into 7 bytes
    out_payload[0] = input->buttons;
    out_payload[1] = (uint8_t)input->axis_x;
    out_payload[2] = (uint8_t)input->axis_y;
    out_payload[3] = (uint8_t)((input->timestamp >> 24) & 0xFF);
    out_payload[4] = (uint8_t)((input->timestamp >> 16) & 0xFF);
    out_payload[5] = (uint8_t)((input->timestamp >> 8) & 0xFF);
    out_payload[6] = (uint8_t)(input->timestamp & 0xFF);
    *out_len = 7;

    // Encrypt payload using current active key
    uint64_t temp_key = thunk->kermit.active_key;
    for (uint8_t i = 0; i < *out_len; i++) {
        uint8_t key_byte = (uint8_t)((temp_key >> ((i % 8) * 8)) & 0xFF);
        out_payload[i] ^= key_byte;
    }

    // Advance packet state to rotate active key
    handle_transmission_feedback(&thunk->kermit, true);
}

/* Thunk validation: Decrypts and verifies incoming controller payload */
bool thunk_verify_input(ControllerThunk *thunk, uint8_t *payload, uint8_t len, RawControllerInput *out_input) {
    if (!thunk || !payload || len != 7 || !out_input) return false;

    // Decrypt using active key
    uint64_t temp_key = thunk->kermit.active_key;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t key_byte = (uint8_t)((temp_key >> ((i % 8) * 8)) & 0xFF);
        payload[i] ^= key_byte;
    }

    // Deserialize into structure
    out_input->buttons = payload[0];
    out_input->axis_x = (int8_t)payload[1];
    out_input->axis_y = (int8_t)payload[2];
    out_input->timestamp = ((uint32_t)payload[3] << 24) |
                           ((uint32_t)payload[4] << 16) |
                           ((uint32_t)payload[5] << 8)  |
                           (uint32_t)payload[6];

    // Advance state to keep keys in sync
    handle_transmission_feedback(&thunk->kermit, true);
    return true;
}
