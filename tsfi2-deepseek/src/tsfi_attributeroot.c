#include "tsfi_attributeroot.h"
#include <string.h>

void tsfi_attributeroot_init(TSFiAttributeRoot *attr, const char *name, const char *type, float default_val) {
    if (!attr) return;
    
    if (name) {
        strncpy(attr->name, name, sizeof(attr->name) - 1);
    } else {
        strcpy(attr->name, "defaultAttr");
    }
    
    if (type) {
        strncpy(attr->type_name, type, sizeof(attr->type_name) - 1);
    } else {
        strcpy(attr->type_name, "float");
    }
    
    attr->default_value = default_val;
    attr->sample_count = 0;
    memset(attr->samples, 0, sizeof(attr->samples));
}

bool tsfi_attributeroot_set_time_sample(TSFiAttributeRoot *attr, float time, float value) {
    if (!attr || attr->sample_count >= TSFI_MAX_TIME_SAMPLES) return false;
    
    // Sort insertion to keep samples ordered by time
    int insert_idx = attr->sample_count;
    for (int i = 0; i < attr->sample_count; i++) {
        if (attr->samples[i].time == time) {
            attr->samples[i].value = value;
            return true;
        }
        if (attr->samples[i].time > time) {
            insert_idx = i;
            break;
        }
    }
    
    // Shift elements to make space
    for (int i = attr->sample_count; i > insert_idx; i--) {
        attr->samples[i] = attr->samples[i - 1];
    }
    
    attr->samples[insert_idx].time = time;
    attr->samples[insert_idx].value = value;
    attr->sample_count++;
    return true;
}

float tsfi_attributeroot_get_value(const TSFiAttributeRoot *attr, float time) {
    if (!attr) return 0.0f;
    if (attr->sample_count == 0) return attr->default_value;
    
    // Edge cases
    if (time <= attr->samples[0].time) return attr->samples[0].value;
    if (time >= attr->samples[attr->sample_count - 1].time) {
        return attr->samples[attr->sample_count - 1].value;
    }
    
    // Locate segment for linear interpolation
    for (int i = 0; i < attr->sample_count - 1; i++) {
        if (time >= attr->samples[i].time && time <= attr->samples[i + 1].time) {
            float t0 = attr->samples[i].time;
            float t1 = attr->samples[i + 1].time;
            float v0 = attr->samples[i].value;
            float v1 = attr->samples[i + 1].value;
            
            float ratio = (time - t0) / (t1 - t0);
            return v0 + ratio * (v1 - v0);
        }
    }
    
    return attr->default_value;
}
