#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>

// STANAG 5066 D_PDU structure definitions (Auncient Subnetwork Profile)
#define STANAG_MAX_PAYLOAD 2048
#define STANAG_PORTS_COUNT 4000
#define STANAG_PORT_MIN 1000
#define STANAG_PORT_MAX (STANAG_PORT_MIN + STANAG_PORTS_COUNT - 1)

// Simulated Intel/generic NIC hardware VFIO BAR registers
#define REG_RDBAL 0x0100  // Receive Descriptor Base Address Low
#define REG_RDBAH 0x0104  // Receive Descriptor Base Address High
#define REG_RDLEN 0x0108  // Receive Descriptor Length
#define REG_RDH   0x0110  // Receive Descriptor Head
#define REG_RDT   0x0114  // Receive Descriptor Tail
#define REG_RCTL  0x0120  // Receive Control Register

#define RCTL_EN   0x00000002 // Receiver Enable
#define RX_DD     0x01       // Descriptor Done (DD) status bit

// Ethernet Header
struct eth_header {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ether_type;
} __attribute__((packed));

// Custom STANAG D_PDU layout
struct stanag_pdu {
    uint16_t dst_port;
    uint16_t src_port;
    uint16_t length;
    uint8_t flags;
    uint8_t seq_num;
    uint8_t data[STANAG_MAX_PAYLOAD];
} __attribute__((packed));

// VFIO Hardware RX Descriptor definition
struct vfio_rx_desc {
    uint64_t buffer_addr; /* Physical address of packet buffer */
    uint16_t length;      /* Packet length */
    uint16_t checksum;
    uint8_t status;       /* Status flags (e.g. DD bit) */
    uint8_t errors;
    uint16_t special;
} __attribute__((packed));

// Thread-safe dynamic interop registrar for STANAG active port listeners
typedef void (*stanag_listener_fn)(uint16_t port, const uint8_t *data, uint16_t len);

struct stanag_port_registry {
    stanag_listener_fn listeners[STANAG_PORTS_COUNT];
    uint64_t rx_packet_counts[STANAG_PORTS_COUNT];
    uint64_t rx_byte_counts[STANAG_PORTS_COUNT];
} interop_stanag_registry;

// Simulated Physical Memory buffers
static uint8_t g_dma_packet_pool[128 * 2048]; // 128 packet buffers
static struct vfio_rx_desc g_rx_ring[128];
static uint32_t g_bar_space[1024]; // 4KB MMIO Register Space

// Dummy listener callback for verification
static uint64_t g_total_verifications = 0;
static void test_listener_callback(uint16_t port, const uint8_t *data, uint16_t len) {
    (void)port;
    (void)data;
    (void)len;
    g_total_verifications++;
}

// Function to resolve listeners and update telemetry in O(1) time
static inline void dispatch_stanag_packet(uint16_t port, const uint8_t *data, uint16_t len) {
    if (port >= STANAG_PORT_MIN && port <= STANAG_PORT_MAX) {
        uint32_t idx = port - STANAG_PORT_MIN;
        interop_stanag_registry.rx_packet_counts[idx]++;
        interop_stanag_registry.rx_byte_counts[idx] += len;
        if (interop_stanag_registry.listeners[idx]) {
            interop_stanag_registry.listeners[idx](port, data, len);
        }
    }
}

// Simulate hardware filling the packet and updating descriptor
static void simulate_hardware_packet(uint32_t ring_idx, uint16_t dst_port, const uint8_t *payload, uint16_t payload_len) {
    struct eth_header *eth = (struct eth_header *)(g_dma_packet_pool + (ring_idx * 2048));
    memset(eth->dest_mac, 0xAA, 6);
    memset(eth->src_mac, 0xBB, 6);
    eth->ether_type = htons(0x8B32); // Custom Auncient STANAG EtherType

    struct stanag_pdu *pdu = (struct stanag_pdu *)(eth + 1);
    pdu->dst_port = dst_port;
    pdu->src_port = 1234;
    pdu->length = payload_len;
    pdu->flags = 0;
    pdu->seq_num = (uint8_t)(ring_idx & 0xFF);
    if (payload && payload_len > 0) {
        memcpy(pdu->data, payload, payload_len);
    }

    g_rx_ring[ring_idx].length = sizeof(struct eth_header) + sizeof(struct stanag_pdu) + payload_len;
    g_rx_ring[ring_idx].status |= RX_DD; // Set Descriptor Done status
}

