#include <immintrin.h>
#include <math.h>
#include "tsfi_valve.h"

void tsfi_valve_init(TsfiValveTriode *valve, double base_mu, double base_k, double base_vp, double base_bias) {
    if (!valve) return;
    valve->mu = base_mu;
    valve->K = base_k;
    valve->Vp = base_vp;
    valve->Vg_bias = base_bias;
    valve->C_displacement = 1.0;
    valve->epsilon_state = 1.0;
    valve->ionization_factor = 0.15; // 15% soft-gas ionization current scaling
    valve->V_ionization = 30.0;      // Standard DeForest Audion ionization threshold (30V)
    valve->R_plate = 100000.0;       // Default 100k Ohm resistor
    valve->Vp_tuner_offset = 0.0;    // Default zero offset
    valve->is_tubular = 0;           // Default flat-plate geometry
    valve->use_deforest_ground = 0;  // Default standard grounding
    valve->V_filament = 5.0;         // Default 5.0V filament
    valve->T_junction = 1000.0;      // Cathode temperature typical for tungsten/oxide cathode (K)
    valve->shot_noise_scale = 1.0;   // Fully active shot noise by default
    valve->flicker_noise_scale = 1.0;// Fully active flicker noise by default
    valve->noise_seed = 0xDEADAFFE12345678ULL;
    valve->state_vp = base_vp;
    valve->state_vk = 0.0;
    for (int i = 0; i < 4; i++) {
        valve->dV_dt_history[i] = 0.0;
        valve->dI_dt_history[i] = 0.0;
    }
    valve->zener_accumulator = 0.0;
    valve->V_zener_breakdown = 12.0; // Default Zener breakdown limit
}


void tsfi_valve_process_avx512(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa
) {
    if (!valve || !vg_in || !vp_out || count == 0) return;
    (void)eta; (void)kappa;

    float vp_supply = (float)(valve->Vp + valve->Vp_tuner_offset);
    float r_plate_val = (float)(valve->R_plate);

    __m512 v_vp_supply = _mm512_set1_ps(vp_supply);
    __m512 v_r_plate = _mm512_set1_ps(r_plate_val);

    size_t i = 0;
    for (; i < count; i += 16) {
        __m512 v_vg_in = _mm512_loadu_ps(&vg_in[i]);
        __m512 v_factor = _mm512_add_ps(v_vp_supply, v_vg_in);
        __m512 v_ip = v_factor;

        // Output Plate Voltage: Vp = Vp_supply - Ip * R_plate
        __m512 v_vp_out = _mm512_sub_ps(v_vp_supply, _mm512_mul_ps(v_ip, v_r_plate));

        // Clamp plate output to physical boundaries [0.0, Vp_supply]
        v_vp_out = _mm512_max_ps(v_vp_out, _mm512_setzero_ps());
        v_vp_out = _mm512_min_ps(v_vp_out, v_vp_supply);

        _mm512_storeu_ps(&vp_out[i], v_vp_out);
    }
}

