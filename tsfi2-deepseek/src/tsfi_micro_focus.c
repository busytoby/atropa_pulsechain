#include "tsfi_micro_focus.h"
#include "tsfi_ramac_layout.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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

int tsfi_mf_stock_inventory_process(const char *raw_record, char *report_line) {
    if (!raw_record || !report_line) return -1;

    char id[5] = {0};
    char name[13] = {0};
    memcpy(id, raw_record, 4);
    memcpy(name, raw_record + 4, 12);

    uint32_t qty = tsfi_mf_comp5_decode((const uint8_t*)(raw_record + 16), 4, 0);
    uint32_t price = tsfi_mf_comp5_decode((const uint8_t*)(raw_record + 20), 4, 0);
    uint32_t value = qty * price;

    const char *status_str = "NORMAL";
    if (qty == 0) {
        status_str = "OUT OF STOCK";
    } else if (qty < 10) {
        status_str = "LOW STOCK";
    }

    sprintf(report_line, "ID:%s NAME:%-12s QTY:%04u VALUE:%08u STATUS:%s", id, name, qty, value, status_str);
    return 0;
}

int tsfi_mf_sales_commission_process(const char *raw_record, char *report_line) {
    if (!raw_record || !report_line) return -1;

    char id[5] = {0};
    char name[13] = {0};
    memcpy(id, raw_record, 4);
    memcpy(name, raw_record + 4, 12);

    uint32_t sales = tsfi_mf_comp5_decode((const uint8_t*)(raw_record + 16), 4, 0);

    uint32_t commission = 0;
    if (sales <= 10000) {
        commission = (sales * 5) / 100;
    } else {
        commission = ((10000 * 5) / 100) + (((sales - 10000) * 8) / 100);
    }

    sprintf(report_line, "REP:%s NAME:%-12s SALES:%06u COMM:%06u", id, name, sales, commission);
    return 0;
}

int tsfi_mf_calculate_diyat_tax(const char *event_code, uint32_t base_value, uint32_t *out_tax) {
    if (!event_code || !out_tax) return -1;

    if (strcmp(event_code, "FULL") == 0) {
        *out_tax = base_value;
        return 0;
    } else if (strcmp(event_code, "SEMI") == 0) {
        *out_tax = (base_value * 120) / 100;
        return 0;
    } else if (strcmp(event_code, "HALF") == 0) {
        *out_tax = base_value / 2;
        return 0;
    } else if (strcmp(event_code, "MUTE") == 0) {
        *out_tax = base_value / 10;
        return 0;
    }

    return -2;
}

int tsfi_diyat_yul_excise_gas_taxes(uint64_t gas_amount, const char *user_address_hex) {
    if (!user_address_hex) return -1;

    uint32_t tax_val = 0;
    int tax_res = tsfi_mf_calculate_diyat_tax("MUTE", (uint32_t)gas_amount, &tax_val);
    if (tax_res != 0) return -2;

    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF199, tax_val);

    extern _Thread_local uint64_t g_transaction_diyat_tax_total;
    g_transaction_diyat_tax_total += tax_val;

    return 0;
}

int tsfi_s370_yul_exec_bridge(const tsfi_uncol_instruction *program, int program_size) {
    if (!program || program_size <= 0) return -1;

    char yul_code[1024];
    int gen_res = tsfi_s370_uncol_to_yul(program, program_size, yul_code, sizeof(yul_code));
    if (gen_res != 0) return -2;

    printf("[S370 YUL BRIDGE] Compiled UNCOL to Yul bytecode:\n%s\n", yul_code);
    return 0;
}

int tsfi_mf_scsi_winchester_handshake(uint32_t keycode, uint8_t *storage_registers) {
    if (!storage_registers) return -1;

    // Emulate Auncient WinchesterMQ.yul hardware SCSI state machine handshake
    storage_registers[100] = 1; // REQ
    storage_registers[101] = keycode;

    if (storage_registers[100] == 1 && storage_registers[101] == keycode) {
        storage_registers[100] = 2; // ACK
        
        // Handle register modifications matching simulated keyboard state maps (Rule 5)
        uint16_t damping = storage_registers[103];
        uint16_t focal = storage_registers[104];
        if (damping == 0) damping = 100;
        if (focal == 0) focal = 230;

        if (keycode == 30) { // keycode 'a'
            damping = (damping > 10) ? damping - 10 : 0;
            focal = (focal > 10) ? focal - 10 : 0;
        } else if (keycode == 32) { // keycode 'd'
            damping = (damping < 250) ? damping + 10 : 250;
            focal = (focal < 250) ? focal + 10 : 250;
        }

        storage_registers[103] = damping;
        storage_registers[104] = focal;
        storage_registers[105]++; // Increment loop count
        storage_registers[100] = 0; // IDLE
    }

    return 0;
}

int tsfi_mf_dat_write_23_node(const char *filepath_dat_bin, uint32_t node_id, const uint8_t *payload, int len) {
    if (!filepath_dat_bin || !payload || len < 0) return -1;

    // Enforce Rule 13: Only support .dat.bin extension
    int filepath_len = strlen(filepath_dat_bin);
    if (filepath_len < 8 || strcmp(filepath_dat_bin + filepath_len - 8, ".dat.bin") != 0) {
        return -2; // Rejected
    }

    FILE *f = fopen(filepath_dat_bin, "wb");
    if (!f) return -3;

    fwrite(&node_id, sizeof(node_id), 1, f);
    fwrite(payload, 1, len, f);
    fclose(f);
    return 0;
}

int tsfi_mf_redirect_space_charge(uint64_t empirical_power, uint64_t *accumulator_state) {
    if (!accumulator_state) return -1;

    // Rule 12: Intercept empirical power law and redirect to non-preferential accumulator
    *accumulator_state += empirical_power;
    return 0;
}

int tsfi_mf_wessler_dsa_link(uint32_t *current_save_area, uint32_t *next_save_area) {
    if (!current_save_area || !next_save_area) return -1;

    uint32_t curr_addr = (uint32_t)(uintptr_t)current_save_area;
    uint32_t next_addr = (uint32_t)(uintptr_t)next_save_area;

    next_save_area[1] = curr_addr;
    current_save_area[2] = next_addr;

    return 0;
}

int tsfi_mf_cics_translate(const char *cics_statement, char *macro_call_out, int max_len) {
    if (!cics_statement || !macro_call_out || max_len <= 0) return -1;

    if (strstr(cics_statement, "EXEC CICS SEND TEXT") != NULL) {
        const char *from_ptr = strstr(cics_statement, "FROM(");
        const char *len_ptr = strstr(cics_statement, "LENGTH(");
        char from_val[64] = "MSG";
        char len_val[16] = "0";

        if (from_ptr) {
            sscanf(from_ptr, "FROM(%63[^)])", from_val);
        }
        if (len_ptr) {
            sscanf(len_ptr, "LENGTH(%15[^)])", len_val);
        }

        snprintf(macro_call_out, max_len, "DFHBMS TYPE=SEND,TEXT='%s',LEN=%s", from_val, len_val);
        return 0;
    }

    snprintf(macro_call_out, max_len, "DFHECALL");
    return 0;
}

int tsfi_mf_cics_getmain(int length, const char *suspend_type, uint8_t *storage_pool, uint32_t *allocated_offset) {
    if (length <= 0 || !storage_pool || !allocated_offset) return -1;

    uint32_t current_free = *allocated_offset;
    uint32_t needed_bytes = 8 + (uint32_t)length;

    uint8_t size_buf[4];
    tsfi_mf_comp5_encode((int64_t)needed_bytes, size_buf, 4);
    memcpy(storage_pool + current_free, size_buf, 4);

    uint32_t suspend_flag = 0;
    if (suspend_type && strcmp(suspend_type, "SUSPEND") == 0) {
        suspend_flag = 1;
    }
    uint8_t suspend_buf[4];
    tsfi_mf_comp5_encode((int64_t)suspend_flag, suspend_buf, 4);
    memcpy(storage_pool + current_free + 4, suspend_buf, 4);

    *allocated_offset = current_free + needed_bytes;
    return (int)(current_free + 8);
}

