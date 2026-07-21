#include "tsfi_parc_ingerman_syntax.h"
#include <stdio.h>
#include <string.h>

int tsfi_ingerman_syntax_init(tsfi_ingerman_syntax_compiler_t *compiler) {
    if (!compiler) return -1;
    memset(compiler, 0, sizeof(tsfi_ingerman_syntax_compiler_t));
    compiler->is_initialized = true;
    compiler->stack_top = -1;
    snprintf(compiler->dynamic_contract_router, sizeof(compiler->dynamic_contract_router),
             "dynamic_0x53796e746178436f6d70696c657231393636564d");
    return 0;
}

int tsfi_ingerman_syntax_register_rule(tsfi_ingerman_syntax_compiler_t *compiler, const char *rule_name, const char *token, uint32_t action_id) {
    if (!compiler || !rule_name || !token) return -1;
    if (compiler->rule_count >= TSFI_INGERMAN_MAX_PRODUCTIONS) return -2;

    tsfi_ingerman_syntax_rule_t *rule = &compiler->rules[compiler->rule_count];
    snprintf(rule->rule_name, sizeof(rule->rule_name), "%s", rule_name);
    snprintf(rule->pattern_token, sizeof(rule->pattern_token), "%s", token);
    rule->action_id = action_id;
    snprintf(rule->target_contract, sizeof(rule->target_contract),
             "dynamic_0x52756c65%04x", action_id);

    compiler->rule_count++;
    return 0;
}

int tsfi_ingerman_syntax_parse_token(tsfi_ingerman_syntax_compiler_t *compiler, const char *token, uint32_t *action_out) {
    if (!compiler || !token || !action_out) return -1;

    for (uint32_t i = 0; i < compiler->rule_count; i++) {
        if (strcmp(compiler->rules[i].pattern_token, token) == 0) {
            *action_out = compiler->rules[i].action_id;

            // Push state to Pushdown Automata Stack
            if (compiler->stack_top < TSFI_INGERMAN_MAX_PUSHDOWN_STACK - 1) {
                compiler->stack_top++;
                compiler->pushdown_stack[compiler->stack_top] = compiler->rules[i].action_id;
            }

            return 0; // Matched syntax rule
        }
    }

    *action_out = 0;
    return -1; // Syntax error / unknown token
}
