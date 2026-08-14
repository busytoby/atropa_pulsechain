#ifndef TSFI_WAUGH_CHANCERY_H
#define TSFI_WAUGH_CHANCERY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define WAUGH_MAX_CABLES 32
#define WAUGH_MAX_TREATIES 16

// Diplomatic & Press Transmission Status (derived from 'Scoop' and 'Black Mischief')
typedef enum {
    WAUGH_DISPATCH_OFFICIAL_COMMUNIQUE = 0, // Authorized royal state dispatch
    WAUGH_DISPATCH_PRESS_CABLE,             // Special correspondent / press transmission
    WAUGH_DISPATCH_IMPERIAL_DECREE          // Unconditional sovereign decree of modernization
} WaughDispatchType;

typedef struct {
    uint32_t cable_id;
    WaughDispatchType type;
    char origin[32];
    char destination[32];
    char content_headline[128];
    uint64_t transmission_epoch;
    bool verified_by_censor;
} WaughDiplomaticCable;

typedef struct {
    uint32_t treaty_id;
    char party_name[64];
    uint64_t customs_tariff_rate_bps;       // Import tariff rate in basis points
    uint64_t sovereign_recognition_saat;    // Royal tribute / treaty recognition deposit
    bool ratified;
} WaughTreatyProtocol;

typedef struct {
    WaughDiplomaticCable cables[WAUGH_MAX_CABLES];
    uint32_t cable_count;
    WaughTreatyProtocol treaties[WAUGH_MAX_TREATIES];
    uint32_t treaty_count;
    uint64_t total_customs_revenue_saat;
    uint64_t chancery_merkle_proof;
} WaughChanceryState;

// Initialize the Waugh Diplomatic Chancery & Protocol Engine
void tsfi_waugh_chancery_init(WaughChanceryState *state);

// Dispatch a diplomatic cable or press transmission
bool tsfi_waugh_dispatch_cable(
    WaughChanceryState *state,
    WaughDispatchType type,
    const char *origin,
    const char *destination,
    const char *headline,
    uint64_t epoch
);

// Ratify a foreign treaty protocol and collect the sovereign customs deposit
bool tsfi_waugh_ratify_treaty(
    WaughChanceryState *state,
    const char *party_name,
    uint64_t tariff_bps,
    uint64_t tribute_saat
);

// Audit the chancery dispatch logs and compute cryptographic state proof
uint64_t tsfi_waugh_chancery_audit(
    WaughChanceryState *state,
    char *out_report,
    size_t max_len
);

#endif /* TSFI_WAUGH_CHANCERY_H */
