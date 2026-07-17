#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "tsfi_ramac_layout.h"
#include "tsfi_mainframe_v370.h"

void tsfi_cp_term_opts_init(tsfi_cp_terminal_options *opts) {
    if (!opts) return;
    opts->chardel_char = '@';
    opts->linedel_char = '#';
    opts->is_chardel_enabled = 1;
    opts->is_linedel_enabled = 1;
}

int tsfi_cp_term_opts_config(tsfi_cp_terminal_options *opts, const char *param, int enable) {
    if (!opts || !param) return -1;
    if (strcasecmp(param, "CHARDEL") == 0) {
        opts->is_chardel_enabled = enable;
        return 0;
    }
    if (strcasecmp(param, "LINEDEL") == 0) {
        opts->is_linedel_enabled = enable;
        return 0;
    }
    return -1;
}

void tsfi_cp_term_opts_process(const tsfi_cp_terminal_options *opts, const char *in_buf, char *out_buf, int out_max) {
    if (!opts || !in_buf || !out_buf || out_max <= 0) return;
    int write_idx = 0;
    for (int i = 0; in_buf[i] != '\0' && write_idx < out_max - 1; i++) {
        char c = in_buf[i];
        if (opts->is_chardel_enabled && c == opts->chardel_char) {
            if (write_idx > 0) {
                write_idx--;
            }
        } else if (opts->is_linedel_enabled && c == opts->linedel_char) {
            write_idx = 0;
        } else {
            out_buf[write_idx++] = c;
        }
    }
    out_buf[write_idx] = '\0';
}

void tsfi_cp_link_init(tsfi_cp_link_manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(tsfi_cp_link_manager));
}

int tsfi_cp_link_register(tsfi_cp_link_manager *mgr, const char *owner, uint32_t vdev, const char *pwd) {
    if (!mgr || !owner || mgr->disk_count >= MAX_MINIDISKS) return -1;
    strncpy(mgr->disks[mgr->disk_count].owner_uid, owner, sizeof(mgr->disks[mgr->disk_count].owner_uid) - 1);
    mgr->disks[mgr->disk_count].owner_uid[sizeof(mgr->disks[mgr->disk_count].owner_uid) - 1] = '\0';
    mgr->disks[mgr->disk_count].owner_vdev = vdev;
    if (pwd) {
        strncpy(mgr->disks[mgr->disk_count].read_write_pwd, pwd, sizeof(mgr->disks[mgr->disk_count].read_write_pwd) - 1);
        mgr->disks[mgr->disk_count].read_write_pwd[sizeof(mgr->disks[mgr->disk_count].read_write_pwd) - 1] = '\0';
    }
    mgr->disk_count++;
    return 0;
}

int tsfi_cp_link_execute(tsfi_cp_link_manager *mgr, const char *requester, const char *owner, uint32_t owner_vdev, uint32_t my_vdev, const char *mode, const char *provided_pwd) {
    if (!mgr || !requester || !owner || !mode) return -1;
    if (mgr->link_count >= MAX_MINIDISK_LINKS) return -3;
    
    int disk_idx = -1;
    for (int i = 0; i < mgr->disk_count; i++) {
        if (strcmp(mgr->disks[i].owner_uid, owner) == 0 && mgr->disks[i].owner_vdev == owner_vdev) {
            disk_idx = i;
            break;
        }
    }
    if (disk_idx == -1) return -1;
    
    int write_access = 0;
    if (strcasecmp(mode, "MR") == 0) {
        if (!provided_pwd || strcmp(mgr->disks[disk_idx].read_write_pwd, provided_pwd) != 0) {
            return -2;
        }
        write_access = 1;
    }
    
    strncpy(mgr->links[mgr->link_count].target_uid, owner, sizeof(mgr->links[mgr->link_count].target_uid) - 1);
    mgr->links[mgr->link_count].target_uid[sizeof(mgr->links[mgr->link_count].target_uid) - 1] = '\0';
    mgr->links[mgr->link_count].my_vdev = my_vdev;
    mgr->links[mgr->link_count].has_write_access = write_access;
    mgr->link_count++;
    return 0;
}

