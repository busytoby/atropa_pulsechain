#ifndef TSFI_NADLER_SYNTACTIC_PARSER_H
#define TSFI_NADLER_SYNTACTIC_PARSER_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    NADLER_STROKE_STEM = 0, // Vertical / Diagonal Line
    NADLER_STROKE_BAR  = 1, // Horizontal Bar
    NADLER_STROKE_ARC  = 2, // Curved Arc
    NADLER_STROKE_LOOP = 3  // Enclosed Loop
} tsfi_nadler_stroke_type_t;

typedef struct {
    uint32_t parsed_rules_count;   // Count of matched syntactic rules
    uint16_t primary_character_id; // Resolved Character ASCII Code
    uint32_t evm_gas_cost;         // 1,000 Gas Cost per Syntactic Graph
    double fet_power_watts;        // Power in Watts under 78.2% FET Drop
    char tape_dat_bin[128];        // Rule 13 Filename (.DAT.BIN)
} tsfi_nadler_syntax_summary_t;

/* Parse primitive stroke sequence using Nadler Contextual Production Rules */
int tsfi_nadler_parse_syntactic_strokes(
    const tsfi_nadler_stroke_type_t *strokes,
    size_t stroke_count,
    tsfi_nadler_syntax_summary_t *out_summary
);

#endif // TSFI_NADLER_SYNTACTIC_PARSER_H
