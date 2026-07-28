#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_parc_tape_label_yul.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TAPE LABEL DYNAMIC VOLUME SPANNING TEST\n");
    printf("=============================================================\n");

    uint8_t eov_buf[720];
    uint8_t next_vol_buf[720];

    int res = tsfi_tape_label_split_volume(eov_buf, next_vol_buf, "VOL001", "VOL002");
    assert(res == 0);

    // Verify EOV1 Label contents
    assert(memcmp(eov_buf, "EOV1", 4) == 0);
    assert(memcmp(eov_buf + 4, "VOL001", 6) == 0);
    assert(memcmp(eov_buf + 37, "AUNCIENT_ZMM01", 14) == 0);

    // Verify VOL1 Label contents
    assert(memcmp(next_vol_buf, "VOL1", 4) == 0);
    assert(memcmp(next_vol_buf + 4, "VOL002", 6) == 0);
    assert(memcmp(next_vol_buf + 37, "AUNCIENT_ZMM01", 14) == 0);

    // Verify link parameters in HDR5
    assert(memcmp(eov_buf + 400, "HDR5", 4) == 0);
    assert(memcmp(eov_buf + 410, "VOL002", 6) == 0); // Next Link

    assert(memcmp(next_vol_buf + 400, "HDR5", 4) == 0);
    assert(memcmp(next_vol_buf + 404, "VOL001", 6) == 0); // Prev Link

    printf("   ✓ EOV1 and VOL1 volume linkages formatted successfully.\n");

    printf("=============================================================\n");
    printf("TAPE LABEL VOLUME SPANNING TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