int tsfi_cp_query_disk(const tsfi_cp_link_manager *mgr, char *out_buf, int out_max) {
    if (!mgr || !out_buf || out_max <= 0) return -1;
    if (mgr->link_count == 0) {
        snprintf(out_buf, out_max, "NO ACTIVE MINIDISKS");
        return 0;
    }
    
    int write_idx = 0;
    out_buf[0] = '\0';
    for (int i = 0; i < mgr->link_count; i++) {
        char temp[64];
        snprintf(temp, sizeof(temp), "%03X: OWNER=%s, ACCESS=%s; ",
                 mgr->links[i].my_vdev,
                 mgr->links[i].target_uid,
                 mgr->links[i].has_write_access ? "RW" : "RO");
        
        int temp_len = strlen(temp);
        if (write_idx + temp_len < out_max - 1) {
            strcpy(out_buf + write_idx, temp);
            write_idx += temp_len;
        } else {
            break;
        }
    }
    return 0;
}

void tsfi_cp_device_table_init(tsfi_cp_device_table *tbl) {
    if (!tbl) return;
    memset(tbl, 0, sizeof(tsfi_cp_device_table));
}

int tsfi_cp_device_define(tsfi_cp_device_table *tbl, const char *type, uint32_t vdev) {
    if (!tbl || !type) return -1;
    if (tbl->count >= MAX_DYN_DEVICES) return -1;
    
    if (strcasecmp(type, "GRAF") != 0 && strcasecmp(type, "LINE") != 0) {
        return -2;
    }
    
    for (int i = 0; i < tbl->count; i++) {
        if (tbl->devices[i].vdev == vdev) {
            return -3;
        }
    }
    
    tbl->devices[tbl->count].vdev = vdev;
    strncpy(tbl->devices[tbl->count].dev_type, type, sizeof(tbl->devices[tbl->count].dev_type) - 1);
    tbl->devices[tbl->count].dev_type[sizeof(tbl->devices[tbl->count].dev_type) - 1] = '\0';
    tbl->count++;
    return 0;
}

int tsfi_cp_query_virtual(const tsfi_cp_device_table *tbl, char *out_buf, int out_max) {
    if (!tbl || !out_buf || out_max <= 0) return -1;
    if (tbl->count == 0) {
        snprintf(out_buf, out_max, "NO VIRTUAL DEVICES");
        return 0;
    }
    
    int write_idx = 0;
    out_buf[0] = '\0';
    for (int i = 0; i < tbl->count; i++) {
        char temp[64];
        snprintf(temp, sizeof(temp), "%03X: TYPE=%s; ",
                 tbl->devices[i].vdev,
                 tbl->devices[i].dev_type);
        
        int temp_len = strlen(temp);
        if (write_idx + temp_len < out_max - 1) {
            strcpy(out_buf + write_idx, temp);
            write_idx += temp_len;
        } else {
            break;
        }
    }
    return 0;
}

void tsfi_cp_logout_execute(tsfi_cp_device_table *tbl, tsfi_cp_link_manager *link_mgr, tsfi_cp_terminal_sleep *sleep_state) {
    if (tbl) {
        tbl->count = 0;
    }
    if (link_mgr) {
        link_mgr->link_count = 0;
    }
    if (sleep_state) {
        sleep_state->is_sleeping = 0;
        sleep_state->remaining_seconds = 0;
    }
}

void tsfi_cp_punch_spooler_init(tsfi_cp_punch_spooler *spl) {
    if (!spl) return;
    memset(spl, 0, sizeof(tsfi_cp_punch_spooler));
}

int tsfi_cp_punch_write(tsfi_cp_punch_spooler *spl, const char *card_data) {
    if (!spl || !card_data) return -1;
    if (spl->card_count >= MAX_PUNCH_QUEUE) return -1;
    
    strncpy(spl->cards[spl->card_count].data, card_data, 79);
    spl->cards[spl->card_count].data[79] = '\0';
    spl->card_count++;
    return 0;
}

int tsfi_cp_punch_set_hold(tsfi_cp_punch_spooler *spl, int hold) {
    if (!spl) return -1;
    spl->is_held = hold;
    return 0;
}

int tsfi_cp_punch_flush(tsfi_cp_punch_spooler *spl, int *cards_flushed) {
    if (!spl || !cards_flushed) return -1;
    if (spl->is_held) {
        return -2;
    }
    *cards_flushed = spl->card_count;
    spl->card_count = 0;
    return 0;
}

void tsfi_cms_filedef_init(tsfi_cms_filedef_table *tbl) {
    if (!tbl) return;
    memset(tbl, 0, sizeof(tsfi_cms_filedef_table));
}

