#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_lore_interop.h"
#include "inc/hathitrust_teddy_bear_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

static void* thread_registration_helper(void *arg) {
    (void)arg;
    // Register from helper thread
    bool ok = hathitrust_lore_interop_register(
        "lore.test001", "Auncient Thread Test", "1950", "/tmp/thread.txt"
    );
    assert(ok);
    return NULL;
}

static void test_lore_registry(void) {
    printf("[Test] Running Lore Interop Registry tests...\n");
    
    hathitrust_lore_interop_clear();
    
    pthread_t thread;
    assert(pthread_create(&thread, NULL, thread_registration_helper, NULL) == 0);
    assert(pthread_join(thread, NULL) == 0);
    
    // Lookup from main thread (thread-safety verification)
    HtrcLoreRecord *rec = hathitrust_lore_interop_lookup("lore.test001");
    assert(rec != NULL);
    assert(strcmp(rec->title, "Auncient Thread Test") == 0);
    assert(strcmp(rec->pub_date, "1950") == 0);
    
    hathitrust_lore_interop_clear();
    printf("[Test] Lore Interop Registry tests passed.\n");
}

static void test_ssa_translation(void) {
    printf("[Test] Running TeddyBear DNA-to-SSA Bridge tests...\n");
    
    unsigned char seed[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    char *ssn = auncient_bridge_dna_to_ssa(seed, 4);
    assert(ssn != NULL);
    printf("[Test] Generated Teddy Bear SSN: %s\n", ssn);
    
    // Assert correct SSN format: xxx-xx-xxxx (11 characters)
    assert(strlen(ssn) == 11);
    assert(ssn[3] == '-');
    assert(ssn[6] == '-');
    
    // Assert deterministic output
    char *ssn2 = auncient_bridge_dna_to_ssa(seed, 4);
    assert(strcmp(ssn, ssn2) == 0);
    
    free(ssn);
    free(ssn2);
    printf("[Test] TeddyBear DNA-to-SSA Bridge tests passed.\n");
}

static void test_hogan_endowment(void) {
    printf("[Test] Running Hogan Account Endowment tests...\n");
    
    HoganAccount *acc = hathitrust_open_hogan_account("123-45-6789", "Hogan Bear");
    assert(acc != NULL);
    
    assert(strcmp(acc->ssn, "123-45-6789") == 0);
    assert(strcmp(acc->bear_name, "Hogan Bear") == 0);
    // Assert default endowment of 1,000,000 Saat
    assert(acc->saat_balance == 1000000.0);
    
    hathitrust_free_hogan_account(acc);
    printf("[Test] Hogan Account Endowment tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust Auncient Interop & TeddyBear Bridge tests...\n");
    test_lore_registry();
    test_ssa_translation();
    test_hogan_endowment();
    printf("[Test] All Interop & TeddyBear tests completed successfully.\n");
    return 0;
}
