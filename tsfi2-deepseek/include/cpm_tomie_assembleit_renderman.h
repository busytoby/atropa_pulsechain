#ifndef TSFI_CPM_TOMIE_ASSEMBLEIT_RENDERMAN_H
#define TSFI_CPM_TOMIE_ASSEMBLEIT_RENDERMAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASSEMBLEIT_MAX_NODES   32
#define ASSEMBLEIT_MAX_STRUTS  64
#define ASSEMBLEIT_MAX_VERTICES 256

typedef struct {
    float x, y, z;
} AssembleitVec3;

typedef struct {
    float w, x, y, z;
} AssembleitQuat;

typedef struct {
    uint32_t node_id;
    AssembleitVec3 position;
    float magnetic_retention_n; /* Default: 13.7 N */
    bool is_grounded;
} AssembleitNode;

typedef struct {
    uint32_t strut_id;
    uint32_t node_a;
    uint32_t node_b;
    float stroke_length_mm;     /* Range: [280.0 .. 430.0] mm */
    float target_length_mm;
    float max_speed_mm_s;
    bool active;
} AssembleitStrut;

typedef struct {
    AssembleitVec3 rest_pos;
    AssembleitVec3 deformed_pos;
    AssembleitVec3 normal;
    float skin_weights[4];
    uint32_t joint_indices[4];
    float u, v;
} AssembleitClayVertex;

typedef struct {
    uint32_t num_nodes;
    uint32_t num_struts;
    uint32_t num_vertices;
    AssembleitNode nodes[ASSEMBLEIT_MAX_NODES];
    AssembleitStrut struts[ASSEMBLEIT_MAX_STRUTS];
    AssembleitClayVertex vertices[ASSEMBLEIT_MAX_VERTICES];
    float clay_volume_mm3;
    bool isostatic_valid;
    uint32_t morton_key;
} AssembleitModel;

/* Engine API */
void assembleit_init_model(AssembleitModel *model);
bool assembleit_add_node(AssembleitModel *model, uint32_t node_id, float x, float y, float z, bool grounded);
bool assembleit_add_strut(AssembleitModel *model, uint32_t strut_id, uint32_t node_a, uint32_t node_b, float length_mm);
bool assembleit_add_clay_vertex(AssembleitModel *model, float x, float y, float z, float u, float v);

bool assembleit_solve_kinematics(AssembleitModel *model, float delta_time_s);
bool assembleit_apply_displacement_shader(AssembleitModel *model, float displacement_scale);
uint32_t assembleit_compute_morton_quadtree_index(const AssembleitModel *model);
bool assembleit_evaluate_renderman_micropolygons(const AssembleitModel *model, size_t *out_micropolygon_count);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_ASSEMBLEIT_RENDERMAN_H */
