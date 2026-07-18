#ifndef TSFI_COMPUTEL_YELLOW_BOX_H
#define TSFI_COMPUTEL_YELLOW_BOX_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t active_sbox_profile;
    uint32_t scramble_key;
    uint32_t call_sign_hash;
} YellowBoxState;

void yellow_box_init(uint32_t key, uint32_t profile_id);
YellowBoxState yellow_box_get_state(void);
int yellow_box_scramble_signal(const uint8_t *in, size_t size, uint8_t *out);

int tsfi_mf_yellow_box_secure_telegram_route(const char *ssn, int dmf_deceased, int ssn_allocated, int defcon_level, uint8_t *out_telegram, size_t *out_size);

int tsfi_mf_yellow_box_scramble_keypad(uint8_t raw_keycode, uint8_t *scrambled_keycode);
int tsfi_mf_gost_encrypt_cfb(const uint8_t *iv, const uint8_t *in, size_t size, const uint32_t *keys_8words, uint8_t *out);

#endif // TSFI_COMPUTEL_YELLOW_BOX_H
