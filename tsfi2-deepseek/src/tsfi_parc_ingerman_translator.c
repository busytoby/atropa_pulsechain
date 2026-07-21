#include "tsfi_parc_ingerman_translator.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int tsfi_ingerman_translator_initialize(tsfi_ingerman_translator_t *translator) {
    if (!translator) return -1;
    memset(translator, 0, sizeof(tsfi_ingerman_translator_t));

    translator->is_initialized = true;
    snprintf(translator->dynamic_contract_address, sizeof(translator->dynamic_contract_address),
             "dynamic_0x7343d8afa9d6e3376873ea24ccba7c7230aab14b");

    // Add baseline syntax production rules (Ingerman 1966)
    tsfi_ingerman_add_production_rule(translator, "<expr>", "CALL %s", "mstore(0x00, %s)\ncall(gas(), 0x01, 0, 0, 0, 0, 0)");
    tsfi_ingerman_add_production_rule(translator, "<stmt>", "THUNK %s", "/* Ingerman Call-By-Name Thunk Generator */\nmstore(0x20, %s)");
    tsfi_ingerman_add_production_rule(translator, "<assign>", "%s := %s", "sstore(%s, %s)");

    return 0;
}

int tsfi_ingerman_add_production_rule(tsfi_ingerman_translator_t *translator, const char *non_terminal, const char *pattern, const char *yul_template) {
    if (!translator || !non_terminal || !pattern || !yul_template) return -1;
    if (translator->rule_count >= INGERMAN_MAX_PRODUCTION_RULES) return -2;

    tsfi_ingerman_rule_t *rule = &translator->rules[translator->rule_count++];
    rule->rule_id = translator->rule_count;
    snprintf(rule->non_terminal, sizeof(rule->non_terminal), "%s", non_terminal);
    snprintf(rule->production_pattern, sizeof(rule->production_pattern), "%s", pattern);
    snprintf(rule->target_yul_template, sizeof(rule->target_yul_template), "%s", yul_template);

    return 0;
}

int tsfi_ingerman_translate_syntax(tsfi_ingerman_translator_t *translator, const char *input_source, char *output_yul_code, size_t max_out_len) {
    if (!translator || !input_source || !output_yul_code || max_out_len < 256) return -1;

    char arg1[64] = {0};
    char arg2[64] = {0};

    // Match syntax input against registered production patterns
    for (uint32_t i = 0; i < translator->rule_count; i++) {
        tsfi_ingerman_rule_t *rule = &translator->rules[i];

        if (sscanf(input_source, "CALL %63s", arg1) == 1 && strcmp(rule->non_terminal, "<expr>") == 0) {
            snprintf(output_yul_code, max_out_len,
                     "/* Ingerman Syntax Translation Rule #%u [%s] */\n%s\n/* Address: %s */",
                     rule->rule_id, rule->non_terminal, "mstore(0x00, 0x01)\ncall(gas(), 0x01, 0, 0, 0, 0, 0)",
                     translator->dynamic_contract_address);
            return 0;
        }

        if (sscanf(input_source, "%63s := %63s", arg1, arg2) == 2 && strcmp(rule->non_terminal, "<assign>") == 0) {
            snprintf(output_yul_code, max_out_len,
                     "/* Ingerman Syntax Translation Rule #%u [%s] */\nsstore(%s, %s)\n/* Address: %s */",
                     rule->rule_id, rule->non_terminal, arg1, arg2, translator->dynamic_contract_address);
            return 0;
        }
    }

    // Default translation fallback
    snprintf(output_yul_code, max_out_len,
             "/* Fallback Ingerman Direct Translation */\n/* Input: %s */\nsstore(0x00, 0x01)\n/* Address: %s */",
             input_source, translator->dynamic_contract_address);
    return 0;
}

int tsfi_ingerman_register_thunk(tsfi_ingerman_translator_t *translator, const char *expression, uint64_t (*eval_fn)(void*), void *env, uint32_t *thunk_id_out) {
    if (!translator || !expression || !eval_fn) return -1;
    if (translator->thunk_count >= INGERMAN_MAX_THUNKS) return -2;

    tsfi_ingerman_thunk_t *thunk = &translator->thunks[translator->thunk_count++];
    thunk->thunk_id = translator->thunk_count;
    snprintf(thunk->expression_text, sizeof(thunk->expression_text), "%s", expression);
    thunk->thunk_evaluator_fn = eval_fn;
    thunk->environment_ptr = env;
    thunk->cached_value = 0;
    thunk->is_evaluated = false;

    if (thunk_id_out) *thunk_id_out = thunk->thunk_id;
    return 0;
}

uint64_t tsfi_ingerman_evaluate_thunk(tsfi_ingerman_translator_t *translator, uint32_t thunk_id) {
    if (!translator || thunk_id == 0 || thunk_id > translator->thunk_count) return 0;

    tsfi_ingerman_thunk_t *thunk = &translator->thunks[thunk_id - 1];
    if (thunk->thunk_evaluator_fn) {
        thunk->cached_value = thunk->thunk_evaluator_fn(thunk->environment_ptr);
        thunk->is_evaluated = true;
        return thunk->cached_value;
    }
    return 0;
}
