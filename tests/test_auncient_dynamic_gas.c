#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define INFINITE_GAS UINT64_MAX
#define UNKNOWN_PEER_INITIAL_GAS 5000ULL

typedef enum {
    SESSION_SYSTEM,
    SESSION_PEER_TRUSTED,
    SESSION_PEER_UNKNOWN
} SessionType;

typedef struct {
    SessionType type;
    uint64_t gas_limit;
    uint64_t gas_consumed;
    bool rate_limited;
    bool flagged_for_review;
} VMContext;

// Initialize context-specific gas properties based on session type
void vm_context_init(VMContext *ctx, SessionType type) {
    ctx->type = type;
    ctx->gas_consumed = 0;
    ctx->rate_limited = false;
    ctx->flagged_for_review = false;
    
    if (type == SESSION_SYSTEM) {
        ctx->gas_limit = INFINITE_GAS;
    } else if (type == SESSION_PEER_UNKNOWN) {
        ctx->gas_limit = UNKNOWN_PEER_INITIAL_GAS;
    } else {
        ctx->gas_limit = 1000000ULL; // Trusted peer default gas limit
    }
}

// Consume gas per instruction cycle
bool vm_consume_gas(VMContext *ctx, uint64_t cost) {
    if (ctx->gas_limit == INFINITE_GAS) {
        ctx->gas_consumed += cost;
        return true; // System processes never run out of gas
    }

    if (ctx->gas_consumed + cost > ctx->gas_limit) {
        ctx->rate_limited = true;
        if (ctx->type == SESSION_PEER_UNKNOWN) {
            ctx->flagged_for_review = true; // Flag unknown peers for admin review
        }
        return false; // Out of gas
    }

    ctx->gas_consumed += cost;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: DYNAMIC CONTEXT-SPECIFIC GAS CONTROLLER\n");
    printf("=============================================================\n");

    // 1. Test System Session (Infinite Gas)
    VMContext sys_ctx;
    vm_context_init(&sys_ctx, SESSION_SYSTEM);
    printf("1. Verifying System session gas consumption...\n");
    
    bool sys_ok = true;
    for (int i = 0; i < 100000; i++) {
        sys_ok = vm_consume_gas(&sys_ctx, 1000ULL);
        if (!sys_ok) break;
    }
    assert(sys_ok == true);
    assert(sys_ctx.rate_limited == false);
    printf("   ✓ System session executed successfully with infinite gas usage.\n");

    // 2. Test Unknown Peer Session (Very Low Initial Gas)
    VMContext peer_ctx;
    vm_context_init(&peer_ctx, SESSION_PEER_UNKNOWN);
    printf("2. Verifying Unknown Peer session rate-limiting...\n");
    
    // Consume standard cost
    bool peer_ok1 = vm_consume_gas(&peer_ctx, 3000ULL);
    assert(peer_ok1 == true);
    
    // Attempt block that exceeds initial limit
    bool peer_ok2 = vm_consume_gas(&peer_ctx, 3000ULL); // Total 6000 > 5000
    assert(peer_ok2 == false);
    assert(peer_ctx.rate_limited == true);
    assert(peer_ctx.flagged_for_review == true);
    printf("   ✓ Unknown peer successfully rate-limited and flagged for admin review.\n");

    printf("\n=============================================================\n");
    printf("AUNCIENT CONTEXT-SPECIFIC GAS TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
