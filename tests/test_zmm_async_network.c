#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// Mock VM Context structures matching ZMM thunk system
typedef struct {
    uint32_t ip;
    uint8_t sp;
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sr;
    uint8_t status; // 1 = Active, 0 = Suspended
    uint8_t idle;   // 1 = Idle, 0 = Active
} GuestPCB;

typedef struct {
    uint32_t head;
    uint32_t tail;
    uint8_t sectors[256][256]; // Mock storage for up to 256 sectors of 256 bytes
} VirtualLUN;

// Shared VM State
GuestPCB g_pcb[52];
VirtualLUN g_luns[8];
uint32_t g_system_jiffies = 0;
uint32_t g_active_card = 0;

// Initialize hardware states
void init_hardware() {
    memset(g_pcb, 0, sizeof(g_pcb));
    memset(g_luns, 0, sizeof(g_luns));
    g_system_jiffies = 0;
    g_active_card = 0;
}

// Emulates WinchesterMQ PUT command
void mock_mq_put(uint8_t lun, uint32_t lba, uint8_t* data) {
    assert(lun < 8);
    assert(lba < 256);
    
    // Copy 256-byte payload to LUN storage
    memcpy(g_luns[lun].sectors[lba], data, 256);
    
    // If writing to a queue tail, increment the tail index
    if (lba == g_luns[lun].tail) {
        g_luns[lun].tail++;
    }
}

// Emulates WinchesterMQ GET command
int mock_mq_get(uint8_t lun, uint32_t lba, uint8_t* out_data) {
    assert(lun < 8);
    assert(lba < 256);

    // Retrieve data
    memcpy(out_data, g_luns[lun].sectors[lba], 256);

    // Expiry Check (TTL): Expiry jiffies stored at bytes 8-15
    uint64_t expiry = 0;
    memcpy(&expiry, &g_luns[lun].sectors[lba][8], 8);
    if (expiry != 0 && g_system_jiffies > expiry) {
        // Expired! Advance head and return failure
        if (lba == g_luns[lun].head) {
            g_luns[lun].head++;
        }
        return -1; // Expired
    }

    // Success read. Advance head index.
    if (lba == g_luns[lun].head) {
        g_luns[lun].head++;
    }
    return 0; // Success
}

// Find a ready card process in the scheduler
uint32_t mock_find_ready_card() {
    for (uint32_t i = 0; i < 52; i++) {
        uint32_t candidate = (g_active_card + i) % 52;
        if (g_pcb[candidate].status == 1) { // Active process
            // Check if there is data in the inbox (LUN 4 is Network interface, CardID index = Inbox)
            uint32_t head = g_luns[4].head;
            uint32_t tail = g_luns[4].tail;
            
            if (head < tail || g_pcb[candidate].idle == 0) {
                return candidate;
            }
        }
    }
    return 0xFFFFFFFF; // None ready
}

// =============================================================================
// TESTS SECTION
// =============================================================================

// Test Case 1: Standard MQPUT & MQGET
void test_standard_mq_put_get() {
    printf("[TEST] Running standard MQ PUT/GET test...\n");
    init_hardware();

    uint8_t msg[256];
    memset(msg, 0, 256);
    msg[0] = 'C'; msg[1] = 'A'; msg[2] = 'R'; msg[3] = 'D'; // Magic header
    msg[12] = 0xAA; // Mock CorrelId

    // Put message to LUN 1 LBA 0 (Inbox)
    mock_mq_put(1, 0, msg);
    assert(g_luns[1].tail == 1);

    // Get message
    uint8_t out[256];
    int res = mock_mq_get(1, 0, out);
    assert(res == 0);
    assert(out[0] == 'C' && out[12] == 0xAA);
    assert(g_luns[1].head == 1);

    printf("[SUCCESS] Standard MQ PUT/GET passed.\n");
}

// Test Case 2: TTL Expiry Check
void test_ttl_expiry() {
    printf("[TEST] Running TTL Expiry validation test...\n");
    init_hardware();

    uint8_t msg[256];
    memset(msg, 0, 256);
    msg[0] = 'M'; msg[1] = 'S'; msg[2] = 'G'; msg[3] = 'Q';
    
    // Set ExpiryJiffies (bytes 8-15) to 100
    uint64_t expiry = 100;
    memcpy(&msg[8], &expiry, 8);

    mock_mq_put(1, 0, msg);

    // Case 2a: Current jiffies is 50 (Not expired)
    g_system_jiffies = 50;
    uint8_t out[256];
    int res = mock_mq_get(1, 0, out);
    assert(res == 0); // Should read successfully

    // Case 2b: Expired message read
    init_hardware();
    mock_mq_put(1, 0, msg);
    g_system_jiffies = 150; // Expired (150 > 100)
    res = mock_mq_get(1, 0, out);
    assert(res == -1); // Should be skipped/expired
    assert(g_luns[1].head == 1); // Head must be advanced past expired block

    printf("[SUCCESS] TTL Expiry check passed.\n");
}

// Test Case 3: PulseChain Network Async Receipt Confirmation
void test_pulsechain_async_confirm() {
    printf("[TEST] Running PulseChain network async confirmation test...\n");
    init_hardware();

    // 1. Setup Card 2 process representing our receiver daemon
    g_pcb[2].status = 1; // Active
    g_pcb[2].idle = 1;   // Idle (Waiting for network confirmation)

    // No ready card initially because Card 2 is idle and its LUN 4 Inbox is empty
    uint32_t ready = mock_find_ready_card();
    assert(ready == 0xFFFFFFFF);

    // 2. Mock host resolving transaction on PulseChain and pushing block receipt to LUN 4
    uint8_t receipt[256];
    memset(receipt, 0, 256);
    receipt[0] = 'T'; receipt[1] = 'X'; receipt[2] = 'R'; receipt[3] = 'Q'; // Magic TX receipt
    receipt[12] = 0xFF; // Correlation ID match
    
    // Write receipt to LUN 4 (Network Inbox)
    mock_mq_put(4, 0, receipt);

    // 3. Scheduler detects the transaction in inbox and wakes up Card 2
    ready = mock_find_ready_card();
    assert(ready == 2); // Card 2 must be ready now

    // 4. Card 2 processes data
    g_pcb[ready].idle = 0; // Wakes up
    uint8_t out_receipt[256];
    int res = mock_mq_get(4, 0, out_receipt);
    assert(res == 0);
    assert(out_receipt[0] == 'T' && out_receipt[12] == 0xFF);

    printf("[SUCCESS] PulseChain Async Confirmation passed.\n");
}

int main() {
    printf("=== RUNNING WINCHESTER_MQ INTEGRATION TEST SUITE ===\n");
    
    test_standard_mq_put_get();
    test_ttl_expiry();
    test_pulsechain_async_confirm();

    printf("=== ALL UNIT TESTS PASSED SUCCESSFULLY ===\n");
    return 0;
}
