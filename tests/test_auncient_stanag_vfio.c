#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/vfio.h>
#include <arpa/inet.h>

#define STANAG_PORTS_COUNT 4000
#define STANAG_PORT_MIN 1000
#define STANAG_PORT_MAX (STANAG_PORT_MIN + STANAG_PORTS_COUNT - 1)

// Intel 82599 10GbE Controller Register Layout
#define IXGBE_RDBAL(n)  (0x01000 + 0x40 * (n))
#define IXGBE_RDBAH(n)  (0x01004 + 0x40 * (n))
#define IXGBE_RDLEN(n)  (0x01008 + 0x40 * (n))
#define IXGBE_RDH(n)    (0x01010 + 0x40 * (n))
#define IXGBE_RDT(n)    (0x01018 + 0x40 * (n))
#define IXGBE_RXDCTL(n) (0x01028 + 0x40 * (n))

struct ixgbe_rx_desc {
    uint64_t pkt_addr; /* Physical address of packet buffer */
    uint64_t status;   /* Status and errors */
} __attribute__((packed));

struct vfio_dma_region {
    void *vaddr;
    uint64_t iova;
    size_t size;
};

// Thread-safe dynamic interop registrar for STANAG active port listeners
typedef void (*stanag_listener_fn)(uint16_t port, const uint8_t *data, uint16_t len);

struct stanag_port_registry {
    stanag_listener_fn listeners[STANAG_PORTS_COUNT];
    uint64_t rx_packet_counts[STANAG_PORTS_COUNT];
    uint64_t rx_byte_counts[STANAG_PORTS_COUNT];
} interop_stanag_registry;

// Real VFIO Context
struct vfio_context {
    int container;
    int group;
    int device;
    volatile uint32_t *regs;
    size_t regs_size;
    struct vfio_dma_region ring_dma;
    struct vfio_dma_region buffer_dma;
} g_vfio_ctx;

// Allocate and map DMA memory via IOMMU Type 1
static bool vfio_dma_alloc_map(struct vfio_dma_region *dma, size_t size, uint64_t iova) {
    dma->size = size;
    dma->iova = iova;
    
    // Allocate page-aligned memory
    dma->vaddr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (dma->vaddr == MAP_FAILED) {
        // Fallback if Hugepages are not configured on host
        dma->vaddr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (dma->vaddr == MAP_FAILED) return false;
    }

    struct vfio_iommu_type1_dma_map dma_map = {
        .argsz = sizeof(dma_map),
        .flags = VFIO_DMA_MAP_FLAG_READ | VFIO_DMA_MAP_FLAG_WRITE,
        .vaddr = (uint64_t)dma->vaddr,
        .iova = dma->iova,
        .size = dma->size
    };

    if (ioctl(g_vfio_ctx.container, VFIO_IOMMU_MAP_DMA, &dma_map) < 0) {
        munmap(dma->vaddr, size);
        return false;
    }
    return true;
}

// Initialize real hardware VFIO mapping
bool vfio_driver_init(const char *group_path, const char *device_name) {
    g_vfio_ctx.container = open("/dev/vfio/vfio", O_RDWR);
    if (g_vfio_ctx.container < 0) return false;

    if (ioctl(g_vfio_ctx.container, VFIO_GET_API_VERSION) != VFIO_API_VERSION) return false;
    if (!ioctl(g_vfio_ctx.container, VFIO_CHECK_EXTENSION, VFIO_TYPE1_IOMMU)) return false;

    g_vfio_ctx.group = open(group_path, O_RDWR);
    if (g_vfio_ctx.group < 0) return false;

    if (ioctl(g_vfio_ctx.group, VFIO_GROUP_SET_CONTAINER, &g_vfio_ctx.container) < 0) return false;
    if (ioctl(g_vfio_ctx.container, VFIO_SET_IOMMU, VFIO_TYPE1_IOMMU) < 0) return false;

    g_vfio_ctx.device = ioctl(g_vfio_ctx.group, VFIO_GROUP_GET_DEVICE_FD, device_name);
    if (g_vfio_ctx.device < 0) return false;

    struct vfio_region_info reg_info = {
        .argsz = sizeof(reg_info),
        .index = VFIO_PCI_BAR0_REGION_INDEX
    };
    if (ioctl(g_vfio_ctx.device, VFIO_DEVICE_GET_REGION_INFO, &reg_info) < 0) return false;

    void *mapped_regs = mmap(NULL, reg_info.size, PROT_READ | PROT_WRITE, MAP_SHARED, g_vfio_ctx.device, *(uint64_t *)((char *)&reg_info + 24));
    if (mapped_regs == MAP_FAILED) return false;

    g_vfio_ctx.regs = (volatile uint32_t *)mapped_regs;
    g_vfio_ctx.regs_size = reg_info.size;

    // Allocate ring descriptor memory (4KB) and packet buffer memory (1MB)
    if (!vfio_dma_alloc_map(&g_vfio_ctx.ring_dma, 4096, 0x1000000)) return false;
    if (!vfio_dma_alloc_map(&g_vfio_ctx.buffer_dma, 1024 * 1024, 0x2000000)) return false;

    return true;
}

