#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_thin_film.h"

#define PI 3.14159265358979323846f

int main() {
    printf("=== TSFi2 & Computer Design Issue #8 Thin-Film Memory Cell Test ===\n");

    TsfiThinFilmCell cell;
    tsfi_thin_film_init(&cell);

    // 1. Initial check
    printf("[THIN-FILM] Initial: phi = %f, state = %d\n", cell.phi, cell.state);
    assert(cell.state == 0);
    assert(fabs(cell.phi - PI) < 1e-5);

    // 2. Applying only longitudinal field (half-select/disturb test)
    printf("[THIN-FILM] Applying only longitudinal field H_L = 0.5 without transverse field...\n");
    int state_disturb = tsfi_thin_film_update(&cell, 0.5f, 0.0f);
    assert(state_disturb == 0);
    assert(fabs(cell.phi - PI) < 1e-5);

    // 3. Applying transverse field (H_T = 1.5) to pull to hard axis, plus longitudinal H_L = 0.5 to tilt towards 1
    printf("[THIN-FILM] Applying transverse H_T = 1.5 and longitudinal H_L = 0.5 (Write 1)...\n");
    tsfi_thin_film_update(&cell, 0.5f, 1.5f);
    // Expected phi: PI/2 - 0.5 = 1.570796 - 0.5 = 1.070796
    printf("[THIN-FILM] Transverse applied: phi = %f\n", cell.phi);
    assert(fabs(cell.phi - (PI / 2.0f - 0.5f)) < 1e-5);

    // 4. Remove fields, vector should snap to easy-axis state 1
    printf("[THIN-FILM] Removing fields: snapping to easy-axis (State 1)...\n");
    int state_snap1 = tsfi_thin_film_update(&cell, 0.0f, 0.0f);
    printf("[THIN-FILM] Snapped: phi = %f, state = %d\n", cell.phi, cell.state);
    assert(state_snap1 == 1);
    assert(cell.state == 1);
    assert(fabs(cell.phi - 0.0f) < 1e-5);

    // 5. Applying only longitudinal field H_L = -0.5 to state 1 without transverse field (half-select)
    printf("[THIN-FILM] Applying only longitudinal field H_L = -0.5 on state 1 without transverse field...\n");
    int state_disturb2 = tsfi_thin_film_update(&cell, -0.5f, 0.0f);
    assert(state_disturb2 == 1);
    assert(fabs(cell.phi - 0.0f) < 1e-5);

    // 6. Write 0: Apply transverse H_T = 1.5 and longitudinal H_L = -0.5
    printf("[THIN-FILM] Applying transverse H_T = 1.5 and longitudinal H_L = -0.5 (Write 0)...\n");
    tsfi_thin_film_update(&cell, -0.5f, 1.5f);
    // Expected phi: PI/2 + 0.5 = 1.570796 + 0.5 = 2.070796
    printf("[THIN-FILM] Transverse applied: phi = %f\n", cell.phi);
    assert(fabs(cell.phi - (PI / 2.0f + 0.5f)) < 1e-5);

    // 7. Remove fields, vector should snap to easy-axis state 0
    printf("[THIN-FILM] Removing fields: snapping to easy-axis (State 0)...\n");
    int state_snap0 = tsfi_thin_film_update(&cell, 0.0f, 0.0f);
    printf("[THIN-FILM] Snapped: phi = %f, state = %d\n", cell.phi, cell.state);
    assert(state_snap0 == 0);
    assert(cell.state == 0);
    assert(fabs(cell.phi - PI) < 1e-5);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_thin_film", NULL);

    printf("[PASS] Thin-Film coherent rotation physics simulation verified.\n");
    return 0;
}
