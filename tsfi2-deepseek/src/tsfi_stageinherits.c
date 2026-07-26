#include "tsfi_stageinherits.h"
#include <string.h>

void tsfi_stageinherits_init(TSFiStageInheritsTable *table) {
    if (!table) return;
    
    table->class_count = 0;
    memset(table->classes, 0, sizeof(table->classes));
}

bool tsfi_stageinherits_define_class(TSFiStageInheritsTable *table, const char *class_path) {
    if (!table || !class_path || table->class_count >= TSFI_MAX_CLASSES) return false;
    
    // Check duplicates
    for (int i = 0; i < table->class_count; i++) {
        if (strcmp(table->classes[i].class_path, class_path) == 0) {
            return true;
        }
    }
    
    TSFiStageClass *c = &table->classes[table->class_count];
    strncpy(c->class_path, class_path, sizeof(c->class_path) - 1);
    tsfi_primroot_init(&c->class_prim, class_path, "Class");
    c->is_active = true;
    
    table->class_count++;
    return true;
}

float tsfi_stageinherits_resolve_inherited_attribute(const TSFiStageInheritsTable *table, const char *class_path, const TSFiPrimNode *instance_node, const char *attr_name, float default_val) {
    if (!table || !class_path || !attr_name) return default_val;
    
    // 1. If the instance has a local opinion (attribute set locally), it overrides the inherited value
    if (instance_node) {
        for (int i = 0; i < instance_node->attribute_count; i++) {
            if (strcmp(instance_node->attributes[i].name, attr_name) == 0) {
                return instance_node->attributes[i].value;
            }
        }
    }
    
    // 2. Otherwise, fall back to class inheritance
    for (int i = 0; i < table->class_count; i++) {
        const TSFiStageClass *c = &table->classes[i];
        if (c->is_active && strcmp(c->class_path, class_path) == 0) {
            return tsfi_primroot_get_attribute(&c->class_prim, attr_name, default_val);
        }
    }
    
    return default_val;
}
