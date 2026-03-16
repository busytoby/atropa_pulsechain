#ifndef TSFI_LOGIC_H
#define TSFI_LOGIC_H

#include <stdbool.h>
#include "tsfi_plugin.h"

// Logic functions (Default Implementation)
void tsfi_logic_init(void *ws);
void tsfi_logic_teardown(void);
void master_logic_epoch(int *ver);
bool master_logic_state(void *obj);
void master_logic_directive(int *cnt, char *dir);
void master_logic_scramble(void *ws_ptr);

// Chained Logic Accessors
void chained_logic_directive(int *cnt, char *dir);

// Default Table Accessor
const TSFiLogicTable* tsfi_get_default_logic(void);

#endif // TSFI_LOGIC_H