int tsfi_mf_cics_bms_cad_render(const char *map_name, const tsfi_cgm_scene *scene, char *terminal_buffer) {
    if (!map_name || !scene || !terminal_buffer) return -1;

    memset(terminal_buffer, ' ', 80 * 24);

    int header_offset = snprintf(terminal_buffer, 80, "%s BMS CAD SCREEN", map_name);
    terminal_buffer[header_offset] = ' ';

    for (int i = 0; i < scene->primitive_count; i++) {
        const tsfi_cgm_primitive *p = &scene->primitives[i];
        
        int line = 12 + (int)(p->position.y * 2.0f);
        int col = 40 + (int)(p->position.x * 4.0f);

        if (line >= 1 && line < 24 && col >= 0 && col < 80) {
            char symbol = 'O';
            if (p->type == CGM_PRIM_PLANE) symbol = '-';
            terminal_buffer[line * 80 + col] = symbol;
        }
    }

    return 0;
}

int tsfi_mf_cics_bms_marauder_map(const char *map_name, const uint32_t *active_node_coordinates, int node_count, char *terminal_buffer) {
    if (!map_name || !active_node_coordinates || !terminal_buffer || node_count < 0) return -1;

    memset(terminal_buffer, ' ', 80 * 24);

    int header_offset = snprintf(terminal_buffer, 80, "%s MARAUDER MAP", map_name);
    terminal_buffer[header_offset] = ' ';

    for (int i = 0; i < node_count; i++) {
        uint32_t val = active_node_coordinates[i];
        uint32_t line = (val >> 16) & 0xFFFF;
        uint32_t col = val & 0xFFFF;

        if (line >= 1 && line < 24 && col < 80) {
            terminal_buffer[line * 80 + col] = '*';
        }
    }

    return 0;
}

static const char TSFI_MAZE_GRID[8][8] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}
};

int tsfi_mf_cics_bms_first_person_render(float player_x, float player_y, float player_angle, char *terminal_buffer) {
    if (!terminal_buffer) return -1;

    memset(terminal_buffer, ' ', 80 * 24);
    float fov = 1.04719f;

    for (int col = 0; col < 80; col++) {
        float ray_angle = (player_angle - fov / 2.0f) + ((float)col / 80.0f) * fov;
        float distance = 0.0f;
        float max_distance = 16.0f;
        int hit_wall = 0;

        float dx = cosf(ray_angle);
        float dy = sinf(ray_angle);

        while (!hit_wall && distance < max_distance) {
            distance += 0.1f;
            int check_x = (int)(player_x + dx * distance);
            int check_y = (int)(player_y + dy * distance);

            if (check_x < 0 || check_x >= 8 || check_y < 0 || check_y >= 8) {
                hit_wall = 1;
                distance = max_distance;
            } else if (TSFI_MAZE_GRID[check_y][check_x]) {
                hit_wall = 1;
            }
        }

        float corrected_dist = distance * cosf(ray_angle - player_angle);
        if (corrected_dist < 0.1f) corrected_dist = 0.1f;

        int wall_height = (int)(24.0f / corrected_dist);
        if (wall_height > 24) wall_height = 24;

        int ceiling = (24 - wall_height) / 2;
        int floor = 24 - ceiling;

        char wall_char = '#';
        if (distance > 6.0f) wall_char = '=';
        else if (distance > 10.0f) wall_char = ':';

        for (int row = 0; row < 24; row++) {
            if (row < ceiling) {
                terminal_buffer[row * 80 + col] = '.';
            } else if (row >= ceiling && row < floor) {
                terminal_buffer[row * 80 + col] = wall_char;
            } else {
                terminal_buffer[row * 80 + col] = '_';
            }
        }
    }

    return 0;
}

int tsfi_mf_cics_bms_pmg_render(const uint8_t *pmg_base, int player_x, int player_y, char *terminal_buffer) {
    if (!pmg_base || !terminal_buffer) return -1;

    memset(terminal_buffer, ' ', 80 * 24);

    int header_offset = snprintf(terminal_buffer, 80, "CICS BMS PMG SPRITE DISPLAY");
    terminal_buffer[header_offset] = ' ';

    for (int y = 0; y < 16; y++) {
        uint8_t byte_val = pmg_base[y];
        int term_y = player_y + y;

        if (term_y >= 1 && term_y < 24) {
            for (int bit = 0; bit < 8; bit++) {
                int term_x = player_x + bit;
                if (term_x >= 0 && term_x < 80) {
                    if (byte_val & (1 << (7 - bit))) {
                        terminal_buffer[term_y * 80 + term_x] = 'X';
                    } else {
                        terminal_buffer[term_y * 80 + term_x] = '.';
                    }
                }
            }
        }
    }

    return 0;
}

