#ifndef HATHITRUST_TEDDY_BEAR_BRIDGE_H
#define HATHITRUST_TEDDY_BEAR_BRIDGE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char *ssn;
    char *bear_name;
    double saat_balance;
} HoganAccount;

/*
 * Translates a newborn teddy bear seed (DNA seed) deterministically to a valid SSN structure.
 * Returns:
 *   Deterministic SSN string (xxx-xx-xxxx format), or NULL on error.
 *   Caller is responsible for freeing the returned string.
 */
char* auncient_bridge_dna_to_ssa(const unsigned char *dna_seed, size_t seed_len);

/*
 * Opens a new Hogan Account for the newborn teddy bear participant.
 * Initializes with the default endowment of 1,000,000 Saat.
 * Returns:
 *   Pointer to HoganAccount, or NULL on error.
 *   Caller is responsible for freeing the returned account.
 */
HoganAccount* hathitrust_open_hogan_account(const char *ssn, const char *bear_name);

/*
 * Frees a HoganAccount structure.
 */
void hathitrust_free_hogan_account(HoganAccount *account);

#endif /* HATHITRUST_TEDDY_BEAR_BRIDGE_H */
