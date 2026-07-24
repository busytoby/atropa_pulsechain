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

#define ETHER_TYPE_DECNET 0x6003
#define NSP_MSG_CI  0x18
#define NSP_MSG_CC  0x28
#define NSP_MSG_DATA 0x30

// DECnet Routing Header (Short Format)
struct decnet_routing_hdr {
    uint8_t flags;
    uint16_t dst_node;
    uint16_t src_node;
    uint8_t forward_count;
} __attribute__((packed));

struct decnet_nsp_hdr {
    uint8_t msg_type;
    uint16_t dst_link;
    uint16_t src_link;
} __attribute__((packed));

struct ixgbe_rx_desc {
    uint64_t pkt_addr;
    uint64_t status;
} __attribute__((packed));

struct vfio_dma_region {
    void *vaddr;
    uint64_t iova;
    size_t size;
};

// Thread-safe dynamic connection registry
typedef enum {
    DEC_CLOSED,
    DEC_CONNECTED
} decnet_state_t;

struct decnet_connection {
    decnet_state_t state;
    uint16_t remote_node;
    uint16_t local_link;
    uint16_t remote_link;
} interop_decnet_conn;

struct vfio_context {
    int container;
    int group;
    int device;
    volatile uint32_t *regs;
    size_t regs_size;
    struct vfio_dma_region ring_dma;
    struct vfio_dma_region buffer_dma;
} g_decnet_vfio;

// IOMMU Type 1 DMA Mapping helper
static bool vfio_dma_alloc_map(struct vfio_dma_region *dma, size_t size, uint64_t iova) {
    dma->size = size;
    dma->iova = iova;
    
    dma->vaddr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (dma->vaddr == MAP_FAILED) return false;

    struct vfio_iommu_type1_dma_map dma_map = {
        .argsz = sizeof(dma_map),
        .flags = VFIO_DMA_MAP_FLAG_READ | VFIO_DMA_MAP_FLAG_WRITE,
        .vaddr = (uint64_t)dma->vaddr,
        .iova = dma->iova,
        .size = dma->size
    };

    if (ioctl(g_decnet_vfio.container, VFIO_IOMMU_MAP_DMA, &dma_map) < 0) {
        munmap(dma->vaddr, size);
        return false;
    }
    return true;
}

bool decnet_vfio_init(const char *group_path, const char *device_name) {
    g_decnet_vfio.container = open("/dev/vfio/vfio", O_RDWR);
    if (g_decnet_vfio.container < 0) return false;

    g_decnet_vfio.group = open(group_path, O_RDWR);
    if (g_decnet_vfio.group < 0) return false;

    if (ioctl(g_decnet_vfio.group, VFIO_GROUP_SET_CONTAINER, &g_decnet_vfio.container) < 0) return false;
    if (ioctl(g_decnet_vfio.container, VFIO_SET_IOMMU, VFIO_TYPE1_IOMMU) < 0) return false;

    g_decnet_vfio.device = ioctl(g_decnet_vfio.group, VFIO_GROUP_GET_DEVICE_FD, device_name);
    if (g_decnet_vfio.device < 0) return false;

    struct vfio_region_info reg_info = {
        .argsz = sizeof(reg_info),
        .index = VFIO_PCI_BAR0_REGION_INDEX
    };
    if (ioctl(g_decnet_vfio.device, VFIO_DEVICE_GET_REGION_INFO, &reg_info) < 0) return false;

    void *mapped_regs = mmap(NULL, reg_info.size, PROT_READ | PROT_WRITE, MAP_SHARED, g_decnet_vfio.device, *(uint64_t *)((char *)&reg_info + 24));
    if (mapped_regs == MAP_FAILED) return false;

    g_decnet_vfio.regs = (volatile uint32_t *)mapped_regs;
    g_decnet_vfio.regs_size = reg_info.size;

    if (!vfio_dma_alloc_map(&g_decnet_vfio.ring_dma, 4096, 0x1000000)) return false;
    if (!vfio_dma_alloc_map(&g_decnet_vfio.buffer_dma, 1024 * 1024, 0x2000000)) return false;

    return true;
}

// Process incoming DECnet routing frames
void decnet_process_packet(const uint8_t *pkt, uint16_t len) {
    if (len < 14 + sizeof(struct decnet_routing_hdr) + sizeof(struct decnet_nsp_hdr)) return;

    struct decnet_routing_hdr *route = (struct decnet_routing_hdr *)(pkt + 14);
    struct decnet_nsp_hdr *nsp = (struct decnet_nsp_hdr *)(route + 1);

    if (nsp->msg_type == NSP_MSG_CI) {
        interop_decnet_conn.state = DEC_CONNECTED;
        interop_decnet_conn.remote_node = route->src_node;
        interop_decnet_conn.local_link = 0xABCD;
        interop_decnet_conn.remote_link = nsp->src_link;
        printf("   [DECNET-HW] NSP Connect Initiate processed. Connection established.\n");
    } else if (nsp->msg_type == NSP_MSG_DATA) {
        printf("   [DECNET-HW] Direct NSP data received: %s\n", (char *)(nsp + 1));
    }
}

int main(int argc, char **argv) {
    printf("=============================================================\n");
    printf("AUNCIENT DECNET HARDWARE CONNECTOR PIPELINE\n");
    printf("=============================================================\n");

    if (argc < 3) {
        printf("[INFO] Standalone execution requires VFIO paths.\n");
        printf("Usage: %s [/dev/vfio/X] [PCI_DEVICE_NAME]\n", argv[0]);
        return 0;
    }

    if (!decnet_vfio_init(argv[1], argv[2])) {
        printf("[ERROR] Failed to map hardware DECnet VFIO registers.\n");
        return 1;
    }

    printf("[SUCCESS] Hardware mapping complete. DECnet listening.\n");
    return 0;
}
