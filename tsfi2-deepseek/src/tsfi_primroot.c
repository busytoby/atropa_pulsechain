#include "tsfi_primroot.h"
#include <string.h>

void tsfi_primroot_init(TSFiPrimNode *node, const char *path, const char *type_name) {
    if (!node) return;
    
    if (path) {
        strncpy(node->path, path, sizeof(node->path) - 1);
    } else {
        strcpy(node->path, "/World");
    }
    
    if (type_name) {
        strncpy(node->type_name, type_name, sizeof(node->type_name) - 1);
    } else {
        strcpy(node->type_name, "Xform");
    }
    
    node->attribute_count = 0;
    node->child_count = 0;
    node->parent = NULL;
    
    memset(node->attributes, 0, sizeof(node->attributes));
    memset(node->children, 0, sizeof(node->children));
}

bool tsfi_primroot_set_attribute(TSFiPrimNode *node, const char *name, float value) {
    if (!node || !name) return false;
    
    // Update existing attribute if it exists
    for (int i = 0; i < node->attribute_count; i++) {
        if (strcmp(node->attributes[i].name, name) == 0) {
            node->attributes[i].value = value;
            return true;
        }
    }
    
    if (node->attribute_count >= TSFI_MAX_PRIM_ATTRIBUTES) return false;
    
    TSFiPrimAttribute *attr = &node->attributes[node->attribute_count];
    strncpy(attr->name, name, sizeof(attr->name) - 1);
    attr->value = value;
    node->attribute_count++;
    return true;
}

float tsfi_primroot_get_attribute(const TSFiPrimNode *node, const char *name, float default_val) {
    if (!node || !name) return default_val;
    
    for (int i = 0; i < node->attribute_count; i++) {
        if (strcmp(node->attributes[i].name, name) == 0) {
            return node->attributes[i].value;
        }
    }
    return default_val;
}

bool tsfi_primroot_add_child(TSFiPrimNode *parent, TSFiPrimNode *child) {
    if (!parent || !child || parent->child_count >= TSFI_MAX_PRIM_CHILDREN) return false;
    
    // Avoid double child additions
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) return false;
    }
    
    parent->children[parent->child_count] = child;
    child->parent = parent;
    parent->child_count++;
    return true;
}
