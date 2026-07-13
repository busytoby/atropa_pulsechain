#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Yul Opcode definition
typedef enum {
    YUL_OP_GAS_TRANSFER = 0x20,
    YUL_OP_QUERY_READ = 0x30
} YulOpcode;

// ACL Level definitions
typedef enum {
    ACL_ANONYMOUS = 0,
    ACL_USER = 1,
    ACL_ADMIN = 2
} AclLevel;

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Represents our 2-Channel data payload on the coaxial bus
typedef struct {
    PPN      ppn;
    uint64_t recipient;
    uint64_t value;
    YulOpcode d_channel;
} TwoChannelTx;

// Global state variables simulating database
uint64_t g_balances[100]; 
AclLevel g_acl_registry[100];
uint32_t g_consecutive_failures[100]; // Telemetry tracker: failed transactions per PPN slot

const uint64_t UNIVERSAL_GAS_FEE = 15;
const uint32_t MAX_ALLOWED_FAILURES = 3;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

uint64_t get_ppn_balance(PPN ppn) {
    return g_balances[get_ppn_slot(ppn)];
}

void set_ppn_balance(PPN ppn, uint64_t balance) {
    g_balances[get_ppn_slot(ppn)] = balance;
}

AclLevel get_acl_level(PPN ppn) {
    return g_acl_registry[get_ppn_slot(ppn)];
}

void set_acl_level(PPN ppn, AclLevel level) {
    g_acl_registry[get_ppn_slot(ppn)] = level;
}

void init_system(void) {
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_acl_registry, 0, sizeof(g_acl_registry));
    memset(g_consecutive_failures, 0, sizeof(g_consecutive_failures));
    
    // Seed PPN [10, 5] as User with 100 Gas
    PPN bob = {10, 5};
    set_ppn_balance(bob, 100);
    set_acl_level(bob, ACL_USER);
}

