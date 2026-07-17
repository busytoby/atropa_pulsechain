#include "tsfi_micro_focus.h"
#include "tsfi_ramac_layout.h"
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
