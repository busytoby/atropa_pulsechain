#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_doxastic_classifier.h"

TSFiDoxasticClassification tsfi_doxastic_classify_region(tsfi_dat *dat, int x_start, int x_end, int y, int z) {
    TSFiDoxasticClassification result = {
        .class_id = CLASS_UNKNOWN,
        .confidence = 0.0f,
        .facts_count = 0,
        .questions_count = 0,
        .retracted_count = 0
    };

    if (!dat) return result;

    int total_cells = 0;
    for (int x = x_start; x <= x_end; x++) {
        char query[128];
        snprintf(query, sizeof(query), "svdag/%d/%d/%d", x, y, z);
        const char *val = tsfi_dat_search(dat, query);
        total_cells++;

        if (val) {
            if (strcmp(val, "RELATION_TRUE") == 0) {
                result.facts_count++;
            } else if (strcmp(val, "QUESTION_PENDING") == 0) {
                result.questions_count++;
            } else if (strcmp(val, "") == 0 || strcmp(val, "UNBOUND") == 0) {
                result.retracted_count++;
            }
        } else {
            result.retracted_count++;
        }
    }

    if (total_cells == 0) return result;

    // Calculate class frequencies
    float fact_ratio = (float)result.facts_count / total_cells;
    float quest_ratio = (float)result.questions_count / total_cells;
    float retract_ratio = (float)result.retracted_count / total_cells;

    if (fact_ratio >= quest_ratio && fact_ratio >= retract_ratio) {
        result.class_id = CLASS_BOND;
        result.confidence = fact_ratio;
    } else if (quest_ratio >= fact_ratio && quest_ratio >= retract_ratio) {
        result.class_id = CLASS_SEED;
        result.confidence = quest_ratio;
    } else {
        result.class_id = CLASS_VOID;
        result.confidence = retract_ratio;
    }

    return result;
}
