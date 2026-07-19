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



// Include modular BMS, Majordomo, and condition handling logic
#include "tsfi_micro_focus_cics.c"