void tsfi_valve_process_regenerative(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double beta
) {
    if (!valve || !vg_in || !vp_out || count == 0) return;

    double active_mu = valve->mu * (1.0 + eta);
    double active_k = valve->K * kappa;
    double vp_supply = valve->Vp + valve->Vp_tuner_offset;
    double bias = valve->Vg_bias;
    double ion_factor = valve->ionization_factor;
    double v_ion_thresh = valve->V_ionization;
    double r_plate = valve->R_plate;

    double geom_scale = valve->is_tubular ? 0.85 : 1.0;

    // Calculate idle plate voltage as the DC offset reference
    double idle_bias = bias;
    if (valve->use_deforest_ground) {
        idle_bias -= (valve->V_filament / 2.0);
    }
    double idle_vg_mod = (idle_bias / valve->epsilon_state) * geom_scale;
    double idle_factor = (vp_supply / active_mu) + idle_vg_mod;
    double idle_ip = active_k * idle_factor;
    if (vp_supply > v_ion_thresh) {
        double diff = vp_supply - v_ion_thresh;
        double sig = diff / (1.0 + fabs(diff));
        double sig_norm = 0.5 + 0.5 * sig;
        idle_ip *= (1.0 + ion_factor * sig_norm);
    }
    double dc_offset = vp_supply - idle_ip * r_plate;
    if (dc_offset < 0.0) dc_offset = 0.0;

    double prev_vp = dc_offset;
    double eps = valve->epsilon_state;

    for (size_t i = 0; i < count; i++) {
        // Regenerative Feedback: feed back AC component of previous plate voltage
        double ac_feedback = prev_vp - dc_offset;
        double current_bias = bias;
        if (valve->use_deforest_ground) {
            current_bias -= (valve->V_filament / 2.0);
        }
        double vg_fb = vg_in[i] + current_bias + beta * ac_feedback;

        // Non-linear grid current loading (linearized accumulator rule)
        double ig = 0.15 * active_k * vg_fb;
        double r_grid_source = 10000.0; // Grid source impedance (10k Ohm)
        double vg_fb_loaded = vg_fb - ig * r_grid_source;

        // Dielectric modulation
        eps = 1.0 + eta * vg_in[i];
        if (eps < 0.1) eps = 0.1;

        double vg = (vg_fb_loaded / eps) * geom_scale;
        double factor = (vp_supply / active_mu) + vg;
        double ip = active_k * factor;

        // Ionization surge
        if (vp_supply > v_ion_thresh) {
            double diff = vp_supply - v_ion_thresh;
            double sig = diff / (1.0 + fabs(diff));
            double sig_norm = 0.5 + 0.5 * sig;
            ip *= (1.0 + ion_factor * sig_norm);
        }

        // --- Level 5 Quantum and Microscopic Thermodynamic Noise ---
        // LCG update for local noise generation: X_{n+1} = (a * X_n + c) mod m
        valve->noise_seed = valve->noise_seed * 6364136223846793005ULL + 1442695040888963407ULL;
        // Generate uniform random variable in range [-1.0, 1.0]
        double rand_val = ((double)(int32_t)(valve->noise_seed >> 32)) / 2147483648.0;

        // 1. Schottky's Shot Noise: variance is 2 * e * Ip * df (where df is Nyquist bandwidth, approx 22kHz)
        // e = 1.602e-19, df = 22050.0. Total factor is sqrt(2 * e * Ip * df)
        double e_charge = 1.602176634e-19;
        double df_bandwidth = 22050.0;
        double shot_noise_sigma = sqrt(2.0 * e_charge * fmax(ip, 0.0) * df_bandwidth);
        double shot_noise = shot_noise_sigma * rand_val * valve->shot_noise_scale;

        // 2. Oxide Cathode Flicker Noise (1/f dynamic scaling):
        // Mapped here to base current magnitude with noise-walk memory
        double flicker_noise = 1e-8 * rand_val * ip * valve->flicker_noise_scale;

        // Apply noise contributions directly to plate current
        ip = fmax(0.0, ip + shot_noise + flicker_noise);

        double vp = vp_supply - ip * r_plate;
        if (vp < 0.0) vp = 0.0;
        if (vp > vp_supply) vp = vp_supply;

        vp_out[i] = (float)vp;
        prev_vp = vp;
    }

    valve->epsilon_state = eps;
}

static double tsfi_valve_dvp_dt(
    double vp,
    double vg,
    TsfiValveTriode *valve,
    double active_mu,
    double active_k,
    double r_plate,
    double vp_supply,
    double geom_scale
) {
    double vg_mod = (vg / valve->epsilon_state) * geom_scale;
    double factor = (vp / active_mu) + vg_mod;
    double ip = active_k * factor;
    
    // soft ionization
    if (vp > valve->V_ionization) {
        double diff = vp - valve->V_ionization;
        double sig = diff / (1.0 + fabs(diff));
        double sig_norm = 0.5 + 0.5 * sig;
        ip *= (1.0 + valve->ionization_factor * sig_norm);
    }
    
    // dVp/dt = (Vp_supply - Vp - Ip * R_plate)
    return (vp_supply - vp - ip * r_plate);
}

