#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagemask.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEMASK VALIDATION TESTS ===\n");

    TSFiStageMask mask;
    tsfi_stagemask_init(&mask);

    // With mask disabled, any path should be allowed
    assert(tsfi_stagemask_is_path_allowed(&mask, "/World/Camera") == true);

    // Add prefix '/World/Props'
    assert(tsfi_stagemask_add_prefix(&mask, "/World/Props"));
    assert(mask.is_enabled == true);

    // Path in '/World/Props' should be allowed
    assert(tsfi_stagemask_is_path_allowed(&mask, "/World/Props/Chair") == true);
    assert(tsfi_stagemask_is_path_allowed(&mask, "/World/Props") == true);

    // Path not matching prefix should be blocked
    assert(tsfi_stagemask_is_path_allowed(&mask, "/World/Camera") == false);
    assert(tsfi_stagemask_is_path_allowed(&mask, "/World/Props2") == false); // Substring boundary check

    printf("   ✓ StageMask disabled layout bypass verified.\n");
    printf("   ✓ Namespace prefix boundary checks verified successfully.\n");
    printf("=== AUNCIENT STAGEMASK VALIDATION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
