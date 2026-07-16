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

TSFiStrainClassification tsfi_doxastic_classify_strain(tsfi_dat *dat, int x_start, int x_end, int y, int z) {
    TSFiStrainClassification result = {
        .class_id = CLASS_STABLE,
        .boundary_gradient = 0.0f
    };

    if (!dat || x_start >= x_end) return result;

    int frontier_transitions = 0;
    int fracture_transitions = 0;
    int total_transitions = 0;

    char prev_val[128] = "";
    bool first = true;

    for (int x = x_start; x <= x_end; x++) {
        char query[128];
        snprintf(query, sizeof(query), "svdag/%d/%d/%d", x, y, z);
        const char *val_ref = tsfi_dat_search(dat, query);
        const char *val = val_ref ? val_ref : "";

        if (!first) {
            total_transitions++;
            bool is_fact_prev = (strcmp(prev_val, "RELATION_TRUE") == 0);
            bool is_fact_curr = (strcmp(val, "RELATION_TRUE") == 0);
            bool is_quest_prev = (strcmp(prev_val, "QUESTION_PENDING") == 0);
            bool is_quest_curr = (strcmp(val, "QUESTION_PENDING") == 0);
            bool is_void_prev = (strcmp(prev_val, "") == 0 || strcmp(prev_val, "UNBOUND") == 0);
            bool is_void_curr = (strcmp(val, "") == 0 || strcmp(val, "UNBOUND") == 0);

            if ((is_fact_prev && is_quest_curr) || (is_quest_prev && is_fact_curr)) {
                frontier_transitions++;
            } else if ((is_fact_prev && is_void_curr) || (is_void_prev && is_fact_curr)) {
                fracture_transitions++;
            }
        }
        strncpy(prev_val, val, sizeof(prev_val) - 1);
        first = false;
    }

    if (total_transitions == 0) return result;

    float frontier_ratio = (float)frontier_transitions / total_transitions;
    float fracture_ratio = (float)fracture_transitions / total_transitions;

    if (frontier_ratio >= fracture_ratio && frontier_ratio > 0.10f) {
        result.class_id = CLASS_FRONTIER;
        result.boundary_gradient = frontier_ratio;
    } else if (fracture_ratio > frontier_ratio && fracture_ratio > 0.10f) {
        result.class_id = CLASS_FRACTURE;
        result.boundary_gradient = fracture_ratio;
    }

    return result;
}