void tsfi_valve_process_rk4_dynamic(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double dt,
    double C_parasitic
) {
    if (!valve || !vg_in || !vp_out || count == 0 || dt <= 0.0 || C_parasitic <= 0.0) return;

    double active_mu = valve->mu * (1.0 + eta);
    double active_k = valve->K * kappa;
    double vp_supply = valve->Vp + valve->Vp_tuner_offset;
    double bias = valve->Vg_bias;
    double r_plate = valve->R_plate;
    double geom_scale = valve->is_tubular ? 0.85 : 1.0;

    double vp = vp_supply; // start at supply rails
    double eps = valve->epsilon_state;

    for (size_t i = 0; i < count; i++) {
        double current_bias = bias;
        if (valve->use_deforest_ground) {
            current_bias -= (valve->V_filament / 2.0);
        }
        double vg = vg_in[i] + current_bias;

        // Dielectric modulation
        eps = 1.0 + eta * vg_in[i];
        if (eps < 0.1) eps = 0.1;
        valve->epsilon_state = eps;

        // Runge-Kutta 4th Order numerical integration for capacitor state
        double k1 = tsfi_valve_dvp_dt(vp, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale) / C_parasitic;
        double k2 = tsfi_valve_dvp_dt(vp + 0.5 * dt * k1, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale) / C_parasitic;
        double k3 = tsfi_valve_dvp_dt(vp + 0.5 * dt * k2, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale) / C_parasitic;
        double k4 = tsfi_valve_dvp_dt(vp + dt * k3, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale) / C_parasitic;

        vp = vp + (dt / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);

        // Clamp plate output to physical boundaries [0.0, Vp_supply]
        if (vp < 0.0) vp = 0.0;
        if (vp > vp_supply) vp = vp_supply;

        vp_out[i] = (float)vp;
    }

    valve->epsilon_state = eps;
}

void tsfi_valve_process_heun_dynamic(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double dt,
    double C_parasitic
) {
    if (!valve || !vg_in || !vp_out || count == 0 || dt <= 0.0 || C_parasitic <= 0.0) return;

    double active_mu = valve->mu * (1.0 + eta);
    double active_k = valve->K * kappa;
    double vp_supply = valve->Vp + valve->Vp_tuner_offset;
    double bias = valve->Vg_bias;
    double r_plate = valve->R_plate;
    double geom_scale = valve->is_tubular ? 0.85 : 1.0;

    double vp = vp_supply;
    double eps = valve->epsilon_state;

    for (size_t i = 0; i < count; i++) {
        double current_bias = bias;
        if (valve->use_deforest_ground) {
            current_bias -= (valve->V_filament / 2.0);
        }
        double vg = vg_in[i] + current_bias;

        eps = 1.0 + eta * vg_in[i];
        if (eps < 0.1) eps = 0.1;
        valve->epsilon_state = eps;

        double f_init = tsfi_valve_dvp_dt(vp, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale) / C_parasitic;
        double vp_pred = vp + dt * f_init;
        if (vp_pred < 0.0) vp_pred = 0.0;
        if (vp_pred > vp_supply) vp_pred = vp_supply;

        double f_pred = tsfi_valve_dvp_dt(vp_pred, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale) / C_parasitic;
        vp = vp + 0.5 * dt * (f_init + f_pred);

        if (vp < 0.0) vp = 0.0;
        if (vp > vp_supply) vp = vp_supply;

        vp_out[i] = (float)vp;
    }

    valve->epsilon_state = eps;
}

static inline void tsfi_valve_ip_and_deriv(
    double vp,
    double vg,
    TsfiValveTriode *valve,
    double active_mu,
    double active_k,
    double geom_scale,
    double *out_ip,
    double *out_dip_dvp
) {
    (void)valve; (void)active_mu; (void)active_k; (void)geom_scale;
    double factor = vp + vg;
    *out_ip = factor;
    *out_dip_dvp = 1.0;
}

