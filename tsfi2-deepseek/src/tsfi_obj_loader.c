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

void tsfi_obj_generate_buffers(const TsfiObjMesh *mesh, TsfiRenderVertex **out_vertices, uint32_t **out_indices, int *out_vertex_count, int *out_index_count) {
    if (!mesh || !out_vertices || !out_indices || !out_vertex_count || !out_index_count) return;

    int index_count = mesh->face_count * 3;
    int vertex_count = index_count;

    TsfiRenderVertex *verts = (TsfiRenderVertex*)malloc(vertex_count * sizeof(TsfiRenderVertex));
    uint32_t *ind = (uint32_t*)malloc(index_count * sizeof(uint32_t));

    int v_idx = 0;
    for (int f = 0; f < mesh->face_count; f++) {
        const TsfiObjFace *face = &mesh->faces[f];
        for (int corner = 0; corner < 3; corner++) {
            TsfiRenderVertex *v = &verts[v_idx];
            memset(v, 0, sizeof(TsfiRenderVertex));

            // OBJ indices are 1-based, check bounds
            int pos_idx = face->v_idx[corner] - 1;
            if (pos_idx >= 0 && pos_idx < mesh->vertex_count) {
                v->x = mesh->vertices[pos_idx].x;
                v->y = mesh->vertices[pos_idx].y;
                v->z = mesh->vertices[pos_idx].z;
            }

            int tex_idx = face->t_idx[corner] - 1;
            if (tex_idx >= 0 && tex_idx < mesh->texcoord_count) {
                v->u = mesh->texcoords[tex_idx].u;
                v->v = mesh->texcoords[tex_idx].v;
            }

            int norm_idx = face->n_idx[corner] - 1;
            if (norm_idx >= 0 && norm_idx < mesh->normal_count) {
                v->nx = mesh->normals[norm_idx].x;
                v->ny = mesh->normals[norm_idx].y;
                v->nz = mesh->normals[norm_idx].z;
            }

            ind[v_idx] = (uint32_t)v_idx;
            v_idx++;
        }
    }

    *out_vertices = verts;
    *out_indices = ind;
    *out_vertex_count = vertex_count;
    *out_index_count = index_count;
}
