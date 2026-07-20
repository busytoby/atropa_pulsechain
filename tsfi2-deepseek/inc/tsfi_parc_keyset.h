#ifndef TSFI_PARC_KEYSET_H
#define TSFI_PARC_KEYSET_H

#include <stdint.h>

/* Decode 5-bit chord keyboard state (1 = pressed, 0 = open) into standard ASCII char */
char tsfi_parc_decode_keyset(uint8_t chord_state);

#endif // TSFI_PARC_KEYSET_H
