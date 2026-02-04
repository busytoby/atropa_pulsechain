#include "tsfi_logic.h"
#include <stddef.h>

static void stub_epoch(int *ver) { if(ver) (*ver)++; }
static void stub_state(bool *ftw) { (void)ftw; }
static void stub_directive(int *cnt, char *dir) { (void)cnt; (void)dir; }
static void stub_scramble(void *ws) { (void)ws; }
static void stub_provenance(void *ws) { (void)ws; }

static const TSFiLogicTable default_table = {
    .logic_epoch = stub_epoch,
    .logic_state = stub_state,
    .logic_directive = stub_directive,
    .logic_scramble = stub_scramble,
    .logic_provenance = stub_provenance
};

const TSFiLogicTable* tsfi_get_default_logic(void) {
    return &default_table;
}
