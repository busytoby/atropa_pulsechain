#ifndef TSFI_PARC_INGERMAN_TRANSLATOR_H
#define TSFI_PARC_INGERMAN_TRANSLATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define INGERMAN_MAX_PRODUCTION_RULES 32
#define INGERMAN_MAX_SYMBOL_LENGTH    32
#define INGERMAN_MAX_THUNKS           16

/* Ingerman Call-By-Name Thunk Metadata */
typedef struct {
    uint32_t thunk_id;
    char expression_text[64];
    uint64_t (*thunk_evaluator_fn)(void *environment);
    void *environment_ptr;
    uint64_t cached_value;
    bool is_evaluated;
} tsfi_ingerman_thunk_t;

/* Ingerman Syntax-Oriented Production Rule Element */
typedef struct {
    char non_terminal[INGERMAN_MAX_SYMBOL_LENGTH];
    char production_pattern[64];
    char target_yul_template[128];
    uint32_t rule_id;
} tsfi_ingerman_rule_t;

/* Ingerman Syntax-Oriented Translator State Machine */
typedef struct {
    bool is_initialized;
    uint32_t rule_count;
    tsfi_ingerman_rule_t rules[INGERMAN_MAX_PRODUCTION_RULES];
    uint32_t thunk_count;
    tsfi_ingerman_thunk_t thunks[INGERMAN_MAX_THUNKS];
    char dynamic_contract_address[64];
} tsfi_ingerman_translator_t;

/* Initialize Ingerman Syntax-Oriented Translator */
int tsfi_ingerman_translator_initialize(tsfi_ingerman_translator_t *translator);

/* Add Syntax-Directed Production Rule (Ingerman 1966) */
int tsfi_ingerman_add_production_rule(tsfi_ingerman_translator_t *translator, const char *non_terminal, const char *pattern, const char *yul_template);

/* Compile Syntax Input to Target Yul Code & Dynamic Contract ZMM State */
int tsfi_ingerman_translate_syntax(tsfi_ingerman_translator_t *translator, const char *input_source, char *output_yul_code, size_t max_out_len);

/* Register Ingerman Call-By-Name Thunk (Ingerman 1961) */
int tsfi_ingerman_register_thunk(tsfi_ingerman_translator_t *translator, const char *expression, uint64_t (*eval_fn)(void*), void *env, uint32_t *thunk_id_out);

/* Evaluate Call-By-Name Thunk with Environment Context */
uint64_t tsfi_ingerman_evaluate_thunk(tsfi_ingerman_translator_t *translator, uint32_t thunk_id);

#endif // TSFI_PARC_INGERMAN_TRANSLATOR_H
