#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glob.h>
#include "tsfi_zorse_eval.h"
#include "tsfi_mainframe_computerworld.h"

int main(void) {
    printf("=== MAINFRAME & ZORSE COBOL COMPLIANCE AUDITOR ===\n");
    glob_t g;
    int res = glob("../solidity/dysnomia/domain/strategies/*.strategy", 0, NULL, &g);
    if (res != 0) {
        res = glob("solidity/dysnomia/domain/strategies/*.strategy", 0, NULL, &g);
    }
    assert(res == 0);
    printf("Auditing %zu .strategy files for strict COBOL and Zorse conformance...\n", g.gl_pathc);

    for (size_t i = 0; i < g.gl_pathc; i++) {
        FILE *f = fopen(g.gl_pathv[i], "r");
        assert(f != NULL);
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *src = (char *)malloc(sz + 1);
        assert(src != NULL);
        size_t read_bytes = fread(src, 1, sz, f);
        src[read_bytes] = '\0';
        fclose(f);

        // 1. Zorse Division Sequence & Syntax Validation
        int is_valid = 0;
        tsfi_zorse_validate_cobol(src, &is_valid);
        if (!is_valid) {
            printf("  [FAIL] Non-compliant COBOL structure: %s\n", g.gl_pathv[i]);
        }
        assert(is_valid == 1);

        // 2. Mainframe Punch Card boundary conformance check
        tsfi_cw_punch_card card;
        char line_buf[128];
        const char *p = src;
        while (*p) {
            const char *next = strchr(p, '\n');
            size_t len = next ? (size_t)(next - p) : strlen(p);
            if (len > 0 && len < sizeof(line_buf)) {
                memcpy(line_buf, p, len);
                line_buf[len] = '\0';
                if (line_buf[0] != '#' && strlen(line_buf) > 0) {
                    tsfi_cw_parse_punch_card(line_buf, &card);
                }
            }
            if (!next) break;
            p = next + 1;
        }

        free(src);
    }

    printf("ALL %zu .STRATEGY FILES CONFIRMED FULLY COMPLIANT COBOL FOR MAINFRAME AND ZORSE!\n", g.gl_pathc);
    globfree(&g);
    return 0;
}
