/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_ZENER_DIODE_SYNTHESIZER_H
#define TSFI_CPM_ZENER_DIODE_SYNTHESIZER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t input_mv;
	uint32_t breakdown_mv;
	uint32_t clamped_mv;
	bool zener_active;
} tsfi_zener_diode_synth_t;

bool tsfi_cpm_zener_diode_synthesizer_init(tsfi_zener_diode_synth_t *synth, uint32_t breakdown_mv);
bool tsfi_cpm_zener_diode_synthesizer_clamp(tsfi_zener_diode_synth_t *synth, uint32_t input_mv, uint32_t *clamped_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_ZENER_DIODE_SYNTHESIZER_H */
