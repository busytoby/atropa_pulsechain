#include "tsfi_taste_cache.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(int argc, char **argv) {
    const char *path = "assets/taste_cache.bin";
    uint64_t hash = 0xDEADBEEF414EULL;

    if (argc > 1 && strcmp(argv[1], "load") == 0) {
        tsfi_taste_cache_init(path);
        const TsfiTasteAtom *atom = tsfi_taste_cache_query(hash);
        if (!atom) {
            fprintf(stderr, "[FAIL] Taste Atom not found!\n");
            return 1;
        }
        printf("[TEST] Restored Taste Atom: Secret[0]=%.3f, Secret[15]=%.3f\n", 
               atom->secrets[0], atom->secrets[15]);
        assert(atom->secrets[0] == 414.0f);
        assert(atom->secrets[15] == 999.0f);
        tsfi_taste_cache_teardown();
        printf("[SUCCESS] Subjective Realism Persistence Verified (64-byte Atom).\n");
    } else {
        tsfi_taste_cache_init(path);
        TsfiTasteAtom atom = {0};
        atom.secrets[0] = 414.0f;
        atom.secrets[15] = 999.0f;
        if (!tsfi_taste_cache_persist(hash, &atom)) {
            fprintf(stderr, "[FAIL] Persist failed!\n");
            return 1;
        }
        tsfi_taste_cache_teardown();
        printf("[TEST] Persistent Taste Atom created.\n");
    }
    return 0;
}
