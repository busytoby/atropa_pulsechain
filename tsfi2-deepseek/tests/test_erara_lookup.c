#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[TEST E-RARA LOOKUP] Starting e-rara.ch Rare Book Title Lookup Unit Test Suite...\n");

    // Clean initial test state
    unlink("erara_catalog.dat.bin");
    unlink("erara_catalog.wal.dat.bin");

    // 1. Register 16th-century rare book (De humani corporis fabrica - Vesalius)
    assert(tsfi_erara_register_title(
        "10.3931/e-rara-1543",
        "De humani corporis fabrica libri septem",
        "Andreas Vesalius",
        1543,
        660,
        "https://www.e-rara.ch/i3f/v20/1543/manifest"
    ) == 0);

    // Record initial timestamp
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t initial_ts = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)(ts.tv_nsec / 1000);
    usleep(10000); // 10ms wait

    // 2. Register second rare book (Sidereus Nuncius - Galileo Galilei)
    assert(tsfi_erara_register_title(
        "10.3931/e-rara-1610",
        "Sidereus Nuncius",
        "Galileo Galilei",
        1610,
        60,
        "https://www.e-rara.ch/i3f/v20/1610/manifest"
    ) == 0);

    // 3. Test Lookup by DOI
    vsen_erara_title_record_t book;
    assert(tsfi_erara_lookup_title("10.3931/e-rara-1543", &book) == 0);
    assert(strcmp(book.author, "Andreas Vesalius") == 0);
    assert(book.pub_year == 1543);
    assert(book.total_pages == 660);

    // 4. Test Lookup by Title Substring
    assert(tsfi_erara_lookup_title("Sidereus", &book) == 0);
    assert(strcmp(book.author, "Galileo Galilei") == 0);
    assert(book.pub_year == 1610);

    // 5. Test Point-In-Time Restore (PITR) Historical Lookup as of initial_ts
    assert(tsfi_erara_lookup_title_as_of("Sidereus", initial_ts, &book) != 0); // Not registered yet at initial_ts
    assert(tsfi_erara_lookup_title_as_of("De humani corporis", initial_ts, &book) == 0); // Registered at initial_ts
    assert(strcmp(book.author, "Andreas Vesalius") == 0);

    // 6. Audit Cryptographic DNA Hash Chain Integrity
    assert(tsfi_vsen_audit_chain_verify("erara_catalog.dat.bin") == 0);

    printf("[TEST E-RARA LOOKUP] All e-rara.ch C title lookup unit tests passed successfully!\n");
    return 0;
}