static uint32_t tsfi_mf_hash_resource(const char *name) {
    uint32_t hash = 5381;
    int c;
    while ((c = (uint8_t)*name++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int tsfi_mf_cics_enq(const char *resource_name, uint32_t task_id, uint32_t *lock_table, int max_locks) {
    if (!resource_name || !lock_table || max_locks <= 0) return -1;

    uint32_t r_hash = tsfi_mf_hash_resource(resource_name);

    for (int i = 0; i < max_locks; i++) {
        if (lock_table[i * 2] == r_hash) {
            if (lock_table[i * 2 + 1] == task_id) {
                return 0;
            }
            return 1;
        }
    }

    for (int i = 0; i < max_locks; i++) {
        if (lock_table[i * 2] == 0) {
            lock_table[i * 2] = r_hash;
            lock_table[i * 2 + 1] = task_id;
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_cics_deq(const char *resource_name, uint32_t task_id, uint32_t *lock_table, int max_locks) {
    if (!resource_name || !lock_table || max_locks <= 0) return -1;

    uint32_t r_hash = tsfi_mf_hash_resource(resource_name);

    for (int i = 0; i < max_locks; i++) {
        if (lock_table[i * 2] == r_hash) {
            if (lock_table[i * 2 + 1] == task_id) {
                lock_table[i * 2] = 0;
                lock_table[i * 2 + 1] = 0;
                return 0;
            }
            return -2;
        }
    }

    return -3;
}

int tsfi_mf_zmachine_bms_room_render(uint32_t room_id, float player_angle, char *terminal_buffer) {
    if (!terminal_buffer) return -1;

    float px = 1.5f + (float)(room_id % 3);
    float py = 1.5f + (float)((room_id / 3) % 3);

    int render_res = tsfi_mf_cics_bms_first_person_render(px, py, player_angle, terminal_buffer);
    if (render_res != 0) return render_res;

    char hud[80];
    int hud_len = snprintf(hud, sizeof(hud), "ZMACHINE ROOM %u | VIEWPORT RESOLVED AT POS (%.1f, %.1f)", room_id, px, py);
    if (hud_len > 0 && hud_len < 80) {
        memcpy(terminal_buffer + 23 * 80, hud, hud_len);
    }

    return 0;
}

int tsfi_mf_majordomo_process(const char *cmd_line, char *out_response, int max_len) {
    if (!cmd_line || !out_response || max_len <= 0) return -1;

    if (strncmp(cmd_line, "subscribe", 9) == 0) {
        char list_name[64] = {0};
        char email[64] = {0};
        int parsed = sscanf(cmd_line, "subscribe %63s %63s", list_name, email);
        if (parsed == 2) {
            snprintf(out_response, max_len, "Succeeded subscribing %s to list %s.", email, list_name);
            return 0;
        }
    } else if (strncmp(cmd_line, "unsubscribe", 11) == 0) {
        char list_name[64] = {0};
        char email[64] = {0};
        int parsed = sscanf(cmd_line, "unsubscribe %63s %63s", list_name, email);
        if (parsed == 2) {
            snprintf(out_response, max_len, "Succeeded unsubscribing %s from list %s.", email, list_name);
            return 0;
        }
    } else if (strncmp(cmd_line, "who", 3) == 0) {
        char list_name[64] = {0};
        int parsed = sscanf(cmd_line, "who %63s", list_name);
        if (parsed == 1) {
            snprintf(out_response, max_len, "Members of list %s: owner@%s", list_name, list_name);
            return 0;
        }
    }

    snprintf(out_response, max_len, "Majordomo command not recognized.");
    return -2;
}

int tsfi_mf_majordomo_approve(uint32_t cookie, const char *action, uint32_t *pending_cookies, int max_pending) {
    if (!action || !pending_cookies || max_pending <= 0) return -1;

    for (int i = 0; i < max_pending; i++) {
        if (pending_cookies[i] == cookie) {
            if (strcmp(action, "APPROVE") == 0) {
                pending_cookies[i] = 0;
                return 0;
            } else if (strcmp(action, "REJECT") == 0) {
                pending_cookies[i] = 0;
                return 1;
            }
            return -2;
        }
    }

    return -3;
}

int tsfi_mf_majordomo_digest(const char **posts, int post_count, char *digest_out, int max_len) {
    if (!posts || post_count <= 0 || !digest_out || max_len <= 0) return -1;

    int offset = snprintf(digest_out, max_len, "MAJORDOMO DIGEST TABLE OF CONTENTS\n");
    if (offset < 0 || offset >= max_len) return -2;

    for (int i = 0; i < post_count; i++) {
        int len = snprintf(digest_out + offset, max_len - offset, "  [%d] Post Entry %d\n", i + 1, i + 1);
        if (len < 0 || offset + len >= max_len) return -2;
        offset += len;
    }

    int sep_len = snprintf(digest_out + offset, max_len - offset, "\n-------------------------------------\n\n");
    if (sep_len < 0 || offset + sep_len >= max_len) return -2;
    offset += sep_len;

    for (int i = 0; i < post_count; i++) {
        int len = snprintf(digest_out + offset, max_len - offset, "[[ Post Entry %d ]]\n%s\n\n", i + 1, posts[i]);
        if (len < 0 || offset + len >= max_len) return -2;
        offset += len;
    }

    return 0;
}

int tsfi_mf_wessler_macro_expand(const char *macro_line, char *hlasm_out, int max_len) {
    if (!macro_line || !hlasm_out || max_len <= 0) return -1;

    if (strstr(macro_line, "IF (") != NULL) {
        char reg1[16] = {0};
        char reg2[16] = {0};
        char cond[16] = {0};
        int parsed = sscanf(macro_line, "IF (%15[^,],%15[^,],%15[^)])", reg1, cond, reg2);
        if (parsed == 3) {
            int mask = 7;
            if (strcmp(cond, "NE") == 0) mask = 8;
            snprintf(hlasm_out, max_len, "CR %s,%s\nBC %d,L1", reg1, reg2, mask);
            return 0;
        }
    } else if (strcmp(macro_line, "ELSE") == 0) {
        snprintf(hlasm_out, max_len, "B L2\nL1 EQU *");
        return 0;
    } else if (strcmp(macro_line, "ENDIF") == 0) {
        snprintf(hlasm_out, max_len, "L2 EQU *");
        return 0;
    }

    snprintf(hlasm_out, max_len, "DS 0H");
    return -2;
}

int tsfi_mf_majordomo_restrict_post(const char *sender_email, const char **members, int member_count) {
    if (!sender_email || !members || member_count < 0) return -1;

    for (int i = 0; i < member_count; i++) {
        if (members[i] && strcmp(members[i], sender_email) == 0) {
            return 0;
        }
    }

    return 1;
}

int tsfi_mf_cics_handle_condition(uint32_t condition_id, uint64_t handler_addr, uint64_t *registry_table, int max_conditions) {
    if (!registry_table || max_conditions <= 0) return -1;

    for (int i = 0; i < max_conditions; i++) {
        if (registry_table[i * 2] == condition_id || registry_table[i * 2] == 0) {
            registry_table[i * 2] = condition_id;
            registry_table[i * 2 + 1] = handler_addr;
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_cics_raise_condition(uint32_t condition_id, uint64_t *registry_table, int max_conditions, uint64_t *handler_out) {
    if (!registry_table || max_conditions <= 0 || !handler_out) return -1;

    for (int i = 0; i < max_conditions; i++) {
        if (registry_table[i * 2] == condition_id) {
            *handler_out = registry_table[i * 2 + 1];
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_majordomo_info(const char *list_name, char *info_out, int max_len) {
    if (!list_name || !info_out || max_len <= 0) return -1;

    if (strcmp(list_name, "zmm-dev") == 0) {
        snprintf(info_out, max_len, "This list coordinates low-level Yul virtual memory development.");
        return 0;
    } else if (strcmp(list_name, "cics-admin") == 0) {
        snprintf(info_out, max_len, "Mailing list for CICS system programmers and administrators.");
        return 0;
    }

    snprintf(info_out, max_len, "Mailing list info not found.");
    return -2;
}

static _Thread_local char g_active_agent_handle[64] = "AuncientDefaultAgent";

int tsfi_mf_set_agent_handle(const char *handle) {
    if (!handle) return -1;
    strncpy(g_active_agent_handle, handle, sizeof(g_active_agent_handle) - 1);
    g_active_agent_handle[sizeof(g_active_agent_handle) - 1] = '\0';
    return 0;
}

const char *tsfi_mf_get_agent_handle(void) {
    return g_active_agent_handle;
}

int tsfi_mf_cics_link(const char *program_name, const uint8_t *commarea, int commarea_len, uint8_t *storage_pool, uint32_t *allocated_offset) {
    if (!program_name || !storage_pool || !allocated_offset || commarea_len < 0) return -1;

    uint32_t current_free = *allocated_offset;
    uint32_t needed_bytes = 12 + (uint32_t)commarea_len;

    uint8_t len_buf[4];
    tsfi_mf_comp5_encode((int64_t)commarea_len, len_buf, 4);
    memcpy(storage_pool + current_free, len_buf, 4);

    char prog_pad[8];
    memset(prog_pad, ' ', 8);
    int p_len = strlen(program_name);
    if (p_len > 8) p_len = 8;
    memcpy(prog_pad, program_name, p_len);
    memcpy(storage_pool + current_free + 4, prog_pad, 8);

    if (commarea && commarea_len > 0) {
        memcpy(storage_pool + current_free + 12, commarea, commarea_len);
    }

    *allocated_offset = current_free + needed_bytes;
    return (int)(current_free + 12);
}

int tsfi_mf_vulkan_appc_layout(uint32_t transaction_id, uint32_t state_flags, char *terminal_buffer) {
    if (!terminal_buffer) return -1;

    memset(terminal_buffer, ' ', 80 * 24);

    int offset = snprintf(terminal_buffer, 80, "VULKAN APPC SESSION LAYOUT | TRANS ID: %08X", transaction_id);
    terminal_buffer[offset] = ' ';

    const char *state_str = "RECEIVE";
    if (state_flags == 1) state_str = "SEND";
    else if (state_flags == 2) state_str = "CONFIRM";
    else if (state_flags == 3) state_str = "SYNCPOINT";

    char status[80];
    int status_len = snprintf(status, sizeof(status), "UX STATE: %s | MAJORDOMO INBOX: ACTIVE", state_str);
    if (status_len > 0 && status_len < 80) {
        memcpy(terminal_buffer + 5 * 80 + 10, status, status_len);
    }

    for (int col = 0; col < 80; col++) {
        terminal_buffer[2 * 80 + col] = '=';
        terminal_buffer[22 * 80 + col] = '=';
    }

    return 0;
}

int tsfi_mf_pmg_handle_collision(uint8_t player_id, uint8_t playfield_id, uint8_t *storage_pool, uint32_t *allocated_offset) {
    if (!storage_pool || !allocated_offset) return -1;

    uint8_t comm_block[2];
    comm_block[0] = player_id;
    comm_block[1] = playfield_id;

    return tsfi_mf_cics_link("COLDETCT", comm_block, 2, storage_pool, allocated_offset);
}

int tsfi_mf_majordomo_help(char *help_out, int max_len) {
    if (!help_out || max_len <= 0) return -1;

    snprintf(help_out, max_len,
             "MAJORDOMO HELP\n"
             "  subscribe <list> [<addr>]\n"
             "  unsubscribe <list> [<addr>]\n"
             "  info <list>\n"
             "  lists\n"
             "  help\n");
    return 0;
}

typedef struct {
    char dataset[16];
    char key[16];
    char value[64];
} TSFIVsamRecord;

static TSFIVsamRecord g_vsam_db[32] = {
    {"USERFILE", "K001", "NAME: J. WESSLER | DEPT: mainframe-systems"},
    {"USERFILE", "K002", "NAME: A. TURING   | DEPT: zmm-development"}
};
static int g_vsam_db_count = 2;

int tsfi_mf_cics_vsam_read(const char *dataset_name, const char *record_key, char *record_out, int max_len) {
    if (!dataset_name || !record_key || !record_out || max_len <= 0) return -1;

    for (int i = 0; i < g_vsam_db_count; i++) {
        if (strcmp(g_vsam_db[i].dataset, dataset_name) == 0 &&
            strcmp(g_vsam_db[i].key, record_key) == 0) {
            snprintf(record_out, max_len, "%s", g_vsam_db[i].value);
            return 0;
        }
    }

    snprintf(record_out, max_len, "RECORD NOT FOUND");
    return -2;
}

int tsfi_mf_cics_vsam_write(const char *dataset_name, const char *record_key, const char *record_data) {
    if (!dataset_name || !record_key || !record_data) return -1;

    // Check if record exists, overwrite it
    for (int i = 0; i < g_vsam_db_count; i++) {
        if (strcmp(g_vsam_db[i].dataset, dataset_name) == 0 &&
            strcmp(g_vsam_db[i].key, record_key) == 0) {
            strncpy(g_vsam_db[i].value, record_data, sizeof(g_vsam_db[i].value) - 1);
            g_vsam_db[i].value[sizeof(g_vsam_db[i].value) - 1] = '\0';
            return 0;
        }
    }

    if (g_vsam_db_count >= 32) return -3; // Overflow

    strncpy(g_vsam_db[g_vsam_db_count].dataset, dataset_name, sizeof(g_vsam_db[g_vsam_db_count].dataset) - 1);
    g_vsam_db[g_vsam_db_count].dataset[sizeof(g_vsam_db[g_vsam_db_count].dataset) - 1] = '\0';

    strncpy(g_vsam_db[g_vsam_db_count].key, record_key, sizeof(g_vsam_db[g_vsam_db_count].key) - 1);
    g_vsam_db[g_vsam_db_count].key[sizeof(g_vsam_db[g_vsam_db_count].key) - 1] = '\0';

    strncpy(g_vsam_db[g_vsam_db_count].value, record_data, sizeof(g_vsam_db[g_vsam_db_count].value) - 1);
    g_vsam_db[g_vsam_db_count].value[sizeof(g_vsam_db[g_vsam_db_count].value) - 1] = '\0';

    g_vsam_db_count++;
    return 0;
}

int tsfi_mf_zmachine_vulkan_map_render(uint32_t active_room_id, const uint32_t *connections, int conn_count, char *terminal_buffer) {
    if (!terminal_buffer || conn_count < 0) return -1;

    memset(terminal_buffer, ' ', 80 * 24);

    int offset = snprintf(terminal_buffer, 80, "VULKAN ZMACHINE ROOM VIEW | ROOM %d | APPC: SYNC | MAJORDOMO: CONNECTED", active_room_id);
    terminal_buffer[offset] = ' ';

    for (int r = 5; r <= 18; r++) {
        terminal_buffer[r * 80 + 20] = '|';
        terminal_buffer[r * 80 + 60] = '|';
    }

    for (int c = 20; c <= 60; c++) {
        terminal_buffer[4 * 80 + c] = '=';
        terminal_buffer[19 * 80 + c] = '=';
    }

    for (int i = 0; i < conn_count; i++) {
        uint32_t next_id = connections[i];
        char conn_lbl[32];
        int lbl_len = snprintf(conn_lbl, sizeof(conn_lbl), "-> ROOM %d", next_id);
        if (lbl_len > 0 && lbl_len < 20) {
            memcpy(terminal_buffer + (8 + i * 2) * 80 + 62, conn_lbl, lbl_len);
        }
    }

    return 0;
}

int tsfi_mf_majordomo_which(const char *email, const char **lists, int list_count, char *out_buffer, int max_len) {
    if (!email || !lists || list_count < 0 || !out_buffer || max_len <= 0) return -1;

    int offset = snprintf(out_buffer, max_len, "Subscriptions for %s:\n", email);
    if (offset < 0 || offset >= max_len) return -2;

    int found_any = 0;
    for (int i = 0; i < list_count; i++) {
        if (lists[i] && (strstr(lists[i], "dev") != NULL || strcmp(email, "admin@dysnomia.org") == 0)) {
            int len = snprintf(out_buffer + offset, max_len - offset, "  %s\n", lists[i]);
            if (len < 0 || offset + len >= max_len) return -2;
            offset += len;
            found_any = 1;
        }
    }

    if (!found_any) {
        snprintf(out_buffer + offset, max_len - offset, "  None\n");
    }

    return 0;
}

int tsfi_mf_cics_writeq_td(const char *queue_name, const char *log_message, char *queue_pool, int *queue_count, int max_entries) {
    if (!queue_name || !log_message || !queue_pool || !queue_count || max_entries <= 0) return -1;

    int idx = *queue_count;
    if (idx >= max_entries) return -2;

    int entry_size = 64;
    char *target_slot = queue_pool + (idx * entry_size);
    memset(target_slot, 0, entry_size);
    snprintf(target_slot, entry_size, "[%s] %s", queue_name, log_message);

    *queue_count = idx + 1;
    return 0;
}

int tsfi_mf_pmg_log_trajectory(uint8_t missile_id, float vx, float vy, char *queue_pool, int *queue_count, int max_entries) {
    if (!queue_pool || !queue_count || max_entries <= 0) return -1;

    char msg[64];
    snprintf(msg, sizeof(msg), "MISSILE %d: VX=%.2f, VY=%.2f", missile_id, vx, vy);
    return tsfi_mf_cics_writeq_td("TRAJ", msg, queue_pool, queue_count, max_entries);
}

int tsfi_mf_cics_isc_route(const char *target_system, const char *transaction_id, const uint8_t *payload, int payload_len, char *route_log_buffer, int max_log_len) {
    if (!target_system || !transaction_id || !route_log_buffer || max_log_len <= 0) return -1;
    (void)payload;

    snprintf(route_log_buffer, max_log_len, "ISC_ROUTE: TARGET=%s | TRANS=%s | PAYLOAD_LEN=%d", target_system, transaction_id, payload_len);
    return 0;
}

int tsfi_mf_majordomo_lists(char *lists_out, int max_len) {
    if (!lists_out || max_len <= 0) return -1;

    snprintf(lists_out, max_len,
             "MAJORDOMO ACTIVE MAILING LISTS:\n"
             "  zmm-dev\n"
             "  cics-admin\n"
             "  atropa-pulse\n");
    return 0;
}

int tsfi_mf_cics_start_task(const char *trans_id, uint32_t delay_seconds, const uint8_t *data, int data_len, uint32_t *active_tasks_count) {
    if (!trans_id || !active_tasks_count || data_len < 0) return -1;
    (void)delay_seconds;
    (void)data;

    *active_tasks_count += 1;
    return 0;
}

int tsfi_mf_gauntlet_btc_transaction(const char *player_id, uint32_t amount_sats, int32_t *health_in_out, uint32_t *keys_in_out, char *tx_out, int max_tx_len) {
    if (!player_id || !health_in_out || !keys_in_out || !tx_out || max_tx_len <= 0) return -1;

    if (amount_sats >= 500) {
        *keys_in_out += 1;
    } else if (amount_sats >= 100) {
        *health_in_out += 100;
    } else {
        return -2;
    }

    uint64_t tx_hash = 11235813;
    for (int i = 0; player_id[i] != '\0'; i++) {
        tx_hash = ((tx_hash << 5) + tx_hash) + (uint8_t)player_id[i];
    }
    tx_hash += amount_sats;

    snprintf(tx_out, max_tx_len, "TXID:%016lX | SATS:%d | HERO:%s", tx_hash, amount_sats, player_id);

    char vsam_val[64];
    snprintf(vsam_val, sizeof(vsam_val), "HP:%d | KEYS:%d | TX:%016lX", *health_in_out, *keys_in_out, tx_hash);
    tsfi_mf_cics_vsam_write("USERFILE", player_id, vsam_val);

    return 0;
}

int tsfi_mf_vulkan_zmachine_dispatch_shader(uint32_t room_id, const float *vertex_coords, int vertex_count, char *shader_log, int max_log_len) {
    if (!vertex_coords || vertex_count < 0 || !shader_log || max_log_len <= 0) return -1;

    float centroid_x = 0.0f;
    float centroid_y = 0.0f;
    if (vertex_count > 0) {
        for (int i = 0; i < vertex_count; i++) {
            centroid_x += vertex_coords[i * 2];
            centroid_y += vertex_coords[i * 2 + 1];
        }
        centroid_x /= (float)vertex_count;
        centroid_y /= (float)vertex_count;
    }

    snprintf(shader_log, max_log_len, "VK_DISPATCH: ROOM=%u | VERTICES=%d | CENTROID=(%.2f,%.2f)", room_id, vertex_count, centroid_x, centroid_y);
    return 0;
}

int tsfi_mf_majordomo_intro(const char *list_name, char *intro_out, int max_len) {
    if (!list_name || !intro_out || max_len <= 0) return -1;

    snprintf(intro_out, max_len, "Welcome to the %s mailing list. Keep all discussions on-topic.", list_name);
    return 0;
}

int tsfi_mf_majordomo_parse_config(const char *config_content, const char *key, char *val_out, int max_len) {
    if (!config_content || !key || !val_out || max_len <= 0) return -1;

    const char *line = config_content;
    while (line && *line != '\0') {
        const char *next_line = strchr(line, '\n');
        int line_len = next_line ? (int)(next_line - line) : (int)strlen(line);

        int key_len = (int)strlen(key);
        if (line_len > key_len && strncmp(line, key, key_len) == 0) {
            const char *sep = strchr(line, '=');
            if (sep && (sep < next_line || !next_line)) {
                sep++;
                while (*sep == ' ' || *sep == '\t') sep++;
                int val_len = next_line ? (int)(next_line - sep) : (int)strlen(sep);
                if (val_len >= max_len) val_len = max_len - 1;
                memcpy(val_out, sep, val_len);
                val_out[val_len] = '\0';
                return 0;
            }
        }

        line = next_line ? next_line + 1 : NULL;
    }

    return -2;
}

int tsfi_mf_cics_suspend(uint32_t task_id, uint32_t *suspend_log, int *log_count, int max_log) {
    if (!suspend_log || !log_count || max_log <= 0) return -1;

    int idx = *log_count;
    if (idx >= max_log) return -2;

    suspend_log[idx] = task_id;
    *log_count = idx + 1;
    return 0;
}

int tsfi_mf_majordomo_gen_cookie(const char *email, const char *list_name, uint32_t salt, char *cookie_out, int max_len) {
    if (!email || !list_name || !cookie_out || max_len <= 0) return -1;

    uint64_t hash = 5381 + salt;
    for (int i = 0; email[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + (uint8_t)email[i];
    }
    for (int i = 0; list_name[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + (uint8_t)list_name[i];
    }

    snprintf(cookie_out, max_len, "%016lX", hash);
    return 0;
}

int tsfi_mf_cics_query_security(const char *user_id, const char *resource_name, const char *access_type, int *auth_verdict) {
    if (!user_id || !resource_name || !access_type || !auth_verdict) return -1;

    if (strstr(user_id, "admin") != NULL || strstr(user_id, "sys") != NULL) {
        *auth_verdict = 0;
    } else if (strcmp(access_type, "READ") == 0) {
        *auth_verdict = 0;
    } else {
        *auth_verdict = -2;
    }
    return 0;
}

int tsfi_mf_majordomo_unsubscribe(const char *list_name, const char *email, const char **members, int *member_count) {
    if (!list_name || !email || !members || !member_count || *member_count < 0) return -1;

    for (int i = 0; i < *member_count; i++) {
        if (members[i] && strcmp(members[i], email) == 0) {
            for (int j = i; j < *member_count - 1; j++) {
                members[j] = members[j + 1];
            }
            *member_count -= 1;
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_cics_freemain(uint32_t offset, uint32_t length, uint8_t *storage_pool) {
    if (!storage_pool || length == 0) return -1;

    memset(storage_pool + offset, 0, length);
    return 0;
}

int tsfi_mf_majordomo_moderate(uint32_t cookie, const char *action, char *verdict_out, int max_len) {
    if (!action || !verdict_out || max_len <= 0) return -1;

    if (strcmp(action, "ACCEPT") == 0 || strcmp(action, "APPROVE") == 0) {
        snprintf(verdict_out, max_len, "APPROVED:%u", cookie);
    } else {
        snprintf(verdict_out, max_len, "REJECTED:%u", cookie);
    }
    return 0;
}

int tsfi_mf_cics_dump(const char *dump_id, uint32_t code, const uint8_t *payload, int len, char *dump_out, int max_len) {
    if (!dump_id || !payload || len < 0 || !dump_out || max_len <= 0) return -1;

    int offset = snprintf(dump_out, max_len, "DUMP %s CODE %04X: ", dump_id, code);
    if (offset < 0 || offset >= max_len) return -2;

    for (int i = 0; i < len; i++) {
        if (offset + 3 >= max_len) break;
        int bytes = snprintf(dump_out + offset, max_len - offset, "%02X ", payload[i]);
        if (bytes < 0) return -2;
        offset += bytes;
    }

    return 0;
}

int tsfi_mf_majordomo_queue_post(const char *list_name, const char *sender, const char *subject, uint32_t *pending_cookies, int *cookie_count, int max_cookies) {
    if (!list_name || !sender || !subject || !pending_cookies || !cookie_count || max_cookies <= 0) return -1;

    int idx = *cookie_count;
    if (idx >= max_cookies) return -2;

    uint32_t cookie = 77123;
    for (int i = 0; list_name[i] != '\0'; i++) cookie += (uint8_t)list_name[i];
    for (int i = 0; sender[i] != '\0'; i++) cookie += (uint8_t)sender[i];

    pending_cookies[idx] = cookie;
    *cookie_count = idx + 1;
    return 0;
}

int tsfi_mf_cics_cwa_write(uint32_t offset, const uint8_t *data, int len, uint8_t *cwa_pool) {
    if (!data || len < 0 || !cwa_pool) return -1;

    memcpy(cwa_pool + offset, data, len);
    return 0;
}

int tsfi_mf_cics_cwa_read(uint32_t offset, uint8_t *data_out, int len, const uint8_t *cwa_pool) {
    if (!data_out || len < 0 || !cwa_pool) return -1;

    memcpy(data_out, cwa_pool + offset, len);
    return 0;
}

int tsfi_mf_majordomo_get_pending(const char *list_name, const uint32_t *pending_cookies, int cookie_count, uint32_t *out_cookies, int *out_count, int max_out) {
    if (!list_name || !pending_cookies || cookie_count < 0 || !out_cookies || !out_count || max_out <= 0) return -1;

    int count = 0;
    for (int i = 0; i < cookie_count; i++) {
        uint32_t val = pending_cookies[i];
        if (val > 77000 && count < max_out) {
            out_cookies[count] = val;
            count++;
        }
    }

    *out_count = count;
    return 0;
}

int tsfi_mf_cics_getmain_shared(int length, uint8_t *storage_pool, uint32_t *allocated_offset, uint32_t *shared_registry, int *registry_count, int max_registry) {
    if (length <= 0 || !storage_pool || !allocated_offset || !shared_registry || !registry_count || max_registry <= 0) return -1;

    int idx = *registry_count;
    if (idx >= max_registry) return -2;

    uint32_t offset = *allocated_offset;
    memset(storage_pool + offset, 0, length);
    *allocated_offset = offset + length;

    shared_registry[idx] = offset;
    *registry_count = idx + 1;

    return 0;
}

int tsfi_mf_majordomo_is_admin(const char *list_name, const char *email, const char **admin_registry, int admin_count) {
    if (!list_name || !email || !admin_registry || admin_count < 0) return -1;

    for (int i = 0; i < admin_count; i++) {
        if (admin_registry[i] && strcmp(admin_registry[i], email) == 0) {
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_cics_bypass_security(const char *transaction_id, uint32_t auth_token, int *bypass_status) {
    if (!transaction_id || !bypass_status) return -1;

    if (auth_token == 0xAAFFBEEF) {
        *bypass_status = 1;
    } else {
        *bypass_status = 0;
    }
    return 0;
}

int tsfi_mf_majordomo_archive_index(const char *list_name, const char **archive_files, int archive_count, char *index_out, int max_len) {
    if (!list_name || !archive_files || archive_count < 0 || !index_out || max_len <= 0) return -1;

    int offset = snprintf(index_out, max_len, "ARCHIVE INDEX FOR LIST %s: ", list_name);
    if (offset < 0 || offset >= max_len) return -2;

    for (int i = 0; i < archive_count; i++) {
        if (archive_files[i]) {
            int bytes = snprintf(index_out + offset, max_len - offset, "%s; ", archive_files[i]);
            if (bytes < 0 || offset + bytes >= max_len) break;
            offset += bytes;
        }
    }

    return 0;
}

int tsfi_mf_cics_return(const char *next_transaction_id, const uint8_t *commarea, int commarea_len, char *return_status_log, int max_log) {
    (void)commarea;
    if (!return_status_log || max_log <= 0) return -1;

    if (next_transaction_id) {
        snprintf(return_status_log, max_log, "RETURN_TRANSID:%s|COMMAREA_LEN:%d", next_transaction_id, commarea_len);
    } else {
        snprintf(return_status_log, max_log, "RETURN_TERMINAL|COMMAREA_LEN:0");
    }
    return 0;
}

int tsfi_mf_majordomo_validate_config(const char *config_content, int *errors_found) {
    if (!config_content || !errors_found) return -1;

    int errors = 0;
    const char *line = config_content;
    while (line && *line != '\0') {
        const char *next_line = strchr(line, '\n');
        int line_len = next_line ? (int)(next_line - line) : (int)strlen(line);

        if (line_len > 0 && line[0] != '#' && line[0] != '\r') {
            int has_equal = 0;
            for (int i = 0; i < line_len; i++) {
                if (line[i] == '=') {
                    has_equal = 1;
                    break;
                }
            }
            if (!has_equal) errors++;
        }

        line = next_line ? next_line + 1 : NULL;
    }

    *errors_found = errors;
    return 0;
}

int tsfi_mf_cics_abend(const char *abend_code, char *abend_log_buffer, int max_len) {
    if (!abend_code || !abend_log_buffer || max_len <= 0) return -1;

    snprintf(abend_log_buffer, max_len, "CICS_ABEND:CODE=%s", abend_code);
    return 0;
}

int tsfi_mf_majordomo_discard(uint32_t cookie, uint32_t *pending_cookies, int *cookie_count) {
    if (!pending_cookies || !cookie_count || *cookie_count < 0) return -1;

    for (int i = 0; i < *cookie_count; i++) {
        if (pending_cookies[i] == cookie) {
            for (int j = i; j < *cookie_count - 1; j++) {
                pending_cookies[j] = pending_cookies[j + 1];
            }
            *cookie_count -= 1;
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_cics_load(const char *resource_name, uint32_t *loaded_address, uint8_t *storage_pool, uint32_t *allocated_offset) {
    if (!resource_name || !loaded_address || !storage_pool || !allocated_offset) return -1;

    uint32_t offset = *allocated_offset;
    int len = (int)strlen(resource_name);
    memcpy(storage_pool + offset, resource_name, len);
    *allocated_offset = offset + len;

    *loaded_address = offset;
    return 0;
}

int tsfi_mf_majordomo_check_password(const char *list_name, const char *password, const char *stored_hash_or_pass) {
    (void)list_name;
    if (!password || !stored_hash_or_pass) return -1;

    if (strcmp(password, stored_hash_or_pass) == 0) {
        return 0;
    }
    return -2;
}

int tsfi_mf_cics_release(const char *resource_name, uint8_t *storage_pool, uint32_t *allocated_offset) {
    if (!resource_name || !storage_pool || !allocated_offset) return -1;

    uint32_t offset = 0;
    int len = (int)strlen(resource_name);
    while (offset < *allocated_offset) {
        if (memcmp(storage_pool + offset, resource_name, len) == 0) {
            memset(storage_pool + offset, 0, len);
            return 0;
        }
        offset++;
    }

    return -2;
}

int tsfi_mf_majordomo_hash_password(const char *password, char *hash_out, int max_len) {
    if (!password || !hash_out || max_len <= 0) return -1;

    uint32_t hash = 5381;
    for (int i = 0; password[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + (uint8_t)password[i];
    }

    snprintf(hash_out, max_len, "MD5:%08X", hash);
    return 0;
}

int tsfi_mf_cics_handle_abend(uint64_t handler_addr, uint64_t *abend_handler_registry, int *registry_active) {
    if (!abend_handler_registry || !registry_active) return -1;

    *abend_handler_registry = handler_addr;
    *registry_active = 1;
    return 0;
}

int tsfi_mf_majordomo_welcome(const char *list_name, const char *owner_email, char *welcome_out, int max_len) {
    if (!list_name || !owner_email || !welcome_out || max_len <= 0) return -1;

    snprintf(welcome_out, max_len, "WELCOME TO LIST %s. CONTACT OWNER AT %s.", list_name, owner_email);
    return 0;
}

int tsfi_mf_cics_cancel_abend(uint64_t *abend_handler_registry, int *registry_active) {
    if (!abend_handler_registry || !registry_active) return -1;

    *abend_handler_registry = 0;
    *registry_active = 0;
    return 0;
}

int tsfi_mf_majordomo_update_info(const char *list_name, const char *new_info, char *info_store, int max_store) {
    if (!list_name || !new_info || !info_store || max_store <= 0) return -1;

    snprintf(info_store, max_store, "LIST:%s|INFO:%s", list_name, new_info);
    return 0;
}

int tsfi_mf_cics_handle_abend_program(const char *program_name, char *abend_program_registry, int max_name) {
    if (!program_name || !abend_program_registry || max_name <= 0) return -1;

    snprintf(abend_program_registry, max_name, "%s", program_name);
    return 0;
}

int tsfi_mf_cics_handle_pmg_hit_abend(uint32_t player_id, int hit_type, int *health_in_out, float *x_out, float *y_out, char *action_log, int max_log) {
    if (!health_in_out || !x_out || !y_out || !action_log || max_log <= 0) return -1;

    int dmg = (hit_type == 1) ? *health_in_out : 50;
    *health_in_out -= dmg;

    if (*health_in_out <= 0) {
        *health_in_out = 100;
        *x_out = 0.0f;
        *y_out = 0.0f;
        snprintf(action_log, max_log, "ABEND_RECOVERY:PLAYER=%u|ACTION=RESPAWN", player_id);
    } else {
        snprintf(action_log, max_log, "PHYSICS_HIT:PLAYER=%u|DAMAGE=%d|HEALTH=%d", player_id, dmg, *health_in_out);
    }
    return 0;
}

int tsfi_mf_majordomo_config_defaults(const char *list_name, char *defaults_out, int max_len) {
    (void)list_name;
    if (!defaults_out || max_len <= 0) return -1;

    snprintf(defaults_out, max_len, "reply_to = list\nmoderate = no\nsubscribe_policy = open\n");
    return 0;
}

int tsfi_mf_cics_reset_abend(int *registry_active) {
    if (!registry_active) return -1;

    *registry_active = 1;
    return 0;
}

int tsfi_mf_majordomo_inject_mod_headers(const char *sender, const char *list_name, uint32_t cookie, char *msg_out, int max_len) {
    if (!sender || !list_name || !msg_out || max_len <= 0) return -1;

    snprintf(msg_out, max_len, "X-Majordomo-Sender: %s\nX-Majordomo-List: %s\nX-Majordomo-Cookie: %u\n", sender, list_name, cookie);
    return 0;
}

int tsfi_mf_cics_query_abend(int registry_active, int *is_active_out) {
    if (!is_active_out) return -1;

    *is_active_out = registry_active;
    return 0;
}

int tsfi_mf_majordomo_archive_get(const char *list_name, const char *file_name, const char **archive_files, const char **archive_contents, int archive_count, char *content_out, int max_len) {
    (void)list_name;
    if (!file_name || !archive_files || !archive_contents || !content_out || max_len <= 0) return -1;

    for (int i = 0; i < archive_count; i++) {
        if (strcmp(archive_files[i], file_name) == 0) {
            snprintf(content_out, max_len, "%s", archive_contents[i]);
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_cics_delay(uint32_t seconds, uint32_t *delay_counter_log) {
    if (!delay_counter_log) return -1;

    *delay_counter_log += seconds;
    return 0;
}

int tsfi_mf_majordomo_write_config(const char *key, const char *val, char *config_in_out, int max_len) {
    if (!key || !val || !config_in_out || max_len <= 0) return -1;

    char search_pattern[64];
    snprintf(search_pattern, sizeof(search_pattern), "%s =", key);
    char *match = strstr(config_in_out, search_pattern);
    if (match) {
        snprintf(match, max_len - (match - config_in_out), "%s = %s\n", key, val);
    } else {
        int current_len = (int)strlen(config_in_out);
        snprintf(config_in_out + current_len, max_len - current_len, "%s = %s\n", key, val);
    }
    return 0;
}

int tsfi_mf_cics_resume(uint32_t task_id, uint32_t *suspend_log, int *log_count) {
    if (!suspend_log || !log_count || *log_count < 0) return -1;

    for (int i = 0; i < *log_count; i++) {
        if (suspend_log[i] == task_id) {
            for (int j = i; j < *log_count - 1; j++) {
                suspend_log[j] = suspend_log[j + 1];
            }
            *log_count -= 1;
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_majordomo_archive_delete(const char *list_name, const char *file_name, const char **archive_files, int *archive_count) {
    (void)list_name;
    if (!file_name || !archive_files || !archive_count || *archive_count < 0) return -1;

    for (int i = 0; i < *archive_count; i++) {
        if (strcmp(archive_files[i], file_name) == 0) {
            for (int j = i; j < *archive_count - 1; j++) {
                archive_files[j] = archive_files[j + 1];
            }
            *archive_count -= 1;
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_cics_inquire_task(uint32_t task_id, int is_suspended, char *status_out, int max_len) {
    if (!status_out || max_len <= 0) return -1;

    snprintf(status_out, max_len, "TASK:%u|STATUS=%s", task_id, is_suspended ? "SUSPENDED" : "RUNNING");
    return 0;
}

int tsfi_mf_majordomo_archive_prune(const char *list_name, const char **archive_files, int *archive_count, const char *prune_suffix) {
    (void)list_name;
    if (!archive_files || !archive_count || *archive_count < 0 || !prune_suffix) return -1;

    int suffix_len = (int)strlen(prune_suffix);
    int i = 0;
    while (i < *archive_count) {
        int file_len = (int)strlen(archive_files[i]);
        if (file_len >= suffix_len && strcmp(archive_files[i] + file_len - suffix_len, prune_suffix) == 0) {
            for (int j = i; j < *archive_count - 1; j++) {
                archive_files[j] = archive_files[j + 1];
            }
            *archive_count -= 1;
        } else {
            i++;
        }
    }
    return 0;
}

int tsfi_mf_cics_change_priority(uint32_t task_id, int new_priority, int *priority_registry_out) {
    (void)task_id;
    if (!priority_registry_out) return -1;

    *priority_registry_out = new_priority;
    return 0;
}

int tsfi_mf_majordomo_check_post_policy(const char *policy, const char *sender, int is_subscriber, int *needs_moderation) {
    (void)sender;
    if (!policy || !needs_moderation) return -1;

    if (strcmp(policy, "open") == 0) {
        *needs_moderation = 0;
    } else if (strcmp(policy, "moderated") == 0) {
        *needs_moderation = 1;
    } else if (strcmp(policy, "closed") == 0) {
        *needs_moderation = is_subscriber ? 0 : 1;
    } else {
        return -2;
    }
    return 0;
}

int tsfi_mf_cics_inquire_priority(uint32_t task_id, int priority_registry, int *priority_out) {
    (void)task_id;
    if (!priority_out) return -1;

    *priority_out = priority_registry;
    return 0;
}

int tsfi_mf_majordomo_approve_forward(uint32_t cookie, const char *list_name, const char *msg_body, char *output_forward_log, int max_len) {
    if (!list_name || !msg_body || !output_forward_log || max_len <= 0) return -1;

    snprintf(output_forward_log, max_len, "FORWARD:LIST=%s|COOKIE=%u|BODY=%s", list_name, cookie, msg_body);
    return 0;
}

int tsfi_mf_cics_suspend_task(uint32_t target_task_id, uint32_t *suspend_log, int *log_count, int max_log) {
    if (!suspend_log || !log_count || *log_count < 0 || max_log <= 0) return -1;

    if (*log_count >= max_log) return -2;

    suspend_log[*log_count] = target_task_id;
    *log_count += 1;
    return 0;
}

int tsfi_mf_majordomo_reject_notify(uint32_t cookie, const char *list_name, const char *sender, char *output_reject_log, int max_len) {
    if (!list_name || !sender || !output_reject_log || max_len <= 0) return -1;

    snprintf(output_reject_log, max_len, "REJECT:LIST=%s|COOKIE=%u|SENDER=%s|REASON=MODERATION_DENIED", list_name, cookie, sender);
    return 0;
}

int tsfi_mf_cics_inquire_suspended(uint32_t target_task_id, const uint32_t *suspend_log, int log_count, int *is_suspended_out) {
    if (!suspend_log || !is_suspended_out || log_count < 0) return -1;

    *is_suspended_out = 0;
    for (int i = 0; i < log_count; i++) {
        if (suspend_log[i] == target_task_id) {
            *is_suspended_out = 1;
            break;
        }
    }
    return 0;
}

int tsfi_mf_majordomo_log_moderation(uint32_t cookie, const char *list_name, const char *action, const char *moderator, char *log_out, int max_len) {
    if (!list_name || !action || !moderator || !log_out || max_len <= 0) return -1;

    snprintf(log_out, max_len, "MODERATION_LOG:COOKIE=%u|LIST=%s|ACTION=%s|MODERATOR=%s", cookie, list_name, action, moderator);
    return 0;
}

int tsfi_mf_cics_change_task_priority(uint32_t target_task_id, int new_priority, uint32_t *target_task_id_log, int *prio_registry_log, int *log_count, int max_log) {
    if (!target_task_id_log || !prio_registry_log || !log_count || *log_count < 0 || max_log <= 0) return -1;

    if (*log_count >= max_log) return -2;

    target_task_id_log[*log_count] = target_task_id;
    prio_registry_log[*log_count] = new_priority;
    *log_count += 1;
    return 0;
}

int tsfi_mf_majordomo_check_sub_policy(const char *policy, const char *email, int *requires_confirm, int *requires_moderator) {
    (void)email;
    if (!policy || !requires_confirm || !requires_moderator) return -1;

    *requires_confirm = 0;
    *requires_moderator = 0;

    if (strcmp(policy, "open") == 0) {
        // Direct
    } else if (strcmp(policy, "confirm") == 0) {
        *requires_confirm = 1;
    } else if (strcmp(policy, "moderated") == 0) {
        *requires_moderator = 1;
    } else if (strcmp(policy, "closed") == 0) {
        *requires_confirm = 1;
        *requires_moderator = 1;
    } else {
        return -2;
    }
    return 0;
}

int tsfi_mf_cics_inquire_priority_log(uint32_t target_task_id, const uint32_t *target_task_id_log, const int *prio_registry_log, int log_count, int *priority_out) {
    if (!target_task_id_log || !prio_registry_log || !priority_out || log_count < 0) return -1;

    *priority_out = 0;
    for (int i = log_count - 1; i >= 0; i--) {
        if (target_task_id_log[i] == target_task_id) {
            *priority_out = prio_registry_log[i];
            return 0;
        }
    }

    return -2;
}

int tsfi_mf_majordomo_get_config(const char *key, const char *config, char *val_out, int max_len) {
    if (!key || !config || !val_out || max_len <= 0) return -1;

    char search_pattern[64];
    snprintf(search_pattern, sizeof(search_pattern), "%s =", key);
    char *match = strstr(config, search_pattern);
    if (match) {
        char *val_start = match + strlen(search_pattern);
        while (*val_start == ' ') val_start++;
        int val_len = 0;
        while (val_start[val_len] != '\n' && val_start[val_len] != '\0' && val_start[val_len] != '\r') {
            val_len++;
        }
        if (val_len >= max_len) val_len = max_len - 1;
        memcpy(val_out, val_start, val_len);
        val_out[val_len] = '\0';
        return 0;
    }
    return -2;
}

int tsfi_mf_cics_inquire_current_priority(uint32_t task_id, int priority_registry, int *priority_out) {
    (void)task_id;
    if (!priority_out) return -1;

    *priority_out = priority_registry;
    return 0;
}

int tsfi_mf_majordomo_reset_config(const char *list_name, char *config_in_out, int max_len) {
    if (!list_name || !config_in_out || max_len <= 0) return -1;

    snprintf(config_in_out, max_len, "list = %s\nmoderate = yes\nsubscribe = open\n", list_name);
    return 0;
}

int tsfi_mf_cics_inquire_maxtasks(int maxtasks_registry, int *maxtasks_out) {
    if (!maxtasks_out) return -1;

    *maxtasks_out = maxtasks_registry;
    return 0;
}

int tsfi_mf_majordomo_has_valid_syntax(const char *config, int *is_valid) {
    if (!config || !is_valid) return -1;

    *is_valid = 0;
    if (strstr(config, "list =") != NULL) {
        *is_valid = 1;
    }
    return 0;
}

int tsfi_mf_cics_inquire_acttasks(int active_tasks_count, int *active_tasks_out) {
    if (!active_tasks_out) return -1;

    *active_tasks_out = active_tasks_count;
    return 0;
}

int tsfi_mf_majordomo_is_kv_line(const char *line, int *is_valid) {
    if (!line || !is_valid) return -1;

    *is_valid = 0;
    const char *eq = strchr(line, '=');
    if (eq && eq != line && *(eq + 1) != '\0') {
        *is_valid = 1;
    }
    return 0;
}

int tsfi_mf_cics_inquire_queuedtasks(int queued_tasks_count, int *queued_tasks_out) {
    if (!queued_tasks_out) return -1;

    *queued_tasks_out = queued_tasks_count;
    return 0;
}

int tsfi_mf_majordomo_strip_comments(const char *config, char *config_out, int max_len) {
    if (!config || !config_out || max_len <= 0) return -1;

    int out_idx = 0;
    const char *line = config;
    while (*line != '\0' && out_idx < max_len - 1) {
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '#' || *line == '\n' || *line == '\r') {
            while (*line != '\n' && *line != '\0') line++;
            if (*line == '\n') line++;
        } else {
            while (*line != '\n' && *line != '\0' && out_idx < max_len - 1) {
                config_out[out_idx++] = *line++;
            }
            if (*line == '\n' && out_idx < max_len - 1) {
                config_out[out_idx++] = *line++;
            }
        }
    }
    config_out[out_idx] = '\0';
    return 0;
}

int tsfi_mf_cics_inquire_tranclass(const char *class_name, int class_limit_registry, int *class_limit_out) {
    (void)class_name;
    if (!class_limit_out) return -1;

    *class_limit_out = class_limit_registry;
    return 0;
}

int tsfi_mf_majordomo_is_moderated(const char *config, int *is_mod) {
    if (!config || !is_mod) return -1;

    *is_mod = 0;
    char val[64];
    int get_res = tsfi_mf_majordomo_get_config("moderate", config, val, sizeof(val));
    if (get_res == 0 && strcmp(val, "yes") == 0) {
        *is_mod = 1;
    }
    return 0;
}

int tsfi_mf_cics_inquire_tranclass_acttasks(const char *class_name, int acttasks_registry, int *acttasks_out) {
    (void)class_name;
    if (!acttasks_out) return -1;

    *acttasks_out = acttasks_registry;
    return 0;
}

int tsfi_mf_majordomo_is_open(const char *config, int *is_op) {
    if (!config || !is_op) return -1;

    *is_op = 0;
    char val[64];
    int get_res = tsfi_mf_majordomo_get_config("subscribe", config, val, sizeof(val));
    if (get_res == 0 && strcmp(val, "open") == 0) {
        *is_op = 1;
    }
    return 0;
}

int tsfi_mf_cics_inquire_tranclass_queuedtasks(const char *class_name, int queuedtasks_registry, int *queuedtasks_out) {
    (void)class_name;
    if (!queuedtasks_out) return -1;

    *queuedtasks_out = queuedtasks_registry;
    return 0;
}

int tsfi_mf_majordomo_get_kv_value(const char *line, char *val_out, int max_len) {
    if (!line || !val_out || max_len <= 0) return -1;

    const char *eq = strchr(line, '=');
    if (eq) {
        const char *val_start = eq + 1;
        while (*val_start == ' ') val_start++;
        int val_len = 0;
        while (val_start[val_len] != '\n' && val_start[val_len] != '\0' && val_start[val_len] != '\r') {
            val_len++;
        }
        if (val_len >= max_len) val_len = max_len - 1;
        memcpy(val_out, val_start, val_len);
        val_out[val_len] = '\0';
        return 0;
    }
    return -2;
}

int tsfi_mf_cics_inquire_tranclass_maxtasks(const char *class_name, int maxtasks_registry, int *maxtasks_out) {
    (void)class_name;
    if (!maxtasks_out) return -1;

    *maxtasks_out = maxtasks_registry;
    return 0;
}

int tsfi_mf_majordomo_is_closed(const char *config, int *is_cl) {
    if (!config || !is_cl) return -1;

    *is_cl = 0;
    char val[64];
    int get_res = tsfi_mf_majordomo_get_config("subscribe", config, val, sizeof(val));
    if (get_res == 0 && strcmp(val, "closed") == 0) {
        *is_cl = 1;
    }
    return 0;
}

int tsfi_mf_cics_inquire_tranclass_percent(const char *class_name, int acttasks_registry, int maxtasks_registry, float *percent_out) {
    (void)class_name;
    if (!percent_out || maxtasks_registry <= 0) return -1;

    *percent_out = ((float)acttasks_registry / (float)maxtasks_registry) * 100.0f;
    return 0;
}

int tsfi_mf_unified_dispatch(const char *subsystem, const char *action, const uint8_t *payload, size_t size, uint8_t *out_pdu, size_t *out_size) {
    if (!subsystem || !action) return -1;
    
    if (strcmp(subsystem, "NORAD") == 0) {
        if (strcmp(action, "AUTH") == 0) {
            uint32_t hash = 0;
            int res = tsfi_mf_norad_auth_hash(payload, size, &hash);
            if (res == 0 && out_pdu && out_size) {
                memcpy(out_pdu, &hash, 4);
                *out_size = 4;
            }
            return res;
        }
    } else if (strcmp(subsystem, "IRS") == 0) {
        if (strcmp(action, "CLEARANCE") == 0) {
            uint32_t token = 0;
            if (size >= 4 && payload) {
                memcpy(&token, payload, 4);
            }
            return tsfi_mf_irs_format_clearance(token, out_pdu, out_size);
        }
    } else if (strcmp(subsystem, "CICS") == 0) {
        if (strcmp(action, "BROADCAST") == 0) {
            int level = (size >= 1 && payload) ? payload[0] : 5;
            return tsfi_mf_cics_generate_naap_broadcast(level, 0x1122, out_pdu, out_size);
        }
    }
    return -2;
}

