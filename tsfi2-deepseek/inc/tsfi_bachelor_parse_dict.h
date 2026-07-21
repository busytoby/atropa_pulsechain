#ifndef TSFI_BACHELOR_PARSE_DICT_H
#define TSFI_BACHELOR_PARSE_DICT_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char symbol[32];                   // Symbol Identifier Name
    uint32_t token_id;                 // SMALGOL Token ID
    uint32_t address_offset;           // ZMM Register Memory Address
} tsfi_bachelor_symbol_t;

typedef struct {
    uint32_t parse_id;                 // Parser ID
    tsfi_bachelor_symbol_t dict[16];   // Compact Symbol Dictionary
    size_t symbol_count;               // Symbol Count
    uint32_t evm_gas_units;             // 240 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_bachelor_parse_dict_t;

/* Initialize Bachelor SMALGOL-61 Parse Table & Symbol Dictionary */
int tsfi_bachelor_parse_dict_init(
    uint32_t parse_id,
    tsfi_bachelor_parse_dict_t *parser
);

/* Insert symbol into Bachelor compact lexicon dictionary */
int tsfi_bachelor_insert_symbol(
    tsfi_bachelor_parse_dict_t *parser,
    const char *symbol,
    uint32_t token_id,
    uint32_t address_offset
);

/* Execute Bachelor syntax-directed state reduction and lookup */
int tsfi_bachelor_parse_reduction(
    const tsfi_bachelor_parse_dict_t *parser,
    const char *target_symbol,
    uint32_t *out_address_offset
);

#endif // TSFI_BACHELOR_PARSE_DICT_H
