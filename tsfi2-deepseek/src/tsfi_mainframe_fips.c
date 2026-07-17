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