// The PMD polling routine for a single descriptor slot
static bool process_rx_descriptor(uint32_t *head_idx) {
    volatile struct vfio_rx_desc *desc = &g_rx_ring[*head_idx];
    if (desc->status & RX_DD) {
        uint8_t *buffer = g_dma_packet_pool + (*head_idx * 2048);
        struct eth_header *eth = (struct eth_header *)buffer;
        
        if (eth->ether_type == htons(0x8B32)) {
            struct stanag_pdu *pdu = (struct stanag_pdu *)(eth + 1);
            dispatch_stanag_packet(pdu->dst_port, pdu->data, pdu->length);
        }
        
        // Reset status to hand control back to "hardware"
        desc->status &= ~RX_DD;
        *head_idx = (*head_idx + 1) % 128;
        return true;
    }
    return false;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
static void test_register_and_dispatch(void) {
    printf("[TEST] Verifying listener registration and direct O(1) dispatch...\n");
    memset(&interop_stanag_registry, 0, sizeof(interop_stanag_registry));
    
    // Register listener on port 1500
    uint16_t test_port = 1500;
    interop_stanag_registry.listeners[test_port - STANAG_PORT_MIN] = test_listener_callback;
    
    // Dispatch test packet
    uint8_t test_data[] = "Auncient Payload";
    g_total_verifications = 0;
    dispatch_stanag_packet(test_port, test_data, sizeof(test_data));
    
    assert(g_total_verifications == 1);
    assert(interop_stanag_registry.rx_packet_counts[test_port - STANAG_PORT_MIN] == 1);
    assert(interop_stanag_registry.rx_byte_counts[test_port - STANAG_PORT_MIN] == sizeof(test_data));
    printf("   ✓ Listener dispatch test passed.\n");
}

static void test_vfio_pmd_sim(void) {
    printf("[TEST] Verifying simulated VFIO PMD packet loop...\n");
    memset(&interop_stanag_registry, 0, sizeof(interop_stanag_registry));
    memset(g_rx_ring, 0, sizeof(g_rx_ring));
    
    uint16_t test_port = 2000;
    interop_stanag_registry.listeners[test_port - STANAG_PORT_MIN] = test_listener_callback;
    
    // Configure simulated registers
    g_bar_space[REG_RCTL / 4] |= RCTL_EN;
    
    // Inject packet at index 5 in the ring
    uint8_t payload[] = "STANAG-5066";
    simulate_hardware_packet(5, test_port, payload, sizeof(payload));
    
    // Start polling from index 5
    uint32_t head = 5;
    g_total_verifications = 0;
    bool success = process_rx_descriptor(&head);
    
    assert(success == true);
    assert(head == 6);
    assert(g_total_verifications == 1);
    printf("   ✓ VFIO PMD packet loop test passed.\n");
}

// -------------------------------------------------------------
// Benchmarks
// -------------------------------------------------------------
static void run_demux_benchmark(void) {
    printf("[BENCHMARK] Running O(1) demux benchmark across %d ports...\n", STANAG_PORTS_COUNT);
    
    // Setup listeners on all ports
    for (int i = 0; i < STANAG_PORTS_COUNT; i++) {
        interop_stanag_registry.listeners[i] = test_listener_callback;
    }
    
    uint8_t payload[256];
    memset(payload, 0xEF, sizeof(payload));
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    uint64_t iterations = 10000000; // 10 Million iterations
    uint16_t base_port = STANAG_PORT_MIN;
    
    for (uint64_t i = 0; i < iterations; i++) {
        uint16_t dst = base_port + (uint16_t)(i % STANAG_PORTS_COUNT);
        dispatch_stanag_packet(dst, payload, sizeof(payload));
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (double)(end.tv_sec - start.tv_sec) + 
                     (double)(end.tv_nsec - start.tv_nsec) / 1e9;
    double tps = (double)iterations / elapsed;
    double latency = (elapsed / (double)iterations) * 1e9;
    
    printf("   - Duration:  %.4f seconds\n", elapsed);
    printf("   - Throughput: %.2f M-packets/sec (MOPS)\n", tps / 1e6);
    printf("   - Latency:    %.2f ns/packet\n", latency);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT STANAG OVER VFIO DRIVER TELEMETRY VALIDATION SUITE\n");
    printf("=============================================================\n");
    
    test_register_and_dispatch();
    test_vfio_pmd_sim();
    run_demux_benchmark();
    
    printf("=============================================================\n");
    printf("STANAG OVER VFIO TELEMETRY TESTS COMPLETED\n");
    printf("=============================================================\n");
    
    return 0;
}
