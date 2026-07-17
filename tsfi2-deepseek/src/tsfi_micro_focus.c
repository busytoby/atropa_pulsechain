#include "tsfi_micro_focus.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// COMP-5 Encoding: Native binary representation (little-endian on x86/ARM)
int tsfi_mf_comp5_encode(int64_t value, uint8_t *buffer, int size_bytes) {
    if (!buffer || (size_bytes != 1 && size_bytes != 2 && size_bytes != 4 && size_bytes != 8)) {
        return -1;
    }
    // Perform native store
    for (int i = 0; i < size_bytes; i++) {
        buffer[i] = (uint8_t)((value >> (8 * i)) & 0xFF);
    }
    return 0;
}

// COMP-5 Decoding: Native binary representation
int64_t tsfi_mf_comp5_decode(const uint8_t *buffer, int size_bytes, int is_signed) {
    if (!buffer || (size_bytes != 1 && size_bytes != 2 && size_bytes != 4 && size_bytes != 8)) {
        return 0;
    }
    uint64_t raw_val = 0;
    for (int i = 0; i < size_bytes; i++) {
        raw_val |= ((uint64_t)buffer[i] << (8 * i));
    }

    if (is_signed) {
        if (size_bytes == 1) return (int8_t)raw_val;
        if (size_bytes == 2) return (int16_t)raw_val;
        if (size_bytes == 4) return (int32_t)raw_val;
        return (int64_t)raw_val;
    }
    return (int64_t)raw_val;
}

// ISAM Record Writer: Write keys and values sequentially to .dat.bin files
int tsfi_mf_isam_write_record(const char *filepath_dat_bin, uint32_t key, const uint8_t *record_data, int record_len) {
    if (!filepath_dat_bin || !record_data || record_len <= 0) return -1;

    // Enforce Rule 13: Only allow .dat.bin extension
    int len = strlen(filepath_dat_bin);
    if (len < 8 || strcmp(filepath_dat_bin + len - 8, ".dat.bin") != 0) {
        return -2;
    }

    FILE *f = fopen(filepath_dat_bin, "ab");
    if (!f) return -3;

    // Write header: 4-byte Key, 4-byte length
    fwrite(&key, 1, 4, f);
    fwrite(&record_len, 1, 4, f);
    fwrite(record_data, 1, record_len, f);

    fclose(f);
    return 0;
}

// ISAM Record Reader: Search keys sequentially from .dat.bin files
int tsfi_mf_isam_read_record(const char *filepath_dat_bin, uint32_t key, uint8_t *record_out, int record_len) {
    if (!filepath_dat_bin || !record_out || record_len <= 0) return -1;

    // Enforce Rule 13
    int len = strlen(filepath_dat_bin);
    if (len < 8 || strcmp(filepath_dat_bin + len - 8, ".dat.bin") != 0) {
        return -2;
    }

    FILE *f = fopen(filepath_dat_bin, "rb");
    if (!f) return -3;

    uint32_t cur_key = 0;
    int cur_len = 0;

    while (fread(&cur_key, 1, 4, f) == 4) {
        if (fread(&cur_len, 1, 4, f) != 4) {
            fclose(f);
            return -4;
        }

        if (cur_key == key) {
            int to_read = (cur_len < record_len) ? cur_len : record_len;
            if (fread(record_out, 1, to_read, f) != (size_t)to_read) {
                fclose(f);
                return -5;
            }
            fclose(f);
            return 0;
        } else {
            // Seek past payload
            if (fseek(f, cur_len, SEEK_CUR) != 0) {
                fclose(f);
                return -6;
            }
        }
    }

    fclose(f);
    return -7; // Not found
}

#include "tsfi_ray_tracer.h"

