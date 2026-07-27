#ifndef AUNCIENT_STL_LOADER_H
#define AUNCIENT_STL_LOADER_H

#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 1)
typedef struct {
    float normal[3];
    float vertices[3][3];
    uint16_t attribute_byte_count;
} AuncientStlFacet;
#pragma pack(pop)

typedef struct {
    uint32_t facet_count;
    AuncientStlFacet *facets;
} AuncientStlMesh;

// Parses a binary STL file stream into an AuncientStlMesh buffer
bool auncient_stl_load_binary(const char *filepath, AuncientStlMesh *mesh_out);

// Frees the allocated facets memory within the mesh structure
void auncient_stl_free_mesh(AuncientStlMesh *mesh);

#endif // AUNCIENT_STL_LOADER_H