// Configure the hardware registers on the physical NIC
void ixgbe_hardware_configure(uint16_t queue_idx, uint32_t desc_count) {
    volatile uint32_t *regs = g_vfio_ctx.regs;
    uint64_t ring_phys = g_vfio_ctx.ring_dma.iova;

    // Set Descriptor base registers
    regs[IXGBE_RDBAL(queue_idx) / 4] = (uint32_t)(ring_phys & 0xFFFFFFFF);
    regs[IXGBE_RDBAH(queue_idx) / 4] = (uint32_t)(ring_phys >> 32);
    regs[IXGBE_RDLEN(queue_idx) / 4] = desc_count * sizeof(struct ixgbe_rx_desc);

    // Initialize RX ring descriptors with packet buffer physical addresses
    struct ixgbe_rx_desc *ring = (struct ixgbe_rx_desc *)g_vfio_ctx.ring_dma.vaddr;
    for (uint32_t i = 0; i < desc_count; i++) {
        ring[i].pkt_addr = g_vfio_ctx.buffer_dma.iova + (i * 2048);
        ring[i].status = 0;
    }

    // Set Head and Tail pointers
    regs[IXGBE_RDH(queue_idx) / 4] = 0;
    regs[IXGBE_RDT(queue_idx) / 4] = desc_count - 1;

    // Enable RX Queue
    regs[IXGBE_RXDCTL(queue_idx) / 4] |= (1 << 25); // RXDCTL.ENABLE
}

// Read packets directly from the physical hardware rings
void vfio_hardware_poll(uint16_t queue_idx, uint32_t desc_count) {
    struct ixgbe_rx_desc *ring = (struct ixgbe_rx_desc *)g_vfio_ctx.ring_dma.vaddr;
    volatile uint32_t *regs = g_vfio_ctx.regs;
    uint32_t head = regs[IXGBE_RDH(queue_idx) / 4];
    uint32_t tail = regs[IXGBE_RDT(queue_idx) / 4];

    while (head != tail) {
        volatile struct ixgbe_rx_desc *desc = &ring[head];
        if (desc->status & (1 << 0)) { // Descriptor Done (DD) status bit
            uint8_t *pkt = (uint8_t *)g_vfio_ctx.buffer_dma.vaddr + (head * 2048);
            uint16_t len = desc->status & 0xFFFF;
            
            // Parse STANAG packet directly from wire
            if (len > 14) {
                uint16_t eth_type = ntohs(*(uint16_t *)(pkt + 12));
                if (eth_type == 0x8B32) {
                    uint16_t dst_port = ntohs(*(uint16_t *)(pkt + 14));
                    if (dst_port >= STANAG_PORT_MIN && dst_port <= STANAG_PORT_MAX) {
                        uint32_t port_idx = dst_port - STANAG_PORT_MIN;
                        interop_stanag_registry.rx_packet_counts[port_idx]++;
                        interop_stanag_registry.rx_byte_counts[port_idx] += len;
                        if (interop_stanag_registry.listeners[port_idx]) {
                            interop_stanag_registry.listeners[port_idx](dst_port, pkt + 16, len - 16);
                        }
                    }
                }
            }
            
            // Clean up descriptor and update Tail pointer
            desc->status = 0;
            regs[IXGBE_RDT(queue_idx) / 4] = head;
            head = (head + 1) % desc_count;
        } else {
            break;
        }
    }
}

int main(int argc, char **argv) {
    printf("=============================================================\n");
    printf("AUNCIENT STANAG OVER VFIO HARDWARE CONTROLLER PIPELINE\n");
    printf("=============================================================\n");

    if (argc < 3) {
        printf("[INFO] Standalone execution requires VFIO paths.\n");
        printf("Usage: %s [/dev/vfio/X] [PCI_DEVICE_NAME]\n", argv[0]);
        return 0;
    }

    if (!vfio_driver_init(argv[1], argv[2])) {
        printf("[ERROR] Failed to initialize hardware VFIO connection.\n");
        return 1;
    }

    printf("[SUCCESS] Hardware mapping complete. Running active PMD polling...\n");
    ixgbe_hardware_configure(0, 128);
    
    // Poll hardware interface
    vfio_hardware_poll(0, 128);

    return 0;
}
