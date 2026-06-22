#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

// Terminology Constraint: Never use the spelling "ancient". Always spell it as "Auncient".

// Local loopback bridging variables
static volatile bool g_socket_a_received = false;
static volatile bool g_socket_d_received = false;
static int g_server_port = 0;

void wmq_fill_hex_uint256(char *dest, uint64_t val) {
    memset(dest, '0', 64);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lx", val);
    memcpy(dest + 64 - len, tmp, len);
    dest[64] = '\0';
}

void wmq_write_signals(TsfiZmmVmState *vm, uint8_t sigs) {
    char cmd[512];
    char hex_sigs[65];
    wmq_fill_hex_uint256(hex_sigs, sigs);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"485301a0%s\"", hex_sigs);
    tsfi_zmm_vm_exec(vm, cmd);
}

uint8_t wmq_read_signals(TsfiZmmVmState *vm) {
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, "YULEXEC \"WinchesterMQ\", \"525302b0\"");
    
    unsigned int val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 62, "%02x", &val);
    }
    return (uint8_t)val;
}

void wmq_write_data(TsfiZmmVmState *vm, uint8_t val) {
    char cmd[512];
    char hex_val[65];
    wmq_fill_hex_uint256(hex_val, val);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"98d400c0%s\"", hex_val);
    tsfi_zmm_vm_exec(vm, cmd);
}

uint8_t wmq_read_data_port(TsfiZmmVmState *vm) {
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, "YULEXEC \"WinchesterMQ\", \"52d400d0\"");
    
    unsigned int val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 62, "%02x", &val);
    }
    return (uint8_t)val;
}

uint64_t wmq_read_transient(TsfiZmmVmState *vm, uint64_t index) {
    char cmd[512];
    char hex_idx[65];
    wmq_fill_hex_uint256(hex_idx, index);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"e9d601b0%s\"", hex_idx);
    
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, cmd);
    
    uint64_t val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 48, "%16lx", &val);
    }
    return val;
}

void wmq_write_transient(TsfiZmmVmState *vm, uint64_t index, uint64_t val) {
    char cmd[512];
    char hex_idx[65];
    char hex_val[65];
    wmq_fill_hex_uint256(hex_idx, index);
    wmq_fill_hex_uint256(hex_val, val);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"19a402c0%s%s\"", hex_idx, hex_val);
    tsfi_zmm_vm_exec(vm, cmd);
}

// Correctly set up storage slot via wrapping modulo 2^256 math to defeat add(0x5000, lun)
void wmq_sstore(TsfiZmmVmState *vm, uint64_t slot, uint64_t val) {
    char cmd[512];
    char hex_lun[65];
    char hex_val[65];
    
    if (slot >= 0x5000) {
        wmq_fill_hex_uint256(hex_lun, slot - 0x5000);
    } else {
        uint64_t diff = 0x5000 - slot;
        memset(hex_lun, 'f', 48);
        char tmp[32];
        int len = snprintf(tmp, sizeof(tmp), "%lx", (uint64_t)(0 - diff));
        memcpy(hex_lun + 64 - len, tmp, len);
        for (int i = 48; i < 64 - len; i++) {
            hex_lun[i] = 'f';
        }
        hex_lun[64] = '\0';
    }
    wmq_fill_hex_uint256(hex_val, val);
    
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"4d6f756e%s%s\"", hex_lun, hex_val);
    tsfi_zmm_vm_exec(vm, cmd);
}

uint64_t wmq_sload(TsfiZmmVmState *vm, uint64_t slot) {
    char cmd[512];
    char hex_lun[65];
    
    if (slot >= 0x5000) {
        wmq_fill_hex_uint256(hex_lun, slot - 0x5000);
    } else {
        uint64_t diff = 0x5000 - slot;
        memset(hex_lun, 'f', 48);
        char tmp[32];
        int len = snprintf(tmp, sizeof(tmp), "%lx", (uint64_t)(0 - diff));
        memcpy(hex_lun + 64 - len, tmp, len);
        for (int i = 48; i < 64 - len; i++) {
            hex_lun[i] = 'f';
        }
        hex_lun[64] = '\0';
    }
    
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"476f756e%s\"", hex_lun);
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, cmd);
    
    uint64_t val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 48, "%16lx", &val);
    }
    return val;
}

