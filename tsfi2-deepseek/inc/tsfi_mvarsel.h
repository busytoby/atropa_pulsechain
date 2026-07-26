#ifndef TSFI_MVARSEL_H
#define TSFI_MVARSEL_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MVARSEL_GOLD,
    MVARSEL_CLAY,
    MVARSEL_CLOTH,
    MVARSEL_COUNT
} MVarSelType;

// Material Variant Selector (MVarSel) context
typedef struct {
    uint32_t variant_index;
} TSFiMVarSel;

// Initialize the variant selector
void tsfi_mvarsel_init(TSFiMVarSel *sel);

// Update variant selection index
void tsfi_mvarsel_set_index(TSFiMVarSel *sel, uint32_t idx);

// Resolves the rendering phase displacement vector mapping within Lissajous space
float tsfi_mvarsel_resolve_phase(const TSFiMVarSel *sel);

// Retrieves the variant descriptor string based on current index
const char* tsfi_mvarsel_get_descriptor(const TSFiMVarSel *sel);

#endif // TSFI_MVARSEL_H
