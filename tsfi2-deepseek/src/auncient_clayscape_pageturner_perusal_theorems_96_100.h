#ifndef AUNCIENT_CLAYSCAPE_PAGETURNER_PERUSAL_THEOREMS_96_100_H
#define AUNCIENT_CLAYSCAPE_PAGETURNER_PERUSAL_THEOREMS_96_100_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define PAGETURNER_TPA_BASE 0x0100
#define MAX_PAGES_IN_VIEW 32

typedef struct {
    uint32_t page_index;
    char     domain_file_path[64];
    uint32_t theorem_id_start;
    uint32_t theorem_id_end;
    float    haptic_curl_resistance_n;
    bool     is_rib_rendered;
    uint32_t page_witness_hash;
} PageTurnerPage;

typedef struct {
    uint32_t current_page_index;
    uint32_t total_pages;
    PageTurnerPage pages[MAX_PAGES_IN_VIEW];
    uint32_t grand_century_master_witness;
    bool pagination_bijective_verified;
    bool haptic_curl_coupling_verified;
    bool renderman_page_rib_verified;
    bool cross_vm_read_isolation_verified;
    bool grand_100theorems_closure_verified;
    uint32_t rule18_parity_checksum;
} ClayscapePageTurnerState;

void auncient_pageturner_perusal_init(ClayscapePageTurnerState *state);
bool auncient_pageturner_flip_page(ClayscapePageTurnerState *state, uint32_t target_page);
bool auncient_pageturner_verify_theorems_96_100(ClayscapePageTurnerState *state);
uint32_t auncient_pageturner_compute_rule18(const ClayscapePageTurnerState *state);

#endif /* AUNCIENT_CLAYSCAPE_PAGETURNER_PERUSAL_THEOREMS_96_100_H */
