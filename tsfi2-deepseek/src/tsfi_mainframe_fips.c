#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_mainframe_fips.h"

// Scenario 142: IBM 3848 Cryptographic Subsystem
void tsfi_crypto_init(tsfi_crypto_subsystem *crypto) {
    if (!crypto) return;
    crypto->master_key = 0;
    crypto->is_key_loaded = 0;
}

int tsfi_crypto_load_master_key(tsfi_crypto_subsystem *crypto, uint64_t master_key) {
    if (!crypto) return -1;
    crypto->master_key = master_key;
    crypto->is_key_loaded = 1;
    return 0;
}

int tsfi_crypto_encrypt(tsfi_crypto_subsystem *crypto, const uint8_t *plain, uint8_t *cipher, int supervisor_state) {
    if (!crypto || !plain || !cipher) return -1;
    if (!supervisor_state) return -2; // Privileged instruction exception
    if (!crypto->is_key_loaded) return -3;
    
    uint32_t left = ((uint32_t)plain[0] << 24) | ((uint32_t)plain[1] << 16) | ((uint32_t)plain[2] << 8) | plain[3];
    uint32_t right = ((uint32_t)plain[4] << 24) | ((uint32_t)plain[5] << 16) | ((uint32_t)plain[6] << 8) | plain[7];
    
    // Round 1
    uint32_t temp = right;
    right = left ^ (right ^ (uint32_t)(crypto->master_key & 0xFFFFFFFF));
    left = temp;
    
    // Round 2
    temp = right;
    right = left ^ (right ^ (uint32_t)((crypto->master_key >> 32) & 0xFFFFFFFF));
    left = temp;
    
    cipher[0] = (left >> 24) & 0xFF;
    cipher[1] = (left >> 16) & 0xFF;
    cipher[2] = (left >> 8) & 0xFF;
    cipher[3] = left & 0xFF;
    cipher[4] = (right >> 24) & 0xFF;
    cipher[5] = (right >> 16) & 0xFF;
    cipher[6] = (right >> 8) & 0xFF;
    cipher[7] = right & 0xFF;
    
    return 0;
}

int tsfi_crypto_decrypt(tsfi_crypto_subsystem *crypto, const uint8_t *cipher, uint8_t *plain, int supervisor_state) {
    if (!crypto || !cipher || !plain) return -1;
    if (!supervisor_state) return -2; // Privileged instruction exception
    if (!crypto->is_key_loaded) return -3;
    
    uint32_t left = ((uint32_t)cipher[0] << 24) | ((uint32_t)cipher[1] << 16) | ((uint32_t)cipher[2] << 8) | cipher[3];
    uint32_t right = ((uint32_t)cipher[4] << 24) | ((uint32_t)cipher[5] << 16) | ((uint32_t)cipher[6] << 8) | cipher[7];
    
    // Reverse Round 2
    uint32_t temp = right;
    right = left;
    left = temp ^ (right ^ (uint32_t)((crypto->master_key >> 32) & 0xFFFFFFFF));
    
    // Reverse Round 1
    temp = right;
    right = left;
    left = temp ^ (right ^ (uint32_t)(crypto->master_key & 0xFFFFFFFF));
    
    plain[0] = (left >> 24) & 0xFF;
    plain[1] = (left >> 16) & 0xFF;
    plain[2] = (left >> 8) & 0xFF;
    plain[3] = left & 0xFF;
    plain[4] = (right >> 24) & 0xFF;
    plain[5] = (right >> 16) & 0xFF;
    plain[6] = (right >> 8) & 0xFF;
    plain[7] = right & 0xFF;
    
    return 0;
}

// Scenario 143: NBS FIPS PUB 60 Standard I/O Channel Interface
void tsfi_fips60_init(tsfi_fips60_interface *fips) {
    if (!fips) return;
    fips->bus_out_command = 0;
    fips->bus_in_status = 0;
    fips->command_pending = 0;
    fips->status_updated = 0;
}

