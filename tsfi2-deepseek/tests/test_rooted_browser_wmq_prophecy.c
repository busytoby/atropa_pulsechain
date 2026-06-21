#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

// Terminology Constraint: Never use the spelling "ancient". Always spell it as "Auncient".

static int main_socket_fd = -1;
static int yt_socket_fd = -1;

static void setup_bridge_sockets() {
    struct sockaddr_in serv_addr;
    
    // Main TCP bridge connection (port 18080)
    main_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket_fd >= 0) {
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(18080);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
        if (connect(main_socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("[Auncient Bridge Client] Main dashboard bridge offline.\n");
            close(main_socket_fd);
            main_socket_fd = -1;
        } else {
            printf("[Auncient Bridge Client] Connected to Main dashboard bridge on port 18080.\n");
        }
    }

    // YouTube TCP bridge connection (port 18081)
    yt_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (yt_socket_fd >= 0) {
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(18081);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
        if (connect(yt_socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("[Auncient Bridge Client] YouTube dashboard bridge offline.\n");
            close(yt_socket_fd);
            yt_socket_fd = -1;
        } else {
            printf("[Auncient Bridge Client] Connected to YouTube dashboard bridge on port 18081.\n");
        }
    }
}

static void close_bridge_sockets() {
    if (main_socket_fd >= 0) close(main_socket_fd);
    if (yt_socket_fd >= 0) close(yt_socket_fd);
}

// Helpers to convert numbers to 32-byte big-endian hex words for Yul calldata
static void wmq_fill_hex_uint256(char *dest, uint64_t val) {
    memset(dest, '0', 64);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lx", val);
    memcpy(dest + 64 - len, tmp, len);
    dest[64] = '\0';
}

static void wmq_write_signals(TsfiZmmVmState *vm, uint8_t sigs) {
    char cmd[512];
    char hex_sigs[65];
    wmq_fill_hex_uint256(hex_sigs, sigs);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"485301a0%s\"", hex_sigs);
    tsfi_zmm_vm_exec(vm, cmd);
}

static uint8_t wmq_read_signals(TsfiZmmVmState *vm) {
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, "YULEXEC \"WinchesterMQ\", \"525302b0\"");
    
    unsigned int val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 62, "%02x", &val);
    }
    return (uint8_t)val;
}

static void wmq_write_data(TsfiZmmVmState *vm, uint8_t val) {
    char cmd[512];
    char hex_val[65];
    wmq_fill_hex_uint256(hex_val, val);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"98d400c0%s\"", hex_val);
    tsfi_zmm_vm_exec(vm, cmd);
}

static void send_keyboard_command(TsfiZmmVmState *vm, const char *event_str) {
    // Reset WinchesterMQ state
    wmq_write_signals(vm, 0x02); // RST = 1
    wmq_write_signals(vm, 0x00); // Clear

    // 1. Select Phase
    wmq_write_signals(vm, 0x01); // SEL = 1

    // Verify controller asserted BSY=1, C/D=1, REQ=1
    uint8_t sigs = wmq_read_signals(vm);
    assert((sigs & 0x01) != 0); // BSY active
    assert((sigs & 0x04) != 0); // C/D active

    // 2. Command phase: send 6-byte CDB [0x0A, 0, 0, 0, 5, 0] (Write Block opcode to LUN 5 Topic Publish Broker)
    uint8_t cdb[6] = {0x0A, 0x00, 0x00, 0x00, 0x05, 0x00};
    for (int i = 0; i < 6; i++) {
        wmq_write_data(vm, cdb[i]);
        wmq_write_signals(vm, 0x05); // Assert ACK (SEL=1, BSY=1, ACK=1)
        wmq_write_signals(vm, 0x01); // Deassert ACK
    }

    // Verify controller transitioned to Data Phase: I/O=0, C/D=0
    sigs = wmq_read_signals(vm);
    assert((sigs & 0x04) == 0); // C/D deasserted
    assert((sigs & 0x08) == 0); // I/O deasserted

    // 3. Data phase: Send 256-byte payload containing the command
    uint8_t payload[256] = {0};
    strncpy((char*)payload, event_str, sizeof(payload) - 1);
    for (int i = 0; i < 256; i++) {
        wmq_write_data(vm, payload[i]);
        wmq_write_signals(vm, 0x05); // Assert ACK
        wmq_write_signals(vm, 0x01); // Deassert ACK
    }

    // Verify controller transitioned to Status phase: C/D=1, I/O=1
    sigs = wmq_read_signals(vm);
    assert((sigs & 0x04) != 0); // C/D asserted
    assert((sigs & 0x08) != 0); // I/O asserted

    // 4. Complete transaction
    wmq_write_signals(vm, 0x05); // Complete Status phase handshake
    wmq_write_signals(vm, 0x00); // Release bus

    // Route event over TCP sockets to the direct wmq bridge
    char route_buf[512];
    snprintf(route_buf, sizeof(route_buf), "%s\n", event_str);
    if (main_socket_fd >= 0) {
        send(main_socket_fd, route_buf, strlen(route_buf), 0);
    }
    if (yt_socket_fd >= 0) {
        send(yt_socket_fd, route_buf, strlen(route_buf), 0);
    }
    // Small sleep for layout propagation latency
    usleep(50000);
}

