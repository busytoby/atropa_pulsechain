#include "tsfi_thin_film.h"
#include <string.h>

#define PI 3.14159265358979323846f

void tsfi_thin_film_init(TsfiThinFilmCell *cell) {
    memset(cell, 0, sizeof(TsfiThinFilmCell));
    cell->phi = PI; // Initial easy axis state 0
    cell->state = 0;
}

int tsfi_thin_film_update(TsfiThinFilmCell *cell, float H_longitudinal, float H_transverse) {
    if (H_transverse > 1.0f) {
        // Hard-axis transverse field applies torque, rotating vector to hard axis (PI/2)
        cell->phi = PI / 2.0f;
        // Longitudinal easy-axis field tilts the vector slightly towards the target state
        if (H_longitudinal > 0.1f) {
            cell->phi -= 0.5f; // Tilt toward 0 (Logic '1')
        } else if (H_longitudinal < -0.1f) {
            cell->phi += 0.5f; // Tilt toward PI (Logic '0')
        }
    } else {
        // Transverse field removed: snaps back to nearest stable easy-axis minimum
        if (cell->phi < PI / 2.0f) {
            cell->phi = 0.0f;
            cell->state = 1;
        } else {
            cell->phi = PI;
            cell->state = 0;
        }
    }

    return cell->state;
}