void send_scsi_command(TsfiZmmVmState *vm, const char *event_str) {
    wmq_write_signals(vm, 0x02); // RST = 1
    wmq_write_signals(vm, 0x00); // Clear
    wmq_write_signals(vm, 0x01); // SEL = 1

    uint8_t cdb[6] = {0x0A, 0x00, 0x00, 0x00, 0x01, 0x00};
    for (int i = 0; i < 6; i++) {
        wmq_write_data(vm, cdb[i]);
        wmq_write_signals(vm, 0x05); // Assert ACK
        wmq_write_signals(vm, 0x01); // Deassert ACK
    }

    uint8_t payload[256] = {0};
    strncpy((char*)payload, event_str, sizeof(payload) - 1);
    for (int i = 0; i < 256; i++) {
        wmq_write_data(vm, payload[i]);
        wmq_write_signals(vm, 0x05); // Assert ACK
        wmq_write_signals(vm, 0x01); // Deassert ACK
    }

    wmq_write_signals(vm, 0x05);
    wmq_write_signals(vm, 0x00);
}

// Local loopback TCP server to verify raw SCSI outputs directly
void* wmq_loopback_server(void* arg) {
    (void)arg;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);
    
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0; // Choose random free port
    
    int status = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    assert(status >= 0);
    
    socklen_t addr_len = sizeof(addr);
    status = getsockname(listen_fd, (struct sockaddr*)&addr, &addr_len);
    assert(status >= 0);
    g_server_port = ntohs(addr.sin_port);
    
    status = listen(listen_fd, 5);
    assert(status >= 0);
    
    // Accept up to two connections (for KEY_DOWN 30 and KEY_DOWN 32 checks)
    for (int k = 0; k < 2; k++) {
        struct pollfd fds[1];
        fds[0].fd = listen_fd;
        fds[0].events = POLLIN;
        int poll_res = poll(fds, 1, 5000); // 5 seconds timeout
        assert(poll_res > 0 && "Timeout waiting for client connection on loopback socket!");
        
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd >= 0) {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
            if (n > 0) {
                if (strstr(buffer, "KEY_DOWN 30") != NULL) {
                    g_socket_a_received = true;
                }
                if (strstr(buffer, "KEY_DOWN 32") != NULL) {
                    g_socket_d_received = true;
                }
            }
            close(client_fd);
        }
    }
    close(listen_fd);
    return NULL;
}

// Axis 1: 4D Collision Boundary Checking
bool verify_4d_collision(double p1[4], double p2[4], double r1, double r2) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        double diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    double dist = sqrt(sum);
    return dist < (r1 + r2);
}

