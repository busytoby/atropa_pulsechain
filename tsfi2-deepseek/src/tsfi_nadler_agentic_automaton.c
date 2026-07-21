#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_nadler_agentic_automaton.h"
#include "tsfi_lowpower_fet.h"

int tsfi_nadler_agentic_step(
    uint16_t input_pixel,
    tsfi_nadler_agent_summary_t *out_summary
) {
    if (!out_summary) return -1;

    // 1. Perception Step: Accumulate input stream
    out_summary->current_state = NADLER_AGENT_STATE_PERCEIVE;
    out_summary->accumulated_perception_word = (out_summary->accumulated_perception_word << 8) | (input_pixel & 0xFF);

    // 2. Decision Step: Evaluate threshold state transition
    out_summary->current_state = NADLER_AGENT_STATE_DECIDE;
    if ((input_pixel & 0xFF) > 128) {
        out_summary->current_state = NADLER_AGENT_STATE_ACT;
        out_summary->agent_actions_taken++;
    }

    // 3. Discharge Physics Step (Rule 10: 78.2% Power Cut)
    out_summary->current_state = NADLER_AGENT_STATE_DISCHARGE;
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    out_summary->fet_discharge_watts = (double)fet_metrics.optimized_power_watts;

    // 4. Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(out_summary->tape_dat_bin, sizeof(out_summary->tape_dat_bin),
             "NADLER_AGENT_%08X.DAT.BIN", (unsigned int)(out_summary->accumulated_perception_word & 0xFFFFFFFF));

    printf("[NADLER AGENT] State: DISCHARGE | Perception Word: 0x%016llX | Actions: %u | FET Power: %.4fW | File: %s\n",
           (unsigned long long)out_summary->accumulated_perception_word, out_summary->agent_actions_taken,
           out_summary->fet_discharge_watts, out_summary->tape_dat_bin);

    return 0; // Agentic step complete
}
