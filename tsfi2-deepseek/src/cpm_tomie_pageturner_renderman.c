#include "cpm_tomie_pageturner_renderman.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#define GRID_SIZE 16

void pageturner_init(PageTurnerRenderManEngine *engine, uint32_t total_pages) {
    if (!engine) return;
    memset(engine, 0, sizeof(PageTurnerRenderManEngine));
    engine->total_pages = total_pages > PAGETURNER_MAX_PAGES ? PAGETURNER_MAX_PAGES : total_pages;
    engine->clay_plasticity = 0.85f;
    engine->displacement_gain = 0.05f;

    for (uint32_t p = 0; p < engine->total_pages; p++) {
        PageTurnerPageMesh *pm = &engine->pages[p];
        pm->page_number = p + 1;
        pm->page_thickness_mm = 0.12f;
        pm->roughness = 0.35f;
        pm->anisotropic_specular = 0.65f;
        pm->num_vertices = 0;

        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                if (pm->num_vertices >= PAGETURNER_MAX_VERTICES) break;
                PageTurnerVertex *v = &pm->vertices[pm->num_vertices++];
                float u = (float)c / (float)(GRID_SIZE - 1);
                float v_coord = (float)r / (float)(GRID_SIZE - 1);
                v->u = u;
                v->v = v_coord;
                v->rest_pos.x = (u - 0.5f) * 210.0f; /* A4 width mm */
                v->rest_pos.y = (v_coord - 0.5f) * 297.0f; /* A4 height mm */
                v->rest_pos.z = (float)p * 0.12f;
                v->deformed_pos = v->rest_pos;
                v->normal.x = 0.0f;
                v->normal.y = 0.0f;
                v->normal.z = 1.0f;
                v->curl_weight = u; /* Spine on left (u=0), free edge on right (u=1) */
            }
        }
    }
}

bool pageturner_set_digital_twin_telemetry(PageTurnerRenderManEngine *engine, float temp, float hum, float co2, float kw, float x, float y, float z) {
    if (!engine) return false;
    engine->twin_sensor.room_temperature_c = temp;
    engine->twin_sensor.relative_humidity_pct = hum;
    engine->twin_sensor.co2_ppm = co2;
    engine->twin_sensor.hvac_energy_kw = kw;
    engine->twin_sensor.bim_anchor_x = x;
    engine->twin_sensor.bim_anchor_y = y;
    engine->twin_sensor.bim_anchor_z = z;
    engine->twin_sensor.timestamp_epoch = 1755500000;
    return true;
}

bool pageturner_apply_clay_curl(PageTurnerRenderManEngine *engine, uint32_t page_idx, float turn_progress) {
    if (!engine || page_idx >= engine->total_pages) return false;
    PageTurnerPageMesh *pm = &engine->pages[page_idx];
    engine->active_page = page_idx;
    engine->turn_progress = turn_progress;

    float theta_max = turn_progress * 3.14159265f; /* Max rotation */
    float cone_apex_distance = 450.0f;

    for (uint32_t i = 0; i < pm->num_vertices; i++) {
        PageTurnerVertex *v = &pm->vertices[i];
        float w = v->curl_weight;
        float local_angle = theta_max * w * engine->clay_plasticity;
        float radius = 35.0f + (1.0f - v->v) * 20.0f; /* Conical radius variation */

        float x0 = v->rest_pos.x;
        float z0 = v->rest_pos.z;

        if (turn_progress > 0.001f && w > 0.05f) {
            float cone_factor = 1.0f + (v->rest_pos.y / cone_apex_distance);
            float curl_arc = w * 210.0f * turn_progress;
            float curl_phase = curl_arc / (radius * cone_factor);

            v->deformed_pos.x = -105.0f + (1.0f - turn_progress) * (x0 + 105.0f) - sinf(curl_phase) * radius;
            v->deformed_pos.z = z0 + (1.0f - cosf(curl_phase)) * radius + sinf(local_angle) * 15.0f;
            v->normal.x = sinf(curl_phase);
            v->normal.y = 0.05f * sinf(local_angle);
            v->normal.z = cosf(curl_phase);
        } else {
            v->deformed_pos = v->rest_pos;
            v->normal.x = 0.0f;
            v->normal.y = 0.0f;
            v->normal.z = 1.0f;
        }
    }
    return true;
}

bool pageturner_apply_displacement_shader(PageTurnerRenderManEngine *engine, uint32_t page_idx) {
    if (!engine || page_idx >= engine->total_pages) return false;
    PageTurnerPageMesh *pm = &engine->pages[page_idx];

    /* DisplacementShader vertex displacement scaling with system register boundary constraints */
    float scale = engine->displacement_gain * (1.0f + 0.02f * (engine->twin_sensor.relative_humidity_pct - 50.0f));

    for (uint32_t i = 0; i < pm->num_vertices; i++) {
        PageTurnerVertex *v = &pm->vertices[i];
        /* High-frequency fiber micro-structure displacement */
        float fiber_noise = sinf(v->u * 48.0f) * cosf(v->v * 64.0f) * 0.5f +
                            sinf(v->u * 128.0f + v->v * 96.0f) * 0.25f;
        float disp = fiber_noise * scale;
        v->deformed_pos.x += v->normal.x * disp;
        v->deformed_pos.y += v->normal.y * disp;
        v->deformed_pos.z += v->normal.z * disp;
    }
    return true;
}

uint32_t pageturner_compute_rule18_checksum(const PageTurnerRenderManEngine *engine) {
    if (!engine) return 0;
    const uint8_t *data = (const uint8_t *)engine;
    size_t len = sizeof(PageTurnerRenderManEngine);

    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}
