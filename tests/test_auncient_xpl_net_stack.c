#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <openssl/sha.h>

#define HASH_SIZE 32

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

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

// 2-3 Merkle Tree Node for Trace Registry
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// XPL VM Context
typedef struct {
    xpl_stack_t stack;
    struct ixgbe_tx_desc tx_ring[16];
    uint32_t tx_head;
    uint8_t tx_buffers[16][1024];
    TwoThreeNode *trace_registry;
    bool parity_error;
    bool overcurrent_error;
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

// Helper to create leaf node
static TwoThreeNode* create_leaf(uint32_t key1, const char *val1) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = true;
    node->num_keys = 1;
    node->keys[0] = key1;
    strcpy(node->values[0], val1);
    
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);
    return node;
}

// Update leaf in 2-3 tree DAT
static bool store_table_data(TwoThreeNode *node, uint32_t key, const char *new_value) {
    if (!node) return false;
    if (node->is_leaf) {
        if (node->keys[0] == key) {
            strcpy(node->values[0], new_value);
            uint8_t temp[256];
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
            sha256(temp, len, node->node_hash);
            return true;
        }
        return false;
    }
    return store_table_data(node->children[0], key, new_value);
}

// Parity Check (Odd Parity)
static bool verify_odd_parity(const uint8_t *payload, uint16_t len) {
    int count = 0;
    for (uint16_t i = 0; i < len; i++) {
        uint8_t b = payload[i];
        for (int j = 0; j < 8; j++) {
            if ((b >> j) & 1) count++;
        }
    }
    return (count % 2) != 0;
}

// -------------------------------------------------------------
// VM Opcode Interpreters
// -------------------------------------------------------------
void execute_push_stanag(xpl_vm_t *vm, const uint8_t *mac, uint16_t port) {
    uint64_t val = 0;
    for (int i = 0; i < 6; i++) {
        val |= ((uint64_t)mac[i]) << (16 + (5 - i) * 8);
    }
    val |= (uint64_t)port;
    xpl_push(&vm->stack, val);
}

void execute_push_decnet(xpl_vm_t *vm, uint8_t area, uint16_t node) {
    uint64_t val = (uint64_t)((area << 10) | (node & 0x03FF));
    xpl_push(&vm->stack, val);
}

bool execute_send_stanag(xpl_vm_t *vm, const uint8_t *payload, uint16_t len) {
    uint64_t addr_element = xpl_pop(&vm->stack);
    vm->parity_error = false;
    vm->overcurrent_error = false;

    // 1. Overcurrent Ring Protection: Max payload limit 512 bytes to protect hardware buffers
    if (len > 512) {
        vm->overcurrent_error = true;
        uint32_t slot = vm->tx_head;
        vm->tx_ring[slot].status = 0x02; // Overcurrent/Length Error Status
        return false;
    }

    // 2. Payload Parity Verification
    if (!verify_odd_parity(payload, len)) {
        vm->parity_error = true;
        return false;
    }

    uint8_t dest_mac[6];
    for (int i = 0; i < 6; i++) {
        dest_mac[i] = (uint8_t)((addr_element >> (16 + (5 - i) * 8)) & 0xFF);
    }
    uint16_t port = (uint16_t)(addr_element & 0xFFFF);

    uint32_t slot = vm->tx_head;
    uint8_t *buf = vm->tx_buffers[slot];
    
    memcpy(buf, dest_mac, 6);
    memset(buf + 6, 0xAA, 6); 
    *(uint16_t *)(buf + 12) = htons(0x8B32); 
    *(uint16_t *)(buf + 14) = htons(port);
    memcpy(buf + 16, payload, len);

    vm->tx_ring[slot].pkt_addr = 0x3000000 + (slot * 1024); 
    vm->tx_ring[slot].length = 16 + len;
    vm->tx_ring[slot].status = 0x00; // Success
    vm->tx_ring[slot].command = (1 << 3) | (1 << 0); 
    
    printf("   [XPL-VM] STANAG packet compiled. Dest MAC: %02X:%02X:%02X:%02X:%02X:%02X, Port: %d, Len: %d\n",
           dest_mac[0], dest_mac[1], dest_mac[2], dest_mac[3], dest_mac[4], dest_mac[5], port, len);
    fflush(stdout);

    // 3. Merkle Trace Logging: update transmission trace database
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "STANAG:PORT=%d,LEN=%d,SLOT=%d", port, len, slot);
    store_table_data(vm->trace_registry, slot, log_msg);

    vm->tx_head = (slot + 1) % 16;
    return true;
}

