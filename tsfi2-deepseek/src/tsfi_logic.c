#include "tsfi_elf_reflect.h"
#include "tsfi_logic.h"
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_types.h"
#include "tsfi_hotloader.h"
#include "tsfi_wiring.h"
#include "tsfi_hilbert.h"
#include "lau_telemetry.h"
#include "tsfi_svdag.h"
#include "tsfi_resonance.h"
#include "tsfi_wire_firmware.h"
#include "vulkan/vulkan_system.h"
#include "tsfi_wavelet_arena.h"
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_io.h"
#include "tsfi_raw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdatomic.h>

// --- Chained Logic State ---
static WaveSystem *g_ws = NULL;
static TSFiHelmholtzSVDAG *g_dag = NULL;

void master_logic_epoch(int *ver) { 
    if (g_ws) {
        // --- 1. Autonomous Firmware/Peripheral Logic ---
        LauWireFirmware *fw = tsfi_wire_firmware_get();
        if (fw) {
            tsfi_wire_firmware_step_peripheral(fw, NULL);
        }

        // --- 2. Resident Session Logic ---
        for (int i = 0; i < 4; i++) {
            if (g_ws->active_sessions[i]) {
                LauWireThread *wt = (LauWireThread *)g_ws->active_sessions[i];
                // Safety check: Only dereference if it's a real pointer (not a mock small value)
                if ((uintptr_t)wt > 0x10000 && wt->pty_geometry) {
                    tsfi_wire_thread_dispatch(wt);
                }
                // Unified Reap in Epoch (Only for real sessions)
                if ((uintptr_t)wt > 0x10000 && tsfi_wire_thread_is_at_rest(wt)) {
                    tsfi_io_printf(stdout, "[MASTER] Reaping Session %d\n", i);
                    tsfi_wire_thread_destroy(wt);
                    g_ws->active_sessions[i] = NULL;
                }
            }
        }

        // --- 3. Sheaf Density Throttle ---
        // If multiple sessions are active, throttle to maintain rigidity (Wavelets with Wavelets)
        int active_count = 0;
        for (int i = 0; i < 4; i++) if (g_ws->active_sessions[i]) active_count++;
        
        // --- 4. Little Petya Spider Coverage (8x5 Grid) ---
        LauTelemetryState *telem = lau_telemetry_get_state();
        if (telem) {
            uint64_t s_grid = 0;
            // Map the 4 potential sessions to grid positions based on their pointer identity
            for (int i = 0; i < 4; i++) {
                if (g_ws->active_sessions[i]) {
                    uint32_t pos = (uint32_t)(((uintptr_t)g_ws->active_sessions[i] >> 12) % 40);
                    s_grid |= (1ULL << pos);
                }
            }
            atomic_store(&telem->spider_grid, s_grid);

            // Calculate Coverage (Self + Adjacent)
            uint64_t c_grid = s_grid;
            for (int i = 0; i < 40; i++) {
                if ((s_grid >> i) & 1) {
                    int x = i % 8; int y = i / 8;
                    if (x > 0) c_grid |= (1ULL << (i - 1)); // Left
                    if (x < 7) c_grid |= (1ULL << (i + 1)); // Right
                    if (y > 0) c_grid |= (1ULL << (i - 8)); // Top
                    if (y < 4) c_grid |= (1ULL << (i + 8)); // Bottom
                }
            }
            atomic_store(&telem->covered_grid, c_grid);
            
            int covered_count = 0;
            for (int i = 0; i < 40; i++) if ((c_grid >> i) & 1) covered_count++;
            atomic_store(&telem->coverage_pct, (float)covered_count / 40.0f);

            // --- 5. XOR Trap (Differentiation Engine for Spiders) ---
            bool differentiation = false;
            if (active_count >= 2) {
                // Find two non-null sessions to compare
                LauWireThread *first = NULL;
                for (int i = 0; i < 4; i++) {
                    if (g_ws->active_sessions[i]) {
                        if (!first) first = (LauWireThread *)g_ws->active_sessions[i];
                        else {
                            LauWireThread *second = (LauWireThread *)g_ws->active_sessions[i];
                            if (first->current_intensity != second->current_intensity) {
                                differentiation = true;
                                break;
                            }
                        }
                    }
                }
            }
            atomic_store(&telem->xor_trap_active, differentiation);

            // --- 6. Spider Portfolio Scan (Active Interest) ---
            // The Spider ignores TRAIT_NONE unless XOR is active, but it actively
            // scans for wavelets it has already claimed (e.g., TSFI_TRAIT_FILE).
            for (int i = 0; i < 4; i++) {
                if (g_ws->active_sessions[i]) {
                    TsfiWavelet *W = (TsfiWavelet *)g_ws->active_sessions[i];
                    if ((uintptr_t)W > 0x10000 && W->telemetry.trait == TSFI_TRAIT_FILE) {
                        if (W->telemetry.current_seal_level == 1) {
                            // Spider demonstrates interest by actively waiting for the content fill
                            tsfi_io_printf(stdout, "[SPIDER] Portfolio Scan: Actively guarding Layer 1 FILE '%s'. Awaiting Content Fill.\n", W->file_cell.filename);
                        }
                    }
                }
            }

            if (active_count >= 2) {                atomic_fetch_add(&telem->throttle_count, 1);
                atomic_fetch_add(&telem->total_stall_ns, 50000000ULL); // 50ms stall
                tsfi_io_printf(stdout, "[THROTTLE] Sheaf Density Surge (%d active). Stalling 50ms.\n", active_count);
                tsfi_io_flush(stdout);
                tsfi_raw_usleep(50000);
            }
        }
    }
    if (ver) (*ver)++;
}

