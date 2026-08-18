#ifndef TSFI_CPM_TOMIE_ASSEMBLEIT_RENDERMAN_H
#define TSFI_CPM_TOMIE_ASSEMBLEIT_RENDERMAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASSEMBLEIT_MAX_NODES       32
#define ASSEMBLEIT_MAX_STRUTS      64
#define ASSEMBLEIT_MAX_VERTICES    256
#define ASSEMBLEIT_MAX_QUAD_NODES  512

/* BDOS Hardware I/O Ports */
#define ASSEMBLEIT_PORT_STROKE_TELEMETRY 0x48
#define ASSEMBLEIT_PORT_IMPEDANCE_ADC    0x49
#define ASSEMBLEIT_PORT_ACOUSTIC_PRESS   0x4A
#define ASSEMBLEIT_PORT_QUADTREE_STATUS  0x4B

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
    uint32_t morton_code;
    float phase_rad;            /* [0 .. 2pi) */
    float amplitude;
    float curvature;            /* |grad^2 phi| */
    uint32_t child_indices[4];  /* NW, NE, SW, SE */
    bool is_leaf;
} AssembleitHoloQuadNode;

typedef struct {
    uint32_t root_index;
    uint32_t node_count;
    AssembleitHoloQuadNode nodes[ASSEMBLEIT_MAX_QUAD_NODES];
    float total_optical_energy;
} AssembleitHoloQuadTree;

typedef struct {
    float mutual_inductance_uh;
    float acoustic_radiation_pa;
    float complex_permittivity_real;
    float complex_permittivity_imag;
    uint16_t adc_raw_12bit;
} AssembleitTelemetry;

typedef struct {
    uint32_t num_nodes;
    uint32_t num_struts;
    uint32_t num_vertices;
    AssembleitNode nodes[ASSEMBLEIT_MAX_NODES];
    AssembleitStrut struts[ASSEMBLEIT_MAX_STRUTS];
    AssembleitClayVertex vertices[ASSEMBLEIT_MAX_VERTICES];
    AssembleitHoloQuadTree holo_quadtree;
    AssembleitTelemetry telemetry;
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

/* Gerchberg-Saxton Holographic Quadtree API */
bool assembleit_synthesize_gerchberg_quadtree(AssembleitModel *model, uint32_t depth_levels);
bool assembleit_save_quadtree_binary(const AssembleitModel *model, const char *dat_bin_path);

/* BDOS Hardware I/O Port API */
uint8_t assembleit_bdos_read_port(AssembleitModel *model, uint8_t port);
void assembleit_bdos_write_port(AssembleitModel *model, uint8_t port, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_ASSEMBLEIT_RENDERMAN_H */
