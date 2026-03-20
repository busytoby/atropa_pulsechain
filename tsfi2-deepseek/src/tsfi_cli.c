#include "tsfi_cli.h"
#include "lau_memory.h"
#include "lau_audit.h"
#include "tsfi_wiring.h"
#include "tsfi_hotloader.h"
#include "tsfi_trace.h"
#include "tsfi_math.h"
#include "tsfi_genetic.h"
#include "tsfi_io.h"
#include "tsfi_pulsechain_rpc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static int handle_default_command(WaveSystem *ws, double new_i, const char *new_d) {
    ws->current_intensity = new_i; 
    
    if (ws->current_directive) {
        lau_free(ws->current_directive);
        ws->current_directive = NULL;
    }
    ws->current_directive = lau_strdup(new_d);
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, ws->current_intensity);
    tsfi_io_printf(stdout, "[AUDIT] %s\n", *ws->resonance_as_status);
    return 0;
}

static int handle_math_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    int bits = 2048;
    sscanf(new_d, "MATH %d", &bits);
    tsfi_io_printf(stdout, "[MATH] Running %d-bit AVX512 Modular Exponentiation...\n", bits);

    TSFiBigInt *b = tsfi_bn_alloc();
    TSFiBigInt *e = tsfi_bn_alloc();
    TSFiBigInt *m = tsfi_bn_alloc();
    TSFiBigInt *res = tsfi_bn_alloc();

    // Setup
    tsfi_bn_randomize(b);
    tsfi_bn_randomize(e);
    tsfi_bn_randomize(m);
    m->limbs[0] |= 1; // Odd modulus

    // Adjust active limbs for requested bits
    int num_limbs = (bits + 51) / 52; // ceil(bits / 52)
    if (num_limbs > TSFI_NUM_LIMBS) num_limbs = TSFI_NUM_LIMBS;
    b->active_limbs = num_limbs;
    e->active_limbs = num_limbs;
    m->active_limbs = num_limbs;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    tsfi_bn_modpow_avx512(res, b, e, m);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    
    uint8_t out[8];
    tsfi_bn_to_bytes(res, out, sizeof(out));
    tsfi_io_printf(stdout, "[MATH] Result (Top 64 bits): ");
    for(int i=0; i<8; i++) tsfi_io_printf(stdout, "%02X", out[7-i]); // Big Endian print
    tsfi_io_printf(stdout, "\n");
    tsfi_io_printf(stdout, "[MATH] Time: %.6f seconds\n", time_taken);

    tsfi_bn_free(b);
    tsfi_bn_free(e);
    tsfi_bn_free(m);
    tsfi_bn_free(res);
    return 0;
}

static int handle_provenance_command(WaveSystem *ws) {
    if (ws->provenance) {
        ws->provenance();
    } else {
        tsfi_io_printf(stdout, "[CLI] Error: Provenance logic not bound.\n");
    }
    return 0;
}

static int handle_query_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    char action[32];
    char address[64];
    char data[512];
    char result[8192];

    if (sscanf(new_d, "%s %s %s", action, address, data) == 3) {
        if (strcmp(action, "CALL") == 0) {
            if (tsfi_pulse_rpc_call(address, data, result, sizeof(result))) {
                tsfi_io_printf(stdout, "[RPC] Call Result: %s\n", result);
            } else {
                tsfi_io_printf(stdout, "[RPC] Error: Call failed.\n");
            }
        } else if (strcmp(action, "STORAGE") == 0) {
            if (tsfi_pulse_rpc_get_storage_at(address, data, result, sizeof(result))) {
                tsfi_io_printf(stdout, "[RPC] Storage Result: %s\n", result);
            } else {
                tsfi_io_printf(stdout, "[RPC] Error: Storage read failed.\n");
            }
        } else {
            tsfi_io_printf(stdout, "[RPC] Error: Unknown query action '%s'.\n", action);
        }
    } else {
        tsfi_io_printf(stdout, "[RPC] Usage: 0.0 <CALL|STORAGE> <address> <hex_data_or_slot>\n");
    }
    return 0;
}

static int handle_scramble_command(WaveSystem *ws) {
    if (ws->current_directive) {
        lau_free(ws->current_directive);
        ws->current_directive = NULL;
    }
    ws->scramble();
    ws->current_directive = NULL;
    tsfi_io_printf(stdout, "[AUDIT] %s\n", *ws->resonance_as_status);
    tsfi_io_printf(stdout, "System ID: %d (Should be random)\n", ws->system_id);
    return 0;
}

