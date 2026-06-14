#ifndef TSFI_OBJ_LOADER_H
#define TSFI_OBJ_LOADER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float x, y, z;
} TsfiObjVertex;

typedef struct {
    float u, v;
} TsfiObjTexCoord;

typedef struct {
    float x, y, z;
} TsfiObjNormal;

typedef struct {
    int v_idx[3];
    int t_idx[3];
    int n_idx[3];
} TsfiObjFace;

typedef struct {
    TsfiObjVertex *vertices;
    int vertex_count;
    int vertex_capacity;

    TsfiObjTexCoord *texcoords;
    int texcoord_count;
    int texcoord_capacity;

    TsfiObjNormal *normals;
    int normal_count;
    int normal_capacity;

    TsfiObjFace *faces;
    int face_count;
    int face_capacity;
} TsfiObjMesh;

TsfiObjMesh* tsfi_obj_load(const char *filepath);
void tsfi_obj_free(TsfiObjMesh *mesh);

#endif // TSFI_OBJ_LOADER_H