int tsfi_fips60_bus_out(tsfi_fips60_interface *fips, uint8_t command_byte, const uint8_t *data, uint16_t data_len) {
    if (!fips) return -1;
    fips->bus_out_command = command_byte;
    fips->command_pending = 1;
    
    // Process standard commands to resolve controller status
    if (command_byte == 0x07) { // SEEK
        fips->bus_in_status = 0x00; // Device normal/idle status
    } else if (command_byte == 0x01) { // WRITE
        fips->bus_in_status = 0x0C; // Channel End + Device End
    } else if (command_byte == 0x02) { // READ
        fips->bus_in_status = 0x0C; // Channel End + Device End
    } else if (command_byte == 0x04) { // SENSE
        fips->bus_in_status = 0x00;
    } else {
        fips->bus_in_status = 0x02; // Unit Check (unknown command)
    }
    
    (void)data;
    (void)data_len;
    fips->status_updated = 1;
    return 0;
}

int tsfi_fips60_status_in(const tsfi_fips60_interface *fips, uint8_t *out_status) {
    if (!fips || !out_status) return -1;
    *out_status = fips->bus_in_status;
    return 0;
}

// Scenario 144: NBS FIPS PUB 54 Computer Output Microform (COM) Spool Formatter
void tsfi_com_init(tsfi_com_formatter *fmt) {
    if (!fmt) return;
    memset(fmt, 0, sizeof(tsfi_com_formatter));
    fmt->current_frame = 1;
}

int tsfi_com_format_record(tsfi_com_formatter *fmt, const char *record_text, uint8_t *out_frame, uint16_t *out_len) {
    if (!fmt || !record_text || !out_frame || !out_len) return -1;
    
    // Auto-advance frames after page frame length limits (e.g. 4 lines per grid frame)
    if (fmt->line_count >= 4) {
        fmt->current_frame++;
        fmt->line_count = 0;
    }
    
    // Format microform line layout block
    int len = snprintf((char*)out_frame, 256, "[COM-F%03d-L%d] %s", fmt->current_frame, fmt->line_count, record_text);
    *out_len = (uint16_t)len;
    
    // Register index keys for the first record on each new frame
    if (fmt->line_count == 0 && fmt->index_count < MAX_COM_INDEX_ENTRIES) {
        tsfi_com_index_entry *entry = &fmt->index_table[fmt->index_count];
        snprintf(entry->key, sizeof(entry->key), "%.15s", record_text);
        entry->frame_number = fmt->current_frame;
        fmt->index_count++;
    }
    
    fmt->line_count++;
    return 0;
}

int tsfi_com_generate_index_frame(const tsfi_com_formatter *fmt, char *out_index_data, int max_len) {
    if (!fmt || !out_index_data || max_len <= 0) return -1;
    
    int written = 0;
    written += snprintf(out_index_data + written, max_len - written, "=== COM MICROFICHE INDEX FRAME ===\n");
    for (int i = 0; i < fmt->index_count; i++) {
        if (written < max_len) {
            written += snprintf(out_index_data + written, max_len - written, "Key: %-15s -> Frame: %03d\n",
                                fmt->index_table[i].key, fmt->index_table[i].frame_number);
        }
    }
    return 0;
}

// Scenario 145: NBS FIPS PUB 48 Personal Identification Token Authenticator
void tsfi_fips48_init(tsfi_fips48_authenticator *auth) {
    if (!auth) return;
    memset(auth, 0, sizeof(tsfi_fips48_authenticator));
}

int tsfi_fips48_register_badge(tsfi_fips48_authenticator *auth, const char *user_id, uint32_t badge_id, uint16_t pin) {
    if (!auth || !user_id || auth->badge_count >= MAX_FIPS48_BADGES) return -1;
    
    for (int i = 0; i < auth->badge_count; i++) {
        if (auth->badges[i].badge_id == badge_id) {
            return -2; // Duplicate badge ID
        }
    }
    
    tsfi_fips48_badge *badge = &auth->badges[auth->badge_count];
    snprintf(badge->user_id, sizeof(badge->user_id), "%s", user_id);
    badge->badge_id = badge_id;
    badge->pin = pin;
    badge->is_active = 1;
    auth->badge_count++;
    return 0;
}

int tsfi_fips48_register_lau_badge(tsfi_fips48_authenticator *auth, const char *user_id, uint32_t lau_token_address, uint16_t pki_key_pin) {
    // Map LAU token address directly as badge_id, and PKI key directly as secure PIN
    return tsfi_fips48_register_badge(auth, user_id, lau_token_address, pki_key_pin);
}

