#ifndef TSFI_THIN_FILM_H
#define TSFI_THIN_FILM_H

#include <stdint.h>

typedef struct {
    float phi;  // Magnetization vector angle (radians, 0.0 to PI)
    int state;  // Binary state (0 or 1)
} TsfiThinFilmCell;

void tsfi_thin_film_init(TsfiThinFilmCell *cell);
int tsfi_thin_film_update(TsfiThinFilmCell *cell, float H_longitudinal, float H_transverse);

#endif /* TSFI_THIN_FILM_H */
