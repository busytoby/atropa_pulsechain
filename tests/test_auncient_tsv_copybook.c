#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_mainframe_computerworld.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TSV COBOL COPYBOOK STANDARDIZATION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    tsfi_cw_copybook cb;
    memset(&cb, 0, sizeof(cb));

    // Define standard TSV copybook structure representing registers
    int rc = tsfi_cw_parse_copybook_line("01 TSV-RECORD.", &cb);
    assert(rc == 0);

    rc = tsfi_cw_parse_copybook_line("  05 TSV-REGS PIC 9(18) USAGE COMP OCCURS 32 TIMES.", &cb);
    assert(rc == 0);

    printf("[Copybook] Validating parsed TSV record structure...\n");
    assert(cb.field_count == 2);
    assert(strcmp(cb.fields[1].name, "TSV-REGS") == 0);
    assert(cb.fields[1].occurs == 32);
    assert(cb.fields[1].usage == COBOL_USAGE_COMP);

    printf("   ✓ COBOL Copybook schema validated successfully.\n");
    printf("=============================================================\n");
    printf("TSV COBOL COPYBOOK TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