int tsfi_cms_filedef_bind(tsfi_cms_filedef_table *tbl, const char *ddname, const char *dsname, const char *device_class) {
    if (!tbl || !ddname || !dsname || !device_class) return -1;
    
    for (int i = 0; i < tbl->count; i++) {
        if (strcasecmp(tbl->entries[i].ddname, ddname) == 0) {
            strncpy(tbl->entries[i].dsname, dsname, sizeof(tbl->entries[i].dsname) - 1);
            tbl->entries[i].dsname[sizeof(tbl->entries[i].dsname) - 1] = '\0';
            strncpy(tbl->entries[i].device_class, device_class, sizeof(tbl->entries[i].device_class) - 1);
            tbl->entries[i].device_class[sizeof(tbl->entries[i].device_class) - 1] = '\0';
            return 0;
        }
    }
    
    if (tbl->count >= MAX_FILEDEFS) return -1;
    strncpy(tbl->entries[tbl->count].ddname, ddname, sizeof(tbl->entries[tbl->count].ddname) - 1);
    tbl->entries[tbl->count].ddname[sizeof(tbl->entries[tbl->count].ddname) - 1] = '\0';
    strncpy(tbl->entries[tbl->count].dsname, dsname, sizeof(tbl->entries[tbl->count].dsname) - 1);
    tbl->entries[tbl->count].dsname[sizeof(tbl->entries[tbl->count].dsname) - 1] = '\0';
    strncpy(tbl->entries[tbl->count].device_class, device_class, sizeof(tbl->entries[tbl->count].device_class) - 1);
    tbl->entries[tbl->count].device_class[sizeof(tbl->entries[tbl->count].device_class) - 1] = '\0';
    tbl->count++;
    return 0;
}

int tsfi_cms_filedef_resolve(const tsfi_cms_filedef_table *tbl, const char *ddname, char *out_dsname, int max_len) {
    if (!tbl || !ddname || !out_dsname || max_len <= 0) return -1;
    for (int i = 0; i < tbl->count; i++) {
        if (strcasecmp(tbl->entries[i].ddname, ddname) == 0) {
            strncpy(out_dsname, tbl->entries[i].dsname, max_len - 1);
            out_dsname[max_len - 1] = '\0';
            return 0;
        }
    }
    return -1;
}

void tsfi_rscs_init(tsfi_rscs_manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(tsfi_rscs_manager));
}

int tsfi_rscs_add_node(tsfi_rscs_manager *mgr, const char *name) {
    if (!mgr || !name || mgr->node_count >= MAX_RSCS_NODES) return -1;
    strncpy(mgr->nodes[mgr->node_count].node_name, name, sizeof(mgr->nodes[mgr->node_count].node_name) - 1);
    mgr->nodes[mgr->node_count].node_name[sizeof(mgr->nodes[mgr->node_count].node_name) - 1] = '\0';
    mgr->nodes[mgr->node_count].is_active = 1;
    mgr->nodes[mgr->node_count].routed_files = 0;
    mgr->node_count++;
    return 0;
}

int tsfi_rscs_route_spool(tsfi_rscs_manager *mgr, const char *target_node, int spool_file_id) {
    if (!mgr || !target_node || spool_file_id < 0) return -1;
    for (int i = 0; i < mgr->node_count; i++) {
        if (strcmp(mgr->nodes[i].node_name, target_node) == 0) {
            if (!mgr->nodes[i].is_active) {
                return -2;
            }
            mgr->nodes[i].routed_files++;
            return 0;
        }
    }
    return -1;
}

int tsfi_rscs_deactivate_node(tsfi_rscs_manager *mgr, const char *name) {
    if (!mgr || !name) return -1;
    for (int i = 0; i < mgr->node_count; i++) {
        if (strcmp(mgr->nodes[i].node_name, name) == 0) {
            mgr->nodes[i].is_active = 0;
            return 0;
        }
    }
    return -1;
}

void tsfi_cp_fcb_init(tsfi_cp_fcb *fcb) {
    if (!fcb) return;
    memset(fcb, 0, sizeof(tsfi_cp_fcb));
    strcpy(fcb->fcb_name, "DEFAULT");
    fcb->page_length_lines = 66;
}

int tsfi_cp_fcb_load(tsfi_cp_fcb *fcb, const char *name, int page_len) {
    if (!fcb || !name || page_len <= 0 || page_len > 256) return -1;
    strncpy(fcb->fcb_name, name, sizeof(fcb->fcb_name) - 1);
    fcb->fcb_name[sizeof(fcb->fcb_name) - 1] = '\0';
    fcb->page_length_lines = page_len;
    memset(fcb->channel_stops, 0, sizeof(fcb->channel_stops));
    return 0;
}

