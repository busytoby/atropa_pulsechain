#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// Audit Log Entry types
typedef enum {
    LOG_CONDUCTION = 0,
    LOG_RED_VIOLATION = 1,
    LOG_BLACK_TRAP = 2
} log_type_t;

// Session Audit Log Entry Structure
typedef struct {
    uint32_t timestamp_seq;
    log_type_t type;
    char details[64];
    uint32_t code_info;
} audit_entry_t;

// Secure Session Logger circular buffer
#define MAX_AUDIT_LOGS 8
static audit_entry_t audit_trail[MAX_AUDIT_LOGS];
static uint32_t audit_count = 0;
static uint32_t sequence_counter = 1000;

// 1. Session Logger append function
static void log_session_event(log_type_t type, const char *details, uint32_t code_info) {
    uint32_t index = audit_count % MAX_AUDIT_LOGS;
    
    audit_trail[index].timestamp_seq = sequence_counter++;
    audit_trail[index].type = type;
    snprintf(audit_trail[index].details, sizeof(audit_trail[index].details), "%s", details);
    audit_trail[index].code_info = code_info;
    
    audit_count++;
}

// 2. Query auditor log count
static uint32_t get_audit_trail_count(void) {
    return audit_count > MAX_AUDIT_LOGS ? MAX_AUDIT_LOGS : audit_count;
}

// 3. Query audit entry by offset
static bool get_audit_entry(uint32_t offset, audit_entry_t *out) {
    uint32_t total = get_audit_trail_count();
    if (offset >= total) return false;

    uint32_t index;
    if (audit_count > MAX_AUDIT_LOGS) {
        index = (audit_count - MAX_AUDIT_LOGS + offset) % MAX_AUDIT_LOGS;
    } else {
        index = offset;
    }

    *out = audit_trail[index];
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TSO SESSLOG SESSION AUDITOR VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: Log initial conduction events
    printf("[TEST] Logging successful VM conduction states...\n");
    log_session_event(LOG_CONDUCTION, "BOOTSTRAP COMPLETED SUCCESSFULLY", 0);
    log_session_event(LOG_CONDUCTION, "VTOC ACCESS APPROVED SYS1.ISPSLIB", 10);
    
    assert(get_audit_trail_count() == 2);
    
    audit_entry_t entry;
    assert(get_audit_entry(0, &entry) == true);
    assert(entry.type == LOG_CONDUCTION);
    assert(strcmp(entry.details, "BOOTSTRAP COMPLETED SUCCESSFULLY") == 0);
    printf("   ✓ Conduction states successfully logged in audit trail.\n");

    // Test 2: Log security violations and interrupts
    printf("[TEST] Logging RED rail security violations and BLACK rail traps...\n");
    log_session_event(LOG_RED_VIOLATION, "SYS1.LINKLIB RESTRICTED WRITE BLOCKED", 1);
    log_session_event(LOG_BLACK_TRAP, "VTOC SPACE EXHAUSTED", 25);
    
    assert(get_audit_trail_count() == 4);
    assert(get_audit_entry(3, &entry) == true);
    assert(entry.type == LOG_BLACK_TRAP);
    assert(entry.code_info == 25);
    printf("   ✓ Security violations and system traps logged correctly.\n");

    // Test 3: Verify circular buffer overwrite limits
    printf("[TEST] Verifying log overrides inside circular buffer bounds...\n");
    // Append 6 more entries to trigger wrap-around (total 10 logs > limit 8)
    for (int i = 0; i < 6; i++) {
        log_session_event(LOG_CONDUCTION, "CONDUCTION STEP ACTIVE", i);
    }
    
    assert(get_audit_trail_count() == MAX_AUDIT_LOGS);
    
    // The earliest entry (offset 0) should now be the third entry logged (offset index 2)
    assert(get_audit_entry(0, &entry) == true);
    assert(strcmp(entry.details, "SYS1.LINKLIB RESTRICTED WRITE BLOCKED") == 0);
    printf("   ✓ Session Logger successfully handled circular logs overflow safely.\n");

    printf("=============================================================\n");
    printf("ALL SESSLOG AUDIT LOGGER TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
