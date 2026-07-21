#include "tsfi_parc_naur_ab14.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING PETER NAUR AB14 QUESTIONNAIRE AUDIT TEST SUITE\n");
    printf("=======================================================\n");

    // Initialize AB14 registry
    assert(tsfi_naur_ab14_registry_initialize() == 0);

    // Verify all 14 questions individually
    for (int i = 1; i <= 14; i++) {
        const tsfi_naur_ab14_resolution_t *res = tsfi_naur_ab14_query_resolution((tsfi_naur_ab14_question_id_t)i);
        assert(res != NULL);
        assert(res->question_identifier == (tsfi_naur_ab14_question_id_t)i);
        assert(res->is_fully_resolved == true);
        assert(res->dynamic_contract_address != NULL);
        assert(strncmp(res->dynamic_contract_address, "dynamic_", 8) == 0); // Rule 9: Dynamic Contract ZMM Resolution

        printf("[PASS] AB14 Question #%02d '%s' Resolved -> Contract: %s\n",
               res->question_identifier,
               res->question_title,
               res->dynamic_contract_address);
    }

    // Execute Global Verification Audit
    uint32_t resolved_count = 0;
    uint32_t total_count = 0;
    int audit_result = tsfi_naur_ab14_execute_verification_audit(&resolved_count, &total_count);

    assert(audit_result == 0);
    assert(resolved_count == 14);
    assert(total_count == 14);

    printf("=======================================================\n");
    printf(" ALL 14 PETER NAUR AB14 PROBLEMS RESOLVED (14/14 100%%) \n");
    printf("=======================================================\n");

    return 0;
}