bool master_logic_state(void *obj) { 
    (void)obj;
    return true; 
}

void master_logic_directive(int *cnt, char *dir) {
    if (!g_ws || !dir) return;
    if (cnt) (*cnt)++;

    // Directives: SPIDER_FILE_INIT <id> <filename>
    //             SPIDER_FILE_FILL <id> <content>
    if (strncmp(dir, "SPIDER_FILE_INIT", 16) == 0) {
        uint64_t id; char filename[64];
        if (sscanf(dir + 17, "%lu %63s", &id, filename) >= 2) {
            // Find the wavelet in the registry
            // (Simplification: we use the session ID for this demo)
            if (id < 4 && g_ws->active_sessions[id]) {
                TsfiWavelet *W = (TsfiWavelet *)g_ws->active_sessions[id];
                W->private_header.role = TSFI_ROLE_FILE_CELL;
                W->telemetry.current_seal_level = 0;
                // Use XOR trap to fill Layer 1
                tsfi_wavelet_xor_signal(NULL, W, (void*)1, (void*)2, (uint8_t*)filename);
            }
        }
    }
    else if (strncmp(dir, "SPIDER_FILE_FILL", 16) == 0) {
        uint64_t id; char content[121];
        if (sscanf(dir + 17, "%lu %120s", &id, content) >= 2) {
            if (id < 4 && g_ws->active_sessions[id]) {
                TsfiWavelet *W = (TsfiWavelet *)g_ws->active_sessions[id];
                if (W->private_header.role == TSFI_ROLE_FILE_CELL) {
                    // Use XOR trap to fill Layer 2
                    tsfi_wavelet_xor_signal(NULL, W, (void*)1, (void*)2, (uint8_t*)content);
                }
            }
        }
    }
}

const TSFiLogicTable* tsfi_get_default_logic(void) {
    static const TSFiLogicTable default_table = {
        .logic_epoch = master_logic_epoch,
        .logic_state = master_logic_state,
        .logic_directive = master_logic_directive,
        .logic_scramble = NULL,
        .logic_provenance = NULL,
        .logic_hilbert = NULL,
        .logic_hilbert_batch = NULL,
        .logic_evolve = NULL
    };
    return &default_table;
}

void tsfi_logic_init(void *ws) {
    g_ws = (WaveSystem *)ws;
    g_dag = tsfi_svdag_create(1024);
}

void tsfi_logic_teardown(void) {
    if (g_dag) {
        tsfi_svdag_destroy(g_dag);
        g_dag = NULL;
    }
}
