#include "tsfi_lc_tank.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


void tsfi_distributed_coil_init(
    TsfiDistributedCoil *coil,
    double diameter,
    double height,
    double turns,
    double wire_gauge_mm,
    double C_tune
) {
    if (!coil) return;
    (void)wire_gauge_mm; // Suppress unused parameter

    // Define physical spatial dimensions for FDTD cell sizes:
    // Radius of simulation domain = outer coil radius + 5 cm boundary margin
    double max_r = (diameter / 2.0) + 0.05;
    // Length of simulation domain = coil height + 5 cm margin on each side
    double max_z = height + 0.10;

    coil->dr = max_r / GRID_R;
    coil->dz = max_z / GRID_Z;
    coil->C_tune = C_tune;
    coil->v_tuning_node = 0.0;
    coil->i_tuning_loop = 0.0;

    double eps0 = 8.8541878128e-12;

    // Initialize EM grids to zero
    for (int r = 0; r < GRID_R; r++) {
        for (int z = 0; z < GRID_Z; z++) {
            coil->Er[r][z] = 0.0;
            coil->Ez[r][z] = 0.0;
            coil->Hphi[r][z] = 0.0;

            // Set air properties by default
            coil->eps[r][z] = eps0;
            coil->sigma[r][z] = 0.0;
        }
    }

    // Map the copper winding boundary into the FDTD conductivity tensor
    // Turns are modeled as discrete rings spaced uniformly along the height of the coil form
    double start_z = 0.05; // 5 cm margin start
    double end_z = start_z + height;
    double turn_spacing = height / turns;

    // Convert coil radius to grid index
    int coil_r_idx = (int)((diameter / 2.0) / coil->dr);
    if (coil_r_idx >= GRID_R) coil_r_idx = GRID_R - 2;

    // Copper conductivity = 5.96e7 S/m
    double sigma_copper = 5.96e7;
    
    // Map turns
    for (int n = 0; n < (int)turns; n++) {
        double turn_z = start_z + n * turn_spacing;
        int turn_z_idx = (int)(turn_z / coil->dz);
        if (turn_z_idx < GRID_Z) {
            // Apply copper conductivity to cells matching the winding radius
            coil->sigma[coil_r_idx][turn_z_idx] = sigma_copper;
        }
    }

    // Map the lossy Bakelite form (tan delta = 0.02, eps_r = 3.5) inside the coil radius
    double eps_form = 3.5 * eps0;
    double loss_tangent = 0.02;
    double form_sigma = 2.0 * M_PI * 1.0e6 * eps_form * loss_tangent; // at 1 MHz reference

    for (int r = 0; r < coil_r_idx; r++) {
        for (int z = (int)(start_z / coil->dz); z < (int)(end_z / coil->dz); z++) {
            if (z < GRID_Z) {
                coil->eps[r][z] = eps_form;
                coil->sigma[r][z] = form_sigma;
            }
        }
    }

    // 5. Perfectly Matched Layer (PML) Absorbing Boundaries at grid edges
    // Quadratic absorption gradient to prevent boundary reflections
    double pml_sigma_max = 5.0; // max conductivity at boundary wall
    int pml_cells = 4;          // 4 cells depth PML layer
    
    // Radial PML boundary (outer radius)
    for (int r = GRID_R - pml_cells; r < GRID_R; r++) {
        double factor = (double)(r - (GRID_R - pml_cells)) / pml_cells;
        double pml_val = pml_sigma_max * factor * factor;
        for (int z = 0; z < GRID_Z; z++) {
            if (pml_val > coil->sigma[r][z]) {
                coil->sigma[r][z] = pml_val;
            }
        }
    }

    // Axial PML boundaries (bottom and top)
    for (int z = 0; z < pml_cells; z++) {
        double factor = (double)(pml_cells - z) / pml_cells;
        double pml_val = pml_sigma_max * factor * factor;
        for (int r = 0; r < GRID_R; r++) {
            if (pml_val > coil->sigma[r][z]) {
                coil->sigma[r][z] = pml_val;
            }
        }
    }
    for (int z = GRID_Z - pml_cells; z < GRID_Z; z++) {
        double factor = (double)(z - (GRID_Z - pml_cells)) / pml_cells;
        double pml_val = pml_sigma_max * factor * factor;
        for (int r = 0; r < GRID_R; r++) {
            if (pml_val > coil->sigma[r][z]) {
                coil->sigma[r][z] = pml_val;
            }
        }
    }
}

