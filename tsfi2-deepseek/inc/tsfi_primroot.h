#ifndef TSFI_PRIMROOT_H
#define TSFI_PRIMROOT_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_PRIM_ATTRIBUTES 8
#define TSFI_MAX_PRIM_CHILDREN 8

typedef struct {
    char name[32];
    float value;
} TSFiPrimAttribute;

typedef struct TSFiPrimNode {
    char path[64];
    char type_name[32];
    
    // Attributes list
    TSFiPrimAttribute attributes[TSFI_MAX_PRIM_ATTRIBUTES];
    int attribute_count;
    
    // Hierarchy links
    struct TSFiPrimNode *parent;
    struct TSFiPrimNode *children[TSFI_MAX_PRIM_CHILDREN];
    int child_count;
} TSFiPrimNode;

// Initialize a PrimNode context
void tsfi_primroot_init(TSFiPrimNode *node, const char *path, const char *type_name);

// Adds a generic attribute float property to the node
bool tsfi_primroot_set_attribute(TSFiPrimNode *node, const char *name, float value);

// Returns the float value of a queried attribute property
float tsfi_primroot_get_attribute(const TSFiPrimNode *node, const char *name, float default_val);

// Connects a child prim node to a parent node
bool tsfi_primroot_add_child(TSFiPrimNode *parent, TSFiPrimNode *child);

#endif // TSFI_PRIMROOT_H
