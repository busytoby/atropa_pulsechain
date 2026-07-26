/* Generated dynamically by Auncient usd_gen_schema tool. Do not edit. */
#ifndef AUNCIENT_CACTUS_SCHEMA_H
#define AUNCIENT_CACTUS_SCHEMA_H

#include <string.h>

typedef struct {
    float density;
} usd_auncient_cactus_schema_t;

static inline void usd_init_auncient_cactus_schema(usd_auncient_cactus_schema_t *obj) {
    obj->density = 1.0f;
}

static inline float usd_cactus_schema_get_density(const usd_auncient_cactus_schema_t *obj) {
    return obj->density;
}

static inline void usd_cactus_schema_set_density(usd_auncient_cactus_schema_t *obj, float val) {
    obj->density = val;
}

typedef struct {
    float stiffness;
    char texture[32];
} usd_auncient_texture_api_t;

static inline void usd_init_auncient_texture_api(usd_auncient_texture_api_t *obj) {
    obj->stiffness = 0.5f;
    strcpy(obj->texture, "cloth");
}

static inline float usd_texture_api_get_stiffness(const usd_auncient_texture_api_t *obj) {
    return obj->stiffness;
}

static inline void usd_texture_api_set_stiffness(usd_auncient_texture_api_t *obj, float val) {
    obj->stiffness = val;
}

typedef struct {
    float mass;
    float damping;
} usd_auncient_physics_api_t;

static inline void usd_init_auncient_physics_api(usd_auncient_physics_api_t *obj) {
    obj->mass = 10.0f;
    obj->damping = 0.1f;
}

static inline float usd_physics_api_get_mass(const usd_auncient_physics_api_t *obj) {
    return obj->mass;
}

static inline void usd_physics_api_set_mass(usd_auncient_physics_api_t *obj, float val) {
    obj->mass = val;
}

static inline float usd_physics_api_get_damping(const usd_auncient_physics_api_t *obj) {
    return obj->damping;
}

static inline void usd_physics_api_set_damping(usd_auncient_physics_api_t *obj, float val) {
    obj->damping = val;
}

typedef struct {
    float write_gate;
    float read_gate;
    float allocation_gate;
} usd_auncient_mann_api_t;

static inline void usd_init_auncient_mann_api(usd_auncient_mann_api_t *obj) {
    obj->write_gate = 0.5f;
    obj->read_gate = 0.5f;
    obj->allocation_gate = 0.5f;
}

static inline float usd_mann_api_get_write_gate(const usd_auncient_mann_api_t *obj) {
    return obj->write_gate;
}

static inline void usd_mann_api_set_write_gate(usd_auncient_mann_api_t *obj, float val) {
    obj->write_gate = val;
}

static inline float usd_mann_api_get_read_gate(const usd_auncient_mann_api_t *obj) {
    return obj->read_gate;
}

static inline void usd_mann_api_set_read_gate(usd_auncient_mann_api_t *obj, float val) {
    obj->read_gate = val;
}

static inline float usd_mann_api_get_allocation_gate(const usd_auncient_mann_api_t *obj) {
    return obj->allocation_gate;
}

static inline void usd_mann_api_set_allocation_gate(usd_auncient_mann_api_t *obj, float val) {
    obj->allocation_gate = val;
}

#endif /* AUNCIENT_CACTUS_SCHEMA_H */