int tsfi_fips48_authenticate(tsfi_fips48_authenticator *auth, uint32_t badge_id, uint16_t pin, int *out_status) {
    if (!auth || !out_status) return -1;
    *out_status = 0;
    
    tsfi_fips48_badge *target = NULL;
    for (int i = 0; i < auth->badge_count; i++) {
        if (auth->badges[i].badge_id == badge_id && auth->badges[i].is_active) {
            target = &auth->badges[i];
            break;
        }
    }
    
    if (!target) {
        auth->failed_attempts++;
        *out_status = -3; // Badge not found/inactive
        return -2;
    }
    
    if (target->pin != pin) {
        auth->failed_attempts++;
        *out_status = -2; // PIN mismatch
        return -2;
    }
    
    auth->successful_attempts++;
    *out_status = 1; // Authenticated
    return 0;
}

// Scenario 146: NBS FIPS PUB 62 Magnetic Tape Subsystem Interface
void tsfi_fips62_tape_init(tsfi_fips62_tape *tape) {
    if (!tape) return;
    tape->block_position = 0;
    tape->is_bot = 1;
    tape->is_eot = 0;
    tape->tape_mark_detected = 0;
}

int tsfi_fips62_tape_command(tsfi_fips62_tape *tape, uint8_t cmd_code, int *out_block_pos) {
    if (!tape) return -1;
    
    if (cmd_code == 0x0F) { // REWIND
        tape->block_position = 0;
        tape->is_bot = 1;
        tape->is_eot = 0;
        tape->tape_mark_detected = 0;
    } else if (cmd_code == 0x3F) { // FORWARD SPACE BLOCK
        tape->block_position++;
        tape->is_bot = 0;
        if (tape->block_position >= 1000) {
            tape->is_eot = 1;
        }
    } else if (cmd_code == 0x27) { // BACKSPACE RECORD
        if (tape->block_position > 0) {
            tape->block_position--;
            tape->is_eot = 0;
        }
        if (tape->block_position == 0) {
            tape->is_bot = 1;
        }
    } else if (cmd_code == 0x1F) { // WRITE TAPE MARK
        tape->tape_mark_detected = 1;
    } else {
        return -2; // Unknown command
    }
    
    if (out_block_pos) {
        *out_block_pos = tape->block_position;
    }
    return 0;
}

int tsfi_fips62_tape_read_to_virtual(tsfi_fips62_tape *tape, uint32_t virtual_addr,
                                     tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables,
                                     uint8_t *memory_pool, int mem_size,
                                     const uint8_t *tape_data, uint16_t data_len) {
    if (!tape || !memory_pool || !tape_data || data_len == 0) return -1;
    
    uint32_t physical_addr = 0;
    int write_protected = 0;
    
    // Perform DAT Address Translation
    int translate_res = tsfi_s370_dat_translate(virtual_addr, seg_table, seg_count, page_tables, &physical_addr, &write_protected);
    if (translate_res != 0) {
        return -3; // DAT Translation Exception
    }
    
    if (write_protected) {
        return -4; // Page Protection Exception
    }
    
    if (physical_addr + data_len > (uint32_t)mem_size) {
        return -5; // Memory out of bounds
    }
    
    // Copy tape block payload to physical real memory
    memcpy(memory_pool + physical_addr, tape_data, data_len);
    tape->block_position++;
    tape->is_bot = 0;
    
    return 0;
}

void tsfi_fips63_disk_init(tsfi_fips63_disk *disk) {
    if (!disk) return;
    disk->current_cylinder = 0;
    disk->current_sector = 0;
    disk->is_ready = 1;
    disk->recalibrate_requested = 0;
}

int tsfi_fips63_disk_command(tsfi_fips63_disk *disk, uint8_t cmd_code, uint32_t cylinder, uint32_t sector, uint8_t *out_status) {
    if (!disk || !out_status) return -1;
    *out_status = 0x00; // Default status is Normal/Success
    
    if (cmd_code == 0x07) { // SEEK CYLINDER
        if (cylinder >= 45) {
            *out_status = 0x02; // Seek Check (Out of range error)
            return -2;
        }
        disk->current_cylinder = cylinder;
    } else if (cmd_code == 0x01) { // WRITE SECTOR
        disk->current_sector = sector;
        *out_status = 0x0C; // Channel End + Device End
    } else if (cmd_code == 0x02) { // READ SECTOR
        disk->current_sector = sector;
        *out_status = 0x0C; // Channel End + Device End
    } else if (cmd_code == 0x03) { // RECALIBRATE DRIVE HEAD
        disk->current_cylinder = 0;
        disk->current_sector = 0;
        disk->recalibrate_requested = 1;
    } else {
        *out_status = 0x04; // Command Reject (Unit Check)
        return -3;
    }
    
    return 0;
}

