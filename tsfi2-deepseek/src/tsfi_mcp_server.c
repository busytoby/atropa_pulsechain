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

typedef struct {
    const char *name;
    const char *path;
    const char *virt_addr;
} DysnomiaContract;

static const DysnomiaContract g_dysnomia_system[] = {
    { "vmreq",        "../solidity/dysnomia/00b_vmreq.sol",              "0x10" },
    { "shafactory",   "../solidity/dysnomia/02c_shafactory.sol",          "0x11" },
    { "shiofactory",  "../solidity/dysnomia/03c_shiofactory.sol",         "0x12" },
    { "yi",           "../solidity/dysnomia/04_yi.sol",                  "0x83a918056aB9316837Dc48a216119D679D561d91" },
    { "zheng",        "../solidity/dysnomia/05_zheng.sol",               "0x14" },
    { "zhou",         "../solidity/dysnomia/06_zhou.sol",                "0xe5d3A6e88590fc2A8037D9CCbd816C05B1ff5f11" },
    { "yau",          "../solidity/dysnomia/07_yau.sol",                 "0xb9A44De20f26a027e467CB6c2F98766F01904189" },
    { "yang",         "../solidity/dysnomia/08_yang.sol",                "0xFE9b99eCC43cb423408b975cc5ff439e5ABaCb61" },
    { "siu",          "../solidity/dysnomia/09_siu.sol",                 "0xb4C1248812dAbF72cb2e82175b4c0aCffE4D2b10" },
    { "void",         "../solidity/dysnomia/10_void.sol",                "0xCd19062a6d3019b02A676D72e51D8de7A398dE25" },
    { "laufactory",   "../solidity/dysnomia/11c_laufactory.sol",         "0x0EB4EE7d5Ff28cbF68565A174f7E5e186c36B4b3" },
    { "lau",          "../solidity/dysnomia/11_lau.sol",                  "0x25" },
    { "cho",          "../solidity/dysnomia/domain/dan/01_cho.sol",      "0x1b" },
    { "map",          "../solidity/dysnomia/domain/map.sol",             "0x1c" },
    { "qi",           "../solidity/dysnomia/domain/soeng/01_qi.sol",     "0x1b8F9E19360D1dc94295D984b7Ca7eA9b810D9ee" },
    { "mai",          "../solidity/dysnomia/domain/soeng/02_mai.sol",     "0xf69e9f943674027Cedf05564A8D5A01041d07c62" },
    { "xia",          "../solidity/dysnomia/domain/soeng/03_xia.sol",     "0x347BC40503E0CE23fE0F5587F232Cd2D07D4Eb89" },
    { "xie",          "../solidity/dysnomia/domain/soeng/04_xie.sol",     "0x2556F7f8d82EbcdD7b821b0981C38D9dA9439CdD" },
    { "chan",         "../solidity/dysnomia/domain/sky/01_chan.sol",     "0x7674516ad438dd67A057fBc1119168d9A7d2a9B1" },
    { "sei",          "../solidity/dysnomia/domain/tang/01_sei.sol",     "0x8B090509eAe0fEB4A0B934de1b4345161fA9a62d" },
    { "choa",         "../solidity/dysnomia/domain/sky/02_choa.sol",     "0xA63F8061A67ecdbf147Cd1B60f91Cf95464E868D" },
    { "cheon",        "../solidity/dysnomia/domain/tang/02_cheon.sol",    "0x840CBD20A70774BECAc4e932Fff6fb1f5417997F" },
    { "cabsMarketMachine", "../solidity/dysnomia/domain/sky/CABSMarketMachine.sol", "0x74ef2B06A1D2035C33244A4a263FF00B84504865" },
    { "zmachine",       "../solidity/bin/zmachine.yul",             "5" },
    { "zmachineParser", "../solidity/bin/zmachineParser.yul",       "6" }
};

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    
    extern void tsfi_wire_firmware_init(void);
    tsfi_wire_firmware_init();

    TsfiZmmVmState state;
    tsfi_zmm_vm_init(&state);

    const char *id = getenv("TSFI_TELEMETRY_ID");
    if (id) {
        tsfi_zmm_vm_attach_telemetry(&state, id);
        tsfi_io_printf(stderr, "[MCP] Attached to Telemetry ID: %s\n", id);
    }

    tsfi_io_printf(stderr, "[ZMM_VM] Autoloading Dysnomia contracts...\n");
    int count = sizeof(g_dysnomia_system) / sizeof(g_dysnomia_system[0]);
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
        snprintf(cmd, sizeof(cmd), "YULINIT \"%s\", \"%s\", %lu", 
                 g_dysnomia_system[i].name, 
                 g_dysnomia_system[i].path, 
                 addr64);
        tsfi_zmm_vm_exec(&state, cmd);
    }
    
    // Create default User LAU token on startup:
    const char *calldata_new = "8c6dc5ec00000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000c5573657220546f6b656e20310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a55534552544f4b454e3100000000000000000000000000000000000000000000";
    char exec_cmd[1024];
    snprintf(exec_cmd, sizeof(exec_cmd), "YULEXEC \"laufactory\", \"%s\"", calldata_new);
    tsfi_io_printf(stderr, "[ZMM_VM] Creating default LAU token: %s\n", exec_cmd);
    tsfi_zmm_vm_exec(&state, exec_cmd);
    
    // Bind zmachineParser (6) to zmachine (5) to enable parseCommand thunking
    tsfi_io_printf(stderr, "[ZMM_VM] Binding zmachineParser to zmachine...\n");
    tsfi_zmm_vm_exec(&state, "YULEXEC \"zmachine\", \"7e1ef7e90000000000000000000000000000000000000000000000000000000000000006\"");

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