int tsfi_cp_fcb_set_channel(tsfi_cp_fcb *fcb, int channel, int line) {
    if (!fcb || channel < 1 || channel > 12 || line < 0 || line > fcb->page_length_lines) {
        return -1;
    }
    fcb->channel_stops[channel - 1] = line;
    return 0;
}

void tsfi_cp_spool_router_init(tsfi_cp_spool_class_router *router) {
    if (!router) return;
    memset(router, 0, sizeof(tsfi_cp_spool_class_router));
}

int tsfi_cp_spool_router_set(tsfi_cp_spool_class_router *router, const char *dev_name, char class_char) {
    if (!router || !dev_name) return -1;
    
    if (class_char != '*' && (class_char < 'A' || class_char > 'Z')) {
        return -2;
    }
    
    for (int i = 0; i < router->count; i++) {
        if (strcmp(router->filters[i].device_name, dev_name) == 0) {
            router->filters[i].active_class = class_char;
            return 0;
        }
    }
    
    if (router->count >= MAX_SPOOL_CLASSES) return -1;
    strncpy(router->filters[router->count].device_name, dev_name, sizeof(router->filters[router->count].device_name) - 1);
    router->filters[router->count].device_name[sizeof(router->filters[router->count].device_name) - 1] = '\0';
    router->filters[router->count].active_class = class_char;
    router->count++;
    return 0;
}

int tsfi_cp_spool_router_match(const tsfi_cp_spool_class_router *router, const char *dev_name, char file_class) {
    if (!router || !dev_name) return -1;
    for (int i = 0; i < router->count; i++) {
        if (strcmp(router->filters[i].device_name, dev_name) == 0) {
            if (router->filters[i].active_class == '*' || router->filters[i].active_class == file_class) {
                return 1;
            }
            return 0;
        }
    }
    return 1;
}

int tsfi_cp_spool_transfer(tsfi_cp_spool_queue_v3 *src_q, tsfi_cp_spool_queue_v3 *dst_q, int file_id) {
    if (!src_q || !dst_q) return -1;
    int moved_count = 0;
    
    for (int i = 0; i < src_q->count; i++) {
        if (src_q->queue[i].file_id != 0 && (file_id == -1 || src_q->queue[i].file_id == file_id)) {
            if (tsfi_cp_spool_push_v3(dst_q, src_q->queue[i].card_data, src_q->queue[i].file_id) == 0) {
                moved_count++;
                src_q->queue[i].file_id = 0;
            }
        }
    }
    
    int write_idx = 0;
    for (int i = 0; i < src_q->count; i++) {
        if (src_q->queue[i].file_id != 0) {
            if (write_idx != i) {
                src_q->queue[write_idx] = src_q->queue[i];
            }
            write_idx++;
        }
    }
    src_q->count = write_idx;
    
    return (moved_count > 0) ? 0 : -1;
}

void tsfi_cp_dcss_init(tsfi_cp_dcss_manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(tsfi_cp_dcss_manager));
}

int tsfi_cp_dcss_register(tsfi_cp_dcss_manager *mgr, const char *name, uint32_t start_addr, uint32_t end_addr) {
    if (!mgr || !name || mgr->count >= MAX_DCSS_SEGMENTS) return -1;
    strncpy(mgr->segments[mgr->count].segment_name, name, sizeof(mgr->segments[mgr->count].segment_name) - 1);
    mgr->segments[mgr->count].segment_name[sizeof(mgr->segments[mgr->count].segment_name) - 1] = '\0';
    mgr->segments[mgr->count].start_address = start_addr;
    mgr->segments[mgr->count].end_address = end_addr;
    mgr->segments[mgr->count].is_loaded = 0;
    mgr->count++;
    return 0;
}

