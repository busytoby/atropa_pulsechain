#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "tsfi_zmm_vm.h"
#include "lau_memory.h"
#include "tsfi_zmm_rpc.h"
#include "lau_telemetry.h"
#include "tsfi_raw.h"
#include "tsfi_io.h"

// In-Memory MCP Loop (Bridges stdio, Network Port 10042, and SHM Command Channel)

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    TsfiZmmVmState state;
    tsfi_zmm_vm_init(&state);

    const char *id = getenv("TSFI_TELEMETRY_ID");
    if (id) {
        tsfi_zmm_vm_attach_telemetry(&state, id);
        tsfi_io_printf(stderr, "[MCP] Attached to Telemetry ID: %s\n", id);
    }

    size_t cap = 1024 * 1024 * 4; 
    char *input = (char*)lau_malloc(cap);
    char *output = (char*)lau_malloc(cap);
    if (!input || !output) return 1;

    // --- Initialize Network Bridge (Google Flow Ingest) ---
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd >= 0) {
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        fcntl(server_fd, F_SETFL, O_NONBLOCK);
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        int port = 10042;
        const char *p_env = getenv("TSFI_MCP_PORT");
        if (p_env) port = atoi(p_env);
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            tsfi_io_printf(stderr, "[MCP] Network Bind Failed on %d\n", port);
            close(server_fd);
            server_fd = -1;
        } else {
            listen(server_fd, 3);
            tsfi_io_printf(stderr, "[MCP] Google Flow Bridge Listening on Port %d\n", port);
        }
    }

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = server_fd;
    fds[1].events = (server_fd >= 0) ? POLLIN : 0;

    // Stream Loop
    while (1) {
        // 1. Check SHM Command Channel
        if (state.telem && state.telem->request_cmd[0] == '{') {
            char cmd[4096];
            snprintf(cmd, sizeof(cmd), "%.4095s", (char*)state.telem->request_cmd);
            state.telem->request_cmd[0] = 0; // Clear it

            if (tsfi_zmm_rpc_dispatch(&state, cmd, output, cap)) {
                // Write response to SHM for Cockpit to see
                snprintf((char*)state.telem->zmm_msg, sizeof(state.telem->zmm_msg), "%s", output);
                // Also log to stdout for debugging
                tsfi_io_printf(stderr, "[MCP SHM] %s", output);
            }
        }

        // 2. Check Input Sources (Non-blocking poll)
        if (poll(fds, 2, 10) > 0) {
            // Check STDIN
            if (fds[0].revents & POLLIN) {
                if (fgets(input, cap, stdin)) {
                    if (strstr(input, "\"method\": \"exit\"") || strstr(input, "\"method\":\"exit\"")) {
                        break;
                    }
                    if (tsfi_zmm_rpc_dispatch(&state, input, output, cap)) {
                        tsfi_io_printf(stdout, "%s", output);
                    }
                }
            }
            
            // Check Network (Accept and handle one request at a time)
            if (server_fd >= 0 && (fds[1].revents & POLLIN)) {
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
                if (client_fd >= 0) {
                    ssize_t n = read(client_fd, input, cap - 1);
                    if (n > 0) {
                        input[n] = 0;
                        if (tsfi_zmm_rpc_dispatch(&state, input, output, cap)) {
                            ssize_t nw = write(client_fd, output, strlen(output));
                            if (nw < 0) perror("write failed");
                        }
                    }
                    close(client_fd);
                }
            }
        }
        
        tsfi_raw_usleep(1000); // Prevent 100% CPU
    }
    
    lau_free(input);
    lau_free(output);
    
    // Decompose sovereign execution geometrieZ spawned by VM execution
    extern void tsfi_wire_firmware_teardown(void);
    extern void tsfi_dysnomia_drain_pools(void);
    extern void tsfi_dl_thunks_teardown(void);
    tsfi_wire_firmware_teardown();
    tsfi_dysnomia_drain_pools();
    tsfi_dl_thunks_teardown();
    
    tsfi_zmm_vm_destroy(&state);
    
    // Explicitly enforce geometric bounds verification
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
