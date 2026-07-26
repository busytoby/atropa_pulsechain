#ifndef TSFI_STAGECOMP_H
#define TSFI_STAGECOMP_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_LIVRPS_LAYERS 6

typedef enum {
    LIVRPS_SPECIALIZES = 0,
    LIVRPS_PAYLOADS,
    LIVRPS_REFERENCES,
    LIVRPS_VARIANTSETS,
    LIVRPS_INHERITS,
    LIVRPS_LOCAL
} LivrpsLayerType;

typedef struct {
    char val[64];
    bool has_value;
} LivrpsLayer;

// USD Stage Composition Resolver (StageComp)
typedef struct {
    LivrpsLayer layers[MAX_LIVRPS_LAYERS];
} TSFiStageComp;

// Initialize the stage composition resolver context
void tsfi_stagecomp_init(TSFiStageComp *comp);

// Set the opinion (value) for a specific layer in the composition stack
void tsfi_stagecomp_set_opinion(TSFiStageComp *comp, LivrpsLayerType layer, const char *value);

// Resolves the terminal attribute state applying the LIVRPS override order (Local > ... > Specializes)
bool tsfi_stagecomp_resolve(const TSFiStageComp *comp, char *out_val);

#endif // TSFI_STAGECOMP_H
