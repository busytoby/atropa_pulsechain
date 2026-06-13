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

    // Determine 3D grid cell size: simulation domain bounds
    double max_x = diameter + 0.10;
    double max_y = diameter + 0.10;
    double max_z = height + 0.10;

    coil->dx = max_x / GRID_X;
    coil->dy = max_y / GRID_Y;
    coil->dz = max_z / GRID_Z;
    coil->C_tune = C_tune;
    coil->v_tuning_node = 0.0;
    coil->i_tuning_loop = 0.0;

    // Initialize nested physical components
    tsfi_zener_init(&coil->zener_clamp, 5.6, 0.003, 96000.0);
    tsfi_tunnel_latch_init(&coil->tunnel_diode);
    coil->w0tkx_state_i_l = 0.0;
    coil->w0tkx_state_v_c = 0.0;

    double eps0 = 8.8541878128e-12;

    // Initialize 3D field tensors to zero
    for (int x = 0; x < GRID_X; x++) {
        for (int y = 0; y < GRID_Y; y++) {
            for (int z = 0; z < GRID_Z; z++) {
                coil->Ex[x][y][z] = 0.0;
                coil->Ey[x][y][z] = 0.0;
                coil->Ez[x][y][z] = 0.0;
                coil->Hx[x][y][z] = 0.0;
                coil->Hy[x][y][z] = 0.0;
                coil->Hz[x][y][z] = 0.0;

                coil->eps[x][y][z] = eps0;
                coil->sigma[x][y][z] = 0.0;
            }
        }
    }

    // Map the circular winding turns along the height (Z-axis) into 3D Cartesian coordinates
    double r_coil = diameter / 2.0;
    double start_z = 0.05;
    double turn_spacing = height / turns;
    double sigma_copper = 5.96e7;

    for (int n = 0; n < (int)turns; n++) {
        double z_turn = start_z + n * turn_spacing;
        int z_idx = (int)(z_turn / coil->dz);
        if (z_idx >= GRID_Z) continue;

        // Trace a circle in X-Y plane at z_idx
        for (int x = 0; x < GRID_X; x++) {
            double rx = (x + 0.5) * coil->dx - (max_x / 2.0);
            for (int y = 0; y < GRID_Y; y++) {
                double ry = (y + 0.5) * coil->dy - (max_y / 2.0);
                double d = sqrt(rx * rx + ry * ry);
                // Winding boundary width is approx half-cell step
                if (fabs(d - r_coil) < (coil->dx * 0.6)) {
                    coil->sigma[x][y][z_idx] = sigma_copper;
                }
            }
        }
    }

    // Map the lossy Bakelite form (tan delta = 0.02, eps_r = 3.5) inside the coil form
    double eps_form = 3.5 * eps0;
    double loss_tangent = 0.02;
    double form_sigma = 2.0 * M_PI * 1.0e6 * eps_form * loss_tangent; // 1 MHz ref

    int z_start_idx = (int)(start_z / coil->dz);
    int z_end_idx = (int)((start_z + height) / coil->dz);

    for (int x = 0; x < GRID_X; x++) {
        double rx = (x + 0.5) * coil->dx - (max_x / 2.0);
        for (int y = 0; y < GRID_Y; y++) {
            double ry = (y + 0.5) * coil->dy - (max_y / 2.0);
            double d = sqrt(rx * rx + ry * ry);
            if (d < r_coil) {
                for (int z = z_start_idx; z < z_end_idx; z++) {
                    if (z < GRID_Z) {
                        coil->eps[x][y][z] = eps_form;
                        coil->sigma[x][y][z] = form_sigma;
                    }
                }
            }
        }
    }

    // 3D PML Absorbing Boundaries on all outer edges
    double pml_sigma_max = 5.0;
    int pml_cells = 2; // Keep PML thin due to smaller 3D grid size to maintain performance

    for (int x = 0; x < GRID_X; x++) {
        for (int y = 0; y < GRID_Y; y++) {
            for (int z = 0; z < GRID_Z; z++) {
                // Determine minimum distance to any boundary in cell units
                int dist_x = x < GRID_X / 2 ? x : (GRID_X - 1 - x);
                int dist_y = y < GRID_Y / 2 ? y : (GRID_Y - 1 - y);
                int dist_z = z < GRID_Z / 2 ? z : (GRID_Z - 1 - z);
                
                int min_dist = dist_x;
                if (dist_y < min_dist) min_dist = dist_y;
                if (dist_z < min_dist) min_dist = dist_z;

                if (min_dist < pml_cells) {
                    double factor = (double)(pml_cells - min_dist) / pml_cells;
                    double pml_val = pml_sigma_max * factor * factor;
                    if (pml_val > coil->sigma[x][y][z]) {
                        coil->sigma[x][y][z] = pml_val;
                    }
                }
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

    double dt = 1.0 / sample_rate;
    double mu0 = 4.0 * M_PI * 1e-7;

    double dx = coil->dx;
    double dy = coil->dy;
    double dz = coil->dz;

    // Process sample by sample
    for (size_t step = 0; step < count; step++) {
        double v_in = (double)input_rf[step];

        // Zener Clamp at input excitation node
        double z_noise = 0.0;
        double clamped_v = tsfi_zener_tick(&coil->zener_clamp, v_in, 1000.0, &z_noise);
        v_in = clamped_v + z_noise;

        // 1. Inject input voltage at bottom center of the coil winding
        int source_x = GRID_X / 2;
        int source_y = GRID_Y / 2 - (GRID_Y / 4); // slightly off-center on the winding radius
        int source_z = (int)(0.05 / dz);
        coil->Ez[source_x][source_y][source_z] += v_in / dz;

        // 2. FDTD 3D Yee updates
        // Update H-field:
        // H = H - (dt / mu) * curl(E)
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < GRID_X - 1; x++) {
            for (int y = 0; y < GRID_Y - 1; y++) {
                for (int z = 0; z < GRID_Z - 1; z++) {
                    double dEz_dy = (coil->Ez[x][y + 1][z] - coil->Ez[x][y][z]) / dy;
                    double dEy_dz = (coil->Ey[x][y][z + 1] - coil->Ey[x][y][z]) / dz;
                    coil->Hx[x][y][z] -= (dt / mu0) * (dEz_dy - dEy_dz);

                    double dEx_dz = (coil->Ex[x][y][z + 1] - coil->Ex[x][y][z]) / dz;
                    double dEz_dx = (coil->Ez[x + 1][y][z] - coil->Ez[x][y][z]) / dx;
                    coil->Hy[x][y][z] -= (dt / mu0) * (dEx_dz - dEz_dx);

                    double dEy_dx = (coil->Ey[x + 1][y][z] - coil->Ey[x][y][z]) / dx;
                    double dEx_dy = (coil->Ex[x][y + 1][z] - coil->Ex[x][y][z]) / dy;
                    coil->Hz[x][y][z] -= (dt / mu0) * (dEy_dx - dEx_dy);
                }
            }
        }

        // W0TKX Class C Transistor Final conductivity modulation at mid-tap
        int mid_x = GRID_X / 2;
        int mid_y = GRID_Y / 2;
        int mid_z = GRID_Z / 2;
        float w0tkx_drive = (float)(coil->Ez[mid_x][mid_y][mid_z] * dz);
        float w0tkx_i_coll = 0.0f;
        if (w0tkx_drive > 0.6f) {
            w0tkx_i_coll = (w0tkx_drive - 0.6f) * 0.2f;
            if (w0tkx_i_coll > 0.5f) w0tkx_i_coll = 0.5f;
        }
        double base_sigma_mid = coil->sigma[mid_x][mid_y][mid_z];
        coil->sigma[mid_x][mid_y][mid_z] += (double)(w0tkx_i_coll * 1.0e6f);

        // Adjust copper conductivity dynamically based on skin effect
        double omega = 2.0 * M_PI * fmax(current_freq_hz, 1000.0);
        double skin_depth = sqrt(2.0 / (omega * mu0 * 5.96e7));
        double wire_radius = (coil->dx * 0.4);
        double skin_ratio = fmin(1.0, skin_depth / wire_radius);
        double dynamic_sigma_copper = 5.96e7 * skin_ratio;

        // Update E-field:
        // E = E + (dt / eps) * (curl(H) - sigma * E)
        #pragma omp parallel for collapse(2)
        for (int x = 1; x < GRID_X - 1; x++) {
            for (int y = 1; y < GRID_Y - 1; y++) {
                for (int z = 1; z < GRID_Z - 1; z++) {
                    // Update Ex
                    double dHz_dy = (coil->Hz[x][y][z] - coil->Hz[x][y - 1][z]) / dy;
                    double dHy_dz = (coil->Hy[x][y][z] - coil->Hy[x][y][z - 1]) / dz;
                    double sig_x = coil->sigma[x][y][z];
                    if (sig_x > 1e6) sig_x = dynamic_sigma_copper;
                    coil->Ex[x][y][z] += (dt / coil->eps[x][y][z]) * ((dHz_dy - dHy_dz) - sig_x * coil->Ex[x][y][z]);

                    // Update Ey
                    double dHx_dz = (coil->Hx[x][y][z] - coil->Hx[x][y][z - 1]) / dz;
                    double dHz_dx = (coil->Hz[x][y][z] - coil->Hz[x - 1][y][z]) / dx;
                    double sig_y = coil->sigma[x][y][z];
                    if (sig_y > 1e6) sig_y = dynamic_sigma_copper;
                    coil->Ey[x][y][z] += (dt / coil->eps[x][y][z]) * ((dHx_dz - dHz_dx) - sig_y * coil->Ey[x][y][z]);

                    // Update Ez
                    if (z == 0) {
                        coil->Ez[x][y][z] = 0.0; // Bottom PEC ground plane
                    } else {
                        double dHy_dx = (coil->Hy[x][y][z] - coil->Hy[x - 1][y][z]) / dx;
                        double dHx_dy = (coil->Hx[x][y][z] - coil->Hx[x][y - 1][z]) / dy;
                        double sig_z = coil->sigma[x][y][z];
                        if (sig_z > 1e6) sig_z = dynamic_sigma_copper;
                        coil->Ez[x][y][z] += (dt / coil->eps[x][y][z]) * ((dHy_dx - dHx_dy) - sig_z * coil->Ez[x][y][z]);
                    }
                }
            }
        }

        // 3. Lumped Tuning Capacitor Boundary at top tap
        int load_x = GRID_X / 2;
        int load_y = GRID_Y / 2 - (GRID_Y / 4);
        int load_z = GRID_Z - 3; // Tap near the top boundary of the simulated Z winding grid
        if (load_z >= GRID_Z) load_z = GRID_Z - 2;

        coil->i_tuning_loop += (coil->Ez[load_x][load_y][load_z] * dz - coil->v_tuning_node) * dt / (mu0 * dx);

        // Solve W3UZN Tunnel Diode series voltage drop (KVL)
        // Convert current from A to mA as trigger for latch step
        float i_loop_ma = (float)(coil->i_tuning_loop * 1e3);
        tsfi_tunnel_latch_step(&coil->tunnel_diode, i_loop_ma - coil->tunnel_diode.i_bias);
        double v_tunnel = (double)coil->tunnel_diode.v_phys;

        // Update loop voltage including series tunnel diode drop
        coil->v_tuning_node += ((coil->i_tuning_loop) / coil->C_tune) * dt - v_tunnel * dt;

        coil->Ez[load_x][load_y][load_z] = coil->v_tuning_node / dz;
        output_grid[step] = (float)coil->v_tuning_node;

        // Restore mid-tap conductivity
        coil->sigma[mid_x][mid_y][mid_z] = base_sigma_mid;
    }
}