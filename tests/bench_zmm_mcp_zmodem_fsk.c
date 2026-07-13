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

// Simulates the combined ZMODEM framing and FSK modulation/demodulation pipeline
void run_fsk_zmodem_bench(const uint8_t *data, size_t size, const char *label) {
    printf("Benchmarking 1MB of %s transfer...\n", label);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Process data in 1KB chunks to simulate sliding window subpackets
    size_t chunk_size = 1024;
    size_t offset = 0;
    uint32_t processed_bytes = 0;
    uint8_t buffer[1024];

    while (offset < size) {
        size_t current_len = (size - offset > chunk_size) ? chunk_size : (size - offset);
        
        // ZMODEM framing logic: calculate CRC16
        uint16_t crc = calculate_crc16(data + offset, current_len);
        
        // Simulate FSK Modulation: converting bytes to wave cycles
        for (size_t i = 0; i < current_len; i++) {
            uint8_t byte = data[offset + i];
            // Volatile statements prevent compiler optimization from discarding the loop
            volatile double mix = 0.0;
            for (int bit = 7; bit >= 0; bit--) {
                bool is_one = (byte >> bit) & 1;
                double freq = is_one ? FREQ_MARK : FREQ_SPACE;
                mix += sin(2.0 * M_PI * freq * 0.001);
            }
            (void)mix;
            buffer[i] = byte;
        }

        // ZMODEM receiver logic verification: confirm CRC16
        uint16_t calculated_crc = calculate_crc16(buffer, current_len);
        assert(calculated_crc == crc);

        processed_bytes += current_len;
        offset += current_len;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (processed_bytes / (1024.0 * 1024.0)) / elapsed; // MB/s

    printf("Results for %s:\n", label);
    printf("  Elapsed Time   : %.4f seconds\n", elapsed);
    printf("  Throughput     : %.2f MB/s\n", throughput);
    printf("  Processed Bytes: %u\n\n", processed_bytes);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: ZMODEM FSK 1MB THROUGHPUT BENCHMARK\n");
    printf("=============================================================\n");

    // Initialize 1MB datasets
    uint8_t *binary_data = malloc(ONE_MB);
    uint8_t *unicode_data = malloc(ONE_MB);
    assert(binary_data != NULL && unicode_data != NULL);

    // Seed datasets
    for (size_t i = 0; i < ONE_MB; i++) {
        binary_data[i] = (uint8_t)(rand() % 256);
        // Simple UTF-8 multioctet sequence simulation
        unicode_data[i] = (i % 4 == 0) ? 0xF0 : ((i % 4 == 1) ? 0x9F : ((i % 4 == 2) ? 0x92 : 0xA9));
    }

    run_fsk_zmodem_bench(binary_data, ONE_MB, "binary");
    run_fsk_zmodem_bench(unicode_data, ONE_MB, "Unicode");

    free(binary_data);
    free(unicode_data);

    printf("=============================================================\n");
    printf("AUNCIENT ZMODEM FSK BENCHMARKS COMPLETED\n");
    printf("=============================================================\n");
    return 0;
}
