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

typedef struct {
    float scale;
    float stuffing;
    float arm_length;
    float leg_length;
    float head_size;
    float ear_size;
    float sickness;
    int missing_eye;
    float lighting_angle;
} usd_AuncientTeddyBearSchema_t;

static inline void usd_init_AuncientTeddyBearSchema(usd_AuncientTeddyBearSchema_t *obj) {
    obj->scale = 1.0f;
    obj->stuffing = 1.5f;
    obj->arm_length = 1.0f;
    obj->leg_length = 1.0f;
    obj->head_size = 1.0f;
    obj->ear_size = 1.0f;
    obj->sickness = 0.0f;
    obj->missing_eye = 0;
    obj->lighting_angle = 135.0f;
}

static inline float usd_physics_api_get_scale(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->scale;
}

static inline void usd_physics_api_set_scale(usd_AuncientTeddyBearSchema_t *obj, float val) {
    obj->scale = val;
}

static inline float usd_physics_api_get_stuffing(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->stuffing;
}

static inline void usd_physics_api_set_stuffing(usd_AuncientTeddyBearSchema_t *obj, float val) {
    obj->stuffing = val;
}

static inline float usd_physics_api_get_arm_length(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->arm_length;
}

static inline void usd_physics_api_set_arm_length(usd_AuncientTeddyBearSchema_t *obj, float val) {
    obj->arm_length = val;
}

static inline float usd_physics_api_get_leg_length(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->leg_length;
}

static inline void usd_physics_api_set_leg_length(usd_AuncientTeddyBearSchema_t *obj, float val) {
    obj->leg_length = val;
}

static inline float usd_physics_api_get_head_size(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->head_size;
}

static inline void usd_physics_api_set_head_size(usd_AuncientTeddyBearSchema_t *obj, float val) {
    obj->head_size = val;
}

static inline float usd_physics_api_get_ear_size(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->ear_size;
}

static inline void usd_physics_api_set_ear_size(usd_AuncientTeddyBearSchema_t *obj, float val) {
    obj->ear_size = val;
}

static inline float usd_physics_api_get_sickness(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->sickness;
}

static inline void usd_physics_api_set_sickness(usd_AuncientTeddyBearSchema_t *obj, float val) {
    obj->sickness = val;
}

static inline int usd_physics_api_get_missing_eye(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->missing_eye;
}

static inline void usd_physics_api_set_missing_eye(usd_AuncientTeddyBearSchema_t *obj, int val) {
    obj->missing_eye = val;
}

static inline float usd_physics_api_get_lighting_angle(const usd_AuncientTeddyBearSchema_t *obj) {
    return obj->lighting_angle;
}

static inline void usd_physics_api_set_lighting_angle(usd_AuncientTeddyBearSchema_t *obj, float val) {
    obj->lighting_angle = val;
}

#endif /* AUNCIENT_CACTUS_SCHEMA_H */
