#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_bachelor_parse_dict.h"
#include "tsfi_lowpower_fet.h"

int tsfi_bachelor_parse_dict_init(
    uint32_t parse_id,
    tsfi_bachelor_parse_dict_t *parser
) {
    if (!parser) return -1;
    memset(parser, 0, sizeof(tsfi_bachelor_parse_dict_t));

    parser->parse_id = parse_id;
    parser->evm_gas_units = 240; // 240 Gas / Auncient Ether Units per parse reduction

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    parser->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(parser->tape_dat_bin, sizeof(parser->tape_dat_bin), "BACHELOR_PARSE_DICT_%08X.DAT.BIN", parse_id);
    return 0;
}

int tsfi_bachelor_insert_symbol(
    tsfi_bachelor_parse_dict_t *parser,
    const char *symbol,
    uint32_t token_id,
    uint32_t address_offset
) {
    if (!parser || !symbol || parser->symbol_count >= 16) return -1;

    tsfi_bachelor_symbol_t *entry = &parser->dict[parser->symbol_count++];
    strncpy(entry->symbol, symbol, sizeof(entry->symbol) - 1);
    entry->token_id = token_id;
    entry->address_offset = address_offset;

    return 0;
}

int tsfi_bachelor_parse_reduction(
    const tsfi_bachelor_parse_dict_t *parser,
    const char *target_symbol,
    uint32_t *out_address_offset
) {
    if (!parser || !target_symbol || !out_address_offset) return -1;

    for (size_t i = 0; i < parser->symbol_count; i++) {
        if (strcmp(parser->dict[i].symbol, target_symbol) == 0) {
            *out_address_offset = parser->dict[i].address_offset;
            printf("[BACHELOR PARSER] Parse ID: %u | Symbol: %s -> Token: %u | Address: 0x%08X | Gas: %u | File: %s\n",
                   parser->parse_id, target_symbol, parser->dict[i].token_id, *out_address_offset, parser->evm_gas_units, parser->tape_dat_bin);
            return 0; // Found & Reduced
        }
    }

    return -1; // Symbol not found
}
