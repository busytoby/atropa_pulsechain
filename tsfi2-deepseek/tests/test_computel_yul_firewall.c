#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/*
 * Auncient Computel CPU-Level Yul Firewall Simulator
 * 
 * Verifies access boundary controls on DTMF dialed switching actions
 * based on the caller's registered Soul address.
 */

#define ADMIN_SOUL 0x05A10A /* Mock Authorized Admin Soul */
#define GUEST_SOUL 0x999999

typedef struct {
    uint32_t caller_soul;
    bool is_firewall_tripped;
    char last_blocked_command[16];
} CpuFirewallState;

/* Validates dialing commands at the virtual CPU boundary */
bool validate_dial_sequence(CpuFirewallState *fw, const char *dial_str) {
    if (!fw || !dial_str) return false;

    // Rule 1: Master Reset (*0) requires Admin Soul authority
    if (strcmp(dial_str, "*0") == 0) {
        if (fw->caller_soul != ADMIN_SOUL) {
            fw->is_firewall_tripped = true;
            strncpy(fw->last_blocked_command, dial_str, 15);
            return false; // BLOCKED
        }
    }

    // Rule 2: Access to Private Carrier Trunk 805 is whitelisted
    if (strcmp(dial_str, "805") == 0) {
        if (fw->caller_soul != ADMIN_SOUL) {
            fw->is_firewall_tripped = true;
            strncpy(fw->last_blocked_command, dial_str, 15);
            return false; // BLOCKED
        }
    }

    return true; // ALLOWED
}

int main(void) {
    printf("[TEST] Initializing Auncient Computel CPU-Level Yul Firewall test suite...\n");

    CpuFirewallState fw;
    
    // Test Case 1: Guest Soul attempts Master Reset (*0) -> Should fail
    fw.caller_soul = GUEST_SOUL;
    fw.is_firewall_tripped = false;
    memset(fw.last_blocked_command, 0, 16);

    bool ok = validate_dial_sequence(&fw, "*0");
    assert(ok == false);
    assert(fw.is_firewall_tripped == true);
    assert(strcmp(fw.last_blocked_command, "*0") == 0);
    printf("[TEST] Guest Master Reset blocked successfully by CPU boundary.\n");

    // Test Case 2: Admin Soul attempts Master Reset (*0) -> Should pass
    fw.caller_soul = ADMIN_SOUL;
    fw.is_firewall_tripped = false;
    memset(fw.last_blocked_command, 0, 16);

    ok = validate_dial_sequence(&fw, "*0");
    assert(ok == true);
    assert(fw.is_firewall_tripped == false);
    printf("[TEST] Admin Master Reset authorized successfully.\n");

    // Test Case 3: Guest attempts accessing Private Trunk 805 -> Should fail
    fw.caller_soul = GUEST_SOUL;
    fw.is_firewall_tripped = false;
    
    ok = validate_dial_sequence(&fw, "805");
    assert(ok == false);
    assert(fw.is_firewall_tripped == true);
    printf("[TEST] Guest access to Private Trunk 805 blocked successfully.\n");

    printf("[SUCCESS] All CPU-Level Yul Firewall boundary tests passed successfully.\n");
    return 0;
}
