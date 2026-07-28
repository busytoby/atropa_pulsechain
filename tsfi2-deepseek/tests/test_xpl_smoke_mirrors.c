#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// VTOC lookup helper matching test_xpl_vtoc.c structure
typedef struct {
    char dataset_name[44];
    uint32_t start_track;
    uint32_t end_track;
    uint32_t block_size;
} vtoc_entry_t;

static vtoc_entry_t vtoc_catalog[] = {
    { "SYS1.ISPSLIB",     10,  20,  800 },
    { "SYS1.ISPPLIB",     21,  30,  800 },
    { "CBTCOV.FILE012",   31,  50, 3120 }
};

static bool verify_workspace_exists(const char *dsn) {
    for (size_t i = 0; i < 3; i++) {
        if (strcmp(vtoc_catalog[i].dataset_name, dsn) == 0) {
            return true;
        }
    }
    return false;
}

// 1. ISPF Parser helper for Smoke & Mirrors panel
static bool parse_smoke_mirrors_panel(const char *filepath, char *dsn_out, char *net_out) {
    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char line[128];
    bool in_init = false;
    int matches = 0;

    while (fgets(line, sizeof(line), f)) {
        // Strip trailing newline/whitespace
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || line[len - 1] == ' ')) {
            line[len - 1] = '\0';
            len--;
        }

        if (strcmp(line, ")INIT") == 0) {
            in_init = true;
            continue;
        }
        if (line[0] == ')' && strcmp(line, ")INIT") != 0) {
            in_init = false;
        }

        if (in_init) {
            char name[64] = {0};
            char val[64] = {0};
            if (sscanf(line, "  &%63[^ ] = '%63[^']'", name, val) == 2) {
                if (strcmp(name, "TARGET_DSN") == 0) {
                    strcpy(dsn_out, val);
                    matches++;
                } else if (strcmp(name, "TARGET_NET") == 0) {
                    strcpy(net_out, val);
                    matches++;
                }
            }
        }
    }
    fclose(f);
    return (matches == 2);
}

// 2. RED/BLACK Workspace Switcher and Router
static bool switch_compiler_workspace(
    const char *dsn,
    uint32_t net_id,
    char *active_dsn_buf,
    uint32_t *active_net_buf
) {
    // RED ANALYZER Gating: Validate workspace existence statically
    if (!verify_workspace_exists(dsn)) {
        return false; // Rejected by RED rail
    }

    // BLACK XPLSM Gating: Validate network scope limits dynamically
    if (net_id == 0 || net_id > 255) {
        return false; // Rejected by BLACK rail
    }

    // Mount workspace successfully
    strcpy(active_dsn_buf, dsn);
    *active_net_buf = net_id;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ISPF SMOKE & MIRRORS WORKSPACE VIRTUALIZATION TEST\n");
    printf("=============================================================\n");

    const char *panel_path = "ispf_libraries/ispplib/smoke_mirrors.ispplib";
    char panel_dsn[64] = {0};
    char panel_net[16] = {0};

    // Test 1: Parse initialization values from Panel
    printf("[TEST] Parsing workspace properties from ISPF Panel...\n");
    assert(parse_smoke_mirrors_panel(panel_path, panel_dsn, panel_net) == true);
    assert(strcmp(panel_dsn, "SYS1.ISPSLIB") == 0);
    assert(strcmp(panel_net, "42") == 0);
    printf("   ✓ Panel variables successfully extracted: DSN=%s, NetID=%s\n", panel_dsn, panel_net);

    // Test 2: Workspace routing and virtualization
    printf("[TEST] Mounting and routing logical workspace coordinates...\n");
    char active_dsn[64] = "NONE";
    uint32_t active_net = 0;

    // Successful mount of SYS1.ISPSLIB workspace
    assert(switch_compiler_workspace(panel_dsn, atoi(panel_net), active_dsn, &active_net) == true);
    assert(strcmp(active_dsn, "SYS1.ISPSLIB") == 0);
    assert(active_net == 42);
    printf("   ✓ Successfully routed task to workspace %s on NetID %u.\n", active_dsn, active_net);

    // Test 3: RED rail rejection of unallocated workspace DSN
    printf("[TEST] Verifying RED rail rejection on unallocated DSN...\n");
    assert(switch_compiler_workspace("UNALLOCATED.DSN", 42, active_dsn, &active_net) == false);
    assert(strcmp(active_dsn, "SYS1.ISPSLIB") == 0); // Active remains unchanged
    printf("   ✓ RED rail blocked invalid workspace switch correctly.\n");

    // Test 4: BLACK rail rejection of out-of-range NetID
    printf("[TEST] Verifying BLACK rail rejection on out-of-range NetID...\n");
    assert(switch_compiler_workspace("SYS1.ISPSLIB", 999, active_dsn, &active_net) == false);
    printf("   ✓ BLACK rail blocked invalid network ID mapping correctly.\n");

    printf("=============================================================\n");
    printf("ALL WORKSPACE VIRTUALIZATION TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