void tsfi_fips68_basic_init(tsfi_fips68_basic *basic) {
    if (!basic) return;
    memset(basic, 0, sizeof(tsfi_fips68_basic));
}

int tsfi_fips68_basic_run(tsfi_fips68_basic *basic, const char *source_code, char *out_buffer, int max_len) {
    if (!basic || !source_code || !out_buffer || max_len <= 0) return -1;
    
    out_buffer[0] = '\0';
    int out_pos = 0;
    
    // Copy source to tokenise lines safely
    char source_copy[1024];
    snprintf(source_copy, sizeof(source_copy), "%s", source_code);
    
    char *line = strtok(source_copy, "\n");
    while (line != NULL) {
        // Skip leading spaces or line numbers (standard BASIC has line numbers like "10 LET A = 5")
        while (*line == ' ' || (*line >= '0' && *line <= '9')) {
            if (*line >= '0' && *line <= '9') {
                basic->last_executed_line = basic->last_executed_line * 10 + (*line - '0');
            }
            line++;
        }
        
        while (*line == ' ') line++;
        
        if (strncmp(line, "LET ", 4) == 0) {
            char var = line[4];
            if (var >= 'A' && var <= 'Z') {
                char *eq = strchr(line, '=');
                if (eq) {
                    basic->variables[var - 'A'] = atoi(eq + 1);
                }
            }
        } else if (strncmp(line, "PRINT ", 6) == 0) {
            char var = line[6];
            if (var >= 'A' && var <= 'Z') {
                int val = basic->variables[var - 'A'];
                out_pos += snprintf(out_buffer + out_pos, max_len - out_pos, "%d\n", val);
            } else if (line[6] == '"') {
                char *end_quote = strchr(line + 7, '"');
                if (end_quote) {
                    *end_quote = '\0';
                    out_pos += snprintf(out_buffer + out_pos, max_len - out_pos, "%s\n", line + 7);
                }
            }
        } else if (strncmp(line, "END", 3) == 0) {
            break;
        }
        
        line = strtok(NULL, "\n");
    }
    
    return 0;
}

int tsfi_fips69_parse_numeric(const char *numeric_str, double *out_val) {
    if (!numeric_str || !out_val) return -1;
    
    // FIPS 69 mandates that a valid numeric string must start with a sign ('+' or '-') or a digit
    // Let's strip leading spaces
    while (*numeric_str == ' ') numeric_str++;
    
    if (*numeric_str != '+' && *numeric_str != '-' && (*numeric_str < '0' || *numeric_str > '9') && *numeric_str != '.') {
        return -2; // Structural compliance error
    }
    
    // Parse using standard strtod
    char *endptr = NULL;
    double val = strtod(numeric_str, &endptr);
    if (endptr == numeric_str) {
        return -3; // Parsing failure
    }
    
    *out_val = val;
    return 0;
}

int tsfi_fips69_format_numeric(double val, char *out_str, int max_len) {
    if (!out_str || max_len <= 0) return -1;
    
    // FIPS 69 mandates explicit signs for non-negative values as well
    int len;
    if (val >= 0.0) {
        len = snprintf(out_str, max_len, "+%.6f", val);
    } else {
        len = snprintf(out_str, max_len, "%.6f", val);
    }
    
    // Remove trailing zeros to normalize
    if (len > 0) {
        int idx = len - 1;
        while (idx > 0 && out_str[idx] == '0') {
            out_str[idx] = '\0';
            idx--;
        }
        if (out_str[idx] == '.') {
            out_str[idx] = '\0';
        }
    }
    
    return 0;
}

