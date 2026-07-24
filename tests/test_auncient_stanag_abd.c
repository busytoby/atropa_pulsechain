#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define NUM_NODES 3

typedef struct {
    uint32_t channel_id;
    uint32_t frequency_hz;
    uint64_t timestamp;
} stanag_config_t;

typedef struct {
    stanag_config_t local_configs[NUM_NODES];
} stanag_network_t;

// -------------------------------------------------------------
// STANAG ABD Configuration Sync Protocol
// -------------------------------------------------------------
bool write_stanag_config(stanag_network_t *net, uint32_t channel, uint32_t freq, uint64_t ts) {
    int acks = 0;

    // Update configuration on all accessible nodes
    for (int i = 0; i < NUM_NODES; i++) {
        // Only overwrite if incoming timestamp is strictly greater
        if (ts > net->local_configs[i].timestamp) {
            net->local_configs[i].channel_id = channel;
            net->local_configs[i].frequency_hz = freq;
            net->local_configs[i].timestamp = ts;
            acks++;
        }
    }

    // Require majority approval (at least 2/3 nodes)
    return (acks > NUM_NODES / 2);
}

stanag_config_t read_stanag_config(stanag_network_t *net) {
    stanag_config_t highest = { .channel_id = 0, .frequency_hz = 0, .timestamp = 0 };

    // Query majority to find highest timestamp
    for (int i = 0; i < NUM_NODES; i++) {
        if (net->local_configs[i].timestamp > highest.timestamp) {
            highest = net->local_configs[i];
        }
    }

    // Read Propagate Phase: Write back the highest found config to a majority
    if (highest.timestamp > 0) {
        write_stanag_config(net, highest.channel_id, highest.frequency_hz, highest.timestamp);
    }

    return highest;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT STANAG-ABD CONFIGURATION CONSENSUS SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    stanag_network_t net = {
        .local_configs = {
            { .channel_id = 1, .frequency_hz = 480000, .timestamp = 1 },
            { .channel_id = 1, .frequency_hz = 480000, .timestamp = 1 },
            { .channel_id = 1, .frequency_hz = 480000, .timestamp = 1 }
        }
    };

    // 1. Write new frequency configuration with higher timestamp
    printf("[TEST] Writing new STANAG configuration (channel = 2, freq = 960000, ts = 5)...\n");
    fflush(stdout);
    bool ok = write_stanag_config(&net, 2, 960000, 5);
    assert(ok == true);

    // 2. Perform read-propagate operation to verify consensus configuration
    printf("[TEST] Reading and propagating STANAG configuration...\n");
    fflush(stdout);
    stanag_config_t current = read_stanag_config(&net);
    assert(current.channel_id == 2);
    assert(current.frequency_hz == 960000);
    assert(current.timestamp == 5);
    printf("   ✓ Configuration converged. Current Frequency: %d Hz.\n", current.frequency_hz);
    fflush(stdout);

    // 3. Stale update proposal check
    printf("[TEST] Rejecting stale configuration write (ts = 4)...\n");
    fflush(stdout);
    ok = write_stanag_config(&net, 3, 1200000, 4);
    assert(ok == false); // Rejected due to stale timestamp

    current = read_stanag_config(&net);
    assert(current.channel_id == 2); // Configuration preserved
    printf("   ✓ Stale update successfully blocked.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("STANAG-ABD TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
