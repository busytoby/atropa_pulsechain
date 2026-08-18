#define _POSIX_C_SOURCE 200809L
#include "auncient_lau_resonator_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void log_immutable_status(const char *status) {
    FILE *log_file = fopen("lau_audit.log", "a");
    if (log_file) {
        time_t now; time(&now);
        char *date = ctime(&now);
        if (date) {
            size_t dlen = strlen(date);
            if (dlen > 0 && date[dlen - 1] == '\n') {
                date[dlen - 1] = '\0';
            }
            fprintf(log_file, "[%s UTC] [KEY:%s] %s\n", date, PROVENANCE_KEY, status);
        }
        fclose(log_file);
    }
}

void lau_final_cleanup(InternalHeader *h, WaveSystem *ws, int sfd) {
    if (h && h->resonance_as_status) { free(h->resonance_as_status); h->resonance_as_status = NULL; }
    if (ws) free(ws);
    if (sfd != -1) close(sfd);
}

void step_safety_epoch(WaveSystem *ws) { *ws->version = 2026; }
void step_safety_state(WaveSystem *ws) { *(ws->ftw) = true; }
void step_executor_directive(WaveSystem *ws) { if (ws->current_directive) (*ws->counter)++; }

void apply_traced_resonance(WaveSystem *ws, void (*augment)(WaveSystem*), const char *fn_name, double intensity) {
    (void)intensity;
    if (*ws->resonance_as_status != NULL) free(*ws->resonance_as_status);
    int prev_counter = *ws->counter;
    augment(ws);
    int delta = *ws->counter - prev_counter;
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "[TRACE] AB-316 | EXEC: %s() | Δ_CTR: %d", fn_name, delta);
    *ws->resonance_as_status = strdup(buffer);
    log_immutable_status(buffer);
}

bool auncient_lau_resonator_run_single_pass(InternalHeader *h, WaveSystem *ws, double intensity, const char *directive) {
    if (!h || !ws || !directive) return false;
    ws->current_intensity = intensity;
    ws->current_directive = (char*)directive;
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, ws->current_intensity);
    return (h->counter > 0 && h->version == 2026 && h->ftw == true);
}

uint32_t auncient_lau_resonator_compute_rule18(const InternalHeader *h, const WaveSystem *ws) {
    if (!h || !ws) return 0;
    uint64_t p0 = 1;
    uint64_t p1 = (h->version + 7ULL);
    uint64_t pn = p1;

    uint64_t alpha = (h->counter * 17ULL) % 256ULL;
    uint64_t beta = (h->ftw ? 31ULL : 1ULL);
    pn = ((h->counter + alpha) * p1 - beta * p0) % 65535ULL;
    return (uint32_t)pn;
}