typedef struct {
    char ch;
    int keycode;
    bool shift;
} KeyMapEntry;

static KeyMapEntry key_map[] = {
    {'d', 32, false}, {'D', 32, true},
    {'y', 21, false}, {'Y', 21, true},
    {'s', 31, false}, {'S', 31, true},
    {'n', 49, false}, {'N', 49, true},
    {'o', 24, false}, {'O', 24, true},
    {'m', 50, false}, {'M', 50, true},
    {'i', 23, false}, {'I', 23, true},
    {'a', 30, false}, {'A', 30, true},
    {' ', 57, false},
    {'p', 25, false}, {'P', 25, true},
    {'r', 19, false}, {'R', 19, true},
    {'e', 18, false}, {'E', 18, true},
    {'h', 35, false}, {'H', 35, true},
    {'c', 46, false}, {'C', 46, true},
    {'\0', 0, false}
};

static KeyMapEntry lookup_key(char ch) {
    for (int i = 0; key_map[i].ch != '\0'; i++) {
        if (key_map[i].ch == ch) {
            return key_map[i];
        }
    }
    KeyMapEntry def = {ch, 30, false};
    return def;
}

int main() {
    printf("=========================================================\n");
    printf("Auncient ZMM VM Yul WinchesterMQ Prophecy Sequence Test\n");
    printf("=========================================================\n");

    // Initialize TCP sockets first
    setup_bridge_sockets();

    // Initialize dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Initialize WinchesterMQ Yul contract in ZMM VM
    printf("[VM INIT] Initializing WinchesterMQ Yul Contract on ZMM VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"WinchesterMQ\", \"../solidity/bin/WinchesterMQ.yul\", 512");

    const char *target_text = "Dysnomia Prophecy";
    printf("[PROPHECY] Injecting target string: \"%s\" character-by-character...\n", target_text);

    for (int idx = 0; target_text[idx] != '\0'; idx++) {
        char ch = target_text[idx];
        KeyMapEntry entry = lookup_key(ch);
        printf("\n--- Processing character: '%c' (Linux Keycode: %d, Shift: %s) ---\n", 
               ch, entry.keycode, entry.shift ? "Yes" : "No");

        char cmd_buf[128];

        if (entry.shift) {
            send_keyboard_command(&vm, "KEY_DOWN 42");
        }

        snprintf(cmd_buf, sizeof(cmd_buf), "KEY_DOWN %d", entry.keycode);
        send_keyboard_command(&vm, cmd_buf);

        snprintf(cmd_buf, sizeof(cmd_buf), "KEY_UP %d", entry.keycode);
        send_keyboard_command(&vm, cmd_buf);

        if (entry.shift) {
            send_keyboard_command(&vm, "KEY_UP 42");
        }
    }

    printf("\n=========================================================\n");
    printf("[SUCCESS] All Auncient keyboard hardware events completed.\n");
    printf("=========================================================\n");

    close_bridge_sockets();
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}