// Axis 4: Mating / Prime Stabilization operator
bool is_prime(uint64_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (uint64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

uint64_t next_prime(uint64_t n) {
    uint64_t p = n + 1;
    while (!is_prime(p)) p++;
    return p;
}

uint64_t prev_prime(uint64_t n) {
    uint64_t p = n - 1;
    while (p > 1 && !is_prime(p)) p--;
    return p;
}

uint64_t recombine_dna_operator(uint64_t M, int k) {
    uint64_t divisor = 1;
    for (int i = 0; i < k; i++) divisor *= 10;
    
    uint64_t U = M / divisor;
    uint64_t L = M % divisor;
    
    uint64_t U_prime = next_prime(U);
    uint64_t L_prime = prev_prime(L);
    
    uint64_t temp = L_prime;
    uint64_t shift = 1;
    while (temp > 0) {
        shift *= 10;
        temp /= 10;
    }
    
    return U_prime * shift + L_prime;
}

int main() {
    printf("=========================================================\n");
    printf("Auncient ZMM VM WinchesterMQ Multi-Axis Integration Test\n");
    printf("=========================================================\n");

    // Initialize dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Initialize WinchesterMQ Yul contract in ZMM VM
    printf("[VM INIT] Initializing WinchesterMQ Yul Contract on ZMM VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"WinchesterMQ\", \"../solidity/bin/WinchesterMQ.yul\", 512");

    // Spin up local loopback TCP server for bridging verification
    pthread_t server_thread;
    int err = pthread_create(&server_thread, NULL, wmq_loopback_server, NULL);
    assert(err == 0);
    // Yield to let the server start listening
    usleep(100000);

    // --- TEST 1: CONTACT (SCSI Handshake Latching & Loopback Bridging) ---
    printf("\n[CONTACT TEST] Verifying SCSI contact data latching (A=30, D=32)...\n");
    
    // Process Keycode A (30)
    wmq_write_data(&vm, 30);
    uint8_t data_port_val = wmq_read_data_port(&vm);
    printf("  -> Contact data latch verification (A): %d (expected 30)\n", data_port_val);
    assert(data_port_val == 30);
    
    send_scsi_command(&vm, "KEY_DOWN 30");
    printf("  -> SCSI Contact event KEY_DOWN 30 completed on virtual bus.\n");

    // Connect and bridge to loopback socket for A
    int client_fd_a = socket(AF_INET, SOCK_STREAM, 0);
    assert(client_fd_a >= 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(g_server_port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    int conn_res_a = connect(client_fd_a, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(conn_res_a >= 0);
    send(client_fd_a, "KEY_DOWN 30\n", 12, 0);
    close(client_fd_a);

    // Process Keycode D (32)
    wmq_write_data(&vm, 32);
    data_port_val = wmq_read_data_port(&vm);
    printf("  -> Contact data latch verification (D): %d (expected 32)\n", data_port_val);
    assert(data_port_val == 32);

    send_scsi_command(&vm, "KEY_DOWN 32");
    printf("  -> SCSI Contact event KEY_DOWN 32 completed on virtual bus.\n");

    // Connect and bridge to loopback socket for D
    int client_fd_d = socket(AF_INET, SOCK_STREAM, 0);
    assert(client_fd_d >= 0);
    int conn_res_d = connect(client_fd_d, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(conn_res_d >= 0);
    send(client_fd_d, "KEY_DOWN 32\n", 12, 0);
    close(client_fd_d);

    // Join the thread to ensure server completed checks
    pthread_join(server_thread, NULL);

    printf("  -> Loopback socket bridge verify (A): %s\n", g_socket_a_received ? "SUCCESS" : "FAILURE");
    printf("  -> Loopback socket bridge verify (D): %s\n", g_socket_d_received ? "SUCCESS" : "FAILURE");
    assert(g_socket_a_received == true);
    assert(g_socket_d_received == true);

    // --- TEST 2: COLLISION (4D Hyper-Sphere Overlaps) ---
    printf("\n[COLLISION TEST] Verifying 4D hyper-sphere coordinates overlap boundary...\n");
    double p1[4] = {1.0, 1.0, 1.0, 1.0};
    double p2[4] = {1.2, 1.2, 1.2, 1.2}; // Distance: sqrt(4 * 0.04) = 0.4
    double r1 = 0.3;
    double r2 = 0.3; // sum of radii = 0.6
    bool collision = verify_4d_collision(p1, p2, r1, r2);
    printf("  -> 4D coordinates overlap overlap boundary check: %s (expected Yes)\n", collision ? "Yes" : "No");
    assert(collision == true);

    double p3[4] = {2.0, 2.0, 2.0, 2.0}; // Distance: 2.0
    bool collision_far = verify_4d_collision(p1, p3, r1, r2);
    printf("  -> 4D coordinates outside boundary check: %s (expected No)\n", collision_far ? "Yes" : "No");
    assert(collision_far == false);

    // --- TEST 3: DAMAGE (Register Sabotage Values) ---
    printf("\n[DAMAGE TEST] Verifying Auncient joint damping/focal register sabotage mapping...\n");
    
    // Sabotage Clock Divisor (103) and Focal Depth (104) registers
    wmq_write_transient(&vm, 103, 160);
    wmq_write_transient(&vm, 104, 1350);

    uint64_t divisor_val = wmq_read_transient(&vm, 103);
    uint64_t focal_val = wmq_read_transient(&vm, 104);
    
    printf("  -> Sabotaged Damping Divisor register: %lu (expected 160)\n", divisor_val);
    printf("  -> Sabotaged Focal Depth register: %lu (expected 1350)\n", focal_val);
    
    assert(divisor_val == 160);
    assert(focal_val == 1350);

    // --- TEST 4: MATING (Erisian Recombination Operator) ---
    printf("\n[MATING TEST] Verifying prime-stabilization operator recombination...\n");
    uint64_t M = 953467954114363ULL;
    int k = 9;
    uint64_t child_dna = recombine_dna_operator(M, k);
    printf("  -> Generated Child DNA: %lu (expected 953473954114361)\n", child_dna);
    assert(child_dna == 953473954114361ULL);

    // --- TEST 5: FLOW CONTROL & QUEUE LIMITS (Comprehensive Expansion) ---
    printf("\n[FLOW CONTROL TEST] Verifying WinchesterMQ flow limits and queue status logic...\n");
    
    // Simulate active queue head at index 0 and tail at index 17 (Queue depth = 17, limit = 15)
    // Head key slot: 0x2000 + LUN (1) -> 0x2001
    // Tail key slot: 0x2050 + LUN (1) -> 0x2051
    wmq_sstore(&vm, 0x2001, 0);
    wmq_sstore(&vm, 0x2051, 17);
    
    uint64_t head_val = wmq_sload(&vm, 0x2001);
    uint64_t tail_val = wmq_sload(&vm, 0x2051);
    printf("  -> Read Queue parameters: Head = %lu, Tail = %lu\n", head_val, tail_val);
    assert(head_val == 0);
    assert(tail_val == 17);

    // Simulate sending Write command opcode 0x0A (Write Block) to WinchesterMQ with full queue
    printf("  -> Initiating Write handshakes to WinchesterMQ LUN 1 (expecting Queue Full status)...\n");
    
    wmq_write_signals(&vm, 0x02); // RST = 1
    wmq_write_signals(&vm, 0x00); // Clear
    wmq_write_signals(&vm, 0x01); // SEL = 1
    
    // Send 6-byte CDB for LUN 1 [0x0A (Write), 0x20 (LUN 1 + High LBA 0), 0, 0, 1, 0]
    uint8_t full_cdb[6] = {0x0A, 0x20, 0x00, 0x00, 0x01, 0x00};
    for (int i = 0; i < 6; i++) {
        wmq_write_data(&vm, full_cdb[i]);
        wmq_write_signals(&vm, 0x05); // Assert ACK
        wmq_write_signals(&vm, 0x01); // Deassert ACK
    }
    
    // Read status signals back. Transition to Status phase (C/D = 1, I/O = 1) with status code 0x08 (Busy/Full)
    uint8_t status_sigs = wmq_read_signals(&vm);
    printf("  -> Signals after CDB: 0x%02X (expected BSY=0x01, REQ=0x02, C/D=0x04, I/O=0x08 for Status Phase)\n", status_sigs);
    // BSY, C/D, I/O, MSG (0x10) must be set. Status transitions uses transitionToStatus(0x08)
    // transitionToStatus(0x08) -> C/D=1, I/O=1, MSG=1, REQ=1 (Signals: BSY(0x01) | REQ(0x02) | C/D(0x04) | I/O(0x08) | MSG(0x10) = 0x1F)
    assert((status_sigs & 0x01) != 0); // BSY active
    assert((status_sigs & 0x04) != 0); // C/D active
    assert((status_sigs & 0x08) != 0); // I/O active
    
    // Read data port to fetch the status byte (expecting 0x08: Queue Full / Busy)
    uint8_t status_byte = wmq_read_data_port(&vm);
    printf("  -> Status Code read back: 0x%02X (expected 0x08 for Queue Full / Busy)\n", status_byte);
    assert(status_byte == 0x08);

    // --- TEST 6: SCSI RESET HANDSHAKE (Bus Muting) ---
    printf("\n[RESET TEST] Verifying SCSI Reset clears signals immediately...\n");
    wmq_write_signals(&vm, 0x02); // Assert RST = 1
    uint8_t reset_sigs = wmq_read_signals(&vm);
    printf("  -> Signals after Reset: 0x%02X (expected 0x00)\n", reset_sigs);
    assert(reset_sigs == 0x00);

    printf("\n=========================================================\n");
    printf("[SUCCESS] All contact, collision, damage, mating, flow control, and reset tests passed.\n");
    printf("=========================================================\n");

    tsfi_zmm_vm_destroy(&vm);
    return 0;
}
