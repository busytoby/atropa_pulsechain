#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include "tsfi_zmm_vm.h"
#include "lau_memory.h"
#include "tsfi_zmm_rpc.h"
#include "lau_telemetry.h"
#include "tsfi_raw.h"
#include "tsfi_io.h"
#include "tsfi_block_monitor.h"

extern void tsfi_zmm_rpc_step_async_llm(TsfiZmmVmState *state);

// In-Memory MCP Loop (Bridges stdio, Network Port 10042, and SHM Command Channel)

typedef struct {
    const char *name;
    const char *path;
    const char *virt_addr;
} DysnomiaContract;

extern const DysnomiaContract g_dysnomia_system[];
extern const size_t g_dysnomia_system_count;

typedef struct {
    int server_fd;
    TsfiZmmVmState *state;
    size_t cap;
} TCPThreadArg;

static void* tcp_server_thread(void *arg) {
    TCPThreadArg *targ = (TCPThreadArg*)arg;
    int server_fd = targ->server_fd;
    TsfiZmmVmState *state = targ->state;
    size_t cap = targ->cap;
    free(targ);

    char *input = (char*)malloc(cap);
    char *output = (char*)malloc(cap);
    if (!input || !output) return NULL;

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd >= 0) {
            static time_t g_lut = 0;
            static int g_urc = 0;
            time_t now = time(NULL);
            if (g_lut == 0 || now - g_lut >= 3600) {
                g_lut = now;
                g_urc = 0;
            }
            if (0) { // Disable rate limit
                const char *err_msg = "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32001,\"message\":\"RATE_LIMIT_EXCEEDED_MAX_1_PER_HOUR\"}}";
                ssize_t nw = write(client_fd, err_msg, strlen(err_msg));
                (void)nw;
                close(client_fd);
                continue;
            }
            g_urc++;

            struct pollfd pfd;
            pfd.fd = client_fd;
            pfd.events = POLLIN;
            if (poll(&pfd, 1, 1000) > 0) {
                ssize_t n = read(client_fd, input, cap - 1);
                if (n > 0) {
                    input[n] = 0;
                    
                    bool is_http = false;
                    bool is_options = false;
                    char *body_ptr = input;
                    
                    if (strncmp(input, "OPTIONS ", 8) == 0) {
                        is_http = true;
                        is_options = true;
                    } else if (strncmp(input, "POST ", 5) == 0) {
                        is_http = true;
                        char *hdr_end = strstr(input, "\r\n\r\n");
                        if (hdr_end && (size_t)(hdr_end - input) < (size_t)n) {
                            body_ptr = hdr_end + 4;
                        } else {
                            body_ptr = input;
                        }
                    }
                    
                    if (is_http) {
                        if (is_options) {
                            const char *resp = 
                                "HTTP/1.1 204 No Content\r\n"
                                "Access-Control-Allow-Origin: *\r\n"
                                "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                                "Access-Control-Allow-Headers: Content-Type\r\n"
                                "Access-Control-Allow-Private-Network: true\r\n"
                                "Connection: close\r\n"
                                "\r\n";
                            ssize_t nw = write(client_fd, resp, strlen(resp));
                            (void)nw;
                        } else {
                            if (tsfi_zmm_rpc_dispatch(state, body_ptr, output, cap)) {
                                char http_header[1024];
                                int h_len = snprintf(http_header, sizeof(http_header),
                                                     "HTTP/1.1 200 OK\r\n"
                                                     "Content-Type: application/json\r\n"
                                                     "Access-Control-Allow-Origin: *\r\n"
                                                     "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                                                     "Access-Control-Allow-Headers: Content-Type\r\n"
                                                     "Access-Control-Allow-Private-Network: true\r\n"
                                                     "Content-Length: %zu\r\n"
                                                     "Connection: close\r\n"
                                                     "\r\n",
                                                     strlen(output));
                                ssize_t nw = write(client_fd, http_header, h_len);
                                (void)nw;
                                nw = write(client_fd, output, strlen(output));
                                (void)nw;
                            }
                        }
                    } else {
                        if (tsfi_zmm_rpc_dispatch(state, input, output, cap)) {
                            ssize_t nw = write(client_fd, output, strlen(output));
                            (void)nw;
                        }
                    }
                }
            }
            close(client_fd);
        } else {
            struct pollfd pfd;
            pfd.fd = server_fd;
            pfd.events = POLLIN;
            poll(&pfd, 1, 100);
        }
    }
    free(input);
    free(output);
    return NULL;
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    
    extern void tsfi_wire_firmware_init(void);
    tsfi_wire_firmware_init();

    TsfiZmmVmState state;
    tsfi_zmm_vm_init(&state);
    tsfi_block_monitor_init();

    const char *id = getenv("TSFI_TELEMETRY_ID");
    if (id) {
        tsfi_zmm_vm_attach_telemetry(&state, id);
        tsfi_io_printf(stderr, "[MCP] Attached to Telemetry ID: %s\n", id);
    }

    tsfi_io_printf(stderr, "[ZMM_VM] Autoloading Dysnomia contracts...\n");
    int count = (int)g_dysnomia_system_count;
    for (int i = 0; i < count; i++) {
        char cmd[1024];
        uint64_t addr64 = 0;
        const char *addr_str = g_dysnomia_system[i].virt_addr;
        if (addr_str[0] == '0' && (addr_str[1] == 'x' || addr_str[1] == 'X')) addr_str += 2;
        size_t len = strlen(addr_str);
        if (len > 16) {
            sscanf(addr_str + len - 16, "%lx", &addr64);
        } else {
            sscanf(addr_str, "%lx", &addr64);
        }
        if (strcmp(g_dysnomia_system[i].name, "shafactory") == 0) {
            for (int r = 0; r < 5; r++) {
                tsfi_zmm_vm_exec(&state, "YULEXEC \"vmreq\", \"604a6fa9\"");
            }
        }
        snprintf(cmd, sizeof(cmd), "YULINIT \"%s\", \"%s\", %lu", 
                 g_dysnomia_system[i].name, 
                 g_dysnomia_system[i].path, 
                 addr64);
        tsfi_zmm_vm_exec(&state, cmd);
    }
    
     // LAU
     const char *calldata_new = "8c6dc5ec00000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000c5573657220546f6b656e20310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a55534552544f4b454e3100000000000000000000000000000000000000000000";
     char exec_cmd[1024];
     snprintf(exec_cmd, sizeof(exec_cmd), "YULEXEC \"laufactory\", \"%s\"", calldata_new);
     tsfi_io_printf(stderr, "[ZMM_VM] Creating default LAU token: %s\n", exec_cmd);
     tsfi_zmm_vm_exec(&state, exec_cmd);
     
     // Bind
     tsfi_io_printf(stderr, "[ZMM_VM] Binding zmachineParser to zmachine...\n");
     tsfi_zmm_vm_exec(&state, "YULEXEC \"zmachine\", \"7e1ef7e90000000000000000000000000000000000000000000000000000000000000006\"");
 
     // Rehydrate
     extern void blue_box_rehydrate_quadtree_states(void);
     blue_box_rehydrate_quadtree_states();
 
     size_t cap = 1024 * 1024 * 4; 
     char *input = (char*)lau_malloc(cap);
     char *output = (char*)lau_malloc(cap);
     if (!input || !output) return 1;
 
     // Bridge
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

    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    if (server_fd >= 0) {
        TCPThreadArg *targ = (TCPThreadArg*)malloc(sizeof(TCPThreadArg));
        targ->server_fd = server_fd;
        targ->state = &state;
        targ->cap = cap;
        pthread_t tid;
        pthread_create(&tid, NULL, tcp_server_thread, targ);
        pthread_detach(tid);
    }

    // Stream Loop
    while (1) {
        // 1. Check SHM Command Channel
        if (state.telem && state.telem->request_cmd[0] == '{') {
            char cmd[4096];
            snprintf(cmd, sizeof(cmd), "%.4095s", (char*)state.telem->request_cmd);
            state.telem->request_cmd[0] = 0; // Clear it

            if (tsfi_zmm_rpc_dispatch(&state, cmd, output, cap)) {
                snprintf((char*)state.telem->zmm_msg, sizeof(state.telem->zmm_msg), "%s", output);
            }
        }

        // 2. Check Input Sources (Non-blocking poll)
        if (poll(fds, 1, 10) > 0) {
            // Check STDIN
            if (fds[0].revents & POLLIN) {
                if (fgets(input, cap, stdin)) {
                    if (strstr(input, "\"method\": \"exit\"") || strstr(input, "\"method\":\"exit\"")) {
                        break;
                    }
                    if (tsfi_zmm_rpc_dispatch(&state, input, output, cap)) {
                        // Stdin commands processed; output not written to stdout as server runs as a daemon
                    }
                }
            }
        }
        
         tsfi_zmm_rpc_step_async_llm(&state);
         tsfi_block_monitor_tick(&state);
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