#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>

// Opcode Definitions for XPL Network Stack
#define OP_PUSH_STANAG 0x50 // Push STANAG [MAC (48 bits) + Port (16 bits)]
#define OP_PUSH_DECNET 0x51 // Push DECnet [Area (6 bits) + Node (10 bits)]
#define OP_SEND_STANAG 0x60 // Send STANAG packet
#define OP_SEND_DECNET 0x61 // Send DECnet packet

// XPL VM Stack Structure
#define XPL_STACK_MAX 256
typedef struct {
    uint64_t data[XPL_STACK_MAX];
    uint32_t top;
} xpl_stack_t;

// Simulated Physical Hardware TX Descriptor
struct ixgbe_tx_desc {
    uint64_t pkt_addr;
    uint32_t length;
    uint8_t status;
    uint8_t command;
    uint16_t special;
} __attribute__((packed));

// XPL VM Context
typedef struct {
    xpl_stack_t stack;
    struct ixgbe_tx_desc tx_ring[16];
    uint32_t tx_head;
    uint8_t tx_buffers[16][1024];
} xpl_vm_t;

static xpl_vm_t g_vm;

// Stack Helper Functions
static void xpl_push(xpl_stack_t *stack, uint64_t val) {
    assert(stack->top < XPL_STACK_MAX);
    stack->data[stack->top++] = val;
}

static uint64_t xpl_pop(xpl_stack_t *stack) {
    assert(stack->top > 0);
    return stack->data[--stack->top];
}

// -------------------------------------------------------------
// VM Opcode Interpreters
// -------------------------------------------------------------
void execute_push_stanag(xpl_vm_t *vm, const uint8_t *mac, uint16_t port) {
    uint64_t val = 0;
    // Embed 48-bit MAC in upper bits, 16-bit Port in lower bits
    for (int i = 0; i < 6; i++) {
        val |= ((uint64_t)mac[i]) << (16 + (5 - i) * 8);
    }
    val |= (uint64_t)port;
    xpl_push(&vm->stack, val);
}

void execute_push_decnet(xpl_vm_t *vm, uint8_t area, uint16_t node) {
    // Embed 6-bit Area and 10-bit Node address
    uint64_t val = (uint64_t)((area << 10) | (node & 0x03FF));
    xpl_push(&vm->stack, val);
}

void execute_send_stanag(xpl_vm_t *vm, const uint8_t *payload, uint16_t len) {
    uint64_t addr_element = xpl_pop(&vm->stack);
    
    // Extract MAC and Port directly from stack frame
    uint8_t dest_mac[6];
    for (int i = 0; i < 6; i++) {
        dest_mac[i] = (uint8_t)((addr_element >> (16 + (5 - i) * 8)) & 0xFF);
    }
    uint16_t port = (uint16_t)(addr_element & 0xFFFF);

    // Write directly to the hardware TX Ring buffer (Option 1 style bypass)
    uint32_t slot = vm->tx_head;
    uint8_t *buf = vm->tx_buffers[slot];
    
    // Frame construction: Ethernet header -> STANAG ports -> Payload
    memcpy(buf, dest_mac, 6);
    memset(buf + 6, 0xAA, 6); // Src MAC
    *(uint16_t *)(buf + 12) = htons(0x8B32); // STANAG EtherType
    *(uint16_t *)(buf + 14) = htons(port);
    memcpy(buf + 16, payload, len);

    // Configure TX Descriptor
    vm->tx_ring[slot].pkt_addr = 0x3000000 + (slot * 1024); // IOVA
    vm->tx_ring[slot].length = 16 + len;
    vm->tx_ring[slot].command = (1 << 3) | (1 << 0); // Report Status & End of Packet
    
    printf("   [XPL-VM] STANAG packet compiled from stack. Dest MAC: %02X:%02X:%02X:%02X:%02X:%02X, Port: %d, Len: %d\n",
           dest_mac[0], dest_mac[1], dest_mac[2], dest_mac[3], dest_mac[4], dest_mac[5], port, len);
    fflush(stdout);

    vm->tx_head = (slot + 1) % 16;
}

void execute_send_decnet(xpl_vm_t *vm, const uint8_t *payload, uint16_t len) {
    uint64_t addr_element = xpl_pop(&vm->stack);
    
    // Extract DECnet Area and Node
    uint8_t area = (uint8_t)((addr_element >> 10) & 0x3F);
    uint16_t node = (uint16_t)(addr_element & 0x03FF);

    // Build target DECnet MAC (AA:00:04:00:xx:xx)
    uint8_t dest_mac[6] = { 0xAA, 0x00, 0x04, 0x00, (uint8_t)(addr_element & 0xFF), (uint8_t)(addr_element >> 8) };

    // Write to hardware TX Ring
    uint32_t slot = vm->tx_head;
    uint8_t *buf = vm->tx_buffers[slot];

    memcpy(buf, dest_mac, 6);
    memset(buf + 6, 0xBB, 6);
    *(uint16_t *)(buf + 12) = htons(0x6003); // DECnet EtherType
    
    // DECnet Routing Header Short format (flags, dst, src, forward)
    buf[14] = 0x02;
    *(uint16_t *)(buf + 15) = (uint16_t)addr_element;
    *(uint16_t *)(buf + 17) = (uint16_t)(area << 10 | 10); // Src node (1.10)
    buf[19] = 0;
    
    // NSP header (Connect/Data)
    buf[20] = 0x30; // Data message
    *(uint16_t *)(buf + 21) = 0x1111; // Dst Link
    *(uint16_t *)(buf + 23) = 0x2222; // Src Link
    
    memcpy(buf + 25, payload, len);

    vm->tx_ring[slot].pkt_addr = 0x3000000 + (slot * 1024);
    vm->tx_ring[slot].length = 25 + len;
    vm->tx_ring[slot].command = (1 << 3) | (1 << 0);

    printf("   [XPL-VM] DECnet packet compiled from stack. Dest Node: %d.%d, Len: %d\n", area, node, len);
    fflush(stdout);

    vm->tx_head = (slot + 1) % 16;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPL NETWORK STACK INTEGRATION VERIFICATION\n");
    printf("=============================================================\n");
    fflush(stdout);

    memset(&g_vm, 0, sizeof(g_vm));

    // 1. Push STANAG address parameters and call send
    printf("[TEST] Executing XPL stack operations for STANAG...\n");
    fflush(stdout);
    uint8_t test_mac[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
    uint16_t test_port = 2020;
    uint8_t payload1[] = "STANAG Broadcast State";
    
    execute_push_stanag(&g_vm, test_mac, test_port);
    execute_send_stanag(&g_vm, payload1, sizeof(payload1));

    // Verify stack clean
    assert(g_vm.stack.top == 0);

    // 2. Push DECnet address parameters (Area 2, Node 50) and call send
    printf("[TEST] Executing XPL stack operations for DECnet...\n");
    fflush(stdout);
    uint8_t test_area = 2;
    uint16_t test_node = 50;
    uint8_t payload2[] = "DECnet NSP Stream State";

    execute_push_decnet(&g_vm, test_area, test_node);
    execute_send_decnet(&g_vm, payload2, sizeof(payload2));

    assert(g_vm.stack.top == 0);

    printf("=============================================================\n");
    printf("XPL NETWORK STACK INTEGRATION VERIFIED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
