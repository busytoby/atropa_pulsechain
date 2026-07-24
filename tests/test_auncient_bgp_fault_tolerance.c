#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STATE_FILE "bgp_fault_states.dat.bin"
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define FLAP_THRESHOLD 3
#define MAX_ROUTES     4

// Disputed Route State
typedef struct {
    uint32_t prefix_space;
    uint32_t originating_as;
    int advertisement_count;
    bool in_quarantine;
} disputed_route_t;

// BGP Fault-Tolerant Router State
typedef struct {
    disputed_route_t routes[MAX_ROUTES];
    int route_count;
    uint32_t status_color;
    bool switch_residency_red;
} bgp_fault_router_t;

static void evaluate_fault_tolerant_route(bgp_fault_router_t *router,
                                          uint32_t prefix_space,
                                          uint32_t originating_as,
                                          uint32_t authorized_as) {
    printf("[BGP FAULT] Route Update: Prefix 0x%X from AS-%u (Authorized: AS-%u)...\n",
           prefix_space, originating_as, authorized_as);
    fflush(stdout);

    // Find if we are tracking this prefix
    int idx = -1;
    for (int i = 0; i < router->route_count; i++) {
        if (router->routes[i].prefix_space == prefix_space) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        if (router->route_count >= MAX_ROUTES) return;
        idx = router->route_count++;
        router->routes[idx].prefix_space = prefix_space;
        router->routes[idx].originating_as = originating_as;
        router->routes[idx].advertisement_count = 1;
        router->routes[idx].in_quarantine = false;
    } else {
        router->routes[idx].advertisement_count++;
    }

    disputed_route_t *r = &router->routes[idx];

    // Hijack attempt from unauthorized AS
    if (originating_as != authorized_as) {
        if (r->advertisement_count >= FLAP_THRESHOLD) {
            // Persistent hijack: quarantine route and trigger RED alert
            r->in_quarantine = true;
            router->status_color = COLOR_RED;
            router->switch_residency_red = true;
            printf("   [ALERT] Persistent hijack confirmed! Prefix 0x%X quarantined. Toggling RED.\n", prefix_space);
            fflush(stdout);
        } else {
            // Transient flapping / race condition: place in grace monitoring period, keep BLACK residency
            r->in_quarantine = false;
            router->status_color = COLOR_BLACK;
            router->switch_residency_red = false;
            printf("   [MONITOR] Disputed update observed (%d/%d). Grace period active.\n",
                   r->advertisement_count, FLAP_THRESHOLD);
            fflush(stdout);
        }
    } else {
        // Legitimate update resolves the dispute
        r->in_quarantine = false;
        r->advertisement_count = 0;
        router->status_color = COLOR_CYAN;
        router->switch_residency_red = false;
        printf("   [RESOLVED] Prefix ownership reconciled successfully.\n");
        fflush(stdout);
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BGP FAULT-TOLERANT RESOLUTION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    bgp_fault_router_t router = {
        .route_count = 0,
        .status_color = COLOR_BLACK,
        .switch_residency_red = false
    };

    // 1. Transient update mismatch (simulating packet out-of-order or flapping)
    printf("[TEST] Scenario 1: Transient route dispute...\n");
    fflush(stdout);
    evaluate_fault_tolerant_route(&router, 0x4000, 65002, 65001); // Unauthorized AS-65002
    assert(router.switch_residency_red == false);
    assert(router.routes[0].in_quarantine == false);

    // 2. Dispute resolved before quarantine threshold
    printf("[TEST] Scenario 2: Reconciliation before threshold...\n");
    fflush(stdout);
    evaluate_fault_tolerant_route(&router, 0x4000, 65001, 65001); // Legitimate AS-65001
    assert(router.switch_residency_red == false);
    assert(router.routes[0].in_quarantine == false);

    // 3. Persistent hijack attempt crossing the threshold
    printf("[TEST] Scenario 3: Persistent unauthorized updates...\n");
    fflush(stdout);
    evaluate_fault_tolerant_route(&router, 0x8000, 65003, 65001); // Update 1
    evaluate_fault_tolerant_route(&router, 0x8000, 65003, 65001); // Update 2
    evaluate_fault_tolerant_route(&router, 0x8000, 65003, 65001); // Update 3 (Trigger alert)
    assert(router.switch_residency_red == true);
    assert(router.routes[1].in_quarantine == true);
    assert(router.status_color == COLOR_RED);

    // 4. Serialize fault state data
    printf("[TEST] Serializing fault-tolerant states to %s...\n", STATE_FILE);
    fflush(stdout);

    FILE *file = fopen(STATE_FILE, "wb");
    assert(file != NULL);
    size_t written = fwrite(&router, sizeof(bgp_fault_router_t), 1, file);
    assert(written == 1);
    fclose(file);

    // Reload and verify
    printf("[TEST] Verifying written database integrity...\n");
    fflush(stdout);

    bgp_fault_router_t reloaded_router;
    file = fopen(STATE_FILE, "rb");
    assert(file != NULL);
    size_t read_bytes = fread(&reloaded_router, sizeof(bgp_fault_router_t), 1, file);
    assert(read_bytes == 1);
    fclose(file);

    remove(STATE_FILE);

    assert(reloaded_router.routes[1].prefix_space == 0x8000);
    assert(reloaded_router.routes[1].in_quarantine == true);

    printf("   ✓ Fault-tolerant database records verified successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BGP FAULT-TOLERANT RESOLUTION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
