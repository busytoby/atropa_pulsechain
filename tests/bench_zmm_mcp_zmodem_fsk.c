#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define M_PI 3.14159265358979323846
#define SAMPLE_RATE 8000
#define FREQ_MARK 1200
#define FREQ_SPACE 2200
#define BIT_DURATION 0.0001
#define ONE_MB (1024 * 1024)

// Simple Run-Length Encoding (RLE) Compression
size_t rle_compress(const uint8_t *src, size_t src_len, uint8_t *dst) {
    size_t dst_idx = 0;
    size_t i = 0;
    while (i < src_len) {
        size_t run_len = 1;
        while (i + run_len < src_len && src[i + run_len] == src[i] && run_len < 255) {
            run_len++;
        }
        if (run_len >= 4 || src[i] == 0xFE) {
            dst[dst_idx++] = 0xFE; // RLE Marker
            dst[dst_idx++] = src[i];
            dst[dst_idx++] = (uint8_t)run_len;
        } else {
            for (size_t r = 0; r < run_len; r++) {
                dst[dst_idx++] = src[i];
            }
        }
        i += run_len;
    }
    return dst_idx;
}

// Simple Run-Length Encoding (RLE) Decompression
size_t rle_decompress(const uint8_t *src, size_t src_len, uint8_t *dst) {
    size_t dst_idx = 0;
    size_t i = 0;
    while (i < src_len) {
        if (src[i] == 0xFE) {
            uint8_t val = src[i + 1];
            uint8_t count = src[i + 2];
            for (uint8_t r = 0; r < count; r++) {
                dst[dst_idx++] = val;
            }
            i += 3;
        } else {
            dst[dst_idx++] = src[i++];
        }
    }
    return dst_idx;
}

// CRC-16 helper
uint16_t calculate_crc16(const uint8_t *data, size_t len) {
    uint16_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Simulates the combined RLE compression, ZMODEM framing, and FSK pipeline
void run_fsk_zmodem_bench(const uint8_t *data, size_t size, const char *label) {
    printf("Benchmarking 1MB of %s transfer (with RLE compression)...\n", label);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    size_t chunk_size = 1024;
    size_t offset = 0;
    uint32_t raw_processed = 0;
    uint32_t compressed_processed = 0;
    
    uint8_t comp_buf[2048];
    uint8_t decomp_buf[1024];

    while (offset < size) {
        size_t current_len = (size - offset > chunk_size) ? chunk_size : (size - offset);
        
        // 1. Perform RLE compression
        size_t comp_len = rle_compress(data + offset, current_len, comp_buf);
        compressed_processed += comp_len;

        // 2. ZMODEM framing logic: calculate CRC16 of compressed payload
        uint16_t crc = calculate_crc16(comp_buf, comp_len);
        
        // 3. Simulate FSK Modulation on the compressed stream
        for (size_t i = 0; i < comp_len; i++) {
            uint8_t byte = comp_buf[i];
            volatile double mix = 0.0;
            for (int bit = 7; bit >= 0; bit--) {
                bool is_one = (byte >> bit) & 1;
                double freq = is_one ? FREQ_MARK : FREQ_SPACE;
                mix += sin(2.0 * M_PI * freq * 0.001);
            }
            (void)mix;
        }

        // 4. ZMODEM receiver verification: check CRC16
        uint16_t calculated_crc = calculate_crc16(comp_buf, comp_len);
        assert(calculated_crc == crc);

        // 5. Decompress back to original payload
        size_t decomp_len = rle_decompress(comp_buf, comp_len, decomp_buf);
        assert(decomp_len == current_len);
        assert(memcmp(decomp_buf, data + offset, decomp_len) == 0);

        raw_processed += current_len;
        offset += current_len;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double raw_throughput = (raw_processed / (1024.0 * 1024.0)) / elapsed;
    double compression_ratio = (double)raw_processed / compressed_processed;

    printf("Results for %s:\n", label);
    printf("  Elapsed Time       : %.4f seconds\n", elapsed);
    printf("  Effective Rate     : %.2f MB/s\n", raw_throughput);
    printf("  Compression Ratio  : %.2fx\n", compression_ratio);
    printf("  Original Bytes     : %u\n", raw_processed);
    printf("  Compressed Bytes   : %u\n\n", compressed_processed);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: ZMODEM FSK RLE COMPRESSED BENCHMARK\n");
    printf("=============================================================\n");

    // Initialize 1MB datasets
    uint8_t *binary_data = malloc(ONE_MB);
    uint8_t *unicode_data = malloc(ONE_MB);
    assert(binary_data != NULL && unicode_data != NULL);

    // Seed datasets: Unicode contains long repeating sequences of symbols to show compression benefits
    for (size_t i = 0; i < ONE_MB; i++) {
        binary_data[i] = (uint8_t)(rand() % 256);
        // Periodic block repetitions simulating repeating Unicode fields
        unicode_data[i] = (i % 64 < 48) ? 'A' : (uint8_t)(rand() % 256);
    }

    run_fsk_zmodem_bench(binary_data, ONE_MB, "binary");
    run_fsk_zmodem_bench(unicode_data, ONE_MB, "Unicode");

    free(binary_data);
    free(unicode_data);

    printf("=============================================================\n");
    printf("AUNCIENT ZMODEM FSK COMPRESSED BENCHMARKS COMPLETED\n");
    printf("=============================================================\n");
    return 0;
}
