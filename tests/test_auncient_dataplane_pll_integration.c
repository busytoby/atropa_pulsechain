#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>

#define PAYLOAD_SIZE 64
#define LOCK_THRESHOLD 0.05

typedef struct {
    uint64_t sequence_counter;
    uint8_t payload[PAYLOAD_SIZE];
    uint16_t checksum;
} integrated_packet_t;

// Phase-Locked Loop clock tracker state
typedef struct {
    double ref_phase;
    double local_phase;
    double phase_error;
    bool pll_locked;
} pll_clock_t;

typedef struct {
    int socket_fds[2];
    uint64_t last_seq;
    pll_clock_t pll;
} integrated_dataplane_t;

static uint16_t calculate_checksum(const uint8_t *data, size_t len) {
    uint16_t checksum = 0;
    for (size_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// Update Phase-Locked Loop feedback correction
static void update_pll_loop(pll_clock_t *pll, double next_ref_phase) {
    pll->ref_phase = next_ref_phase;
    
    // Compute phase error
    pll->phase_error = pll->ref_phase - pll->local_phase;

    // Stable proportional feedback convergence towards reference phase
    pll->local_phase += pll->phase_error * 0.35;

    // Check if lock has been achieved
    pll->pll_locked = (fabs(pll->phase_error) < LOCK_THRESHOLD);
}

// Ingest and transmit packet over Dataplane loops (requires active PLL clock lock)
static bool transmit_integrated_packet(integrated_dataplane_t *dp, uint64_t seq, const uint8_t *data) {
    // Structural Guard Gate: block routing if PLL clock is unlocked (drift state)
    if (!dp->pll.pll_locked) {
        return false;
    }

    integrated_packet_t packet;
    packet.sequence_counter = seq;
    memcpy(packet.payload, data, PAYLOAD_SIZE);
    packet.checksum = calculate_checksum(data, PAYLOAD_SIZE);

    if (write(dp->socket_fds[0], &packet, sizeof(integrated_packet_t)) < 0) {
        return false;
    }
    return true;
}

static bool receive_integrated_packet(integrated_dataplane_t *dp, integrated_packet_t *rx_packet) {
    if (read(dp->socket_fds[1], rx_packet, sizeof(integrated_packet_t)) < 0) {
        return false;
    }

    // Monotonicity check
    if (rx_packet->sequence_counter <= dp->last_seq) {
        return false;
    }
    dp->last_seq = rx_packet->sequence_counter;

    // Checksum validation
    uint16_t computed = calculate_checksum(rx_packet->payload, PAYLOAD_SIZE);
    if (computed != rx_packet->checksum) {
        return false;
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DATAPLANE AND PLL INTEGRATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    integrated_dataplane_t dp;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, dp.socket_fds) < 0) {
        return 1;
    }
    dp.last_seq = 0;

    // Initialize PLL clock tracker
    dp.pll.ref_phase = 1.0;
    dp.pll.local_phase = 1.0;
    dp.pll.phase_error = 0.0;
    dp.pll.pll_locked = true;

    uint8_t data[PAYLOAD_SIZE];
    memset(data, 0xA5, PAYLOAD_SIZE);

    // 1. Send package under locked state (conduction path open)
    printf("[TEST] Transmitting packet under locked PLL clock phase...\n");
    fflush(stdout);
    bool ok = transmit_integrated_packet(&dp, 10, data);
    assert(ok == true);

    integrated_packet_t rx;
    ok = receive_integrated_packet(&dp, &rx);
    assert(ok == true);
    assert(rx.sequence_counter == 10);
    printf("   ✓ Packet routed successfully.\n");
    fflush(stdout);

    // 2. Inject frequency/phase drift (local clock drifts, locking drops)
    printf("[TEST] Injecting phase drift into PLL clock (unlocked state)...\n");
    fflush(stdout);
    dp.pll.local_phase = 5.0; // Simulated clock slip
    update_pll_loop(&(dp.pll), 1.0);
    assert(dp.pll.pll_locked == false); // Lock should drop

    // Dataplane must block packet queue transmission
    ok = transmit_integrated_packet(&dp, 20, data);
    assert(ok == false);
    printf("   ✓ Transmission blocked successfully by PLL guard gate.\n");
    fflush(stdout);

    // 3. Run feedback cycles to stabilize clock
    printf("[TEST] Running PLL feedback loop cycles to restore synchronization...\n");
    fflush(stdout);
    for (int i = 0; i < 30; i++) {
        update_pll_loop(&(dp.pll), 1.0);
        if (dp.pll.pll_locked) {
            break;
        }
    }
    assert(dp.pll.pll_locked == true);
    printf("   ✓ PLL clock locked onto carrier phase (error: %.4f).\n", dp.pll.phase_error);
    fflush(stdout);

    // 4. Resume packet routing
    printf("[TEST] Resuming Dataplane transmission queue...\n");
    fflush(stdout);
    ok = transmit_integrated_packet(&dp, 20, data);
    assert(ok == true);

    ok = receive_integrated_packet(&dp, &rx);
    assert(ok == true);
    assert(rx.sequence_counter == 20);
    printf("   ✓ Packet routed successfully after lock restoration.\n");
    fflush(stdout);

    close(dp.socket_fds[0]);
    close(dp.socket_fds[1]);

    printf("=============================================================\n");
    printf("INTEGRATED DATAPLANE AND PLL TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
