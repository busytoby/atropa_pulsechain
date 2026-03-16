#define _POSIX_C_SOURCE 200809L
#include "tsfi_types.h"
#include "tsfi_wiring.h"
#include "lau_memory.h"
#include "tsfi_cli.h"
#include "tsfi_io.h"
#include "tsfi_wire_firmware.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

int main(int argc, char **argv) {
    bool auto_gemini = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--gemini") == 0) auto_gemini = true;
    }

    // Initialize System
    WaveSystem *ws = tsfi_create_system();
    if (!ws) {
        fprintf(stderr, "FATAL: System creation failed\n");
        return 1;
    }
    LauWireFirmware *fw = (LauWireFirmware *)ws->fw;

    if (fw && fw->cell_printf) fw->cell_printf(0, "--- SYSTEM-11: AUDITED (2026)---\n");
    
    // Initial Provenance Check (Loads Plugins)
    if (ws->provenance) ws->provenance();

    // Initial Epoch Phase (BEFORE Window Open)
    if (ws->step_safety_epoch) ws->step_safety_epoch();

    // First State Phase (Triggers Window Open via Directive)
    if (ws->step_safety_state) {
        if (ws->current_directive) {
            lau_free(ws->current_directive);
            ws->current_directive = NULL;
        }
        ws->current_directive = lau_strdup("OPEN_WINDOW");
        ws->step_safety_state();
        
        if (auto_gemini) {
            if (ws->current_directive) lau_free(ws->current_directive);
            const char *cmd = "/usr/local/bin/gemini";
            char dir_buf[512];
            snprintf(dir_buf, sizeof(dir_buf), "START_SESSION 0 %s", cmd);
            ws->current_directive = lau_strdup(dir_buf);
            if (fw && fw->cell_printf) fw->cell_printf(0, "[INFO] Auto-launching Gemini manifold...\n");
            ws->step_safety_state();
        }

        // Final Epoch Phase (After Window Open)
        if (ws->step_safety_epoch) ws->step_safety_epoch();
        
        // Clear directive
        if (ws->current_directive) {
            lau_free(ws->current_directive);
            ws->current_directive = NULL;
        }
    }

    while (1) {
        char input[256] = {0};

        // 1. Physical Arbitration (Firmware Domain)
        if (!fw || !fw->cell_hardware_poll || fw->cell_hardware_poll(16, input, sizeof(input)) < 0) break;

        if (input[0] != '\0') {
            if (tsfi_cli_process_line(ws, input) != 0) {
                break;
            }
            if (fw && fw->cell_printf) fw->cell_printf(0, "\nLAU Command (Intensity Directive) > ");
        }
        
        // Autonomous Logic Step (Wayland Dispatch, AI Analysis, etc.)
        if (ws->step_safety_epoch) {
            ws->step_safety_epoch();
        }
    }
    
    if (fw && fw->cell_printf) fw->cell_printf(0, "SYSTEM_AT_REST_SUCCESS\n");
    lau_final_cleanup(ws, -1);
    lau_report_memory_to_log(); // Persist metrics
    lau_report_memory_metrics();
    return 0;
}
