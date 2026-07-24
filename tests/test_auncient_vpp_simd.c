#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define VECTOR_SIZE 8
#define PAYLOAD_BLOCKS 2 // 2 * 32 bytes = 64 bytes

// 32-byte aligned payload block to emulate AVX-256 register loads
typedef union {
    uint8_t u8[32];
    uint64_t u64[4];
} __attribute__((aligned(32))) avx_block_t;

typedef struct {
    uint64_t sequence;
    uint64_t checksum;
    avx_block_t payload[PAYLOAD_BLOCKS];
} __attribute__((aligned(32))) aligned_packet_t;

// Emulates SIMD 256-bit XOR checksum validation
static uint64_t compute_simd_checksum(const aligned_packet_t *pkt) {
    uint64_t accumulator = 0;
    
    // Process two 32-byte (256-bit) blocks using 64-bit parallel operations
    for (int b = 0; b < PAYLOAD_BLOCKS; b++) {
        accumulator ^= pkt->payload[b].u64[0];
        accumulator ^= pkt->payload[b].u64[1];
        accumulator ^= pkt->payload[b].u64[2];
        accumulator ^= pkt->payload[b].u64[3];
    }
    return accumulator;
}

// Vector Verification Node: validates entire vector batch in a cache-aligned pass
static int vpp_node_simd_verify(aligned_packet_t *vector, int count) {
    int valid_count = 0;
    
    for (int i = 0; i < count; i++) {
        uint64_t computed = compute_simd_checksum(&vector[i]);
        if (computed == vector[i].checksum) {
            if (i != valid_count) {
                vector[valid_count] = vector[i]; // Compact valid packets
            }
            valid_count++;
        }
    }
    return valid_count;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VPP SIMD-ALIGNED VECTOR VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Allocate 32-byte aligned memory for the packet vector
    aligned_packet_t *vector = NULL;
    int res = posix_memalign((void**)&vector, 32, sizeof(aligned_packet_t) * VECTOR_SIZE);
    assert(res == 0);
    assert(vector != NULL);

    printf("   ✓ Allocated 32-byte aligned packet vector buffer.\n");
    fflush(stdout);

    // 1. Initialize vector packets with valid SIMD checksums
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vector[i].sequence = i + 1;
        
        // Populate 64 bytes of payload
        for (int b = 0; b < PAYLOAD_BLOCKS; b++) {
            vector[i].payload[b].u64[0] = 0x1111111111111111ULL * (i + 1);
            vector[i].payload[b].u64[1] = 0x2222222222222222ULL * (i + 1);
            vector[i].payload[b].u64[2] = 0x3333333333333333ULL * (i + 1);
            vector[i].payload[b].u64[3] = 0x4444444444444444ULL * (i + 1);
        }
        
        vector[i].checksum = compute_simd_checksum(&vector[i]);
    }

    // 2. Ingest and verify all packets (should all be valid)
    printf("[TEST] Verifying all packets under SIMD checksum node...\n");
    fflush(stdout);
    int valid = vpp_node_simd_verify(vector, VECTOR_SIZE);
    assert(valid == VECTOR_SIZE);
    printf("   ✓ Checked %d packets. All verified successfully.\n", valid);
    fflush(stdout);

    // 3. Inject checksum corruption in packet 3 and 6
    printf("[TEST] Injecting corruption and checking validation compactions...\n");
    fflush(stdout);
    vector[2].checksum ^= 0xFFFFFFFF; // Corrupt packet 3
    vector[5].checksum ^= 0xFFFFFFFF; // Corrupt packet 6

    valid = vpp_node_simd_verify(vector, VECTOR_SIZE);
    assert(valid == 6); // 8 - 2 = 6 valid packets remaining
    
    // Verify compaction matches sequences (1, 2, 4, 5, 7, 8)
    assert(vector[0].sequence == 1);
    assert(vector[1].sequence == 2);
    assert(vector[2].sequence == 4);
    assert(vector[3].sequence == 5);
    assert(vector[4].sequence == 7);
    assert(vector[5].sequence == 8);

    printf("   ✓ Corrupted packets successfully dropped and vector compacted.\n");
    fflush(stdout);

    free(vector);

    printf("=============================================================\n");
    printf("VPP SIMD VECTOR VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
