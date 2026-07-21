#ifndef TSFI_PARC_NAUR_AB14_H
#define TSFI_PARC_NAUR_AB14_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Peter Naur AB14 Questionnaire 14 Core Category Enums */
typedef enum {
    NAUR_AB14_QUESTION_1_SIDE_EFFECTS               = 1,
    NAUR_AB14_QUESTION_2_DYNAMIC_ARRAYS             = 2,
    NAUR_AB14_QUESTION_3_PARAMETER_PASSING          = 3,
    NAUR_AB14_QUESTION_4_BLOCK_SCOPE_BINDING        = 4,
    NAUR_AB14_QUESTION_5_TYPE_COERCION_PRECISION    = 5,
    NAUR_AB14_QUESTION_6_RECURSION_ACTIVATION_RECORDS = 6,
    NAUR_AB14_QUESTION_7_DYNAMIC_SWITCH_ROUTING      = 7,
    NAUR_AB14_QUESTION_8_FOR_LOOP_STEP_LIMITS        = 8,
    NAUR_AB14_QUESTION_9_DANGLING_ELSE_RESOLUTION   = 9,
    NAUR_AB14_QUESTION_10_PROCEDURE_CLOSURES        = 10,
    NAUR_AB14_QUESTION_11_IO_STREAM_DEVICE_MAPPING   = 11,
    NAUR_AB14_QUESTION_12_COMPILER_PASS_FOOTPRINT    = 12,
    NAUR_AB14_QUESTION_13_SUBSET_EXTENSION_GOVERNANCE = 13,
    NAUR_AB14_QUESTION_14_FORMAL_LANGUAGE_MAINTENANCE = 14
} tsfi_naur_ab14_question_id_t;

/* AB14 Question Resolution Record */
typedef struct {
    tsfi_naur_ab14_question_id_t question_identifier;
    const char *question_title;
    const char *naur_1962_problem_statement;
    const char *auncient_dysnomia_vm_solution;
    const char *dynamic_contract_address;
    bool is_fully_resolved;
} tsfi_naur_ab14_resolution_t;

/* Initialize AB14 Questionnaire Registry with all 14 Resolutions */
int tsfi_naur_ab14_registry_initialize(void);

/* Query AB14 Question Resolution by ID */
const tsfi_naur_ab14_resolution_t *tsfi_naur_ab14_query_resolution(tsfi_naur_ab14_question_id_t question_id);

/* Execute Verification Audit for all 14 AB14 Questionnaire Problems */
int tsfi_naur_ab14_execute_verification_audit(uint32_t *resolved_count_out, uint32_t *total_count_out);

#endif // TSFI_PARC_NAUR_AB14_H
