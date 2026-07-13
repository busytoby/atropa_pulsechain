#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define SAMPLE_RATE 8000
#define FREQ_MARK 1200  // Binary 1 Carrier
#define FREQ_SPACE 2200 // Binary 0 Carrier
#define BIT_DURATION 0.005 // 5ms per bit

// ZMODEM Frame definition constants
#define ZPAD '*'
#define ZDLE 0x18
#define ZBIN 'A' // Binary Header Type

// ZMODEM Header types
#define ZRQINIT 1 // Request init
#define ZDATA   2 // Data frame

// CRC-16 generation lookup or calculation
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

// Encode ZMODEM capability frame into FSK signal
size_t encode_zmodem_fsk(uint8_t frame_type, const uint8_t *payload, size_t len, double *out_signal) {
    // Construct packet: ZPAD, ZDLE, ZBIN, frame_type, payload, CRC16
    size_t packet_len = 5 + len + 2;
    uint8_t *packet = malloc(packet_len);
    packet[0] = ZPAD;
    packet[1] = ZDLE;
    packet[2] = ZBIN;
    packet[3] = frame_type;
    memcpy(packet + 4, payload, len);
    
    uint16_t crc = calculate_crc16(packet, 4 + len);
    packet[4 + len] = (uint8_t)(crc >> 8);
    packet[5 + len] = (uint8_t)(crc & 0xFF);

    // Convert packet bytes to FSK samples
    size_t samples_per_bit = (size_t)(SAMPLE_RATE * BIT_DURATION);
    size_t total_samples = packet_len * 8 * samples_per_bit;
    size_t sample_idx = 0;

    for (size_t i = 0; i < packet_len; i++) {
        uint8_t byte = packet[i];
        for (int bit = 7; bit >= 0; bit--) {
            bool is_one = (byte >> bit) & 1;
            double freq = is_one ? FREQ_MARK : FREQ_SPACE;
            for (size_t s = 0; s < samples_per_bit; s++) {
                double t = (double)sample_idx / SAMPLE_RATE;
                out_signal[sample_idx++] = sin(2.0 * M_PI * freq * t);
            }
        }
    }
    
    free(packet);
    return total_samples;
}

// Decode FSK signal back to ZMODEM bytes
size_t decode_fsk_zmodem(const double *signal, size_t total_samples, uint8_t *out_payload, uint8_t *out_frame_type, const char **out_err) {
    size_t samples_per_bit = (size_t)(SAMPLE_RATE * BIT_DURATION);
    size_t total_bits = total_samples / samples_per_bit;
    size_t packet_len = total_bits / 8;
    
    uint8_t *packet = malloc(packet_len);
    size_t sample_idx = 0;

    for (size_t i = 0; i < packet_len; i++) {
        uint8_t byte = 0;
        for (int bit = 7; bit >= 0; bit--) {
            // Demodulate using simple zero-crossing frequency estimation
            int crossings = 0;
            double prev = signal[sample_idx];
            for (size_t s = 1; s < samples_per_bit; s++) {
                double curr = signal[sample_idx + s];
                if ((prev < 0.0 && curr >= 0.0) || (prev > 0.0 && curr <= 0.0)) {
                    crossings++;
                }
                prev = curr;
            }
            sample_idx += samples_per_bit;

            double estimated_freq = (crossings * SAMPLE_RATE) / (2.0 * samples_per_bit);
            // Threshold frequency: mid-way is 1700 Hz
            if (estimated_freq < 1700.0) {
                byte |= (1 << bit); // Binary 1
            }
        }
        packet[i] = byte;
    }

    // Verify ZMODEM sync indicators
    if (packet[0] != ZPAD || packet[1] != ZDLE || packet[2] != ZBIN) {
        *out_err = "REVERT: ZMODEM_SYNC_FAILED";
        free(packet);
        return 0;
    }

    *out_frame_type = packet[3];
    size_t payload_len = packet_len - 7;
    memcpy(out_payload, packet + 4, payload_len);

    // Verify CRC
    uint16_t decoded_crc = ((uint16_t)packet[4 + payload_len] << 8) | packet[5 + payload_len];
    uint16_t calculated_crc = calculate_crc16(packet, 4 + payload_len);
    if (decoded_crc != calculated_crc) {
        *out_err = "REVERT: CRC_VERIFICATION_FAILED";
        free(packet);
        return 0;
    }

    free(packet);
    return payload_len;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: ZMODEM-FRAMED FSK ACOUSTIC PEERING TESTS\n");
    printf("=============================================================\n");

    const char *capability_data = "ACL_UPGRADE_ADMIN";
    size_t data_len = strlen(capability_data);

    // Allocate signal buffer (size: packet_len * 8 * samples_per_bit)
    // Packet len = 5 + 17 + 2 = 24 bytes. Bits = 192. Samples = 192 * 40 = 7680
    double *signal = calloc(8000, sizeof(double));
    assert(signal != NULL);

    printf("1. Encoding capability payload into ZMODEM FSK wave...\n");
    size_t total_samples = encode_zmodem_fsk(ZDATA, (const uint8_t *)capability_data, data_len, signal);
    printf("   ✓ FSK encoding completed: %lu audio samples generated.\n\n", total_samples);

    printf("2. Demodulating and decoding FSK signal...\n");
    uint8_t decoded_payload[64] = {0};
    uint8_t frame_type = 0;
    const char *err = NULL;
    size_t decoded_len = decode_fsk_zmodem(signal, total_samples, decoded_payload, &frame_type, &err);
    
    assert(decoded_len > 0);
    assert(frame_type == ZDATA);
    assert(strcmp((char *)decoded_payload, capability_data) == 0);
    
    printf("   ✓ Decoded payload matches original: \"%s\"\n", decoded_payload);
    printf("   ✓ ZMODEM CRC-16 validation succeeded.\n\n");

    free(signal);
    printf("=============================================================\n");
    printf("AUNCIENT ZMODEM FSK TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
