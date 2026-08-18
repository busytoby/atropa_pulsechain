#include "cpm_tomie_assembleit_renderman.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#define CLAMP(v, min_v, max_v) (((v) < (min_v)) ? (min_v) : (((v) > (max_v)) ? (max_v) : (v)))

void assembleit_init_model(AssembleitModel *model) {
    if (!model) return;
    memset(model, 0, sizeof(AssembleitModel));
    model->clay_volume_mm3 = 500000.0f; /* 500 cm^3 baseline clay mass */
    model->isostatic_valid = true;
}

bool assembleit_add_node(AssembleitModel *model, uint32_t node_id, float x, float y, float z, bool grounded) {
    if (!model || model->num_nodes >= ASSEMBLEIT_MAX_NODES) return false;
    AssembleitNode *node = &model->nodes[model->num_nodes++];
    node->node_id = node_id;
    node->position.x = x;
    node->position.y = y;
    node->position.z = z;
    node->magnetic_retention_n = 13.7f;
    node->is_grounded = grounded;
    return true;
}

bool assembleit_add_strut(AssembleitModel *model, uint32_t strut_id, uint32_t node_a, uint32_t node_b, float length_mm) {
    if (!model || model->num_struts >= ASSEMBLEIT_MAX_STRUTS) return false;
    if (length_mm < 280.0f || length_mm > 430.0f) return false;
    AssembleitStrut *strut = &model->struts[model->num_struts++];
    strut->strut_id = strut_id;
    strut->node_a = node_a;
    strut->node_b = node_b;
    strut->stroke_length_mm = length_mm;
    strut->target_length_mm = length_mm;
    strut->max_speed_mm_s = 50.0f;
    strut->active = true;
    return true;
}

bool assembleit_add_clay_vertex(AssembleitModel *model, float x, float y, float z, float u, float v) {
    if (!model || model->num_vertices >= ASSEMBLEIT_MAX_VERTICES) return false;
    AssembleitClayVertex *vert = &model->vertices[model->num_vertices++];
    vert->rest_pos.x = x;
    vert->rest_pos.y = y;
    vert->rest_pos.z = z;
    vert->deformed_pos = vert->rest_pos;
    vert->u = u;
    vert->v = v;
    vert->normal.x = 0.0f;
    vert->normal.y = 1.0f;
    vert->normal.z = 0.0f;
    vert->skin_weights[0] = 1.0f;
    vert->joint_indices[0] = 0;
    return true;
}

bool assembleit_solve_kinematics(AssembleitModel *model, float delta_time_s) {
    if (!model) return false;
    for (uint32_t i = 0; i < model->num_struts; i++) {
        AssembleitStrut *s = &model->struts[i];
        if (!s->active) continue;
        float diff = s->target_length_mm - s->stroke_length_mm;
        float max_step = s->max_speed_mm_s * delta_time_s;
        if (fabsf(diff) <= max_step) {
            s->stroke_length_mm = s->target_length_mm;
        } else {
            s->stroke_length_mm += (diff > 0.0f ? max_step : -max_step);
        }
        s->stroke_length_mm = CLAMP(s->stroke_length_mm, 280.0f, 430.0f);
    }
    return true;
}

bool assembleit_apply_displacement_shader(AssembleitModel *model, float displacement_scale) {
    if (!model) return false;
    float avg_expansion = 0.0f;
    if (model->num_struts > 0) {
        for (uint32_t i = 0; i < model->num_struts; i++) {
            avg_expansion += (model->struts[i].stroke_length_mm - 280.0f) / 150.0f;
        }
        avg_expansion /= (float)model->num_struts;
    }

    for (uint32_t i = 0; i < model->num_vertices; i++) {
        AssembleitClayVertex *v = &model->vertices[i];
        float disp = sinf(v->u * 6.2831853f) * cosf(v->v * 6.2831853f) * displacement_scale * (1.0f + avg_expansion);
        v->deformed_pos.x = v->rest_pos.x + v->normal.x * disp;
        v->deformed_pos.y = v->rest_pos.y + v->normal.y * disp;
        v->deformed_pos.z = v->rest_pos.z + v->normal.z * disp;
    }
    return true;
}

uint32_t assembleit_compute_morton_quadtree_index(const AssembleitModel *model) {
    if (!model || model->num_struts < 2) return 0;
    uint32_t s1 = (uint32_t)(((model->struts[0].stroke_length_mm - 280.0f) / 150.0f) * 4095.0f);
    uint32_t s2 = (uint32_t)(((model->struts[1].stroke_length_mm - 280.0f) / 150.0f) * 4095.0f);
    if (s1 > 4095) s1 = 4095;
    if (s2 > 4095) s2 = 4095;

    uint32_t morton = 0;
    for (int i = 0; i < 12; i++) {
        morton |= ((s1 & (1 << i)) << i) | ((s2 & (1 << i)) << (i + 1));
    }
    return morton;
}

bool assembleit_evaluate_renderman_micropolygons(const AssembleitModel *model, size_t *out_micropolygon_count) {
    if (!model || !out_micropolygon_count) return false;
    /* REYES sub-pixel dicing: 16 sub-micropolygons per clay mesh quadrilateral */
    *out_micropolygon_count = (size_t)(model->num_vertices * 16);
    return true;
}