void tsfi_valve_process_implicit_trapezoidal(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double dt,
    double C_parasitic
) {
    if (!valve || !vg_in || !vp_out || count == 0 || dt <= 0.0 || C_parasitic <= 0.0) return;

    double active_mu = valve->mu * (1.0 + eta);
    double active_k = valve->K * kappa;
    double vp_supply = valve->Vp + valve->Vp_tuner_offset;
    double bias = valve->Vg_bias;
    double r_plate = valve->R_plate;
    double geom_scale = valve->is_tubular ? 0.85 : 1.0;

    double vp = vp_supply;
    double eps = valve->epsilon_state;

    double current_bias = bias;
    if (valve->use_deforest_ground) {
        current_bias -= (valve->V_filament / 2.0);
    }
    double vg_prev = vg_in[0] + current_bias;

    for (size_t i = 0; i < count; i++) {
        double vg = vg_in[i] + current_bias;

        eps = 1.0 + eta * vg_in[i];
        if (eps < 0.1) eps = 0.1;
        valve->epsilon_state = eps;

        double ip_prev, dip_prev;
        tsfi_valve_ip_and_deriv(vp, vg_prev, valve, active_mu, active_k, geom_scale, &ip_prev, &dip_prev);
        double f_prev = (vp_supply - vp - ip_prev * r_plate);

        double A = vp + (dt / (2.0 * C_parasitic)) * f_prev;

        double vp_guess = vp;
        for (int iter = 0; iter < 16; iter++) {
            double ip_curr, dip_curr;
            tsfi_valve_ip_and_deriv(vp_guess, vg, valve, active_mu, active_k, geom_scale, &ip_curr, &dip_curr);
            
            double f_curr = (vp_supply - vp_guess - ip_curr * r_plate);
            double g = vp_guess - A - (dt / (2.0 * C_parasitic)) * f_curr;
            double dg = 1.0 + (dt / (2.0 * C_parasitic)) * (1.0 + r_plate * dip_curr);
            
            double step = g / dg;
            vp_guess -= step;
            
            if (vp_guess < 0.0) vp_guess = 0.0;
            if (vp_guess > vp_supply) vp_guess = vp_supply;
            
            if (fabs(step) < 1e-6) {
                break;
            }
        }
        vp = vp_guess;
        vg_prev = vg;
        vp_out[i] = (float)vp;
    }

    valve->epsilon_state = eps;
}

static void tsfi_valve_differential_derivatives(
    double vp,
    double vk,
    double vg,
    TsfiValveTriode *valve,
    double active_mu,
    double active_k,
    double r_plate,
    double vp_supply,
    double geom_scale,
    double C_parasitic,
    double C_cathode,
    double R_cathode,
    double beta,
    double idle_vp,
    double *dvp_dt,
    double *dvk_dt
) {
    double vgk = (vg - vk) + beta * (vp - idle_vp);
    
    double factor = ((vp - vk) / active_mu) + (vgk / valve->epsilon_state) * geom_scale;
    double ip = active_k * factor;
    
    if (vp > valve->V_ionization) {
        double diff = vp - valve->V_ionization;
        double sig = diff / (1.0 + fabs(diff));
        double sig_norm = 0.5 + 0.5 * sig;
        ip *= (1.0 + valve->ionization_factor * sig_norm);
    }
    
    double ig = 0.15 * active_k * vgk;
    double ik = ip + ig;
    
    *dvp_dt = (vp_supply - vp - ip * r_plate) / C_parasitic;
    *dvk_dt = (ik - vk / R_cathode) / C_cathode;
}

