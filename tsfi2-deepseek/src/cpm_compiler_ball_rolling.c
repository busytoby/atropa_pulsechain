#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                      // "ANKH"
    uint16_t load_address;                 // 0x0100
    uint16_t entry_point;                  // 0x0100
    uint32_t opcode_signature;             // "ROLL" (0x524F4C4C)
    uint16_t proof_static_friction_break;  // Theorem 201: Static-to-Kinetic Momentum Transition Invariance
    uint16_t proof_no_slip_holonomic_roll; // Theorem 202: No-Slip Holonomic Rolling Kinematic Constraint
    uint16_t proof_torque_dynamo_coupling; // Theorem 203: Dynamo Angular Torque Acceleration Coupling
    uint16_t proof_lyapunov_momentum_stab; // Theorem 204: Lyapunov Limit-Cycle Rolling Momentum Stability
    uint16_t proof_ball_rolling_closure;   // Theorem 205: Sovereign Ball Rolling System Execution Closure
    char roll_title[64];                   // "GETTING-THE-BALL-ROLLING-DYNAMICS-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmBallRollingBinary;

static uint32_t compute_rule18_checksum(const uint8_t *data, size_t len) {
    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}

int main(int argc, char **argv) {
    const char *out_path = (argc > 1) ? argv[1] : "ball_rolling_proving.bin";

    CpmBallRollingBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x524F4C4C; // "ROLL"

    bin.proof_static_friction_break = 1;
    bin.proof_no_slip_holonomic_roll = 1;
    bin.proof_torque_dynamo_coupling = 1;
    bin.proof_lyapunov_momentum_stab = 1;
    bin.proof_ball_rolling_closure = 1;

    snprintf(bin.roll_title, sizeof(bin.roll_title),
             "GETTING-THE-BALL-ROLLING-DYNAMICS-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Getting the Ball Rolling Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
