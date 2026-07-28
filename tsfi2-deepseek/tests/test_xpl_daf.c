#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// Audit record types matching SESSLOG
typedef enum {
    TYPE_CONDUCT = 0,
    TYPE_RED_ERR = 1,
    TYPE_BLK_TRP = 2
} log_type_t;

typedef struct {
    uint32_t seq;
    log_type_t type;
    char dataset_name[44];
    char job_name[16];
    uint32_t excp_count;
} smf_record_t;

// Mock SMF database
static smf_record_t mock_smf_db[] = {
    { 1001, TYPE_CONDUCT, "SYS1.ISPSLIB",  "COMPJCL1", 45  },
    { 1002, TYPE_RED_ERR, "SYS1.LINKLIB",  "BADJOB",    0  },
    { 1003, TYPE_BLK_TRP, "SYS1.ISPPLIB",  "OVERFLOW", 120 }
};
#define SMF_COUNT (sizeof(mock_smf_db)/sizeof(smf_smf_db[0]))

// DAF selection criteria filters
typedef struct {
    char filter_dsn[44];
    char filter_job[16];
    bool has_dsn_filter;
    bool has_job_filter;
} daf_filter_t;

// 1. RED ANALYZER Gating: Static validation of filter inputs
static bool red_analyzer_validate_daf_filters(const daf_filter_t *filter) {
    if (!filter) return false;

    // Check DSN filter naming standard if present
    if (filter->has_dsn_filter) {
        size_t len = strlen(filter->filter_dsn);
        if (len < 1 || len > 44) return false;
        // Verify valid character set (alphanumeric or dots)
        for (size_t i = 0; i < len; i++) {
            char c = filter->filter_dsn[i];
            if ((c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '.') {
                return false; // Rejected by RED rail
            }
        }
    }

    // Check Job filter naming standard if present
    if (filter->has_job_filter) {
        size_t len = strlen(filter->filter_job);
        if (len < 1 || len > 8) return false; // Job names max 8 characters
    }

    return true; // Filters valid
}

// 2. BLACK XPLSM Gating: DAF report generator and dynamic filter executor
static uint32_t black_xplsm_run_daf_report(const daf_filter_t *filter, char *report_out, size_t max_len) {
    if (!filter || !report_out) return 0;

    char *dst = report_out;
    char *end = report_out + max_len;
    uint32_t match_count = 0;

    dst += snprintf(dst, end - dst, "--- DAF AUDIT LOG REPORT ---\n");
    dst += snprintf(dst, end - dst, "SEQ   TYPE     DSN                        JOB       EXCPS\n");
    dst += snprintf(dst, end - dst, "---   ----     ---                        ---       -----\n");

    for (size_t i = 0; i < 3; i++) {
        smf_record_t rec = mock_smf_db[i];

        // Apply filters
        if (filter->has_dsn_filter && strcmp(rec.dataset_name, filter->filter_dsn) != 0) {
            continue;
        }
        if (filter->has_job_filter && strcmp(rec.job_name, filter->filter_job) != 0) {
            continue;
        }

        const char *type_str = (rec.type == TYPE_CONDUCT) ? "CONDUCT" :
                               (rec.type == TYPE_RED_ERR) ? "RED_ERR" : "BLK_TRP";

        dst += snprintf(dst, end - dst, "%-5u %-8s %-26s %-9s %-5u\n",
                        rec.seq, type_str, rec.dataset_name, rec.job_name, rec.excp_count);
        match_count++;
    }

    dst += snprintf(dst, end - dst, "--- END OF REPORT (MATCHES: %u) ---\n", match_count);
    return match_count;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DAF DATASET AUDIT FACILITY VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: RED rail filter validation
    printf("[TEST] Checking filter inputs against RED rail...\n");
    daf_filter_t valid_filter = {
        .filter_dsn = "SYS1.ISPSLIB",
        .filter_job = "COMPJCL1",
        .has_dsn_filter = true,
        .has_job_filter = true
    };
    assert(red_analyzer_validate_daf_filters(&valid_filter) == true);

    // Invalid filters: lowercase in DSN or job name too long
    daf_filter_t invalid_filter = {
        .filter_dsn = "sys1.ispslib",
        .has_dsn_filter = true
    };
    assert(red_analyzer_validate_daf_filters(&invalid_filter) == false);
    printf("   ✓ RED rail successfully gated static filter selection rules.\n");

    // Test 2: BLACK rail report generation and filter checks
    printf("[TEST] Running DAF search queries and generating report...\n");
    char report[1024] = {0};
    uint32_t matches = 0;

    // Filter by DSN only
    daf_filter_t dsn_filter = {
        .filter_dsn = "SYS1.ISPSLIB",
        .has_dsn_filter = true
    };
    matches = black_xplsm_run_daf_report(&dsn_filter, report, sizeof(report));
    assert(matches == 1);
    printf("%s\n", report);

    // Filter matching nothing
    daf_filter_t empty_filter = {
        .filter_dsn = "SYS1.NONEXIST",
        .has_dsn_filter = true
    };
    matches = black_xplsm_run_daf_report(&empty_filter, report, sizeof(report));
    assert(matches == 0);
    printf("   ✓ BLACK rail successfully filtered SMF records and generated formatted diagnostics.\n");

    printf("=============================================================\n");
    printf("ALL DAF DATASET AUDIT FACILITY TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