int tsfi_mf_sif_parse(const char *sif_line, tsfi_cgm_scene *scene) {
    if (!sif_line || !scene) return -1;

    if (strncmp(sif_line, "SIF_SPHERE", 10) == 0) {
        float x = 0.0f, y = 0.0f, z = 0.0f, rad = 1.0f;
        char col_char = 'R';
        int parsed = sscanf(sif_line, "SIF_SPHERE X:%f Y:%f Z:%f R:%f COLOR:%c", &x, &y, &z, &rad, &col_char);
        if (parsed < 5) return -2;

        tsfi_rt_vec3 color = {1.0f, 0.0f, 0.0f}; // Default red
        if (col_char == 'G') color = (tsfi_rt_vec3){0.0f, 1.0f, 0.0f};
        else if (col_char == 'B') color = (tsfi_rt_vec3){0.0f, 0.0f, 1.0f};

        return tsfi_cgm_scene_add_primitive(scene, CGM_PRIM_SPHERE, (tsfi_rt_vec3){x, y, z}, color, rad, (tsfi_rt_vec3){0,0,0});
    }
    if (strncmp(sif_line, "SIF_PLANE", 9) == 0) {
        float nx = 0.0f, ny = 1.0f, nz = 0.0f, d = 0.0f;
        char col_char = 'R';
        int parsed = sscanf(sif_line, "SIF_PLANE NX:%f NY:%f NZ:%f D:%f COLOR:%c", &nx, &ny, &nz, &d, &col_char);
        if (parsed < 5) return -2;

        tsfi_rt_vec3 color = {1.0f, 0.0f, 0.0f}; // Default red
        if (col_char == 'G') color = (tsfi_rt_vec3){0.0f, 1.0f, 0.0f};
        else if (col_char == 'B') color = (tsfi_rt_vec3){0.0f, 0.0f, 1.0f};

        return tsfi_cgm_scene_add_primitive(scene, CGM_PRIM_PLANE, (tsfi_rt_vec3){nx, ny, nz}, color, d, (tsfi_rt_vec3){0,0,0});
    }
    return -2;
}

int tsfi_mf_screen_section_render(const char *screen_def, char *terminal_buffer, int width, int height) {
    if (!screen_def || !terminal_buffer || width <= 0 || height <= 0) return -1;

    memset(terminal_buffer, ' ', width * height);

    char def_copy[256];
    strncpy(def_copy, screen_def, sizeof(def_copy) - 1);
    def_copy[sizeof(def_copy) - 1] = '\0';

    int line = 1, col = 1;
    char value[128] = {0};

    int parsed = sscanf(def_copy, "LINE:%d COL:%d VALUE:%[^\n]", &line, &col, value);
    if (parsed >= 3) {
        int start_r = line - 1;
        int start_c = col - 1;

        if (start_r >= 0 && start_r < height && start_c >= 0 && start_c < width) {
            int val_len = strlen(value);
            int write_len = (start_c + val_len > width) ? (width - start_c) : val_len;
            memcpy(&terminal_buffer[start_r * width + start_c], value, write_len);
            return 0;
        }
    }
    return -2;
}

int tsfi_mf_sif_serialize(const tsfi_cgm_scene *scene, char *out_buffer, size_t max_len) {
    if (!scene || !out_buffer || max_len == 0) return -1;

    out_buffer[0] = '\0';
    size_t current_len = 0;

    for (int i = 0; i < scene->primitive_count; i++) {
        const tsfi_cgm_primitive *p = &scene->primitives[i];
        char line[256];
        char col_char = 'R';
        if (p->color.y > 0.5f) col_char = 'G';
        else if (p->color.z > 0.5f) col_char = 'B';

        if (p->type == CGM_PRIM_SPHERE) {
            snprintf(line, sizeof(line), "SIF_SPHERE X:%.1f Y:%.1f Z:%.1f R:%.1f COLOR:%c\n",
                     p->position.x, p->position.y, p->position.z, p->param1, col_char);
        } else if (p->type == CGM_PRIM_PLANE) {
            snprintf(line, sizeof(line), "SIF_PLANE NX:%.1f NY:%.1f NZ:%.1f D:%.1f COLOR:%c\n",
                     p->position.x, p->position.y, p->position.z, p->param1, col_char);
        } else {
            continue;
        }

        size_t line_len = strlen(line);
        if (current_len + line_len + 1 > max_len) {
            return -2;
        }

        strcat(out_buffer, line);
        current_len += line_len;
    }

    return 0;
}
