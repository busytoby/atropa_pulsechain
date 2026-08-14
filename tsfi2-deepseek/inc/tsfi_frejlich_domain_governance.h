#ifndef TSFI_FREJLICH_DOMAIN_GOVERNANCE_H
#define TSFI_FREJLICH_DOMAIN_GOVERNANCE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define FREJLICH_MAX_SECTORS 32
#define FREJLICH_MAX_EMISSARIES 64
#define FREJLICH_MAX_LOGISTICS_ROUTES 128

// Administrative Sector Classification derived from Frejlich's 1863 Territorial Model
typedef enum {
    FREJLICH_SECTOR_CIVIL_CADRE = 0,    // Civil administration & public treasury
    FREJLICH_SECTOR_LOGISTICS_SUPPLY,   // Weapons, provisions, & cross-border conduits
    FREJLICH_SECTOR_TACTICAL_COMMAND,   // Field operations & decentralized detachments
    FREJLICH_SECTOR_COMMUNICATIONS,     // Secret courier lines & cipher dispatch
    FREJLICH_SECTOR_ARCHIVAL_AUDIT      // Continuous historical ledger & integrity proofs
} FrejlichSectorType;

typedef struct {
    uint32_t sector_id;
    FrejlichSectorType type;
    char sector_name[64];
    uint64_t endowment_saat;          // Allocated Saat endowment (Rule 16 standard)
    uint32_t active_emissaries;
    uint32_t operational_status;      // 1=Active, 2=Mobilized, 3=Autonomous
    uint64_t last_audit_timestamp;
    uint64_t dna_hash;                // Cryptographic FNV-1a state proof
} FrejlichDomainSector;

typedef struct {
    uint32_t route_id;
    uint32_t origin_sector_id;
    uint32_t dest_sector_id;
    uint64_t bandwidth_bps;
    uint64_t security_weight;
    bool active;
} FrejlichLogisticsConduit;

typedef struct {
    FrejlichDomainSector sectors[FREJLICH_MAX_SECTORS];
    uint32_t sector_count;
    FrejlichLogisticsConduit conduits[FREJLICH_MAX_LOGISTICS_ROUTES];
    uint32_t conduit_count;
    uint64_t treasury_total_saat;
    uint32_t governance_epoch;
    uint64_t realm_merkle_root;       // Root cryptographic hash of entire realm state
} FrejlichRealmGovernanceState;

// Initialize Realm Domain Governance based on Frejlich's archival mechanics
void tsfi_frejlich_governance_init(FrejlichRealmGovernanceState *state);

// Register a new territorial domain sector
bool tsfi_frejlich_register_sector(
    FrejlichRealmGovernanceState *state,
    FrejlichSectorType type,
    const char *name,
    uint64_t initial_endowment
);

// Establish secure communications/logistics conduit between sectors
bool tsfi_frejlich_establish_conduit(
    FrejlichRealmGovernanceState *state,
    uint32_t origin_id,
    uint32_t dest_id,
    uint64_t bandwidth
);

// Compute cryptographic Merkle root across all sector states
uint64_t tsfi_frejlich_compute_merkle_root(FrejlichRealmGovernanceState *state);

// Execute an audit pass reconciling all domain budgets and logistics
bool tsfi_frejlich_reconcile_realm(
    FrejlichRealmGovernanceState *state,
    uint64_t current_timestamp,
    char *out_report,
    size_t report_max_len
);

#endif /* TSFI_FREJLICH_DOMAIN_GOVERNANCE_H */
