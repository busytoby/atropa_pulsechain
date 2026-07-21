#ifndef TSFI_NAUR_ENGINE_H
#define TSFI_NAUR_ENGINE_H

#include <stdint.h>
#include <stddef.h>

#define MAX_BNF_RULES 32
#define MAX_SYMBOL_LEN 64
#define NUM_NAUR_QUESTIONS 10

typedef struct {
    uint32_t question_id;
    char topic[64];
    char question_text[128];
    int status_affirmed;
} tsfi_naur_question_item_t;

typedef struct {
    char lhs_nonterminal[MAX_SYMBOL_LEN];
    char rhs_productions[4][MAX_SYMBOL_LEN];
    size_t production_count;
} tsfi_naur_bnf_rule_t;

typedef struct {
    uint32_t naur_id;
    tsfi_naur_bnf_rule_t bnf_rules[MAX_BNF_RULES];
    size_t rule_count;
    tsfi_naur_question_item_t questionnaire[NUM_NAUR_QUESTIONS];
    uint32_t gier_stack_pointer;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_naur_engine_t;

/* Initialize Peter Naur BNF & GIER ALGOL Engine */
int tsfi_naur_engine_init(
    uint32_t naur_id,
    tsfi_naur_engine_t *engine
);

/* Add BNF syntax rule definition (<nonterminal> ::= <prod1> | <prod2>) */
int tsfi_naur_add_bnf_rule(
    tsfi_naur_engine_t *engine,
    const char *lhs,
    const char *rhs_expr
);

/* Evaluate token stream against registered Peter Naur BNF grammar rules */
int tsfi_naur_validate_syntax(
    tsfi_naur_engine_t *engine,
    const char *start_symbol,
    const char *token_stream,
    int *out_valid
);

/* Evaluate Peter Naur ALGOL Bulletin No. 14 Questionnaire (10-Point Governance Audit) */
int tsfi_naur_eval_questionnaire(
    tsfi_naur_engine_t *engine,
    uint32_t *out_affirmed_count
);

/* Simulate GIER ALGOL multi-pass compiler stack frame allocation */
int tsfi_naur_gier_alloc_frame(
    tsfi_naur_engine_t *engine,
    size_t vars_count,
    uint32_t *out_frame_addr
);

#endif // TSFI_NAUR_ENGINE_H
