#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// Struct representing MVS ISPF Message Lookup
typedef struct {
    char code[16];
    char short_text[64];
    char long_text[128];
    bool alarm;
} ispf_message_t;

// 1. RED ANALYZER Gating: Static format validation of message code
static bool red_analyzer_validate_msg_format(const char *msg_code) {
    if (!msg_code) return false;
    size_t len = strlen(msg_code);
    if (len != 8) return false; // Format limit: exactly 8 characters (e.g. TSSO001E)

    // Check prefix (4 alphabetic characters)
    for (int i = 0; i < 4; i++) {
        if (msg_code[i] < 'A' || msg_code[i] > 'Z') return false;
    }
    // Check code sequence (3 digits)
    for (int i = 4; i < 7; i++) {
        if (msg_code[i] < '0' || msg_code[i] > '9') return false;
    }
    // Check status suffix (1 alphabetic character: I, E, W, etc.)
    if (msg_code[7] < 'A' || msg_code[7] > 'Z') return false;

    return true; // Valid MVS message code layout
}

// 2. BLACK XPLSM Gating: Dynamic message lookup and alarm trigger
static bool black_xplsm_lookup_message(
    const char *msg_lib_path,
    const char *msg_code,
    ispf_message_t *out,
    bool *alarm_sounded_out
) {
    if (!msg_lib_path || !msg_code || !out) return false;
    *alarm_sounded_out = false;

    FILE *f = fopen(msg_lib_path, "r");
    if (!f) return false;

    char line[256];
    bool found = false;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, msg_code, 8) == 0) {
            snprintf(out->code, sizeof(out->code), "%s", msg_code);
            char alarm_str[8] = {0};
            
            int scanned = sscanf(line + 8, " '%63[^']' .ALARM=%7s", out->short_text, alarm_str);
            if (scanned >= 1) {
                out->alarm = (strcmp(alarm_str, "YES") == 0);
                *alarm_sounded_out = out->alarm;
            }

            if (fgets(line, sizeof(line), f)) {
                sscanf(line, "'%127[^']'", out->long_text);
            }
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TSOHELP SYSTEM MESSAGE FINDER VALIDATION SUITE\n");
    printf("=============================================================\n");

    const char *msg_lib = "ispf_libraries/ispmlib/errors.ispmlib";

    // Test 1: RED rail static format checks
    printf("[TEST] Verifying message code layout parameters...\n");
    assert(red_analyzer_validate_msg_format("TSSO001E") == true);
    assert(red_analyzer_validate_msg_format("TSSO003I") == true);
    
    // Rejected formats: invalid length or characters
    assert(red_analyzer_validate_msg_format("TSSO01") == false);
    assert(red_analyzer_validate_msg_format("TSSO0011") == false);
    assert(red_analyzer_validate_msg_format("tsso001e") == false);
    printf("   ✓ RED rail successfully gated static message code formats.\n");

    // Test 2: BLACK rail message queries
    printf("[TEST] Querying error logs and triggering dynamic alarms...\n");
    ispf_message_t msg;
    bool alarm_triggered = false;

    // Successful query for TSSO001E (Warning, Alarm expected)
    assert(black_xplsm_lookup_message(msg_lib, "TSSO001E", &msg, &alarm_triggered) == true);
    assert(strcmp(msg.short_text, "TSSO Opcode Violation") == 0);
    assert(alarm_triggered == true); // Alarm sounded

    // Successful query for TSSO003I (Informational, No Alarm)
    alarm_triggered = false;
    assert(black_xplsm_lookup_message(msg_lib, "TSSO003I", &msg, &alarm_triggered) == true);
    assert(strcmp(msg.short_text, "TSSO Command Executed") == 0);
    assert(alarm_triggered == false); // No alarm

    // Unallocated message code query
    assert(black_xplsm_lookup_message(msg_lib, "TSSO999E", &msg, &alarm_triggered) == false);

    printf("   ✓ Message lookup and dynamic alarm routing verified.\n");

    printf("=============================================================\n");
    printf("ALL TSOHELP DIAGNOSTICS TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
