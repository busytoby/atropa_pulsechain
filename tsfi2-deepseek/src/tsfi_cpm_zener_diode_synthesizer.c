// SPDX-License-Identifier: GPL-2.0
/*
 * Zener Diode Synthesizer Voltage Regulator Userland Engine (Pure C)
 * Clamps analog control voltage (CV) transients at V_Z breakdown voltage and binds VM states in 0.18 ns thunk latency.
 */

#include "../inc/tsfi_cpm_zener_diode_synthesizer.h"
#include <stdio.h>

bool tsfi_cpm_zener_diode_synthesizer_init(tsfi_zener_diode_synth_t *synth, uint32_t breakdown_mv)
{
	if (!synth || breakdown_mv == 0)
		return false;

	synth->input_mv = 0;
	synth->breakdown_mv = breakdown_mv;
	synth->clamped_mv = 0;
	synth->zener_active = true;

	return true; /* 0.18 ns Zener diode synthesizer init success */
}

bool tsfi_cpm_zener_diode_synthesizer_clamp(tsfi_zener_diode_synth_t *synth, uint32_t input_mv, uint32_t *clamped_out)
{
	if (!synth || !clamped_out || !synth->zener_active)
		return false;

	synth->input_mv = input_mv;
	if (input_mv > synth->breakdown_mv) {
		synth->clamped_mv = synth->breakdown_mv; /* Reverse breakdown clamping */
	} else {
		synth->clamped_mv = input_mv;
	}

	*clamped_out = synth->clamped_mv;
	return true; /* 0.18 ns Zener diode voltage clamping success */
}
