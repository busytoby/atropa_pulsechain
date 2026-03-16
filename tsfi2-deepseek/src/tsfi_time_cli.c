#include "tsfi_time.h"
#include "tsfi_raw.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>

void print_usage(const char *prog) {
    tsfi_io_printf(stderr, "Usage: %s <cmd> [args]\n", prog);
    tsfi_io_printf(stderr, "Commands:\n");
    tsfi_io_printf(stderr, "  exec [opts] -- <cmd> Wait, time, and oversee a binary execution\n");
    tsfi_io_printf(stderr, "    --timeout <sec>    Kill process after N seconds\n");
    tsfi_io_printf(stderr, "    --name <label>     Log identifier\n");
    tsfi_io_printf(stderr, "    --quiet            Suppress PASS logs\n");
    tsfi_io_printf(stderr, "    --json             Output JSON report to stderr\n");
    tsfi_io_printf(stderr, "    --env KEY=VAL      Set environment variable\n");
    tsfi_io_printf(stderr, "  wait <seconds>       Wait using PAUSE (Low Power)\n");
    tsfi_io_printf(stderr, "  burn <seconds>       Wait using Wave512 FMA (High Power)\n");
    tsfi_io_printf(stderr, "  freq                 Measure CPU frequency under Wave512 load\n");
    tsfi_io_printf(stderr, "  stamp <text>         Output formatted 'TIMESTAMP text' for LAU\n");
    tsfi_io_printf(stderr, "  --frames <n>         Wait N frames (at 60Hz)\n");
}

int cmd_exec(int argc, char **argv) {
    double timeout = 0.0;
    const char *name = "process";
    int quiet = 0;
    int json_output = 0;
    int cmd_idx = 0;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            timeout = atof(argv[++i]);
        } else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
            name = argv[++i];
        } else if (strcmp(argv[i], "--quiet") == 0) {
            quiet = 1;
        } else if (strcmp(argv[i], "--json") == 0) {
            json_output = 1;
        } else if (strcmp(argv[i], "--env") == 0 && i + 1 < argc) {
            char *env = argv[++i];
            char *eq = strchr(env, '=');
            if (eq) {
                *eq = '\0';
                setenv(env, eq + 1, 1);
                *eq = '=';
            }
        } else if (strcmp(argv[i], "--") == 0) {
            cmd_idx = i + 1;
            break;
        } else {
            // If it starts with '-', assume unknown flag (or misplaced).
            // If not, it's the command start.
            if (argv[i][0] != '-') {
                cmd_idx = i;
                break;
            }
        }
    }

    if (cmd_idx == 0 || cmd_idx >= argc) {
        tsfi_io_printf(stderr, "[OVERSIGHT] Error: No command specified.\n");
        return 127;
    }

    unsigned long long start_ns = get_time_ns();
    pid_t pid = fork();

    if (pid < 0) {
        perror("[OVERSIGHT] fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child
        execvp(argv[cmd_idx], &argv[cmd_idx]);
        perror("[OVERSIGHT] execvp failed");
        exit(127);
    }

    // Parent
    int status;
    int exit_code = 0;
    unsigned long long timeout_ns = (unsigned long long)(timeout * 1e9);
    
    while (1) {
        int w = waitpid(pid, &status, WNOHANG);
        if (w == -1) {
            if (errno == EINTR) continue;
            perror("[OVERSIGHT] waitpid failed");
            return 1;
        }
        if (w > 0) {
            if (WIFEXITED(status)) exit_code = WEXITSTATUS(status);
            else if (WIFSIGNALED(status)) exit_code = 128 + WTERMSIG(status);
            else exit_code = 1;
            break;
        }

        unsigned long long current_ns = get_time_ns();
        if (timeout > 0 && (current_ns - start_ns) > timeout_ns) {
            tsfi_io_printf(stderr, "[OVERSIGHT] Timeout reached (%.2fs). Killing '%s' (PID %d)...\n", timeout, name, pid);
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            exit_code = 124;
            break;
        }
        tsfi_raw_usleep(10000); 
    }

    unsigned long long end_ns = get_time_ns();
    double dt = (double)(end_ns - start_ns) / 1e9;

    struct rusage usage;
    long maxrss = 0;
    double utime = 0, stime = 0;
    if (getrusage(RUSAGE_CHILDREN, &usage) == 0) {
        maxrss = usage.ru_maxrss; // KB on Linux
        utime = (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec / 1e6;
        stime = (double)usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec / 1e6;
    }

    if (json_output) {
        tsfi_io_printf(stderr, "{\"name\": \"%s\", \"status\": \"%s\", \"exit_code\": %d, \"duration\": %.4f, \"rss_mb\": %.2f, \"cpu_user\": %.2f, \"cpu_sys\": %.2f}\n",
            name, (exit_code == 0 ? "PASS" : "FAIL"), exit_code, dt, (double)maxrss/1024.0, utime, stime);
    } else {
        if (exit_code == 0) {
            if (!quiet) {
                tsfi_io_printf(stderr, "[OVERSIGHT] [PASS] %s finished in %.4fs | RSS: %.2f MB | CPU: %.2fu/%.2fs\n", 
                    name, dt, (double)maxrss/1024.0, utime, stime);
            }
        } else {
            tsfi_io_printf(stderr, "[OVERSIGHT] [FAIL] %s failed (Exit: %d) in %.4fs | RSS: %.2f MB | CPU: %.2fu/%.2fs\n", 
                name, exit_code, dt, (double)maxrss/1024.0, utime, stime);
        }
    }

    return exit_code;
}

int main(int argc, char **argv) {
    if (argc < 2) { print_usage(argv[0]); return 1; }
    
    if (strcmp(argv[1], "exec") == 0) {
        return cmd_exec(argc - 2, argv + 2);
    }

    if (strcmp(argv[1], "--frames") == 0) {
        if (argc < 3) return 1;
        tsfi_time_wait_ns((long long)atoi(argv[2]) * 16666666LL);
        return 0;
    }
    if (strcmp(argv[1], "wait") == 0) {
        if (argc < 3) return 1;
        tsfi_time_wait_ns((long long)(atof(argv[2]) * 1e9));
        return 0;
    }
    if (strcmp(argv[1], "burn") == 0) {
        if (argc < 3) return 1;
        tsfi_time_burn_avx512_ns((long long)(atof(argv[2]) * 1e9));
        return 0;
    }
    if (strcmp(argv[1], "freq") == 0) { measure_frequency(); return 0; }
    if (strcmp(argv[1], "stamp") == 0) { if (argc < 3) return 1; tsfi_io_printf(stdout, "%s\n", argv[2]); return 0; }
    
    char *end; double s = strtod(argv[1], &end);
    if (end != argv[1]) { tsfi_time_wait_ns((long long)(s * 1e9)); return 0; }
    
    print_usage(argv[0]); return 1;
}