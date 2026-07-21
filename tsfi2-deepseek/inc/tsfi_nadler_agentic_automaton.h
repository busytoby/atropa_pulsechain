#ifndef TSFI_NADLER_AGENTIC_AUTOMATON_H
#define TSFI_NADLER_AGENTIC_AUTOMATON_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    NADLER_AGENT_STATE_PERCEIVE  = 0,
    NADLER_AGENT_STATE_DECIDE    = 1,
    NADLER_AGENT_STATE_ACT       = 2,
    NADLER_AGENT_STATE_DISCHARGE = 3
} tsfi_nadler_agent_state_t;

typedef struct {
    tsfi_nadler_agent_state_t current_state;
    uint64_t accumulated_perception_word;
    double fet_discharge_watts;
    uint32_t agent_actions_taken;
    char tape_dat_bin[128]; // Rule 13 Filename (.DAT.BIN)
} tsfi_nadler_agent_summary_t;

/* Step the Morton Nadler Agentic Automaton state machine */
int tsfi_nadler_agentic_step(
    uint16_t input_pixel,
    tsfi_nadler_agent_summary_t *out_summary
);

#endif // TSFI_NADLER_AGENTIC_AUTOMATON_H
