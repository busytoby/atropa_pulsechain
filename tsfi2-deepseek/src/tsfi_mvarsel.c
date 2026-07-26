#include "tsfi_mvarsel.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void tsfi_mvarsel_init(TSFiMVarSel *sel) {
    if (!sel) return;
    sel->variant_index = MVARSEL_GOLD;
}

void tsfi_mvarsel_set_index(TSFiMVarSel *sel, uint32_t idx) {
    if (!sel) return;
    if (idx < MVARSEL_COUNT) {
        sel->variant_index = idx;
    }
}

float tsfi_mvarsel_resolve_phase(const TSFiMVarSel *sel) {
    if (!sel) return 0.0f;
    return ((float)sel->variant_index * (float)M_PI) / 3.0f;
}

const char* tsfi_mvarsel_get_descriptor(const TSFiMVarSel *sel) {
    if (!sel) return "UNKNOWN";
    switch (sel->variant_index) {
        case MVARSEL_GOLD: return "GOLD";
        case MVARSEL_CLAY: return "CLAY";
        case MVARSEL_CLOTH: return "CLOTH";
        default: return "UNKNOWN";
    }
}

void tsfi_mvarsel_update_from_hogan_market(TSFiMVarSel *sel, double market_price, double tax_rate) {
    if (!sel) return;
    if (market_price > 1000.0) {
        sel->variant_index = MVARSEL_GOLD;
    } else if (tax_rate > 0.20) {
        sel->variant_index = MVARSEL_CLAY;
    } else {
        sel->variant_index = MVARSEL_CLOTH;
    }
}
