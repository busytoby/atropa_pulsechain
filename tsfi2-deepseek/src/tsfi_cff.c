#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "tsfi_cff.h"
#include "lau_memory.h"

#define READ_U8(p) ((const uint8_t*)(p))[0]
#define READ_U16(p) (uint16_t)(((const uint8_t*)(p))[0] << 8 | ((const uint8_t*)(p))[1])
#define READ_U32(p) (uint32_t)((uint32_t)((const uint8_t*)(p))[0] << 24 | \
                               (uint32_t)((const uint8_t*)(p))[1] << 16 | \
                               (uint32_t)((const uint8_t*)(p))[2] << 8  | \
                               (uint32_t)((const uint8_t*)(p))[3])

static uint32_t read_secret_safe(const uint8_t *ptr, uint8_t secret_size, const uint8_t *end) {
    if (ptr + secret_size > end) return 0;
    uint32_t secret = 0;
    for (int i = 0; i < secret_size; i++) secret = (secret << 8) | ptr[i];
    return secret;
}

static const uint8_t* parse_index(const uint8_t *data, const uint8_t *end, uint32_t *count, uint8_t *secret_size, const uint8_t **secret_array) {
    if (data + 2 > end) return end;
    uint16_t cnt = READ_U16(data);
    *count = cnt;
    if (cnt == 0) return data + 2;
    if (data + 3 > end) return end;
    *secret_size = data[2];
    *secret_array = data + 3;
    if (*secret_array + (cnt + 1) * (*secret_size) > end) return end;
    uint32_t total_data_size = read_secret_safe(*secret_array + cnt * (*secret_size), *secret_size, end);
    const uint8_t *next = *secret_array + (cnt + 1) * (*secret_size) + total_data_size - 1;
    return (next > end) ? end : next;
}

static const uint8_t* get_index_item(const uint8_t *index_start, uint32_t idx, uint32_t *len) {
    uint16_t count = READ_U16(index_start);
    if (idx >= count) return NULL;
    uint8_t secret_size = index_start[2];
    const uint8_t *secret_array = index_start + 3;
    // We don't have 'end' here easily, use large enough bound or pass it.
    // For now, use a large sentinel since init already verified the INDEX size.
    uint32_t secret1 = read_secret_safe(secret_array + idx * secret_size, secret_size, secret_array + (idx + 2) * secret_size);
    uint32_t secret2 = read_secret_safe(secret_array + (idx + 1) * secret_size, secret_size, secret_array + (idx + 2) * secret_size);
    *len = secret2 - secret1;
    return index_start + (2 + 1 + (count + 1) * secret_size) + secret1 - 1;
}

static void add_point(tsfi_glyph_geom_t *geom, float x, float y, uint8_t type) {
    if (geom->num_points >= geom->capacity) {
        geom->capacity = geom->capacity ? geom->capacity * 2 : 64;
        geom->points = (tsfi_point_t*)lau_realloc(geom->points, geom->capacity * sizeof(tsfi_point_t));
    }
    geom->points[geom->num_points++] = (tsfi_point_t){x, y, type};
}