int tsfi_fips79_parse_label(const uint8_t *label_block, char *out_file_id, uint32_t *out_serial, int *out_block_count) {
    if (!label_block || !out_file_id || !out_serial || !out_block_count) return -1;
    
    // Label identifiers must match VOL1, HDR1, or EOF1
    if (strncmp((const char*)label_block, "VOL1", 4) != 0 &&
        strncmp((const char*)label_block, "HDR1", 4) != 0 &&
        strncmp((const char*)label_block, "EOF1", 4) != 0) {
        return -2; // Unknown or invalid label structure
    }
    
    // Parse File ID (offset 4 to 20, 16 chars)
    snprintf(out_file_id, 17, "%.16s", (const char*)label_block + 4);
    
    // Parse Serial (offset 20 to 26, 6 digits)
    char serial_str[7];
    snprintf(serial_str, sizeof(serial_str), "%.6s", (const char*)label_block + 20);
    *out_serial = (uint32_t)atoi(serial_str);
    
    // Parse Block Count (offset 26 to 32, 6 digits)
    char count_str[7];
    snprintf(count_str, sizeof(count_str), "%.6s", (const char*)label_block + 26);
    *out_block_count = atoi(count_str);
    
    return 0;
}

int tsfi_fips79_format_label(uint8_t *out_block, const char *file_id, uint32_t serial, int block_count, const char *label_type) {
    if (!out_block || !file_id || !label_type) return -1;
    
    // Format EBCDIC/ASCII 80-character standard label record
    int len = snprintf((char*)out_block, 81, "%-4s%-16.16s%06u%06d%-48s", label_type, file_id, serial, block_count, "");
    if (len < 80) {
        // Space pad up to 80 characters
        memset(out_block + len, ' ', 80 - len);
    }
    out_block[80] = '\0';
    return 0;
}

int tsfi_fips81_encrypt_cbc(tsfi_crypto_subsystem *crypto, const uint8_t *plain, uint8_t *cipher, int blocks, const uint8_t *iv, int supervisor_state) {
    if (!crypto || !plain || !cipher || blocks <= 0 || !iv) return -1;
    if (!supervisor_state) return -2;
    
    uint8_t prev[8];
    memcpy(prev, iv, 8);
    
    for (int b = 0; b < blocks; b++) {
        uint8_t block_in[8];
        for (int i = 0; i < 8; i++) {
            block_in[i] = plain[b * 8 + i] ^ prev[i];
        }
        
        int res = tsfi_crypto_encrypt(crypto, block_in, cipher + b * 8, supervisor_state);
        if (res != 0) return res;
        
        memcpy(prev, cipher + b * 8, 8);
    }
    return 0;
}

int tsfi_fips81_decrypt_cbc(tsfi_crypto_subsystem *crypto, const uint8_t *cipher, uint8_t *plain, int blocks, const uint8_t *iv, int supervisor_state) {
    if (!crypto || !cipher || !plain || blocks <= 0 || !iv) return -1;
    if (!supervisor_state) return -2;
    
    uint8_t prev[8];
    memcpy(prev, iv, 8);
    
    for (int b = 0; b < blocks; b++) {
        uint8_t decrypted[8];
        int res = tsfi_crypto_decrypt(crypto, cipher + b * 8, decrypted, supervisor_state);
        if (res != 0) return res;
        
        for (int i = 0; i < 8; i++) {
            plain[b * 8 + i] = decrypted[i] ^ prev[i];
        }
        
        memcpy(prev, cipher + b * 8, 8);
    }
    return 0;
}

void tsfi_fips94_power_init(tsfi_fips94_monitor *mon) {
    if (!mon) return;
    memset(mon, 0, sizeof(tsfi_fips94_monitor));
}

int tsfi_fips94_audit_voltage(tsfi_fips94_monitor *mon, double nominal_voltage, double actual_voltage, int *out_fault_type) {
    if (!mon || !out_fault_type || nominal_voltage <= 0.0) return -1;
    *out_fault_type = 0; // Normal status
    
    double deviation = actual_voltage / nominal_voltage;
    
    if (deviation <= 0.90) {
        mon->sag_count++;
        mon->total_events++;
        *out_fault_type = 1; // Sag Fault
        mon->unsafe_power_state = 1;
        return -2;
    } else if (deviation >= 1.10 && deviation < 1.30) {
        mon->surge_count++;
        mon->total_events++;
        *out_fault_type = 2; // Surge Fault
        mon->unsafe_power_state = 1;
        return -3;
    } else if (deviation >= 1.30) {
        mon->transient_count++;
        mon->total_events++;
        *out_fault_type = 3; // Transient Spike
        mon->unsafe_power_state = 1;
        return -4;
    }
    
    // Normal operation range
    mon->unsafe_power_state = 0;
    return 0;
}

