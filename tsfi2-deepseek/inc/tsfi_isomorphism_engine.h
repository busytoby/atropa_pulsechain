#ifndef TSFI_ISOMORPHISM_ENGINE_H
#define TSFI_ISOMORPHISM_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    ISO_DOMAIN_YUL_VM = 1,
    ISO_DOMAIN_AUTODIN_HEADER,
    ISO_DOMAIN_LISSAJOUS_GEOM,
    ISO_DOMAIN_SCSI_WINCHESTER
} tsfi_iso_domain_t;

/* Translate Yul contract slot index to tape header offset using Yul-compiled contract resolution */
int tsfi_iso_yul_to_tape_offset(uint32_t slot_id);

/* Resolve WinchesterMQ SCSI register keycode from compiled Yul mapping rules */
uint8_t tsfi_iso_scsi_keycode(uint8_t reg_id);

/* Map EDO-22 octave step to frequency ratio using Yul contract math */
uint32_t tsfi_iso_edo22_frequency(uint32_t step);

#endif // TSFI_ISOMORPHISM_ENGINE_H
