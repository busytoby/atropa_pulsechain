#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STATE_FILE "bgp_precedence_paths.dat.bin"
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define MAX_ROUTES     4

// BGP Path Precedence Record
typedef struct {
    uint32_t prefix_space;
    uint32_t as_path_length;
    uint32_t local_preference;
    uint32_t resolved_hop_address;
} bgp_path_t;

// BGP Router Routing Engine State
typedef struct {
    bgp_path_t routing_table[MAX_ROUTES];
    int route_count;
    uint32_t status_color;
    bool switch_residency_red;
} bgp_engine_t;

static bool select_best_bgp_path(bgp_engine_t *engine, const bgp_path_t *new_path) {
    printf("[BGP ENGINE] Evaluating path to prefix 0x%X: AS Path Len: %u, Local Pref: %u...\n",
           new_path->prefix_space, new_path->as_path_length, new_path->local_preference);
    fflush(stdout);

    // Loop detection constraint: Disallow AS paths that create cyclical routes
    if (new_path->as_path_length > 10) {
        printf("   [BGP LOOP ALERT] AS path loop detected! Toggling RED.\n");
        engine->status_color = COLOR_RED;
        engine->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    // Determine if we already have a route to this prefix
    int existing_idx = -1;
    for (int i = 0; i < engine->route_count; i++) {
        if (engine->routing_table[i].prefix_space == new_path->prefix_space) {
            existing_idx = i;
            break;
        }
    }

    if (existing_idx == -1) {
        // New prefix path: Add directly
        if (engine->route_count >= MAX_ROUTES) {
            return false;
        }
        engine->routing_table[engine->route_count++] = *new_path;
        printf("   [BGP PATH ACCEPT] Added new route prefix 0x%X via Hop 0x%X.\n",
               new_path->prefix_space, new_path->resolved_hop_address);
        fflush(stdout);
    } else {
        // Existing prefix path: Select best path based on Precedence rules
        bgp_path_t *existing = &engine->routing_table[existing_idx];

        // Rule 1: Higher local preference wins
        // Rule 2: Shortest AS path length wins (fallback)
        bool replace = false;
        if (new_path->local_preference > existing->local_preference) {
            replace = true;
        } else if (new_path->local_preference == existing->local_preference &&
                   new_path->as_path_length < existing->as_path_length) {
            replace = true;
        }

        if (replace) {
            printf("   [BGP PATH UPDATE] Replacing old route via Hop 0x%X with better path via Hop 0x%X.\n",
                   existing->resolved_hop_address, new_path->resolved_hop_address);
            *existing = *new_path;
            fflush(stdout);
        } else {
            printf("   [BGP PATH IGNORE] Discarding inferior path route via Hop 0x%X.\n",
                   new_path->resolved_hop_address);
            fflush(stdout);
        }
    }

    engine->status_color = COLOR_CYAN;
    engine->switch_residency_red = false;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BGP ROUTING PATH PRECEDENCE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    bgp_engine_t engine = {
        .route_count = 0,
        .status_color = COLOR_BLACK,
        .switch_residency_red = false
    };

    // 1. Initial route to prefix 0x4000
    bgp_path_t route1 = {
        .prefix_space = 0x4000,
        .as_path_length = 3,
        .local_preference = 100,
        .resolved_hop_address = 0x1111
    };
    bool ok = select_best_bgp_path(&engine, &route1);
    assert(ok == true);
    assert(engine.route_count == 1);

    // 2. Peer advertises a better path (higher local preference)
    bgp_path_t route2 = {
        .prefix_space = 0x4000,
        .as_path_length = 2,
        .local_preference = 200,
        .resolved_hop_address = 0x2222
    };
    ok = select_best_bgp_path(&engine, &route2);
    assert(ok == true);
    assert(engine.routing_table[0].resolved_hop_address == 0x2222);

    // 3. Peer advertises a path creating a loop
    bgp_path_t route3 = {
        .prefix_space = 0x4000,
        .as_path_length = 99, // Loop simulation
        .local_preference = 300,
        .resolved_hop_address = 0x3333
    };
    ok = select_best_bgp_path(&engine, &route3);
    assert(ok == false);
    assert(engine.switch_residency_red == true);
    assert(engine.status_color == COLOR_RED);

    // 4. Serialize precedence paths to DAT file
    printf("[TEST] Deploying BGP path table to database %s...\n", STATE_FILE);
    fflush(stdout);

    FILE *file = fopen(STATE_FILE, "wb");
    assert(file != NULL);
    size_t written = fwrite(engine.routing_table, sizeof(bgp_path_t), engine.route_count, file);
    assert(written == (size_t)engine.route_count);
    fclose(file);

    // Read back and verify database
    printf("[TEST] Verifying written database records...\n");
    fflush(stdout);

    file = fopen(STATE_FILE, "rb");
    assert(file != NULL);

    bgp_path_t reloaded_paths[MAX_ROUTES];
    size_t read_bytes = fread(reloaded_paths, sizeof(bgp_path_t), engine.route_count, file);
    assert(read_bytes == (size_t)engine.route_count);
    fclose(file);

    // Clean up temporary DAT file
    remove(STATE_FILE);

    assert(reloaded_paths[0].prefix_space == 0x4000);
    assert(reloaded_paths[0].resolved_hop_address == 0x2222);
    printf("   ✓ BGP routing path precedence database serialization validated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BGP ROUTING PATH PRECEDENCE TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