void tsfi_fips73_audit_init(tsfi_fips73_auditor *auditor) {
    if (!auditor) return;
    memset(auditor, 0, sizeof(tsfi_fips73_auditor));
}

int tsfi_fips73_audit_transaction(tsfi_fips73_auditor *auditor, const char *record_name, int payload_val) {
    if (!auditor) return -1;
    
    auditor->parsed_transactions++;
    
    // Validation checks under FIPS 73 Processing Controls
    if (!record_name || record_name[0] == '\0' || payload_val <= 0) {
        auditor->validation_failures++;
        return -2; // Validation check failed
    }
    
    auditor->valid_transactions++;
    return 0;
}

int tsfi_fips38_audit_document(const char *doc_content, int *out_completeness_percent) {
    if (!doc_content || !out_completeness_percent) return -1;
    
    int score = 0;
    
    // Check for FIPS PUB 38 Phase 1: Functional Requirements
    if (strstr(doc_content, "Functional Requirements") != NULL) {
        score += 25;
    }
    
    // Check for FIPS PUB 38 Phase 2: System/Subsystem Specifications
    if (strstr(doc_content, "System Specifications") != NULL || strstr(doc_content, "Subsystem Specifications") != NULL) {
        score += 25;
    }
    
    // Check for FIPS PUB 38 Phase 3: Program Specifications
    if (strstr(doc_content, "Program Specifications") != NULL) {
        score += 25;
    }
    
    // Check for FIPS PUB 38 Phase 4: User Manual
    if (strstr(doc_content, "User Manual") != NULL) {
        score += 25;
    }
    
    *out_completeness_percent = score;
    return 0;
}

int tsfi_fips1_ascii_to_ebcdic(const char *ascii_in, uint8_t *ebcdic_out, int len) {
    if (!ascii_in || !ebcdic_out || len <= 0) return -1;
    for (int i = 0; i < len; i++) {
        char c = ascii_in[i];
        if (c >= 'A' && c <= 'I') ebcdic_out[i] = 0xC1 + (c - 'A');
        else if (c >= 'J' && c <= 'R') ebcdic_out[i] = 0xD1 + (c - 'J');
        else if (c >= 'S' && c <= 'Z') ebcdic_out[i] = 0xE2 + (c - 'S');
        else if (c >= 'a' && c <= 'i') ebcdic_out[i] = 0x81 + (c - 'a');
        else if (c >= 'j' && c <= 'r') ebcdic_out[i] = 0x91 + (c - 'j');
        else if (c >= 's' && c <= 'z') ebcdic_out[i] = 0xA2 + (c - 's');
        else if (c >= '0' && c <= '9') ebcdic_out[i] = 0xF0 + (c - '0');
        else if (c == ' ') ebcdic_out[i] = 0x40;
        else ebcdic_out[i] = 0x6F; // Default question mark replacement
    }
    return 0;
}

int tsfi_fips1_ebcdic_to_ascii(const uint8_t *ebcdic_in, char *ascii_out, int len) {
    if (!ebcdic_in || !ascii_out || len <= 0) return -1;
    for (int i = 0; i < len; i++) {
        uint8_t e = ebcdic_in[i];
        if (e >= 0xC1 && e <= 0xC9) ascii_out[i] = 'A' + (e - 0xC1);
        else if (e >= 0xD1 && e <= 0xD9) ascii_out[i] = 'J' + (e - 0xD1);
        else if (e >= 0xE2 && e <= 0xE9) ascii_out[i] = 'S' + (e - 0xE2);
        else if (e >= 0x81 && e <= 0x89) ascii_out[i] = 'a' + (e - 0x81);
        else if (e >= 0x91 && e <= 0x99) ascii_out[i] = 'j' + (e - 0x91);
        else if (e >= 0xA2 && e <= 0xA9) ascii_out[i] = 's' + (e - 0xA2);
        else if (e >= 0xF0 && e <= 0xF9) ascii_out[i] = '0' + (e - 0xF0);
        else if (e == 0x40) ascii_out[i] = ' ';
        else ascii_out[i] = '?';
    }
    return 0;
}

