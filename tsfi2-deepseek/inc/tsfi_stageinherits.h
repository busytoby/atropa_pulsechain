#ifndef TSFI_STAGEINHERITS_H
#define TSFI_STAGEINHERITS_H

#include "tsfi_primroot.h"
#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_CLASSES 8

typedef struct {
    char class_path[64];
    TSFiPrimNode class_prim;
    bool is_active;
} TSFiStageClass;

typedef struct {
    TSFiStageClass classes[TSFI_MAX_CLASSES];
    int class_count;
} TSFiStageInheritsTable;

// Initialize the StageInherits context
void tsfi_stageinherits_init(TSFiStageInheritsTable *table);

// Defines a new abstract class base primitive
bool tsfi_stageinherits_define_class(TSFiStageInheritsTable *table, const char *class_path);

// Inherits and resolves attribute values from a class to a target instance node
float tsfi_stageinherits_resolve_inherited_attribute(const TSFiStageInheritsTable *table, const char *class_path, const TSFiPrimNode *instance_node, const char *attr_name, float default_val);

#endif // TSFI_STAGEINHERITS_H