static int handle_exit_command(WaveSystem *ws) {
    lau_log_status("INTENTIONAL_EXIT_COMMAND_RECEIVED");
    if (ws->current_directive) {
        lau_free(ws->current_directive);
        ws->current_directive = NULL;
    }
    ws->current_directive = lau_strdup("CLOSE_WINDOW");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 0.0);
    return 1;
}

static int handle_genetic_command(WaveSystem *ws, const char *new_d) {
    (void)ws;
    int iterations = 10000;
    if (strlen(new_d) > 8) {
        sscanf(new_d, "GENETIC %d", &iterations);
    }
    tsfi_io_printf(stdout, "[GENETIC] Running Benchmark with %d iterations (AVX-512 Crossover)...\n", iterations);
    
    char report[8192];
    tsfi_genetic_benchmark_run(iterations, report, sizeof(report));
    tsfi_io_printf(stdout, "%s", report);
    return 0;
}

static int handle_promote_plane71_command(WaveSystem *ws) {
    (void)ws;
    tsfi_io_printf(stdout, "[ACTION] Promoting Simulation to Physical Plane 71...\n");
    
    int unlocked_fd = tsfi_acquire_hardware_lease();
    if (unlocked_fd >= 0) {
        tsfi_io_printf(stdout, "[SUCCESS] Physical Plane 71 Promoted. Hardware Lease Secured (FD: %d).\n", unlocked_fd);
        close(unlocked_fd); 
    } else {
        tsfi_io_printf(stderr, "[FRACTURE] Failed to acquire Hardware Lease for Plane 71.\n");
    }
    return 0;
}

static int handle_load_command(WaveSystem *ws, const char *param) {
    char output_so[300];
    char source_c[300];
    snprintf(output_so, sizeof(output_so), "./%s.so", param);
    snprintf(source_c, sizeof(source_c), "%s.c", param);
    
    int compile_status = 0;
    
    if (access(source_c, F_OK) == 0) {
        // Source exists, compile it
        compile_status = tsfi_compile_plugin(source_c, output_so);
    } else if (access(output_so, F_OK) == 0) {
        // Source missing, but SO exists. Use pre-compiled.
        tsfi_io_printf(stdout, "[CLI] Source '%s' not found. Using existing '%s'.\n", source_c, output_so);
        compile_status = 0;
    } else {
        tsfi_io_printf(stdout, "[CLI] Error: Cannot find source '%s' or object '%s'.\n", source_c, output_so);
        compile_status = -1;
    }
    
    if (compile_status == 0) {
        TSFiLogicTable new_logic;
        if (tsfi_load_plugin(output_so, &new_logic) == 0) {
            lau_update_logic(ws, &new_logic);
        }
    }
    return 0;
}

int tsfi_cli_process_line(WaveSystem *ws, char *input) {
    input[strcspn(input, "\n")] = 0;
    double new_i;
    char new_d[256];
    char param[256];
    
    // Parse LOAD command
    if (sscanf(input, "LOAD %255s", param) == 1) {
        return handle_load_command(ws, param);
    }

    if (sscanf(input, "%lf %[^\n]", &new_i, new_d) == 2) {
        if (strcmp(new_d, "EXIT") == 0) { 
            return handle_exit_command(ws); 
        }
        if (strcmp(new_d, "SCRAMBLE") == 0) {
            return handle_scramble_command(ws);
        }
        if (strcmp(new_d, "PROVENANCE") == 0) {
            return handle_provenance_command(ws);
        }
        if (strncmp(new_d, "CALL", 4) == 0 || strncmp(new_d, "STORAGE", 7) == 0) {
            return handle_query_command(ws, new_d);
        }
        if (strncmp(new_d, "MATH", 4) == 0) {
            return handle_math_command(ws, new_d);
        }
        if (strncmp(new_d, "GENETIC", 7) == 0) {
            return handle_genetic_command(ws, new_d);
        }
        if (strcmp(new_d, "PROMOTE_PLANE_71") == 0) {
            return handle_promote_plane71_command(ws);
        }
        // Default behavior if previous commands not matched
        return handle_default_command(ws, new_i, new_d);
    }
    
    return 0;
}
