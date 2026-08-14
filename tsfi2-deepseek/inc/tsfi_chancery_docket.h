#ifndef TSFI_CHANCERY_DOCKET_H
#define TSFI_CHANCERY_DOCKET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CHANCERY_MAX_DOCKET_ITEMS 64

// Formal Ruling Taxonomy for Docketed Assertions
typedef enum {
    DOCKET_RULING_PENDING = 0,             // Unexamined / under investigation
    DOCKET_RULING_AUTHENTIC_STREAM = 1,    // Formally proven true (Axiom / Law)
    DOCKET_RULING_UNAUTHORIZED_BLOCK = 2,  // Verified perimeter barrier / access denial
    DOCKET_RULING_CONTRADICTION = 3,       // Formally disproven (Violates 3-term recurrence)
    DOCKET_RULING_QUARANTINED = 4          // Speculative noise isolated from ledger
} ChanceryDocketRuling;

typedef struct {
    uint32_t docket_id;
    uint32_t parent_docket_id;             // 0 if root; non-zero if subordinate to parent ruling
    char assertion_summary[128];
    char target_prover_path[128];          // Path to matching std .algol61 prover
    uint32_t zmm_opcode;                   // ZMM hardware instruction mapping
    int64_t r15_register_result;           // Mainframe return code on R15
    ChanceryDocketRuling ruling;
    uint64_t docket_timestamp;
    uint64_t claim_dna_hash;
} ChanceryDocketEntry;

typedef struct {
    ChanceryDocketEntry entries[CHANCERY_MAX_DOCKET_ITEMS];
    uint32_t entry_count;
    uint64_t total_resolved_count;
    uint64_t total_quarantined_count;
    uint64_t docket_merkle_root;
} ChanceryDocketState;

// Initialize Chancery Docket & Dispatch System
void tsfi_chancery_docket_init(ChanceryDocketState *state);

// File an assertion onto the Chancery Docket
uint32_t tsfi_chancery_docket_file(
    ChanceryDocketState *state,
    const char *summary,
    const char *prover_path,
    uint64_t timestamp
);

// File a subordinate assertion onto the Chancery Docket tied to a parent ruling
uint32_t tsfi_chancery_docket_file_subordinate(
    ChanceryDocketState *state,
    uint32_t parent_docket_id,
    const char *summary,
    const char *prover_path,
    uint64_t timestamp
);

// Execute Zorse LLM Traversal & COBOL ZMM Prover over R15 register
bool tsfi_chancery_docket_resolve_zmm_r15(
    ChanceryDocketState *state,
    uint32_t docket_id,
    int64_t r15_input_code,
    ChanceryDocketRuling ruling
);

// Close a subordinate docket entry by direct reference and inheritance of parent ruling
bool tsfi_chancery_docket_resolve_subordinate(
    ChanceryDocketState *state,
    uint32_t subordinate_docket_id,
    uint32_t parent_docket_id
);

// Compute Merkle proof and emit Chancery Docket Audit Report
uint64_t tsfi_chancery_docket_audit(
    ChanceryDocketState *state,
    char *out_report,
    size_t max_len
);

#endif /* TSFI_CHANCERY_DOCKET_H */
