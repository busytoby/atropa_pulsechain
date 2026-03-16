#include "lau_telemetry.h"
#include "tsfi_io.h"
#include "tsfi_dai.h"
#include "tsfi_raw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/select.h>
#include <signal.h>
#include <stdbool.h>
#include <nmmintrin.h>
#include <immintrin.h>

static uint32_t calculate_telemetry_crc(LauTelemetryState *s) {
    if (!s) return 0;
    uint64_t crc = 0;
    crc = _mm_crc32_u64(crc, s->magic);
    crc = _mm_crc32_u64(crc, s->pid);
    crc = _mm_crc32_u64(crc, s->total_allocs);
    crc = _mm_crc32_u64(crc, s->total_frees);
    crc = _mm_crc32_u64(crc, s->active_allocs);
    crc = _mm_crc32_u64(crc, s->active_bytes);
    crc = _mm_crc32_u64(crc, s->peak_bytes);
    crc = _mm_crc32_u64(crc, s->exec_steps);
    crc = _mm_crc32_u64(crc, s->exec_last_ts);
    crc = _mm_crc32_u64(crc, s->throttle_count);
    crc = _mm_crc32_u64(crc, s->total_stall_ns);
    uint64_t *dir_ptr = (uint64_t*)s->last_directive_str;
    crc = _mm_crc32_u64(crc, *dir_ptr);
    crc = _mm_crc32_u64(crc, (uint64_t)s->recip_symmetry);
    return (uint32_t)crc;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        tsfi_io_printf(stderr, "Usage: %s <cmd> [args...]\n", argv[0]);
        return 1;
    }

    char id_str[32];
    snprintf(id_str, sizeof(id_str), "cockpit_%d", getpid());
    const char *id = id_str;
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_%s", id);

    tsfi_shm_unlink(shm_name);
    int fd = tsfi_shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (fd == -1) { perror("tsfi_shm_open"); return 1; }
    if (ftruncate(fd, sizeof(LauTelemetryState)) == -1) { perror("ftruncate"); return 1; }
    
    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) { perror("mmap"); return 1; }
    
    memset(telem, 0, sizeof(LauTelemetryState));

    pid_t pid = fork();
    if (pid == 0) {
        setenv("TSFI_TELEMETRY_ID", id, 1);
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(127);
    }

        if (isatty(STDOUT_FILENO)) {
        tsfi_io_printf(stdout, "\033[2J\033[H"); 
    } 

    int final_status = 0;
    time_t start_time = time(NULL);
    while (1) {
        int status;
        if (waitpid(pid, &status, WNOHANG) > 0) {
            final_status = status;
            break;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        struct timeval tv = {0, 0}; 

        int select_res = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
        if (select_res > 0) {
            char input_buf[1024];
            if (fgets(input_buf, sizeof(input_buf), stdin)) {
                input_buf[strcspn(input_buf, "\n")] = 0;
                if (strncmp(input_buf, "read ", 5) == 0) {
                    snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                        "{\"jsonrpc\": \"2.0\", \"method\": \"read_file\", \"params\": {\"path\": \"%.3000s\"}, \"id\": 99}", 
                        input_buf + 5);
                } else if (strncmp(input_buf, "grep ", 5) == 0) {
                    snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                        "{\"jsonrpc\": \"2.0\", \"method\": \"grep_search\", \"params\": {\"pattern\": \"%.3000s\"}, \"id\": 100}", 
                        input_buf + 5);
                } else if (strncmp(input_buf, "find ", 5) == 0) {
                    snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                        "{\"jsonrpc\": \"2.0\", \"method\": \"glob\", \"params\": {\"pattern\": \"%.3000s\"}, \"id\": 101}", 
                        input_buf + 5);
                } else if (strncmp(input_buf, "store ", 6) == 0) {
                    char *payload = input_buf + 6;
                    char *space = strchr(payload, ' ');
                    if (space) {
                        *space = 0;
                        char *val = space + 1;
                        snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                            "{\"jsonrpc\": \"2.0\", \"method\": \"record_knowledge\", \"params\": {\"topic\": \"%.100s\", \"insight\": \"%.3000s\"}, \"id\": 102}", 
                            payload, val);
                    }
                } else if (strncmp(input_buf, "wave ", 5) == 0) {
                    snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                        "{\"jsonrpc\": \"2.0\", \"method\": \"tsfi_wave\", \"params\": {\"content\": \"%.3000s\"}, \"id\": 106}", 
                        input_buf + 5);
                } else if (strncmp(input_buf, "think ", 6) == 0) {
                    snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                        "{\"jsonrpc\": \"2.0\", \"method\": \"genetic.establish_llm\", \"params\": {\"architecture\": \"WLOAD W0, 0.5\\nWLOAD W1, 0.1\\nWADD W2, W0, W1\\nWSTORE W2, 0\", \"strategy\": \"%.3000s\"}, \"id\": 107}", 
                        input_buf + 6);
                } else if (strncmp(input_buf, "fetch ", 6) == 0) {
                    char *url = input_buf + 6;
                    char *out = strchr(url, ' ');
                    if (out) {
                        *out = 0;
                        char *out_path = out + 1;
                        char fcmd[4096];
                        snprintf(fcmd, sizeof(fcmd), "./bin/tsfi_fetch %s %s", url, out_path);
                        if (system(fcmd) != 0) {
                            tsfi_io_printf(stderr, "[COCKPIT] Fetch wavefront failed.\n");
                        }
                    }
                } else if (strcmp(input_buf, "tools") == 0) {
                    snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                        "{\"jsonrpc\": \"2.0\", \"method\": \"mcp.list_tools\", \"params\": {}, \"id\": 103}");
                } else if (strcmp(input_buf, "resources") == 0) {
                    snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                        "{\"jsonrpc\": \"2.0\", \"method\": \"mcp.list_resources\", \"params\": {}, \"id\": 104}");
                } else if (strncmp(input_buf, "call ", 5) == 0) {
                    char *tool = input_buf + 5;
                    char *space = strchr(tool, ' ');
                    if (space) {
                        *space = 0;
                        char *params = space + 1;
                        snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                            "{\"jsonrpc\": \"2.0\", \"method\": \"%.100s\", \"params\": %.3000s, \"id\": 105}", 
                            tool, params);
                    } else {
                        snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                            "{\"jsonrpc\": \"2.0\", \"method\": \"%.100s\", \"params\": {}, \"id\": 105}", 
                            tool);
                    }
                } else if (strncmp(input_buf, "optimize ", 9) == 0) {
                    snprintf((char*)telem->request_cmd, sizeof(telem->request_cmd), 
                        "{\"jsonrpc\": \"2.0\", \"method\": \"genetic.autonomous_optimize\", \"params\": {\"target\": \"%.100s\"}, \"id\": 108}", 
                        input_buf + 9);
                } else if (strcmp(input_buf, "exit") == 0) {
                    kill(pid, SIGTERM);
                    break;
                }
            } else {
                tsfi_raw_usleep(10000);
            }
        } else {
            void *paddrs[1] = { (void*)&telem->header_crc };
            uint32_t pmasks[1] = { telem->header_crc };
            tsfi_io_wait_wavefront(paddrs, pmasks, 1, 50);
        }

        uint32_t current_crc = calculate_telemetry_crc(telem);
        bool integrity_valid = (current_crc == telem->header_crc);

        // --- Stut-Detection Logic ---
        static uint64_t last_exec_steps = 0;
        static time_t last_step_time = 0;
        if (telem->exec_steps != last_exec_steps) {
            last_exec_steps = telem->exec_steps;
            last_step_time = time(NULL);
        } else if (last_step_time != 0 && (time(NULL) - last_step_time) > 1) {
            // Target has stalled for 1 second without advancing steps
            if (telem->throttle_count == 0) {
                tsfi_io_printf(stderr, "\n[FRACTURE] Systemic Stall Detected (No Throttle Signaling).\n");
            }
        }

        static int is_tty = -1;
        if (is_tty == -1) is_tty = isatty(STDOUT_FILENO);

        if (is_tty) {
            tsfi_io_printf(stdout, "\033[H");
            tsfi_io_printf(stdout, "=== TSFI GLASS COCKPIT ===\n");
            tsfi_io_printf(stdout, "Target: %d | Integrity: %s | read, grep, find, store, wave, tools, resources, call, exit\n", pid, integrity_valid ? "OK" : "ERR");
            tsfi_io_printf(stdout, "----------------------------------------------------------------------\n");
            tsfi_io_printf(stdout, "ALLOCS : %lu | FREES: %lu | ACTIVE: %lu | HEAP: %.2f MB\n", 
                   telem->total_allocs, telem->total_frees, telem->active_allocs, (double)telem->active_bytes / 1048576.0);
            tsfi_io_printf(stdout, "----------------------------------------------------------------------\n");
            tsfi_io_printf(stdout, "EXEC   : %lu | LAST: %s\n", telem->exec_steps, (char*)telem->last_directive_str);
            tsfi_io_printf(stdout, "THROTTLE: %lu | STALL: %.2f ms\n", telem->throttle_count, (double)telem->total_stall_ns / 1000000.0);
            tsfi_io_printf(stdout, "----------------------------------------------------------------------\n");
            
            // --- Little Petya Spider Forensic Grid (8x5) ---
            tsfi_io_printf(stdout, "ARENA SPIDER COVERAGE: %.1f%%", telem->coverage_pct * 100.0f);
            if (telem->xor_trap_active) {
                tsfi_io_printf(stdout, " | \033[33mXOR TRAP ACTIVE (SPIDER FILL)\033[0m\n");
            } else {
                tsfi_io_printf(stdout, " | XOR Trap Idle\n");
            }
            uint64_t s_grid = atomic_load(&telem->spider_grid);
            uint64_t c_grid = atomic_load(&telem->covered_grid);
            for (int y = 0; y < 5; y++) {
                tsfi_io_printf(stdout, "  ");
                for (int x = 0; y * 8 + x < 40 && x < 8; x++) {
                    int bit = y * 8 + x;
                    bool has_spider = (s_grid >> bit) & 1;
                    bool is_covered = (c_grid >> bit) & 1;
                    if (has_spider) tsfi_io_printf(stdout, "\033[32m[X]\033[0m "); // Green X for Spider
                    else if (is_covered) tsfi_io_printf(stdout, "[.] ");           // Covered cell
                    else tsfi_io_printf(stdout, "\033[31m[?]\033[0m ");           // Red ? for Fracture
                }
                tsfi_io_printf(stdout, "\n");
            }
            tsfi_io_printf(stdout, "----------------------------------------------------------------------\n");
            
            tsfi_io_printf(stdout, "ZMM MSG: %.70s\n", (char*)telem->zmm_msg);
            tsfi_io_printf(stdout, "ZMM VAL: %lu\n", telem->zmm_val);
            tsfi_io_printf(stdout, "----------------------------------------------------------------------\n");
            
            // --- Physical Forensic Display ---
            tsfi_io_printf(stdout, "3D RENDER FORENSICS:\n");
            tsfi_io_printf(stdout, "  -> Blue Secret Flux: %.4f\n", telem->render_telemetry.blue_secret_jitter);
            tsfi_io_printf(stdout, "  -> Structural Rigidity: %.2f%%\n", telem->render_telemetry.magnetic_yi_rigidity * 100.0f);
            tsfi_io_printf(stdout, "  -> Petal Irid Shift: %.3f\n", telem->render_telemetry.petal_irid_shift);
            tsfi_io_printf(stdout, "  -> Path-Trace Power: %.1f M-evals/s\n", telem->render_telemetry.path_trace_m_evals);
            const char* stages[8] = {"SEED", "SPROUT", "STALK", "TYPE", "BUD", "BLOOM", "BLOOM", "SENESCENCE"};
            tsfi_io_printf(stdout, "  -> Lifecycle Epoch: [%s]\n", stages[telem->render_telemetry.current_stage % 8]);
            tsfi_io_printf(stdout, "----------------------------------------------------------------------\n");

            // Render the last 5 events
            uint32_t head = atomic_load(&telem->event_head);
            tsfi_io_printf(stdout, "RECENT TELEMETRY EVENTS:\n");
            for (int i = 0; i < 5; i++) {
                if (head <= (uint32_t)i) break;
                uint32_t idx = (head - 1 - i) % LAU_TELEM_RING_SIZE;
                LauTelemetryEvent *e = &telem->events[idx];
                const char *type_str = "UNKNOWN";
                if (e->type == 1) type_str = "ALLOC";
                if (e->type == 3) type_str = "FREE";
                if (e->type == 4) type_str = "SEAL";
                if (e->type == 5) type_str = "UNSEAL";
                tsfi_io_printf(stdout, "  [%s] Addr: 0x%lx | Size: %lu\n", type_str, (unsigned long)e->ptr_addr, (unsigned long)e->size);
            }
            tsfi_io_printf(stdout, "----------------------------------------------------------------------\n");
            tsfi_io_printf(stdout, "RECIPROCITY WAVE\n");
            tsfi_io_printf(stdout, "Symmetry: %.1f%%\n", telem->recip_symmetry * 100.0f);
            tsfi_io_printf(stdout, "----------------------------------------------------------------------\n");
            tsfi_io_printf(stdout, "\033[J");
        } else {
            // Write structured log entry
            tsfi_io_printf(stdout, "[COCKPIT_TICK] Target: %d | ALLOCS: %lu | ACTIVE: %lu | HEAP: %.2f MB | COVERAGE: %.1f%% | LAST_DIR: %s | ZMM: %.70s (%lu)\n", 
                   pid, telem->total_allocs, telem->active_allocs, (double)telem->active_bytes / 1048576.0, 
                   telem->coverage_pct * 100.0f, (char*)telem->last_directive_str, (char*)telem->zmm_msg, telem->zmm_val);
        }
        tsfi_io_flush(stdout);
        
        // Native 5 second timeout to safely cap test execution
        time_t current_time = time(NULL);
        if (current_time - start_time >= 5) {
            tsfi_io_printf(stdout, "\n[COCKPIT] Target exceeded 5 second timeout. Terminating.\n");
            kill(pid, SIGTERM);
            // Give it a moment to die gracefully
            struct timeval tv_die = {0, 100000}; // 100ms
            select(0, NULL, NULL, NULL, &tv_die);
            if (waitpid(pid, &status, WNOHANG) == 0) {
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
            }
            final_status = status;
            break;
        }
    }
    
    int exit_code = 0;
    if (WIFEXITED(final_status)) {
        exit_code = WEXITSTATUS(final_status);
        tsfi_io_printf(stdout, "\nTarget exited with status %d\n", exit_code);
    } else if (WIFSIGNALED(final_status)) {
        exit_code = 128 + WTERMSIG(final_status);
        tsfi_io_printf(stdout, "\nTarget terminated by signal %d\n", WTERMSIG(final_status));
    }

    tsfi_shm_unlink(shm_name);
    
    extern void tsfi_io_cleanup(void);
    tsfi_io_cleanup();

    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return exit_code;
}
