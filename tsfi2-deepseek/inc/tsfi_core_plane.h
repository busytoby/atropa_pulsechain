#ifndef TSFI_CORE_PLANE_H
#define TSFI_CORE_PLANE_H

#include "tsfi_dtl_core.h"

typedef struct {
    TsfiMagneticCore cores[8][8];
} TsfiCorePlane;

void tsfi_core_plane_init(TsfiCorePlane *plane);
void tsfi_core_plane_write(TsfiCorePlane *plane, int x, int y, int bit);
void tsfi_core_plane_write_inhibited(TsfiCorePlane *plane, int x, int y, int bit_to_write);
int tsfi_core_plane_read_destructive(TsfiCorePlane *plane, int x, int y);

#endif /* TSFI_CORE_PLANE_H */