bool execute_send_decnet(xpl_vm_t *vm, const uint8_t *payload, uint16_t len) {
    uint64_t addr_element = xpl_pop(&vm->stack);
    vm->parity_error = false;
    vm->overcurrent_error = false;

    // 1. Overcurrent Ring Protection
    if (len > 512) {
        vm->overcurrent_error = true;
        uint32_t slot = vm->tx_head;
        vm->tx_ring[slot].status = 0x02;
        return false;
    }

    // 2. Payload Parity Verification
    if (!verify_odd_parity(payload, len)) {
        vm->parity_error = true;
        return false;
    }

    uint8_t area = (uint8_t)((addr_element >> 10) & 0x3F);
    uint16_t node = (uint16_t)(addr_element & 0x03FF);

    uint32_t slot = vm->tx_head;
    uint8_t *buf = vm->tx_buffers[slot];

    memset(buf, 0xFF, 6); 
    memset(buf + 6, 0xBB, 6); 
    *(uint16_t *)(buf + 12) = htons(0x6003); 
    *(uint16_t *)(buf + 14) = htons((area << 10) | node);
    memcpy(buf + 16, payload, len);

    vm->tx_ring[slot].pkt_addr = 0x3000000 + (slot * 1024);
    vm->tx_ring[slot].length = 16 + len;
    vm->tx_ring[slot].status = 0x00;
    vm->tx_ring[slot].command = (1 << 3) | (1 << 0);

    printf("   [XPL-VM] DECnet packet compiled. Dest Node: %d.%d, Len: %d\n", area, node, len);
    fflush(stdout);

    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "DECNET:NODE=%d.%d,LEN=%d,SLOT=%d", area, node, len, slot);
    store_table_data(vm->trace_registry, slot, log_msg);

    vm->tx_head = (slot + 1) % 16;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPL NETWORK STACK INTEGRATION VERIFICATION\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize 2-3 Merkle Tree trace nodes
    TwoThreeNode *trace_root = create_leaf(0, "INIT_TRACE_STATE");
    memset(&g_vm, 0, sizeof(g_vm));
    g_vm.trace_registry = trace_root;

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, trace_root->node_hash, HASH_SIZE);

    // 1. Executing compliant STANAG packet stream (odd parity payload)
    printf("[TEST] Executing XPL stack operations for STANAG...\n");
    fflush(stdout);
    uint8_t dest_mac[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
    execute_push_stanag(&g_vm, dest_mac, 2020);
    // "XPL_STANAG_OK_PAYLOAD_DATA" has 117 ones (odd parity)
    const uint8_t *payload_ok = (const uint8_t *)"XPL_STANAG_OK_PAYLOAD_DATA";
    bool ok = execute_send_stanag(&g_vm, payload_ok, strlen((const char *)payload_ok));
    assert(ok == true);
    assert(g_vm.parity_error == false);
    assert(g_vm.overcurrent_error == false);
    assert(memcmp(initial_hash, trace_root->node_hash, HASH_SIZE) != 0); // Merkle state updated
    printf("   ✓ STANAG transmission compiled and trace logged successfully.\n");
    fflush(stdout);

    // Save updated hash
    uint8_t updated_hash[HASH_SIZE];
    memcpy(updated_hash, trace_root->node_hash, HASH_SIZE);

    // 2. Parity check failure (even parity payload)
    printf("[TEST] Executing parity error handling validations...\n");
    fflush(stdout);
    execute_push_stanag(&g_vm, dest_mac, 2020);
    // "A" (ASCII 65, binary 01000001) has 2 ones (even parity)
    const uint8_t *payload_even = (const uint8_t *)"A";
    ok = execute_send_stanag(&g_vm, payload_even, strlen((const char *)payload_even));
    assert(ok == false);
    assert(g_vm.parity_error == true);
    assert(memcmp(updated_hash, trace_root->node_hash, HASH_SIZE) == 0); // Unaltered
    printf("   ✓ Even parity transmission blocked successfully.\n");
    fflush(stdout);

    // 3. Overcurrent payload size error validation
    printf("[TEST] Executing packet overcurrent limit validations...\n");
    fflush(stdout);
    execute_push_stanag(&g_vm, dest_mac, 2020);
    uint8_t large_payload[600];
    memset(large_payload, 0x31, sizeof(large_payload)); // Odd count of ones
    large_payload[599] = '\0';
    ok = execute_send_stanag(&g_vm, large_payload, 600);
    assert(ok == false);
    assert(g_vm.overcurrent_error == true);
    printf("   ✓ Payload overcurrent limit trapped successfully.\n");
    fflush(stdout);

    // 4. Executing compliant DECnet packet stream
    printf("[TEST] Executing XPL stack operations for DECnet...\n");
    fflush(stdout);
    execute_push_decnet(&g_vm, 2, 50);
    // "C" (ASCII 67, binary 01000011) has 3 ones (odd parity)
    const uint8_t *decnet_ok = (const uint8_t *)"C";
    ok = execute_send_decnet(&g_vm, decnet_ok, strlen((const char *)decnet_ok));
    assert(ok == true);
    printf("   ✓ DECnet packet compiled and transmitted successfully.\n");
    fflush(stdout);

    free(trace_root);
    printf("=============================================================\n");
    printf("XPL NETWORK STACK INTEGRATION VERIFIED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
