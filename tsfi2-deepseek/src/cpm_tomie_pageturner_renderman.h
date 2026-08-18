#ifndef CPM_TOMIE_PAGETURNER_RENDERMAN_H
#define CPM_TOMIE_PAGETURNER_RENDERMAN_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PAGETURNER_MAX_VERTICES 256
#define PAGETURNER_MAX_PAGES 4
#define CPM_TPA_BASE 0x0100

typedef struct {
    float x, y, z;
} PageTurnerVec3;

typedef struct {
    PageTurnerVec3 rest_pos;
    PageTurnerVec3 deformed_pos;
    PageTurnerVec3 normal;
    float u, v;
    float curl_weight;
} PageTurnerVertex;

typedef struct {
    uint32_t page_number;
    uint32_t text_bytes;
    float curl_angle_rad;
    float page_thickness_mm;
    float roughness;
    float anisotropic_specular;
    uint32_t num_vertices;
    PageTurnerVertex vertices[PAGETURNER_MAX_VERTICES];
} PageTurnerPageMesh;

typedef struct {
    float room_temperature_c;
    float relative_humidity_pct;
    float co2_ppm;
    float hvac_energy_kw;
    float bim_anchor_x;
    float bim_anchor_y;
    float bim_anchor_z;
    uint32_t timestamp_epoch;
} DigitalTwinTelemetryNode;

typedef struct {
    uint32_t active_page;
    uint32_t total_pages;
    float turn_progress; /* 0.0 to 1.0 */
    float clay_plasticity;
    float displacement_gain;
    PageTurnerPageMesh pages[PAGETURNER_MAX_PAGES];
    DigitalTwinTelemetryNode twin_sensor;
} PageTurnerRenderManEngine;

void pageturner_init(PageTurnerRenderManEngine *engine, uint32_t total_pages);
bool pageturner_set_digital_twin_telemetry(PageTurnerRenderManEngine *engine, float temp, float hum, float co2, float kw, float x, float y, float z);
bool pageturner_apply_clay_curl(PageTurnerRenderManEngine *engine, uint32_t page_idx, float turn_progress);
bool pageturner_apply_displacement_shader(PageTurnerRenderManEngine *engine, uint32_t page_idx);
uint32_t pageturner_compute_rule18_checksum(const PageTurnerRenderManEngine *engine);

#endif /* CPM_TOMIE_PAGETURNER_RENDERMAN_H */
