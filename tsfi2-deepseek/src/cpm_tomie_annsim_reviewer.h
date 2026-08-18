#ifndef CPM_TOMIE_ANNSIM_REVIEWER_H
#define CPM_TOMIE_ANNSIM_REVIEWER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CPM_TPA_BASE 0x0100
#define ANNSIM_OPCODE_SIG 0x414E4E53 /* "ANNS" */

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                    /* "ANKH" */
    uint16_t load_address;               /* 0x0100 */
    uint16_t entry_point;                /* 0x0100 */
    uint32_t opcode_signature;           /* "ANNS" */
    uint32_t total_streams;
    uint32_t text_bytes_extracted;
    uint32_t living_lab_mentions;
    uint32_t digital_twin_mentions;
    uint32_t bim_workflow_mentions;
    uint32_t sensor_telemetry_mentions;
    uint32_t leed_sustainability_mentions;
    uint32_t honeywell_bas_mentions;
    uint32_t proof_phase_witness[5];
    char target_pdf[64];
    uint32_t checksum_rule18;
} CpmTomieAnnSimReviewRecord;

typedef struct {
    size_t total_streams;
    size_t extracted_bytes;
    size_t living_lab_count;
    size_t digital_twin_count;
    size_t bim_count;
    size_t sensor_count;
    size_t leed_count;
    size_t bas_count;
    size_t simaud_count;
    bool phase1_ingestion_valid;
    bool phase2_structural_valid;
    bool phase3_domain_axioms_valid;
    bool phase4_rule18_valid;
    bool phase5_formal_closure_valid;
    uint32_t final_checksum;
} CpmTomieAnnSimReviewResult;

CpmTomieAnnSimReviewResult *cpm_tomie_review_annsim_pdf(const char *pdf_path);
void cpm_tomie_review_result_free(CpmTomieAnnSimReviewResult *res);
uint32_t cpm_tomie_annsim_rule18_checksum(const uint8_t *data, size_t len);

#endif /* CPM_TOMIE_ANNSIM_REVIEWER_H */
