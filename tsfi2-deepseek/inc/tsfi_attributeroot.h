#ifndef TSFI_ATTRIBUTEROOT_H
#define TSFI_ATTRIBUTEROOT_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_TIME_SAMPLES 8

typedef struct {
    float time;
    float value;
} TSFiTimeSample;

typedef struct {
    char name[32];
    char type_name[16]; // e.g. "float", "int"
    float default_value;
    
    // Time-sampled keyframes list
    TSFiTimeSample samples[TSFI_MAX_TIME_SAMPLES];
    int sample_count;
} TSFiAttributeRoot;

// Initialize the AttributeRoot context
void tsfi_attributeroot_init(TSFiAttributeRoot *attr, const char *name, const char *type, float default_val);

// Adds a time-varying keyframed sample to the attribute
bool tsfi_attributeroot_set_time_sample(TSFiAttributeRoot *attr, float time, float value);

// Evaluates the attribute value at a specific time code using linear interpolation
float tsfi_attributeroot_get_value(const TSFiAttributeRoot *attr, float time);

#endif // TSFI_ATTRIBUTEROOT_H