int tsfi_cp_dcss_diagnose_find(const tsfi_cp_dcss_manager *mgr, const char *name) {
    if (!mgr || !name) return -1;
    for (int i = 0; i < mgr->count; i++) {
        if (strcmp(mgr->segments[i].segment_name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int tsfi_cp_dcss_diagnose_load(tsfi_cp_dcss_manager *mgr, const char *name) {
    if (!mgr || !name) return -1;
    for (int i = 0; i < mgr->count; i++) {
        if (strcmp(mgr->segments[i].segment_name, name) == 0) {
            mgr->segments[i].is_loaded = 1;
            return 0;
        }
    }
    return -1;
}

int tsfi_cp_dcss_diagnose_purge(tsfi_cp_dcss_manager *mgr, const char *name) {
    if (!mgr || !name) return -1;
    for (int i = 0; i < mgr->count; i++) {
        if (strcmp(mgr->segments[i].segment_name, name) == 0) {
            mgr->segments[i].is_loaded = 0;
            return 0;
        }
    }
    return -1;
}

void tsfi_vsam_init(tsfi_vsam_file *file) {
    if (!file) return;
    memset(file, 0, sizeof(tsfi_vsam_file));
}

int tsfi_vsam_open(tsfi_vsam_file *file) {
    if (!file) return -1;
    file->is_opened = 1;
    return 0;
}

int tsfi_vsam_close(tsfi_vsam_file *file) {
    if (!file) return -1;
    file->is_opened = 0;
    return 0;
}

int tsfi_vsam_put(tsfi_vsam_file *file, const char *key, const char *val) {
    if (!file || !file->is_opened || !key || !val) return -1;
    
    for (int i = 0; i < file->count; i++) {
        if (strcmp(file->records[i].key, key) == 0) {
            strncpy(file->records[i].val, val, sizeof(file->records[i].val) - 1);
            file->records[i].val[sizeof(file->records[i].val) - 1] = '\0';
            return 0;
        }
    }
    
    if (file->count >= MAX_VSAM_RECORDS) return -1;
    
    int insert_idx = file->count;
    while (insert_idx > 0 && strcmp(file->records[insert_idx - 1].key, key) > 0) {
        file->records[insert_idx] = file->records[insert_idx - 1];
        insert_idx--;
    }
    
    strncpy(file->records[insert_idx].key, key, sizeof(file->records[insert_idx].key) - 1);
    file->records[insert_idx].key[sizeof(file->records[insert_idx].key) - 1] = '\0';
    strncpy(file->records[insert_idx].val, val, sizeof(file->records[insert_idx].val) - 1);
    file->records[insert_idx].val[sizeof(file->records[insert_idx].val) - 1] = '\0';
    file->count++;
    return 0;
}

int tsfi_vsam_get(const tsfi_vsam_file *file, const char *key, char *out_val, int max_len) {
    if (!file || !file->is_opened || !key || !out_val || max_len <= 0) return -1;
    for (int i = 0; i < file->count; i++) {
        if (strcmp(file->records[i].key, key) == 0) {
            strncpy(out_val, file->records[i].val, max_len - 1);
            out_val[max_len - 1] = '\0';
            return 0;
        }
    }
    return -1;
}

void tsfi_cp_vma_init(tsfi_cp_vma_controller *ctrl) {
    if (!ctrl) return;
    ctrl->assist_enabled = 0;
    ctrl->software_intercepts = 0;
    ctrl->assisted_instructions = 0;
}

int tsfi_cp_vma_set(tsfi_cp_vma_controller *ctrl, int enable) {
    if (!ctrl) return -1;
    ctrl->assist_enabled = enable;
    return 0;
}

int tsfi_cp_vma_execute(tsfi_cp_vma_controller *ctrl, const char *instr_type) {
    if (!ctrl || !instr_type) return -1;
    if (strcasecmp(instr_type, "SVC") != 0 && strcasecmp(instr_type, "LPSW") != 0) {
        return -1;
    }
    if (ctrl->assist_enabled) {
        ctrl->assisted_instructions++;
        return 0;
    } else {
        ctrl->software_intercepts++;
        return 1;
    }
}

void tsfi_cp_apl_screen_init(tsfi_cp_apl_screen *scr) {
    if (!scr) return;
    memset(scr->screen_buffer, ' ', sizeof(scr->screen_buffer));
}

int tsfi_cp_apl_screen_write(tsfi_cp_apl_screen *scr, int row, int col, const char *data) {
    if (!scr || !data || row < 0 || row >= APL_SCREEN_ROWS || col < 0 || col >= APL_SCREEN_COLS) {
        return -1;
    }
    int len = strlen(data);
    for (int i = 0; i < len && (col + i) < APL_SCREEN_COLS; i++) {
        scr->screen_buffer[row][col + i] = data[i];
    }
    return 0;
}

int tsfi_cp_apl_copy_to_printer(const tsfi_cp_apl_screen *scr, tsfi_cp_spool_printer *prt) {
    if (!scr || !prt) return -1;
    for (int r = 0; r < APL_SCREEN_ROWS; r++) {
        char temp_row[APL_SCREEN_COLS + 1];
        for (int c = 0; c < APL_SCREEN_COLS; c++) {
            char ch = scr->screen_buffer[r][c];
            if (ch == 0x01) {
                temp_row[c] = 'Q';
            } else {
                temp_row[c] = ch;
            }
        }
        temp_row[APL_SCREEN_COLS] = '\0';
        tsfi_cp_printer_write_record(prt, temp_row);
    }
    return 0;
}

void tsfi_cp_share_init(tsfi_cp_share_table *tbl) {
    if (!tbl) return;
    memset(tbl, 0, sizeof(tsfi_cp_share_table));
}

int tsfi_cp_share_set(tsfi_cp_share_table *tbl, const char *uid, int absolute, int val) {
    if (!tbl || !uid) return -1;
    if (absolute == 1 && (val < 1 || val > 100)) return -2;
    if (absolute == 0 && (val < 1 || val > 10000)) return -3;
    
    for (int i = 0; i < tbl->count; i++) {
        if (strcmp(tbl->entries[i].userid, uid) == 0) {
            tbl->entries[i].is_absolute = absolute;
            tbl->entries[i].value = val;
            return 0;
        }
    }
    
    if (tbl->count >= MAX_SHARE_ENTRIES) return -1;
    strncpy(tbl->entries[tbl->count].userid, uid, sizeof(tbl->entries[tbl->count].userid) - 1);
    tbl->entries[tbl->count].userid[sizeof(tbl->entries[tbl->count].userid) - 1] = '\0';
    tbl->entries[tbl->count].is_absolute = absolute;
    tbl->entries[tbl->count].value = val;
    tbl->count++;
    return 0;
}

int tsfi_cp_share_calculate_slice(const tsfi_cp_share_table *tbl, const char *uid, int total_weight_sum, int *slice_ms) {
    if (!tbl || !uid || !slice_ms) return -1;
    for (int i = 0; i < tbl->count; i++) {
        if (strcmp(tbl->entries[i].userid, uid) == 0) {
            if (tbl->entries[i].is_absolute) {
                *slice_ms = tbl->entries[i].value * 1000 / 100;
            } else {
                if (total_weight_sum <= 0) {
                    *slice_ms = 100;
                } else {
                    *slice_ms = tbl->entries[i].value * 1000 / total_weight_sum;
                }
            }
            return 0;
        }
    }
    return -1;
}

void tsfi_cp_mss_init(tsfi_cp_mss_manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(tsfi_cp_mss_manager));
}

int tsfi_cp_mss_register_group(tsfi_cp_mss_manager *mgr, const char *group_name) {
    if (!mgr || !group_name || mgr->count >= MAX_MSS_GROUPS) return -1;
    strncpy(mgr->groups[mgr->count].group_name, group_name, sizeof(mgr->groups[mgr->count].group_name) - 1);
    mgr->groups[mgr->count].group_name[sizeof(mgr->groups[mgr->count].group_name) - 1] = '\0';
    mgr->groups[mgr->count].is_mounted = 0;
    mgr->groups[mgr->count].owner_vm[0] = '\0';
    mgr->count++;
    return 0;
}

int tsfi_cp_mss_mount(tsfi_cp_mss_manager *mgr, const char *group_name, const char *vm_name) {
    if (!mgr || !group_name || !vm_name) return -1;
    for (int i = 0; i < mgr->count; i++) {
        if (strcmp(mgr->groups[i].group_name, group_name) == 0) {
            if (mgr->groups[i].is_mounted) {
                if (strcmp(mgr->groups[i].owner_vm, vm_name) == 0) {
                    return 0;
                }
                return -2;
            }
            strncpy(mgr->groups[i].owner_vm, vm_name, sizeof(mgr->groups[i].owner_vm) - 1);
            mgr->groups[i].owner_vm[sizeof(mgr->groups[i].owner_vm) - 1] = '\0';
            mgr->groups[i].is_mounted = 1;
            return 0;
        }
    }
    return -1;
}

int tsfi_cp_mss_unmount(tsfi_cp_mss_manager *mgr, const char *group_name, const char *vm_name) {
    if (!mgr || !group_name || !vm_name) return -1;
    for (int i = 0; i < mgr->count; i++) {
        if (strcmp(mgr->groups[i].group_name, group_name) == 0) {
            if (!mgr->groups[i].is_mounted) {
                return 0;
            }
            if (strcmp(mgr->groups[i].owner_vm, vm_name) != 0) {
                return -2;
            }
            mgr->groups[i].is_mounted = 0;
            mgr->groups[i].owner_vm[0] = '\0';
            return 0;
        }
    }
    return -1;
}

int tsfi_cp_mss_query(const tsfi_cp_mss_manager *mgr, const char *group_name, char *out_owner, int max_len) {
    if (!mgr || !group_name || !out_owner || max_len <= 0) return -1;
    for (int i = 0; i < mgr->count; i++) {
        if (strcmp(mgr->groups[i].group_name, group_name) == 0) {
            if (mgr->groups[i].is_mounted) {
                strncpy(out_owner, mgr->groups[i].owner_vm, max_len - 1);
                out_owner[max_len - 1] = '\0';
                return 1;
            }
            out_owner[0] = '\0';
            return 0;
        }
    }
    return -1;
}

void tsfi_cp_iucv_mp_init(tsfi_cp_iucv_multipath *mp) {
    if (!mp) return;
    memset(mp, 0, sizeof(tsfi_cp_iucv_multipath));
}

int tsfi_cp_iucv_mp_connect(tsfi_cp_iucv_multipath *mp, int path_id, const char *target) {
    if (!mp || !target || path_id < 0) return -1;
    
    for (int i = 0; i < mp->count; i++) {
        if (mp->paths[i].path_id == path_id) {
            strncpy(mp->paths[i].target_userid, target, sizeof(mp->paths[i].target_userid) - 1);
            mp->paths[i].target_userid[sizeof(mp->paths[i].target_userid) - 1] = '\0';
            mp->paths[i].is_active = 1;
            return 0;
        }
    }
    
    if (mp->count >= MAX_IUCV_PATHS) return -1;
    mp->paths[mp->count].path_id = path_id;
    strncpy(mp->paths[mp->count].target_userid, target, sizeof(mp->paths[mp->count].target_userid) - 1);
    mp->paths[mp->count].target_userid[sizeof(mp->paths[mp->count].target_userid) - 1] = '\0';
    mp->paths[mp->count].is_active = 1;
    mp->count++;
    return 0;
}

int tsfi_cp_iucv_mp_send(const tsfi_cp_iucv_multipath *mp, int path_id, const char *msg, char *out_target) {
    if (!mp || !msg || !out_target) return -1;
    for (int i = 0; i < mp->count; i++) {
        if (mp->paths[i].path_id == path_id) {
            if (!mp->paths[i].is_active) {
                return -2;
            }
            strncpy(out_target, mp->paths[i].target_userid, 15);
            out_target[15] = '\0';
            return 0;
        }
    }
    return -1;
}

int tsfi_cp_iucv_mp_disconnect(tsfi_cp_iucv_multipath *mp, int path_id) {
    if (!mp) return -1;
    for (int i = 0; i < mp->count; i++) {
        if (mp->paths[i].path_id == path_id) {
            mp->paths[i].is_active = 0;
            return 0;
        }
    }
    return -1;
}

void tsfi_cp_nucleus_init(tsfi_cp_nucleus_table *tbl) {
    if (!tbl) return;
    memset(tbl, 0, sizeof(tsfi_cp_nucleus_table));
}

int tsfi_cp_nucleus_register(tsfi_cp_nucleus_table *tbl, const char *name, uint32_t entry_addr) {
    if (!tbl || !name || tbl->count >= MAX_NUCLEUS_EXTENSIONS) return -1;
    strncpy(tbl->extensions[tbl->count].cmd_name, name, sizeof(tbl->extensions[tbl->count].cmd_name) - 1);
    tbl->extensions[tbl->count].cmd_name[sizeof(tbl->extensions[tbl->count].cmd_name) - 1] = '\0';
    tbl->extensions[tbl->count].entry_point = entry_addr;
    tbl->count++;
    return 0;
}

int tsfi_cp_resolve_command(const tsfi_cp_nucleus_table *nuc_tbl, const tsfi_cp_dcss_manager *dcss_mgr, const char *cmd, char *out_loc, uint32_t *out_addr) {
    if (!cmd || !out_loc || !out_addr) return -1;
    
    if (nuc_tbl) {
        for (int i = 0; i < nuc_tbl->count; i++) {
            if (strcmp(nuc_tbl->extensions[i].cmd_name, cmd) == 0) {
                strcpy(out_loc, "NUCLEUS");
                *out_addr = nuc_tbl->extensions[i].entry_point;
                return 0;
            }
        }
    }
    
    if (dcss_mgr) {
        int idx = tsfi_cp_dcss_diagnose_find(dcss_mgr, cmd);
        if (idx >= 0) {
            strcpy(out_loc, "DCSS");
            *out_addr = dcss_mgr->segments[idx].start_address;
            return 1;
        }
    }
    
    return -1;
}

void tsfi_cms_plist_init(tsfi_cms_plist *plist) {
    if (!plist) return;
    memset(plist->words, ' ', sizeof(plist->words));
    plist->count = 0;
    plist->sentinel = 0xFFFFFFFFFFFFFFFFULL;
}

int tsfi_cms_plist_parse(tsfi_cms_plist *plist, const char *cmd_line) {
    if (!plist || !cmd_line) return -1;
    tsfi_cms_plist_init(plist);
    
    int i = 0;
    const char *ptr = cmd_line;
    while (*ptr && i < MAX_PLIST_WORDS) {
        while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n')) {
            ptr++;
        }
        if (!*ptr) break;
        
        const char *start = ptr;
        while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\r' && *ptr != '\n') {
            ptr++;
        }
        int len = ptr - start;
        
        for (int c = 0; c < 8; c++) {
            if (c < len) {
                char ch = start[c];
                if (ch >= 'a' && ch <= 'z') {
                    plist->words[i][c] = ch - 'a' + 'A';
                } else {
                    plist->words[i][c] = ch;
                }
            } else {
                plist->words[i][c] = ' ';
            }
        }
        i++;
    }
    plist->count = i;
    return i;
}

void tsfi_cms_tape_init(tsfi_cms_tape_drive *tape) {
    if (!tape) return;
    memset(tape, 0, sizeof(tsfi_cms_tape_drive));
}

int tsfi_cms_tape_write_record(tsfi_cms_tape_drive *tape, const char *record) {
    if (!tape || !record || tape->block_count >= MAX_TAPE_BLOCKS) return -1;
    strncpy(tape->blocks[tape->block_count].data, record, sizeof(tape->blocks[tape->block_count].data) - 1);
    tape->blocks[tape->block_count].data[sizeof(tape->blocks[tape->block_count].data) - 1] = '\0';
    tape->blocks[tape->block_count].is_tape_mark = 0;
    tape->block_count++;
    return 0;
}

int tsfi_cms_tape_write_mark(tsfi_cms_tape_drive *tape) {
    if (!tape || tape->block_count >= MAX_TAPE_BLOCKS) return -1;
    tape->blocks[tape->block_count].data[0] = '\0';
    tape->blocks[tape->block_count].is_tape_mark = 1;
    tape->block_count++;
    return 0;
}

int tsfi_cms_tape_read_record(tsfi_cms_tape_drive *tape, char *out_record, int max_len, int *out_is_mark) {
    if (!tape || !out_record || !out_is_mark || tape->current_pointer >= tape->block_count) {
        return -1;
    }
    int ptr = tape->current_pointer;
    *out_is_mark = tape->blocks[ptr].is_tape_mark;
    if (tape->blocks[ptr].is_tape_mark) {
        out_record[0] = '\0';
    } else {
        strncpy(out_record, tape->blocks[ptr].data, max_len - 1);
        out_record[max_len - 1] = '\0';
    }
    tape->current_pointer++;
    return 0;
}

int tsfi_cms_tape_rewind(tsfi_cms_tape_drive *tape) {
    if (!tape) return -1;
    tape->current_pointer = 0;
    return 0;
}

int tsfi_cms_tape_skip_file(tsfi_cms_tape_drive *tape, int count) {
    if (!tape) return -1;
    if (count > 0) {
        int marks_seen = 0;
        while (tape->current_pointer < tape->block_count && marks_seen < count) {
            if (tape->blocks[tape->current_pointer].is_tape_mark) {
                marks_seen++;
            }
            tape->current_pointer++;
        }
    } else if (count < 0) {
        int target_marks = -count;
        int marks_seen = 0;
        while (tape->current_pointer > 0 && marks_seen < target_marks) {
            tape->current_pointer--;
            if (tape->blocks[tape->current_pointer].is_tape_mark) {
                marks_seen++;
            }
        }
    }
    return 0;
}