// Coaxial execution pipeline enforcing telemetry-driven rate-limiting
bool process_coaxial_transaction(TwoChannelTx *tx, const char **out_err) {
    PPN caller = tx->ppn;
    uint32_t slot = get_ppn_slot(caller);
    
    // 1. Telemetry-driven ACL rate limiting check
    if (g_consecutive_failures[slot] >= MAX_ALLOWED_FAILURES) {
        // Automatically downgrade permissions to Anonymous
        if (g_acl_registry[slot] > ACL_ANONYMOUS) {
            printf("   [Telemetry Limiter] Downgrading PPN [%u,%u] to ACL_ANONYMOUS due to %u consecutive failures.\n",
                   caller.project, caller.programmer, g_consecutive_failures[slot]);
            g_acl_registry[slot] = ACL_ANONYMOUS;
        }
    }
    
    AclLevel acl = get_acl_level(caller);
    uint64_t balance = get_ppn_balance(caller);
    
    // Check Gas
    if (balance < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        g_consecutive_failures[slot]++; // Log failure
        return false;
    }
    
    // Check ACL authorization
    if (tx->d_channel == YUL_OP_GAS_TRANSFER && acl < ACL_USER) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED";
        g_consecutive_failures[slot]++; // Log failure
        return false;
    }
    
    // Execute opcode
    set_ppn_balance(caller, balance - UNIVERSAL_GAS_FEE);
    
    if (tx->d_channel == YUL_OP_QUERY_READ) {
        printf("   [READ] PPN [%u,%u] read query executed (Charged: %lu Gas)\n",
               caller.project, caller.programmer, UNIVERSAL_GAS_FEE);
    } else if (tx->d_channel == YUL_OP_GAS_TRANSFER) {
        uint64_t usable_bal = balance - UNIVERSAL_GAS_FEE;
        if (usable_bal < tx->value) {
            *out_err = "REVERT: INSUFFICIENT_BALANCE_FOR_TRANSFER";
            g_consecutive_failures[slot]++; // Log failure
            return false;
        }
        PPN recipient = { .project = (uint16_t)(tx->recipient >> 16), .programmer = (uint16_t)(tx->recipient & 0xFFFF) };
        set_ppn_balance(caller, usable_bal - tx->value);
        set_ppn_balance(recipient, get_ppn_balance(recipient) + tx->value);
        printf("   [TRANSFER] Sent %lu to PPN [%u,%u] (Charged: %lu Gas)\n",
               tx->value, recipient.project, recipient.programmer, UNIVERSAL_GAS_FEE);
    } else {
        *out_err = "REVERT: INVALID_OPCODE";
        g_consecutive_failures[slot]++; // Log failure
        return false;
    }
    
    // On success, reset the consecutive failure count
    g_consecutive_failures[slot] = 0;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: TELEMETRY-DRIVEN RATE LIMITER TESTS\n");
    printf("=============================================================\n");

    init_system();
    const char *err = NULL;

    PPN job_bob = { .project = 10, .programmer = 5 };
    uint64_t target_encoded = (10LL << 16) | 6; // Bob sends to programmer 6

    // --- TEST 1: Accumulating failures ---
    printf("1. Simulating 3 consecutive transaction failures...\n");
    
    // Failure 1: Transfer with insufficient balance (value is 1000, balance is 100)
    TwoChannelTx tx_fail1 = { .ppn = job_bob, .recipient = target_encoded, .value = 1000, .d_channel = YUL_OP_GAS_TRANSFER };
    bool res = process_coaxial_transaction(&tx_fail1, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: INSUFFICIENT_BALANCE_FOR_TRANSFER") == 0);
    assert(g_consecutive_failures[get_ppn_slot(job_bob)] == 1);
    printf("   ✓ Failure 1 logged.\n");

    // Failure 2: Transfer with insufficient balance again
    res = process_coaxial_transaction(&tx_fail1, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: INSUFFICIENT_BALANCE_FOR_TRANSFER") == 0);
    assert(g_consecutive_failures[get_ppn_slot(job_bob)] == 2);
    printf("   ✓ Failure 2 logged.\n");

    // Failure 3: Transfer with insufficient balance again
    res = process_coaxial_transaction(&tx_fail1, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: INSUFFICIENT_BALANCE_FOR_TRANSFER") == 0);
    assert(g_consecutive_failures[get_ppn_slot(job_bob)] == 3);
    printf("   ✓ Failure 3 logged.\n\n");

    // --- TEST 2: ACL Downgrade Triggered ---
    printf("2. Submitting 4th transaction. Expecting dynamic ACL downgrade to ANONYMOUS...\n");
    // Verify Bob is still user prior to 4th processing check
    assert(get_acl_level(job_bob) == ACL_USER);
    
    // Submit transfer (requires ACL_USER)
    TwoChannelTx tx_fail2 = { .ppn = job_bob, .recipient = target_encoded, .value = 10, .d_channel = YUL_OP_GAS_TRANSFER };
    res = process_coaxial_transaction(&tx_fail2, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: ACL_PERMISSION_DENIED") == 0);
    assert(get_acl_level(job_bob) == ACL_ANONYMOUS);
    printf("   ✓ ACL successfully downgraded. Bob is now: ACL_ANONYMOUS\n\n");

    // --- TEST 3: Recovery via Success ---
    printf("3. Testing recovery. Anonymous read query is still permitted...\n");
    TwoChannelTx tx_read = { .ppn = job_bob, .recipient = 0, .value = 0, .d_channel = YUL_OP_QUERY_READ };
    res = process_coaxial_transaction(&tx_read, &err);
    assert(res == true);
    assert(g_consecutive_failures[get_ppn_slot(job_bob)] == 0); // Reset failures on success
    printf("   ✓ Read query succeeded. Failure counter reset to: %u\n\n", g_consecutive_failures[get_ppn_slot(job_bob)]);

    printf("=============================================================\n");
    printf("AUNCIENT TELEMETRY RATE LIMITER TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
