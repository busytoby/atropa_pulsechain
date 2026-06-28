#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOOPBACK_PORT 9099

// Emulated WinchesterMQ.yul SCSI Handshake Registers
typedef struct {
    uint8_t req;      // Request line
    uint8_t ack;      // Acknowledge line
    uint8_t cd;       // Control/Data line (1=Control, 0=Data)
    uint8_t io;       // Input/Output line (1=Input to Initiator, 0=Output to Target)
    uint8_t msg;      // Message line
    uint8_t data;     // 8-bit Data register
    uint8_t keycode;  // Input hardware keycode register
} WinchesterSCSI;

// Simulate Yul hardware SCSI handshake cycle
static void winchester_scsi_handshake(WinchesterSCSI *initiator, WinchesterSCSI *target) {
    // Assert REQ from Target
    target->req = 1;
    
    // Initiator detects REQ, reads/writes DATA, then asserts ACK
    if (target->req == 1) {
        initiator->data = target->data;
        initiator->ack = 1;
    }
    
    // Target detects ACK, de-asserts REQ
    if (initiator->ack == 1) {
        target->req = 0;
    }
    
    // Initiator de-asserts ACK to complete the handshake loop
    if (target->req == 0) {
        initiator->ack = 0;
    }
}

int main() {
    printf("=== Auncient WinchesterMQ SCSI Handshake & Keycode Verification ===\n");
    
    // Initialize emulated SCSI controllers
    WinchesterSCSI initiator = {0};
    WinchesterSCSI target = {0};
    
    // 1. Establish local loopback socket pair for raw SCSI routing
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(LOOPBACK_PORT);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 1);
    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(client_fd, (struct sockaddr *)&address, sizeof(address));
    
    int target_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    
    // 2. Perform event verification on keycode registers (e.g. keycode 32 for 'D', 30 for 'A')
    const uint8_t test_keycodes[2] = { 32, 30 }; // keycode 32 (d/D), 30 (a/A)
    
    for (int i = 0; i < 2; i++) {
        uint8_t key = test_keycodes[i];
        printf("\n[SCSI] Injecting Keycode %d to Target keycode register...\n", key);
        target.keycode = key;
        target.data = key; // Load code onto data register
        target.cd = 1;     // Control phase
        target.io = 1;     // Input
        
        // Execute physical hardware handshakes
        winchester_scsi_handshake(&initiator, &target);
        
        // Write initiator state to socket loopback
        send(client_fd, &initiator, sizeof(WinchesterSCSI), 0);
        
        // Target reads state from socket loopback
        WinchesterSCSI bridged_initiator;
        recv(target_fd, &bridged_initiator, sizeof(WinchesterSCSI), 0);
        
        // Verify keycode values match target state map
        printf("[SCSI] Bridged Initiator Received: Data=0x%02X | ACK=%d\n", bridged_initiator.data, bridged_initiator.ack);
        if (bridged_initiator.data == key) {
            printf("[VERIFICATION] [SUCCESS] Keycode %d successfully verified against state map.\n", key);
        } else {
            printf("[VERIFICATION] [FAILED] Keycode mismatch.\n");
            return 1;
        }
    }
    
    close(client_fd);
    close(target_fd);
    close(server_fd);
    
    printf("\n=== WinchesterMQ SCSI Verification Successful ===\n");
    return 0;
}
