#ifndef TSFI_DOXASTIC_CLASSIFIER_H
#define TSFI_DOXASTIC_CLASSIFIER_H

#include "tsfi_dat.h"

typedef enum {
    CLASS_UNKNOWN = 0,
    CLASS_SEED, // Potential / Pending questions
    CLASS_BOND, // Crystallized facts
    CLASS_VOID  // Retracted / Empty space
} TSFiDoxasticClass;

typedef struct {
    TSFiDoxasticClass class_id;
    float confidence;
    int facts_count;
    int questions_count;
    int retracted_count;
} TSFiDoxasticClassification;

// Classify a specific 3D coordinate region in the DAT/SVDAG database
TSFiDoxasticClassification tsfi_doxastic_classify_region(tsfi_dat *dat, int x_start, int x_end, int y, int z);

#endif // TSFI_DOXASTIC_CLASSIFIER_H
