#include "tsfi_curveswrap.h"

void tsfi_curveswrap_init(TSFiCurvesWrap *cw, TSFiCurvesWrapType wrap_type) {
    if (!cw) return;
    cw->wrap_type = wrap_type;
}

int tsfi_curveswrap_resolve_index(const TSFiCurvesWrap *cw, int index, int count) {
    if (!cw || count <= 0) return 0;
    
    if (cw->wrap_type == TSFI_WRAP_PERIODIC) {
        // Enforce periodic wrapping modulo operations
        int resolved = index % count;
        if (resolved < 0) {
            resolved += count;
        }
        return resolved;
    } else {
        // Clamp indices for non-periodic wrap mode
        if (index < 0) return 0;
        if (index >= count) return count - 1;
        return index;
    }
}