int tsfi_fips16_serialize(uint8_t byte, int use_even_parity, uint16_t *out_bits) {
    if (!out_bits) return -1;
    
    // Count 1s in byte for parity
    int ones = 0;
    for (int i = 0; i < 8; i++) {
        if ((byte >> i) & 1) ones++;
    }
    
    // Compute parity bit
    int parity_bit = 0;
    if (use_even_parity) {
        parity_bit = (ones % 2 != 0) ? 1 : 0;
    } else {
        parity_bit = (ones % 2 == 0) ? 1 : 0;
    }
    
    // Format output bit sequencing (LSB first, then parity)
    uint16_t bits = 0;
    for (int i = 0; i < 8; i++) {
        bits |= (((byte >> i) & 1) << i);
    }
    bits |= (parity_bit << 8);
    
    *out_bits = bits;
    return 0;
}

int tsfi_fips16_deserialize(uint16_t bits, int use_even_parity, uint8_t *out_byte) {
    if (!out_byte) return -1;
    
    uint8_t byte = (uint8_t)(bits & 0xFF);
    int parity_bit = (bits >> 8) & 1;
    
    // Validate parity
    int ones = 0;
    for (int i = 0; i < 8; i++) {
        if ((byte >> i) & 1) ones++;
    }
    
    int expected_parity = 0;
    if (use_even_parity) {
        expected_parity = (ones % 2 != 0) ? 1 : 0;
    } else {
        expected_parity = (ones % 2 == 0) ? 1 : 0;
    }
    
    if (parity_bit != expected_parity) {
        return -2; // Parity Check Failure
    }
    
    *out_byte = byte;
    return 0;
}

int tsfi_fips41_authorize(const char *user_role, const char *action) {
    if (!user_role || !action) return -1;
    
    if (strcmp(user_role, "Owner") == 0) {
        return 0; // Authorized for all actions
    } else if (strcmp(user_role, "Operator") == 0) {
        if (strcmp(action, "read") == 0 || strcmp(action, "write") == 0) {
            return 0;
        }
    } else if (strcmp(user_role, "Auditor") == 0) {
        if (strcmp(action, "read") == 0 || strcmp(action, "audit") == 0) {
            return 0;
        }
    }
    
    return -2; // Privilege violation
}

int tsfi_fips113_generate_mac(tsfi_crypto_subsystem *crypto, const uint8_t *data, int len, uint8_t *out_mac, int supervisor_state) {
    if (!crypto || !data || len <= 0 || !out_mac) return -1;
    if (!supervisor_state) return -2;
    
    // Determine block counts, zero pad last block if needed
    int num_blocks = (len + 7) / 8;
    uint8_t zero_iv[8] = { 0 };
    uint8_t prev[8];
    memcpy(prev, zero_iv, 8);
    
    for (int b = 0; b < num_blocks; b++) {
        uint8_t block_in[8] = { 0 };
        int bytes_to_copy = (b == num_blocks - 1) ? (len - b * 8) : 8;
        memcpy(block_in, data + b * 8, bytes_to_copy);
        
        uint8_t xor_in[8];
        for (int i = 0; i < 8; i++) {
            xor_in[i] = block_in[i] ^ prev[i];
        }
        
        uint8_t block_out[8];
        int res = tsfi_crypto_encrypt(crypto, xor_in, block_out, supervisor_state);
        if (res != 0) return res;
        
        memcpy(prev, block_out, 8);
    }
    
    memcpy(out_mac, prev, 8);
    return 0;
}

int tsfi_fips113_verify_mac(tsfi_crypto_subsystem *crypto, const uint8_t *data, int len, const uint8_t *expected_mac, int supervisor_state) {
    if (!crypto || !data || len <= 0 || !expected_mac) return -1;
    
    uint8_t computed_mac[8];
    int res = tsfi_fips113_generate_mac(crypto, data, len, computed_mac, supervisor_state);
    if (res != 0) return res;
    
    if (memcmp(computed_mac, expected_mac, 8) == 0) {
        return 0; // Verified successfully
    }
    
    return -3; // MAC mismatch
}
