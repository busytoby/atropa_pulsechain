#ifndef TSFI_PARC_INGERMAN_SYNTAX_H
#define TSFI_PARC_INGERMAN_SYNTAX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_INGERMAN_MAX_PRODUCTIONS 32
#define TSFI_INGERMAN_MAX_PUSHDOWN_STACK 64

typedef struct {
    char rule_name[32];
    char pattern_token[32];
    uint32_t action_id;
    char target_contract[64];
} tsfi_ingerman_syntax_rule_t;

typedef struct {
    bool is_initialized;
    uint32_t rule_count;
    tsfi_ingerman_syntax_rule_t rules[TSFI_INGERMAN_MAX_PRODUCTIONS];
    uint32_t pushdown_stack[TSFI_INGERMAN_MAX_PUSHDOWN_STACK];
    int stack_top;
    char dynamic_contract_router[64];
} tsfi_ingerman_syntax_compiler_t;

/* Initialize Peter Z. Ingerman 1966 Syntax-Directed Compiler Machine */
int tsfi_ingerman_syntax_init(tsfi_ingerman_syntax_compiler_t *compiler);

/* Register Syntax Production Rule */
int tsfi_ingerman_syntax_register_rule(tsfi_ingerman_syntax_compiler_t *compiler, const char *rule_name, const char *token, uint32_t action_id);

/* Syntax-Directed Parse and Pushdown Reduction Step */
int tsfi_ingerman_syntax_parse_token(tsfi_ingerman_syntax_compiler_t *compiler, const char *token, uint32_t *action_out);

#endif // TSFI_PARC_INGERMAN_SYNTAX_H