void tsfi_distributed_coil_process(
    TsfiDistributedCoil *coil,
    const float *input_rf,
    float *output_grid,
    size_t count,
    double sample_rate,
    double current_freq_hz
) {
    if (!coil || !input_rf || !output_grid || count == 0) return;
    (void)current_freq_hz; // Suppress unused parameter

    double dt = 1.0 / sample_rate;
    double mu0 = 4.0 * M_PI * 1e-7;

    double dr = coil->dr;
    double dz = coil->dz;

    // Process sample by sample
    for (size_t step = 0; step < count; step++) {
        double v_in = (double)input_rf[step];

        // 1. Inject input voltage at the bottom boundary of the coil winding
        int source_z_idx = (int)(0.05 / dz);
        int coil_r_idx = (int)((GRID_R - 2) * 0.8);
        coil->Ez[coil_r_idx][source_z_idx] += v_in / dz;

        // 2. FDTD Update Equations for Cylindrical coordinates
        // Update Hphi (magnetic field curl):
        // d(Hphi)/dt = (1/mu) * (d(Er)/dz - d(Ez)/dr)
        for (int r = 0; r < GRID_R - 1; r++) {
            for (int z = 0; z < GRID_Z - 1; z++) {
                double dEr_dz = (coil->Er[r][z + 1] - coil->Er[r][z]) / dz;
                double dEz_dr = (coil->Ez[r + 1][z] - coil->Ez[r][z]) / dr;
                coil->Hphi[r][z] += (dt / mu0) * (dEr_dz - dEz_dr);
            }
        }

        // Update E-fields incorporating conductive boundaries (sigma)
        // d(Er)/dt = (1/eps) * (-d(Hphi)/dz - sigma * Er)
        for (int r = 0; r < GRID_R; r++) {
            for (int z = 1; z < GRID_Z - 1; z++) {
                double dHphi_dz = (coil->Hphi[r][z] - coil->Hphi[r][z - 1]) / dz;
                double cond_term = coil->sigma[r][z] * coil->Er[r][z];
                coil->Er[r][z] += (dt / coil->eps[r][z]) * (-dHphi_dz - cond_term);
            }
        }

        // d(Ez)/dt = (1/eps) * ((1/r) * d(r * Hphi)/dr - sigma * Ez)
        for (int r = 1; r < GRID_R - 1; r++) {
            for (int z = 0; z < GRID_Z; z++) {
                double r_val = r * dr;
                double d_rHphi_dr = ((r_val + dr) * coil->Hphi[r + 1][z] - r_val * coil->Hphi[r][z]) / dr;
                double cond_term = coil->sigma[r][z] * coil->Ez[r][z];
                coil->Ez[r][z] += (dt / coil->eps[r][z]) * ((1.0 / r_val) * d_rHphi_dr - cond_term);
            }
        }

        // 3. Lumped Tuning Capacitor Boundary Condition at top node
        int load_z_idx = (int)((0.05 + 0.10) / dz);
        if (load_z_idx >= GRID_Z) load_z_idx = GRID_Z - 2;

        // Current flows through lumped capacitor: d(v_tune)/dt = i_loop / C_tune
        coil->i_tuning_loop += (coil->Ez[coil_r_idx][load_z_idx] * dz - coil->v_tuning_node) * dt / (mu0 * dr);
        coil->v_tuning_node += (coil->i_tuning_loop / coil->C_tune) * dt;

        // Apply tuning potential back as a boundary load on Ez
        coil->Ez[coil_r_idx][load_z_idx] = coil->v_tuning_node / dz;

        // Output grid node potential (audion feed) is tapped at the top of the winding
        output_grid[step] = (float)coil->v_tuning_node;
    }
}