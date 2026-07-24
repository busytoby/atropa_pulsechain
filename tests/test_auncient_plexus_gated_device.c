#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Conduction state definitions
typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated Gated Network Device
typedef struct {
    uint32_t tx_register;
    uint32_t status_register;
    gate_state_t hw_write_gate; // Gated by memory tag validation
    gate_state_t hw_wakeup_gate; // Gated by Aho-Corasick match triggers
    bool session_active;
} plexus_gated_device_t;

// Context Tags
#define TAG_KERNEL 0x01
#define TAG_USER   0x02

// -------------------------------------------------------------
// PL/EXUS Gated Device Controls
// -------------------------------------------------------------

// Write to device registers (Physically gated by hardware tag checks)
void plexus_gated_device_write(plexus_gated_device_t *dev, uint32_t val, uint8_t context_tag) {
    // Hardware Write Gate conducts ONLY if context_tag == TAG_KERNEL (active-high gating)
    dev->hw_write_gate = (context_tag == TAG_KERNEL) ? CONDUC_STATE : CUTOFF_STATE;

    // Direct hardware register update resolved by gate conduction status (branchless)
    uint32_t target_val = (val * (dev->hw_write_gate == CONDUC_STATE)) + 
                          (dev->tx_register * (dev->hw_write_gate == CUTOFF_STATE));
    dev->tx_register = target_val;
}

// Signal-Gated Device Wakeup (Toggled directly by AC filter matches)
void plexus_gated_device_signal(plexus_gated_device_t *dev, bool trigger_matched) {
    // Wakeup gate conducts if trigger matched (active-high BJT trigger)
    dev->hw_wakeup_gate = trigger_matched ? CONDUC_STATE : CUTOFF_STATE;

    // Resumes VM session directly from hardware conduction status
    dev->session_active = (dev->hw_wakeup_gate == CONDUC_STATE);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/EXUS GATED DEVICE INTERFACE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    plexus_gated_device_t dev = {
        .tx_register = 0,
        .status_register = 0,
        .hw_write_gate = CUTOFF_STATE,
        .hw_wakeup_gate = CUTOFF_STATE,
        .session_active = false
    };

    // 1. Validate Tagged Write Gating (Kernel context vs User context)
    printf("[TEST] Attempting write from User context (unauthorized)...\n");
    fflush(stdout);
    plexus_gated_device_write(&dev, 0x9999, TAG_USER);
    assert(dev.hw_write_gate == CUTOFF_STATE);
    assert(dev.tx_register == 0); // Decoupled: write blocked
    printf("   ✓ Unauthorized write blocked successfully.\n");
    fflush(stdout);

    printf("[TEST] Attempting write from Kernel context (authorized)...\n");
    fflush(stdout);
    plexus_gated_device_write(&dev, 0x7777, TAG_KERNEL);
    assert(dev.hw_write_gate == CONDUC_STATE);
    assert(dev.tx_register == 0x7777); // Conducted: write completed
    printf("   ✓ Authorized write completed successfully.\n");
    fflush(stdout);

    // 2. Validate Signal-Gated Wakeup
    printf("[TEST] Checking device session wakeup on matching trigger...\n");
    fflush(stdout);
    assert(dev.session_active == false);

    // Trigger match event
    plexus_gated_device_signal(&dev, true);
    assert(dev.hw_wakeup_gate == CONDUC_STATE);
    assert(dev.session_active == true); // Wakeup completed
    printf("   ✓ Signal-gated device session wakeup verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("GATED DEVICE INTERFACE VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