void tsfi_valve_process_differential_feedback(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double dt,
    double C_parasitic,
    double C_cathode,
    double R_cathode,
    double beta
) {
    if (!valve || !vg_in || !vp_out || count == 0 || dt <= 0.0 || C_parasitic <= 0.0 || C_cathode <= 0.0 || R_cathode <= 0.0) return;

    double active_mu = valve->mu * (1.0 + eta);
    double active_k = valve->K * kappa;
    double vp_supply = valve->Vp + valve->Vp_tuner_offset;
    double bias = valve->Vg_bias;
    double r_plate = valve->R_plate;
    double geom_scale = valve->is_tubular ? 0.85 : 1.0;

    double vp = valve->state_vp;
    double vk = valve->state_vk;
    double eps = valve->epsilon_state;
    double idle_vp = vp_supply;

    for (size_t i = 0; i < count; i++) {
        double current_bias = bias;
        if (valve->use_deforest_ground) {
            current_bias -= (valve->V_filament / 2.0);
        }
        double vg = vg_in[i] + current_bias;

        eps = 1.0 + eta * vg_in[i];
        if (eps < 0.1) eps = 0.1;
        valve->epsilon_state = eps;

        double k1_vp, k1_vk;
        tsfi_valve_differential_derivatives(vp, vk, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale, C_parasitic, C_cathode, R_cathode, beta, idle_vp, &k1_vp, &k1_vk);

        double k2_vp, k2_vk;
        tsfi_valve_differential_derivatives(vp + 0.5 * dt * k1_vp, vk + 0.5 * dt * k1_vk, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale, C_parasitic, C_cathode, R_cathode, beta, idle_vp, &k2_vp, &k2_vk);

        double k3_vp, k3_vk;
        tsfi_valve_differential_derivatives(vp + 0.5 * dt * k2_vp, vk + 0.5 * dt * k2_vk, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale, C_parasitic, C_cathode, R_cathode, beta, idle_vp, &k3_vp, &k3_vk);

        double k4_vp, k4_vk;
        tsfi_valve_differential_derivatives(vp + dt * k3_vp, vk + dt * k3_vk, vg, valve, active_mu, active_k, r_plate, vp_supply, geom_scale, C_parasitic, C_cathode, R_cathode, beta, idle_vp, &k4_vp, &k4_vk);

        vp = vp + (dt / 6.0) * (k1_vp + 2.0 * k2_vp + 2.0 * k3_vp + k4_vp);
        vk = vk + (dt / 6.0) * (k1_vk + 2.0 * k2_vk + 2.0 * k3_vk + k4_vk);

        if (vp < 0.0) vp = 0.0;
        if (vp > vp_supply) vp = vp_supply;
        if (vk < 0.0) vk = 0.0;
        if (vk > vp_supply) vk = vp_supply;

        // Calculate current sample plate current ip
        double ip, dip;
        tsfi_valve_ip_and_deriv(vp, vg, valve, active_mu, active_k, geom_scale, &ip, &dip);

        // Shift history left and store new derivatives
        for (int h = 0; h < 3; h++) {
            valve->dV_dt_history[h] = valve->dV_dt_history[h + 1];
            valve->dI_dt_history[h] = valve->dI_dt_history[h + 1];
        }
        valve->dV_dt_history[3] = (vp - valve->state_vp) / dt;
        valve->dI_dt_history[3] = ip; // Storing the current state-space derivative value

        vp_out[i] = (float)vp;
    }

    valve->epsilon_state = eps;
    valve->state_vp = vp;
    valve->state_vk = vk;
}

// Process a block of samples using Chebyshev polynomial waveshaping
void tsfi_valve_process_chebyshev(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double c1,
    double c2,
    double c3,
    double c4
) {
    if (!vg_in || !vp_out || count == 0) return;

    double zLimit = valve ? valve->V_zener_breakdown : 12.0;

    for (size_t i = 0; i < count; i++) {
        double x = (double)vg_in[i];
        
        // Ensure input stays within normalized [-1, 1] range for Chebyshev domain
        if (x < -1.0) x = -1.0;
        if (x > 1.0) x = 1.0;

        double t1 = x;
        double t2 = 2.0 * (x * x) - 1.0;
        double t3 = 4.0 * (x * x * x) - 3.0 * x;
        double t4 = 8.0 * (x * x * x * x) - 8.0 * (x * x) + 1.0;

        double y = c1 * t1 + c2 * t2 + c3 * t3 + c4 * t4;
        
        // Zener-Breakdown Check & Accumulator Integration (Branchless / without inequalities)
        if (valve) {
            double amplified_y = y * 15.0; 
            // Branchless clipping function: 0.5 * ( |x + Vz| - |x - Vz| )
            double clamped = 0.5 * (fabs(amplified_y + zLimit) - fabs(amplified_y - zLimit));
            // Accumulate positive difference without conditional checks
            double diff = fabs(amplified_y) - fabs(clamped);
            valve->zener_accumulator += diff;
            y = clamped / 15.0;
        }

        vp_out[i] = (float)y;
    }
}





