#include "tsfi_usdshade.h"
#include <string.h>
#include <math.h>

void tsfi_usdshade_init(TSFiUsdShade *shade, const char *material_id) {
    if (!shade) return;
    
    if (material_id) {
        strncpy(shade->material_id, material_id, sizeof(shade->material_id) - 1);
    } else {
        strcpy(shade->material_id, "/auncient/material/default");
    }
    
    shade->albedo[0] = 0.8f;
    shade->albedo[1] = 0.8f;
    shade->albedo[2] = 0.8f;
    shade->roughness = 0.5f;
    shade->metallic = 0.0f;
    
    // Initialize the cloth-simulating FET node grid
    for (int r = 0; r < TSFI_FET_GRID_SIZE; r++) {
        for (int c = 0; c < TSFI_FET_GRID_SIZE; c++) {
            TSFiFetDischargeNode *node = &shade->nodes[r][c];
            node->position[0] = (float)c * 1.5f;
            node->position[1] = (float)r * 1.5f;
            node->position[2] = 0.0f;
            
            node->prev_position[0] = node->position[0];
            node->prev_position[1] = node->position[1];
            node->prev_position[2] = node->position[2];
            
            node->charge_level = 1.0f;
            node->mass = 1.0f;
        }
    }
}

void tsfi_usdshade_step_discharge(TSFiUsdShade *shade, float delta_time) {
    if (!shade || delta_time <= 0.0f) return;
    
    // 1. Verlet integration for nodes in the FET discharge cycle grid
    for (int r = 0; r < TSFI_FET_GRID_SIZE; r++) {
        for (int c = 0; c < TSFI_FET_GRID_SIZE; c++) {
            TSFiFetDischargeNode *node = &shade->nodes[r][c];
            
            float temp_x = node->position[0];
            float temp_y = node->position[1];
            float temp_z = node->position[2];
            
            // Accelerations from electrostatic charge repulsion forces
            float acc_z = node->charge_level * 9.8f;
            
            node->position[0] += (node->position[0] - node->prev_position[0]);
            node->position[1] += (node->position[1] - node->prev_position[1]);
            node->position[2] += (node->position[2] - node->prev_position[2]) + acc_z * delta_time * delta_time;
            
            node->prev_position[0] = temp_x;
            node->prev_position[1] = temp_y;
            node->prev_position[2] = temp_z;
        }
    }
    
    // 2. Solve structural spring constraints between adjacent nodes (mass-spring cloth model)
    for (int r = 0; r < TSFI_FET_GRID_SIZE; r++) {
        for (int c = 0; c < TSFI_FET_GRID_SIZE; c++) {
            TSFiFetDischargeNode *n1 = &shade->nodes[r][c];
            
            // Check structural neighbors (Right and Down)
            if (c + 1 < TSFI_FET_GRID_SIZE) {
                TSFiFetDischargeNode *n2 = &shade->nodes[r][c + 1];
                float dx = n2->position[0] - n1->position[0];
                float dy = n2->position[1] - n1->position[1];
                float dz = n2->position[2] - n1->position[2];
                float dist = sqrtf(dx*dx + dy*dy + dz*dz);
                float rest_len = 1.5f;
                if (dist > 1e-4f) {
                    float diff = (rest_len - dist) / dist * 0.5f;
                    float displacement_x = dx * diff;
                    float displacement_y = dy * diff;
                    float displacement_z = dz * diff;
                    
                    n1->position[0] -= displacement_x;
                    n1->position[1] -= displacement_y;
                    n1->position[2] -= displacement_z;
                    
                    n2->position[0] += displacement_x;
                    n2->position[1] += displacement_y;
                    n2->position[2] += displacement_z;
                }
            }
            if (r + 1 < TSFI_FET_GRID_SIZE) {
                TSFiFetDischargeNode *n2 = &shade->nodes[r + 1][c];
                float dx = n2->position[0] - n1->position[0];
                float dy = n2->position[1] - n1->position[1];
                float dz = n2->position[2] - n1->position[2];
                float dist = sqrtf(dx*dx + dy*dy + dz*dz);
                float rest_len = 1.5f;
                if (dist > 1e-4f) {
                    float diff = (rest_len - dist) / dist * 0.5f;
                    float displacement_x = dx * diff;
                    float displacement_y = dy * diff;
                    float displacement_z = dz * diff;
                    
                    n1->position[0] -= displacement_x;
                    n1->position[1] -= displacement_y;
                    n1->position[2] -= displacement_z;
                    
                    n2->position[0] += displacement_x;
                    n2->position[1] += displacement_y;
                    n2->position[2] += displacement_z;
                }
            }
        }
    }
}

void tsfi_usdshade_bind_registers(TSFiUsdShade *shade, float base_charge, float decay_rate) {
    if (!shade) return;
    
    float total_charge = 0.0f;
    for (int r = 0; r < TSFI_FET_GRID_SIZE; r++) {
        for (int c = 0; c < TSFI_FET_GRID_SIZE; c++) {
            TSFiFetDischargeNode *node = &shade->nodes[r][c];
            node->charge_level = base_charge * expf(-decay_rate * (float)(r + c));
            total_charge += node->charge_level;
        }
    }
    
    // Dynamic material parameter adjustment based on simulated grid charge states
    float normalized_charge = total_charge / (float)(TSFI_FET_GRID_SIZE * TSFI_FET_GRID_SIZE);
    shade->albedo[0] = 0.2f + 0.8f * normalized_charge;
    shade->albedo[1] = 0.1f + 0.9f * normalized_charge;
    shade->albedo[2] = 0.3f + 0.7f * normalized_charge;
    shade->roughness = 1.0f - normalized_charge;
    shade->metallic = normalized_charge;
}
