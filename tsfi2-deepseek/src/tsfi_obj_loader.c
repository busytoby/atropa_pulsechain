#include "tsfi_obj_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 256

TsfiObjMesh* tsfi_obj_load(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("[OBJ] Error: Cannot open file %s\n", filepath);
        return NULL;
    }

    TsfiObjMesh *mesh = (TsfiObjMesh*)calloc(1, sizeof(TsfiObjMesh));
    if (!mesh) {
        fclose(file);
        return NULL;
    }

    mesh->vertex_capacity = INITIAL_CAPACITY;
    mesh->vertices = (TsfiObjVertex*)malloc(mesh->vertex_capacity * sizeof(TsfiObjVertex));

    mesh->texcoord_capacity = INITIAL_CAPACITY;
    mesh->texcoords = (TsfiObjTexCoord*)malloc(mesh->texcoord_capacity * sizeof(TsfiObjTexCoord));

    mesh->normal_capacity = INITIAL_CAPACITY;
    mesh->normals = (TsfiObjNormal*)malloc(mesh->normal_capacity * sizeof(TsfiObjNormal));

    mesh->face_capacity = INITIAL_CAPACITY;
    mesh->faces = (TsfiObjFace*)malloc(mesh->face_capacity * sizeof(TsfiObjFace));

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            if (mesh->vertex_count >= mesh->vertex_capacity) {
                mesh->vertex_capacity *= 2;
                mesh->vertices = (TsfiObjVertex*)realloc(mesh->vertices, mesh->vertex_capacity * sizeof(TsfiObjVertex));
            }
            TsfiObjVertex *v = &mesh->vertices[mesh->vertex_count++];
            sscanf(line, "v %f %f %f", &v->x, &v->y, &v->z);
        } else if (line[0] == 'v' && line[1] == 't') {
            if (mesh->texcoord_count >= mesh->texcoord_capacity) {
                mesh->texcoord_capacity *= 2;
                mesh->texcoords = (TsfiObjTexCoord*)realloc(mesh->texcoords, mesh->texcoord_capacity * sizeof(TsfiObjTexCoord));
            }
            TsfiObjTexCoord *vt = &mesh->texcoords[mesh->texcoord_count++];
            sscanf(line, "vt %f %f", &vt->u, &vt->v);
        } else if (line[0] == 'v' && line[1] == 'n') {
            if (mesh->normal_count >= mesh->normal_capacity) {
                mesh->normal_capacity *= 2;
                mesh->normals = (TsfiObjNormal*)realloc(mesh->normals, mesh->normal_capacity * sizeof(TsfiObjNormal));
            }
            TsfiObjNormal *vn = &mesh->normals[mesh->normal_count++];
            sscanf(line, "vn %f %f %f", &vn->x, &vn->y, &vn->z);
        } else if (line[0] == 'f' && line[1] == ' ') {
            if (mesh->face_count >= mesh->face_capacity) {
                mesh->face_capacity *= 2;
                mesh->faces = (TsfiObjFace*)realloc(mesh->faces, mesh->face_capacity * sizeof(TsfiObjFace));
            }
            TsfiObjFace *f = &mesh->faces[mesh->face_count++];
            memset(f, 0, sizeof(TsfiObjFace));

            // Try different face formats
            int matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                                 &f->v_idx[0], &f->t_idx[0], &f->n_idx[0],
                                 &f->v_idx[1], &f->t_idx[1], &f->n_idx[1],
                                 &f->v_idx[2], &f->t_idx[2], &f->n_idx[2]);
            if (matches != 9) {
                matches = sscanf(line, "f %d//%d %d//%d %d//%d",
                                 &f->v_idx[0], &f->n_idx[0],
                                 &f->v_idx[1], &f->n_idx[1],
                                 &f->v_idx[2], &f->n_idx[2]);
                if (matches != 6) {
                    matches = sscanf(line, "f %d/%d %d/%d %d/%d",
                                     &f->v_idx[0], &f->t_idx[0],
                                     &f->v_idx[1], &f->t_idx[1],
                                     &f->v_idx[2], &f->t_idx[2]);
                    if (matches != 6) {
                        sscanf(line, "f %d %d %d",
                               &f->v_idx[0], &f->v_idx[1], &f->v_idx[2]);
                    }
                }
            }
        }
    }

    fclose(file);
    printf("[OBJ] Loaded %s: %d vertices, %d faces\n", filepath, mesh->vertex_count, mesh->face_count);
    return mesh;
}

void tsfi_obj_free(TsfiObjMesh *mesh) {
    if (!mesh) return;
    free(mesh->vertices);
    free(mesh->texcoords);
    free(mesh->normals);
    free(mesh->faces);
    free(mesh);
}