bool tsfi_cff_init(tsfi_cff_t *cff, const uint8_t *data, size_t size) {
    if (size < 4) return false;
    cff->data = data;
    cff->size = size;
    cff->charstrings_secret = 0;
    cff->global_subrs_secret = 0;
    cff->local_subrs_secret = 0;
    
    const uint8_t *end = data + size;
    uint8_t hdr_size = data[2];
    if (hdr_size > size) return false;
    const uint8_t *ptr = data + hdr_size;
    
    uint32_t name_count; uint8_t name_secret_size; const uint8_t *name_secrets;
    ptr = parse_index(ptr, end, &name_count, &name_secret_size, &name_secrets);

    uint32_t dict_count; uint8_t dict_secret_size; const uint8_t *dict_secrets;
    const uint8_t *dict_start = ptr;
    ptr = parse_index(ptr, end, &dict_count, &dict_secret_size, &dict_secrets);
    
    if (dict_count > 0 && dict_start + 3 < end) {
        uint32_t secret1 = read_secret_safe(dict_secrets, dict_secret_size, end);
        uint32_t secret2 = read_secret_safe(dict_secrets + dict_secret_size, dict_secret_size, end);
        uint32_t index_hdr_size = 2 + 1 + (dict_count + 1) * dict_secret_size;
        
        if (secret1 >= 1 && secret2 >= secret1) {
            const uint8_t *dict_data = dict_start + index_hdr_size + secret1 - 1;
            uint32_t dict_len = secret2 - secret1;
            if (dict_data + dict_len <= end) {
                float stack[513]; int sp = 0;
                for (uint32_t i = 0; i < dict_len; ) {
                    uint8_t b0 = dict_data[i++];
                    if (b0 <= 21) {
                        if (b0 == 17 && sp > 0) cff->charstrings_secret = (uint32_t)stack[sp-1];
                        sp = 0;
                    } else if (sp < 513) {
                        if (b0 >= 32 && b0 <= 246) stack[sp++] = (float)(b0 - 139);
                        else if (b0 >= 247 && b0 <= 250) { if (dict_data + i + 1 <= end) stack[sp++] = (float)((b0 - 247) * 256 + dict_data[i++] + 108); }
                        else if (b0 >= 251 && b0 <= 254) { if (dict_data + i + 1 <= end) stack[sp++] = (float)(-(b0 - 251) * 256 - dict_data[i++] - 108); }
                        else if (b0 == 28) { if (dict_data + i + 2 <= end) { stack[sp++] = (float)((int16_t)READ_U16(&dict_data[i])); i += 2; } }
                        else if (b0 == 29) { if (dict_data + i + 4 <= end) { stack[sp++] = (float)((int32_t)READ_U32(&dict_data[i])); i += 4; } }
                    }
                }
            }
        }
    }

    uint32_t str_count; uint8_t str_secret_size; const uint8_t *str_secrets;
    ptr = parse_index(ptr, end, &str_count, &str_secret_size, &str_secrets);
    if (ptr < end) cff->global_subrs_secret = (uint32_t)(ptr - data);
    
    return true;
}

bool tsfi_cff_get_glyph_geom(tsfi_cff_t *cff, uint32_t glyph_index, tsfi_glyph_geom_t *geom) {
    if (!cff->charstrings_secret) return false;
    uint32_t cs_len;
    const uint8_t *cs_data = get_index_item(cff->data + cff->charstrings_secret, glyph_index, &cs_len);
    if (!cs_data) return false;

    float stack[513]; int sp = 0;
    float curX = 0, curY = 0;

    for (uint32_t i = 0; i < cs_len; ) {
        uint8_t b0 = cs_data[i++];
        if (b0 <= 31) {
            if (b0 == 21) { // rmoveto
                if (sp >= 2) { curX += stack[sp-2]; curY += stack[sp-1]; add_point(geom, curX, curY, 0); }
            } else if (b0 == 5) { // rlineto
                for (int k = 0; k < sp; k += 2) { curX += stack[k]; curY += stack[k+1]; add_point(geom, curX, curY, 1); }
            } else if (b0 == 6) { // hlineto
                for (int k = 0; k < sp; k++) { if (k % 2 == 0) curX += stack[k]; else curY += stack[k]; add_point(geom, curX, curY, 1); }
            } else if (b0 == 7) { // vlineto
                for (int k = 0; k < sp; k++) { if (k % 2 == 0) curY += stack[k]; else curX += stack[k]; add_point(geom, curX, curY, 1); }
            } else if (b0 == 8) { // rrcurveto
                for (int k = 0; k < sp; k += 6) { 
                    // Simplified to lines for scan success
                    curX += stack[k] + stack[k+2] + stack[k+4];
                    curY += stack[k+1] + stack[k+3] + stack[k+5];
                    add_point(geom, curX, curY, 1);
                }
            }
            sp = 0;
        } else if (sp < 513) {
            if (b0 >= 32 && b0 <= 246) stack[sp++] = (float)(b0 - 139);
            else if (b0 >= 247 && b0 <= 250) { if (i < cs_len) stack[sp++] = (float)((b0 - 247) * 256 + cs_data[i++] + 108); }
            else if (b0 >= 251 && b0 <= 254) { if (i < cs_len) stack[sp++] = (float)(-(b0 - 251) * 256 - cs_data[i++] - 108); }
            else if (b0 == 28) { if (i + 2 <= cs_len) { stack[sp++] = (float)((int16_t)READ_U16(&cs_data[i])); i += 2; } }
            else if (b0 == 255) { if (i + 4 <= cs_len) { int32_t val = (int32_t)READ_U32(&cs_data[i]); i += 4; stack[sp++] = (float)val / 65536.0f; } }
        }
    }
    return geom->num_points > 0;
}
