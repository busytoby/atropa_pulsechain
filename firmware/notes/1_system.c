#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/signalfd.h>
#include <poll.h>
#include <time.h>

#define PROVENANCE_KEY "SIG_2026_USLM_AFFIRMED"

typedef struct {
    int version;
    char *resonance_as_status; 
    bool ftw; 
    int counter;
    bool is_autonomous_excuse_active;
} InternalHeader;

#define DEFINE_MAPPED_STRUCT(name, ...) \
    typedef struct { \
        struct { \
            int * const version; \
            char ** const resonance_as_status; \
            bool * const ftw; \
            int * const counter; \
            bool * const is_autonomous_excuse_active; \
        }; \
        __VA_ARGS__ \
    } name;

#define WIRE_BIJECTION(d, h) ( \
    *(int**)&(d)->version = &(h)->version, \
    *(char***)&(d)->resonance_as_status = &(h)->resonance_as_status, \
    *(bool**)&(d)->ftw = &(h)->ftw, \
    *(int**)&(d)->counter = &(h)->counter, \
    *(bool**)&(d)->is_autonomous_excuse_active = &(h)->is_autonomous_excuse_active \
)

DEFINE_MAPPED_STRUCT(WaveSystem, int system_id; char *current_directive; double current_intensity; char *provenance_sig;)

void log_immutable_status(const char *status) {
    FILE *log_file = fopen("lau_audit.log", "a");
    if (log_file) {
        time_t now; time(&now);
        char *date = ctime(&now);
        date[strlen(date) - 1] = '\0';
        fprintf(log_file, "[%s UTC] [KEY:%s] %s\n", date, PROVENANCE_KEY, status);
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
    if (*ws->resonance_as_status != NULL) free(*ws->resonance_as_status);
    int prev_counter = *ws->counter;
    augment(ws);
    int delta = *ws->counter - prev_counter;
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "[TRACE] AB-316 | EXEC: %s() | Δ_CTR: %d", fn_name, delta);
    *ws->resonance_as_status = strdup(buffer);
    log_immutable_status(buffer);
}

#define STEP(ws, func, val) apply_traced_resonance(ws, func, #func, val);

#define HELMHOLTZ_RESONANCE_LIST(X, ws, i) \
    X(ws, step_safety_epoch, 1.25) \
    X(ws, step_safety_state, 0.50) \
    X(ws, step_executor_directive, i)

int main() {
    sigset_t mask; sigemptyset(&mask); sigaddset(&mask, SIGINT); sigprocmask(SIG_BLOCK, &mask, NULL);
    int sfd = signalfd(-1, &mask, 0);

    InternalHeader h = { .resonance_as_status = strdup("LAU_SYNC_START") };
    WaveSystem *ws = malloc(sizeof(WaveSystem));
    WIRE_BIJECTION(ws, &h);

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; fds[0].events = POLLIN;
    fds[1].fd = sfd; fds[1].events = POLLIN;

    printf("--- SYSTEM-11: AUDITED (2026) ---\n");
    char input[256];
    while (1) {
        printf("\nLAU Command (Intensity Directive) > "); fflush(stdout);
        if (poll(fds, 2, -1) > 0) {
            if (fds[1].revents & POLLIN) { break; }
            if (fds[0].revents & POLLIN) {
                if (fgets(input, sizeof(input), stdin)) {
                    input[strcspn(input, "\n")] = 0;
                    double new_i; char new_d[256];
                    if (sscanf(input, "%lf %255s", &new_i, new_d) == 2) {
                        if (strcmp(new_d, "EXIT") == 0) { 
                            log_immutable_status("INTENTIONAL_EXIT_COMMAND_RECEIVED"); 
                            break; 
                        }
                        ws->current_intensity = new_i; ws->current_directive = new_d;
                        HELMHOLTZ_RESONANCE_LIST(STEP, ws, ws->current_intensity);
                        printf("[AUDIT] %s\n", *ws->resonance_as_status);
                    }
                }
            }
        }
    }
    lau_final_cleanup(&h, ws, sfd);
    printf("SYSTEM_AT_REST_SUCCESS\n");
    return 0;
}
