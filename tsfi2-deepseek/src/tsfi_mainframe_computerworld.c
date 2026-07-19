#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Include modular mainframe emulator subcomponents
#include "tsfi_mainframe_y2k.c"
#include "tsfi_mainframe_ebcdic.c"
#include "tsfi_mainframe_cobol.c"
#include "tsfi_mainframe_vsam.c"
#include "tsfi_mainframe_jcl.c"

// Include newly split domain subcomponents (keeping all source files under 68KB limit)
#include "tsfi_mainframe_business.c"
#include "tsfi_mainframe_operations.c"
#include "tsfi_mainframe_icp.c"

#include <math.h>

int tsfi_b5500_prt_resolve_descriptor(uint64_t descriptor, uint32_t offset, const uint64_t *memory_pool, uint32_t mem_size, uint64_t *val_out) {
    if (!memory_pool || !val_out) return -1;
    int present = (descriptor & (1ULL << 47)) ? 1 : 0;
    if (!present) return -5;
    uint32_t limit = (descriptor >> 30) & 0xFFFF;
    if (offset >= limit) return -6;
    uint32_t origin = descriptor & 0x3FFFFFFF;
    if (origin + offset >= mem_size) return -7;
    *val_out = memory_pool[origin + offset];
    return 0;
}

int tsfi_cw_honeywell_bcd_to_ebcdic(const uint8_t *bcd_in, int bcd_len, uint8_t *ebcdic_out) {
    if (!bcd_in || !ebcdic_out || bcd_len < 0) return -1;
    static const uint8_t bcd_to_ebcdic_map[64] = {
        0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
        0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
        0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
        0xE7, 0xE8, 0xE9, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4,
        0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x4B, 0x4C, 0x4D,
        0x4E, 0x4F, 0x50, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
        0x5F, 0x60, 0x61, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x50, 0x7D
    };
    for (int i = 0; i < bcd_len; i++) {
        uint8_t code = bcd_in[i] & 0x3F;
        ebcdic_out[i] = bcd_to_ebcdic_map[code];
    }
    return 0;
}

double tsfi_cw_univac_to_double(uint64_t univac_val) {
    int sign = (univac_val & (1ULL << 35)) ? 1 : 0;
    int exponent = (univac_val >> 27) & 0xFF;
    uint64_t mantissa = univac_val & 0x7FFFFFFULL;
    if (exponent == 0 && mantissa == 0) return 0.0;
    int true_exp = exponent - 128;
    double frac = (double)mantissa / (double)(1ULL << 27);
    double result = frac * pow(2.0, true_exp);
    return sign ? -result : result;
}

uint64_t tsfi_cw_double_to_univac(double val) {
    if (val == 0.0) return 0ULL;
    uint64_t sign = 0;
    if (val < 0.0) {
        sign = 1ULL << 35;
        val = -val;
    }
    int exponent = 0;
    double frac = frexp(val, &exponent);
    int biased_exp = exponent + 128;
    if (biased_exp < 0) biased_exp = 0;
    if (biased_exp > 255) biased_exp = 255;
    uint64_t mantissa = (uint64_t)(frac * (double)(1ULL << 27));
    if (mantissa > 0x7FFFFFFULL) mantissa = 0x7FFFFFFULL;
    return sign | ((uint64_t)biased_exp << 27) | mantissa;
}

int tsfi_cw_esj_analyze_paging(const tsfi_cw_esj_paging_metrics *metrics, double *thrashing_index_out, int *alert_flag_out) {
    if (!metrics || !thrashing_index_out || !alert_flag_out) return -1;
    if (metrics->total_frames < 0 || metrics->free_frames < 0 || metrics->free_frames > metrics->total_frames) return -2;
    
    int active_frames = metrics->total_frames - metrics->free_frames;
    *thrashing_index_out = (double)(metrics->page_ins + metrics->page_outs) / (double)(active_frames + 1);
    
    if (metrics->uic < 30 || *thrashing_index_out > 0.05) {
        *alert_flag_out = 1;
    } else {
        *alert_flag_out = 0;
    }
    return 0;
}

int tsfi_cw_marist_optimize_migration(const tsfi_cw_marist_zvm_migration *config, double *est_time_out, int *can_migrate_out) {
    if (!config || !est_time_out || !can_migrate_out) return -1;
    if (config->vm_memory_mb <= 0.0 || config->network_speed_mbps <= 0.0) return -2;
    
    double net_speed_mb = config->network_speed_mbps / 8.0;
    double net_rate = net_speed_mb - config->dirty_rate_mb_per_sec;
    
    if (net_rate <= 0.0) {
        *est_time_out = -1.0;
        *can_migrate_out = 0;
    } else {
        *est_time_out = config->vm_memory_mb / net_rate;
        *can_migrate_out = (*est_time_out <= config->max_migration_time_sec) ? 1 : 0;
    }
    return 0;
}

int tsfi_cw_marist_audit_sdn(const tsfi_cw_marist_sdn_rule *rules, int rule_count, const char *src_ip, const char *dest_ip, int *action_out) {
    if (!rules || rule_count < 0 || !src_ip || !dest_ip || !action_out) return -1;
    
    for (int i = 0; i < rule_count; i++) {
        if (strcmp(rules[i].src_ip, src_ip) == 0 && strcmp(rules[i].dest_ip, dest_ip) == 0) {
            *action_out = rules[i].action;
            return 0;
        }
    }
    *action_out = 2; // Default DROP
    return 0;
}

int tsfi_cw_marist_audit_tenant(const tsfi_cw_marist_tenant *tenant, int *is_nominal_out) {
    if (!tenant || !is_nominal_out) return -1;
    
    if (tenant->used_cores <= tenant->allocated_cores && tenant->used_mem_gb <= tenant->allocated_mem_gb) {
        *is_nominal_out = 1;
    } else {
        *is_nominal_out = 0;
    }
    return 0;
}

int tsfi_cw_marist_audit_isolation(const tsfi_cw_marist_guest_profile *profile, int *is_secure_out) {
    if (!profile || !is_secure_out) return -1;
    
    *is_secure_out = (profile->has_read_any_spool == 0 && (profile->privilege_class == 'G' || profile->allow_inter_vm_comm == 0)) ? 1 : 0;
    return 0;
}

int tsfi_cw_marist_audit_sysplex(const tsfi_cw_marist_sysplex_cf *cf, int *alert_out) {
    if (!cf || !alert_out) return -1;
    
    if (cf->failed_requests > 10 || cf->avg_response_time_us > 500 || cf->buffer_util_percent > 90.0) {
        *alert_out = 1;
    } else {
        *alert_out = 0;
    }
    return 0;
}

int tsfi_cw_marist_calc_cpu_shares(const tsfi_cw_marist_zvm_scheduler *sched, double share_allocs_out[16]) {
    if (!sched || !share_allocs_out) return -1;
    if (sched->vm_count < 0 || sched->vm_count > 16) return -2;
    
    for (int i = 0; i < sched->vm_count; i++) {
        if (sched->total_shares > 0) {
            share_allocs_out[i] = (double)sched->shares[i] / (double)sched->total_shares;
        } else {
            share_allocs_out[i] = 0.0;
        }
    }
    return 0;
}

int tsfi_cw_marist_audit_rmf_cpu(const tsfi_cw_marist_rmf_cpu *cpus, int cpu_count, double *avg_busy_out, int *overloaded_out) {
    if (!cpus || cpu_count <= 0 || !avg_busy_out || !overloaded_out) return -1;
    
    double total_busy = 0.0;
    *overloaded_out = 0;
    
    for (int i = 0; i < cpu_count; i++) {
        total_busy += cpus[i].busy_percent;
        if (cpus[i].dispatch_delay_ms > 20 && cpus[i].busy_percent > 95.0) {
            *overloaded_out = 1;
        }
    }
    *avg_busy_out = total_busy / cpu_count;
    return 0;
}

int tsfi_cw_marist_optimize_vswitch(const tsfi_cw_marist_vswitch *vsw, int *needs_failover_out) {
    if (!vsw || !needs_failover_out) return -1;
    
    *needs_failover_out = (vsw->drop_rate_percent > 2.0 && vsw->backup_ports_configured > 0) ? 1 : 0;
    return 0;
}

int tsfi_cw_marist_audit_wlm(const tsfi_cw_marist_wlm_service *srv, int *needs_adjustment_out) {
    if (!srv || !needs_adjustment_out) return -1;
    
    double velocity = 100.0 * srv->active_states_count / (srv->active_states_count + srv->delay_states_count + 1);
    *needs_adjustment_out = (velocity < srv->goal_velocity) ? 1 : 0;
    return 0;
}

int tsfi_cw_marist_alloc_crypto(const tsfi_cw_marist_crypto_coproc *cfg, double guest_allocs_out[16]) {
    if (!cfg || !guest_allocs_out) return -1;
    if (cfg->guest_count < 0 || cfg->guest_count > 16) return -2;
    
    for (int i = 0; i < cfg->guest_count; i++) {
        if (cfg->total_crypto_units > 0) {
            guest_allocs_out[i] = (double)cfg->guest_shares[i] / (double)cfg->total_crypto_units;
        } else {
            guest_allocs_out[i] = 0.0;
        }
    }
    return 0;
}

int tsfi_cw_marist_audit_minidisk_cache(const tsfi_cw_marist_minidisk_cache *cache, double *hit_ratio_out, int *needs_resize_out) {
    if (!cache || !hit_ratio_out || !needs_resize_out) return -1;
    
    *hit_ratio_out = (cache->read_requests > 0) ? (double)cache->read_hits / (double)cache->read_requests : 0.0;
    *needs_resize_out = (*hit_ratio_out < 0.60 && cache->read_requests > 100) ? 1 : 0;
    return 0;
}

int tsfi_cw_marist_audit_isglock(const tsfi_cw_marist_isglock *lock, int *alert_out) {
    if (!lock || !alert_out) return -1;
    
    double contention_ratio = (lock->lock_requests > 0) ? (double)lock->contended_requests / lock->lock_requests : 0.0;
    if (lock->timeouts > 0 || lock->avg_grant_delay_ms > 50 || contention_ratio > 0.15) {
        *alert_out = 1;
    } else {
        *alert_out = 0;
    }
    return 0;
}

int tsfi_cw_niu_hlasm_exec(uint32_t regs[16], const char *op, int r1, int r2, int32_t immediate, int *overflow_out) {
    if (!regs || !op || !overflow_out) return -1;
    if (r1 < 0 || r1 >= 16 || r2 < 0 || r2 >= 16) return -2;
    
    *overflow_out = 0;
    if (strcmp(op, "AR") == 0) {
        uint64_t res = (uint64_t)regs[r1] + regs[r2];
        regs[r1] = (uint32_t)res;
        *overflow_out = (res > 0xFFFFFFFFULL) ? 1 : 0;
    } else if (strcmp(op, "SR") == 0) {
        regs[r1] = regs[r1] - regs[r2];
    } else if (strcmp(op, "L") == 0) {
        regs[r1] = (uint32_t)immediate;
    } else if (strcmp(op, "ST") == 0) {
        // Simulated store
    } else {
        return -3; // Unknown opcode
    }
    return 0;
}

int tsfi_cw_niu_validate_jcl(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = (strncmp(jcl_line, "//", 2) == 0 && (strstr(jcl_line, " JOB ") != NULL || strstr(jcl_line, " EXEC ") != NULL || strstr(jcl_line, " DD ") != NULL)) ? 1 : 0;
    return 0;
}

int tsfi_cw_niu_expand_macro(const char *macro_def, const char *param, char *output_out, size_t max_len) {
    if (!macro_def || !param || !output_out || max_len == 0) return -1;
    
    const char *placeholder = "&PARAM";
    const char *found = strstr(macro_def, placeholder);
    if (!found) {
        strncpy(output_out, macro_def, max_len - 1);
        output_out[max_len - 1] = '\0';
        return 0;
    }
    
    size_t prefix_len = found - macro_def;
    if (prefix_len >= max_len) prefix_len = max_len - 1;
    
    memcpy(output_out, macro_def, prefix_len);
    output_out[prefix_len] = '\0';
    
    if (strlen(output_out) + strlen(param) < max_len) {
        strcat(output_out, param);
    }
    
    const char *suffix = found + strlen(placeholder);
    if (strlen(output_out) + strlen(suffix) < max_len) {
        strcat(output_out, suffix);
    }
    
    return 0;
}

int tsfi_cw_niu_audit_copybook(const tsfi_cw_niu_copybook_field *fields, int field_count, int *total_bytes_out) {
    if (!fields || field_count < 0 || !total_bytes_out) return -1;
    
    *total_bytes_out = 0;
    for (int i = 0; i < field_count; i++) {
        *total_bytes_out += fields[i].size_bytes;
    }
    return 0;
}

int tsfi_cw_niu_audit_jcl_dd(const char *dd_statement, int *is_valid_path_out) {
    if (!dd_statement || !is_valid_path_out) return -1;
    
    *is_valid_path_out = 0;
    const char *dsn = strstr(dd_statement, "DSN=");
    if (!dsn) dsn = strstr(dd_statement, "DSNAME=");
    if (!dsn) return 0;
    
    // Skip key
    if (strncmp(dsn, "DSN=", 4) == 0) dsn += 4;
    else dsn += 7;
    
    // Validate characters: A-Z, 0-9, @, #, $, .
    int seg_len = 0;
    int has_seg = 0;
    while (*dsn && *dsn != ',' && *dsn != ' ' && *dsn != '\n' && *dsn != '\r') {
        char c = *dsn;
        if (c == '.') {
            if (seg_len == 0 || seg_len > 8) return 0;
            seg_len = 0;
        } else {
            // First char of segment must be letter or @, #, $
            if (seg_len == 0) {
                if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '@' || c == '#' || c == '$')) {
                    return 0;
                }
            } else {
                if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '@' || c == '#' || c == '$')) {
                    return 0;
                }
            }
            seg_len++;
            has_seg = 1;
        }
        dsn++;
    }
    
    if (seg_len == 0 || seg_len > 8 || !has_seg) return 0;
    
    *is_valid_path_out = 1;
    return 0;
}

int tsfi_cw_niu_audit_working_storage(const tsfi_cw_niu_cobol_var *vars, int var_count, int *uninitialized_count_out) {
    if (!vars || var_count < 0 || !uninitialized_count_out) return -1;
    
    *uninitialized_count_out = 0;
    for (int i = 0; i < var_count; i++) {
        if (!vars[i].has_value_clause) {
            (*uninitialized_count_out)++;
        }
    }
    return 0;
}

int tsfi_cw_rmu_audit_cics_security(const char *transaction_id, const char *user_security_key, int *is_authorized_out) {
    if (!transaction_id || !user_security_key || !is_authorized_out) return -1;
    
    *is_authorized_out = (strncmp(user_security_key, "SEC_", 4) == 0 || strcmp(user_security_key, "ADMIN") == 0) ? 1 : 0;
    return 0;
}

int tsfi_cw_isu_leap_defense_audit(const tsfi_cw_isu_leap_app *app, int defcon_level, const char *transaction_id, const char *user_security_key, double *criticality_risk_out) {
    if (!app || !criticality_risk_out) return -1;
    
    int auth = 0;
    tsfi_cw_rmu_audit_cics_security(transaction_id, user_security_key, &auth);
    
    double base = (app->database_size_gb * 0.05) + (app->transaction_rate_per_sec * 0.1);
    double nato_factor = (5.0 - app->nato_irs_compliance_rating) * 15.0;
    double mult = 1.0;
    
    switch (defcon_level) {
        case 1: mult = 3.0; break;
        case 2: mult = 2.0; break;
        case 3: mult = 1.5; break;
        case 4: mult = 1.2; break;
        case 5: default: mult = 1.0; break;
    }
    
    *criticality_risk_out = (base + nato_factor) * mult;
    if (!auth) {
        *criticality_risk_out += 500.0; // massive penalty for unauthorized transaction
    }
    return 0;
}

int tsfi_cw_isu_ulid_ssa_match(const char *ulid, const char *ssn_last4, const char *transaction_id, const char *user_security_key, int *is_match_out) {
    if (!ulid || !ssn_last4 || !is_match_out) return -1;
    if (strlen(ssn_last4) != 4) return -2;
    
    int auth = 0;
    tsfi_cw_rmu_audit_cics_security(transaction_id, user_security_key, &auth);
    if (!auth) {
        *is_match_out = 0;
        return -3; // security block
    }
    
    size_t ulid_len = strlen(ulid);
    if (ulid_len == 0) {
        *is_match_out = 0;
        return 0;
    }
    
    *is_match_out = (ulid[ulid_len - 1] == ssn_last4[3]) ? 1 : 0;
    return 0;
}

int tsfi_cw_isu_audit_email(const tsfi_cw_isu_email_log *log, int *is_valid_out) {
    if (!log || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    const char *at = strchr(log->recipient_addr, '@');
    if (!at) return 0;
    if (strcmp(at, "@ilstu.edu") != 0 && strcmp(at, "@rs6000.cmp.ilstu.edu") != 0) return 0;
    
    if (log->is_mf_crossover) {
        // Enforce Micro Focus bounds: message size <= 64KB and non-empty sender
        size_t ulid_len = strlen(log->sender_ulid);
        if (log->message_bytes > 65536 || ulid_len < 2 || ulid_len > 8) {
            return 0;
        }
    }
    
    *is_valid_out = 1;
    return 0;
}

int tsfi_cw_isu_audit_sf_sla(const tsfi_cw_isu_state_farm_sla *sla, int *is_compliant_out) {
    if (!sla || !is_compliant_out) return -1;
    
    int limit = sla->max_allowed_seconds;
    if (sla->micro_focus_compat_flags & 0x01) {
        // Strict Mode: Max allowed is reduced by 20%
        limit = (int)(limit * 0.8);
    }
    
    *is_compliant_out = (sla->elapsed_seconds <= limit) ? 1 : 0;
    return 0;
}

int tsfi_cw_isu_audit_enrollment(const tsfi_cw_isu_course_enrollment *enroll, int *can_register_out) {
    if (!enroll || !can_register_out) return -1;
    
    *can_register_out = (enroll->current_enrolled < enroll->max_capacity && enroll->prereqs_met_count == enroll->total_applicants) ? 1 : 0;
    return 0;
}

int tsfi_cw_isu_audit_gateway(const tsfi_cw_isu_gateway_status *gw, int *alert_out) {
    if (!gw || !alert_out) return -1;
    
    double drop_rate = (gw->packets_sent > 0) ? (double)gw->packets_dropped / (double)gw->packets_sent : 0.0;
    *alert_out = (drop_rate > 0.05 || gw->rtt_ms > 250) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_score_contestant(const tsfi_cw_unt_mtm_contestant *student, int *score_out) {
    if (!student || !score_out) return -1;
    
    int score = 0;
    if (student->part1_completed) score += 10;
    if (student->part2_completed) score += 30;
    score += student->part3_completed_tasks * 20;
    if (student->vsam_modified) score += 10;
    score -= student->jcl_errors * 15;
    
    *score_out = (score < 0) ? 0 : score;
    return 0;
}

int tsfi_cw_unt_audit_pipeline(const tsfi_cw_unt_crypto_pipeline *pipe, int *is_compliant_out) {
    if (!pipe || !is_compliant_out) return -1;
    
    *is_compliant_out = (strcmp(pipe->encryption_protocol, "AES-256") == 0 && pipe->unencrypted_fields_count == 0 && pipe->checksum_verified == 1) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_optimize_vsam(const tsfi_cw_unt_vsam_ksds *ksds, int *recommendation_flags_out) {
    if (!ksds || !recommendation_flags_out) return -1;
    
    *recommendation_flags_out = 0;
    if (ksds->control_interval_splits > 50) {
        *recommendation_flags_out |= 0x01;
    }
    if (ksds->buffer_space_allocated_kb < 128) {
        *recommendation_flags_out |= 0x02;
    }
    return 0;
}

int tsfi_cw_unt_audit_racf(const tsfi_cw_unt_racf_log *logs, int log_count, int *violations_out) {
    if (!logs || log_count < 0 || !violations_out) return -1;
    
    *violations_out = 0;
    for (int i = 0; i < log_count; i++) {
        if (!logs[i].is_authorized) {
            (*violations_out)++;
        }
    }
    return 0;
}

int tsfi_cw_raf_allocate_vital(const tsfi_cw_raf_vital_request *req, int *approved_out, int *priority_score_out) {
    if (!req || !approved_out || !priority_score_out) return -1;
    
    *approved_out = (req->stock_available >= req->qty_requested) ? 1 : 0;
    *priority_score_out = 0;
    if (strcmp(req->priority_level, "AOG") == 0) {
        *priority_score_out = 100;
    } else if (strcmp(req->priority_level, "ROUT") == 0) {
        *priority_score_out = 10;
    }
    return 0;
}

int tsfi_cw_raf_audit_ridels(const tsfi_cw_raf_ridels_audit *items, int item_count, double *total_discrepancy_value_out) {
    if (!items || item_count < 0 || !total_discrepancy_value_out) return -1;
    
    *total_discrepancy_value_out = 0.0;
    for (int i = 0; i < item_count; i++) {
        int diff = items[i].physical_count - items[i].system_count;
        if (diff < 0) diff = -diff;
        *total_discrepancy_value_out += diff * items[i].unit_cost;
    }
    return 0;
}

int tsfi_cw_unt_audit_pds(const tsfi_cw_unt_pds_status *pds, int *needs_compress_out) {
    if (!pds || !needs_compress_out) return -1;
    
    *needs_compress_out = (pds->total_directory_blocks > 0 && (double)pds->used_directory_blocks / (double)pds->total_directory_blocks >= 0.9) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_profile_cics(const tsfi_cw_unt_cics_tran *trans, int tran_count, double *avg_response_time_out, int *slow_count_out) {
    if (!trans || tran_count <= 0 || !avg_response_time_out || !slow_count_out) return -1;
    
    double total_res = 0.0;
    *slow_count_out = 0;
    for (int i = 0; i < tran_count; i++) {
        total_res += trans[i].response_time_ms;
        if (trans[i].response_time_ms > 1000) {
            (*slow_count_out)++;
        }
    }
    *avg_response_time_out = total_res / tran_count;
    return 0;
}

int tsfi_cw_unt_cics_audit_session(const tsfi_cw_unt_cics_session *session, int *needs_reset_out) {
    if (!session || !needs_reset_out) return -1;
    
    *needs_reset_out = (session->is_active && session->latency_ms > 2000) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_cics_audit_queue(const tsfi_cw_unt_cics_queue *queue, int *alert_out) {
    if (!queue || !alert_out) return -1;
    
    *alert_out = (queue->total_bytes > 50000000 || queue->item_count > 10000) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_cics_inject_ballistic(const char *data, int data_len, tsfi_cw_unt_cics_queue *queue, int *processed_bytes_out) {
    if (!data || data_len <= 0 || !queue || !processed_bytes_out) return -1;
    
    queue->item_count++;
    queue->total_bytes += data_len;
    *processed_bytes_out = data_len;
    return 0;
}

int g_ims_violations = 0;

int tsfi_cw_rmu_audit_ims(const tsfi_cw_rmu_ims_segment *segments, int segment_count, int *invalid_pointers_out) {
    if (!segments || segment_count < 0 || !invalid_pointers_out) return -1;
    
    *invalid_pointers_out = 0;
    for (int i = 0; i < segment_count; i++) {
        if (segments[i].pointer_address == 0 || !segments[i].is_valid) {
            (*invalid_pointers_out)++;
            continue;
        }
        if (strcmp(segments[i].parent_segment_name, "ROOT") != 0) {
            int found = 0;
            for (int j = 0; j < segment_count; j++) {
                if (strcmp(segments[j].segment_name, segments[i].parent_segment_name) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                (*invalid_pointers_out)++;
            }
        }
    }
    g_ims_violations = *invalid_pointers_out;
    return 0;
}

int tsfi_cw_rmu_audit_cics_web_gateway(const tsfi_cw_rmu_cics_web_request *req, int *is_compliant_out) {
    if (!req || !is_compliant_out) return -1;
    
    *is_compliant_out = (req->is_authorized && req->payload_bytes <= 1048576 && strcmp(req->header_content_type, "application/json") == 0) ? 1 : 0;
    return 0;
}

int tsfi_cw_chase_audit_micr(const tsfi_cw_chase_micr_check *check, int *is_valid_out) {
    if (!check || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    if (strlen(check->routing_number) != 9) return 0;
    if (check->amount_dollars <= 0.0) return 0;
    
    int d[9];
    for (int i = 0; i < 9; i++) {
        if (check->routing_number[i] < '0' || check->routing_number[i] > '9') return 0;
        d[i] = check->routing_number[i] - '0';
    }
    
    int sum = 3 * (d[0] + d[3] + d[6]) + 7 * (d[1] + d[4] + d[7]) + (d[2] + d[5] + d[8]);
    if (sum % 10 == 0) {
        *is_valid_out = 1;
    }
    return 0;
}

int tsfi_cw_chase_audit_atm(const tsfi_cw_chase_atm_transaction *tx, int *is_valid_out) {
    if (!tx || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    if (strlen(tx->pin) != 4) return 0;
    for (int i = 0; i < 4; i++) {
        if (tx->pin[i] < '0' || tx->pin[i] > '9') return 0;
    }
    if (strlen(tx->card_number) < 12) return 0;
    
    // Hash/convert card number to key
    uint64_t card_key = 0;
    for (int i = 0; tx->card_number[i] != '\0'; i++) {
        if (tx->card_number[i] >= '0' && tx->card_number[i] <= '9') {
            card_key = card_key * 10 + (tx->card_number[i] - '0');
        }
    }
    
    // Attempt to load from evm_storage.dat.bin
    typedef struct {
        uint64_t key[4];
        uint64_t val[4];
        uint64_t addr;
    } EvmStorageRowAtm;
    
    static EvmStorageRowAtm rows[4096];
    uint32_t row_count = 0;
    
    const char *path = "evm_storage.dat.bin";
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        path = "tsfi2-deepseek/evm_storage.dat.bin";
        fp = fopen(path, "rb");
    }
    
    if (fp) {
        if (fread(&row_count, sizeof(row_count), 1, fp) == 1) {
            if (row_count > 4096) row_count = 4096;
            size_t read_bytes = fread(rows, sizeof(EvmStorageRowAtm), row_count, fp);
            (void)read_bytes;
        }
        fclose(fp);
    }
    
    // Find or create card entry in btc_rails_vm storage (address 0x9419441970718915ULL)
    uint64_t btc_rails_addr = 0x9419441970718915ULL;
    int found_index = -1;
    for (uint32_t i = 0; i < row_count; i++) {
        if (rows[i].addr == btc_rails_addr && rows[i].key[0] == card_key) {
            found_index = (int)i;
            break;
        }
    }
    
    double current_balance = 0.0;
    if (found_index != -1) {
        current_balance = (double)rows[found_index].val[0];
    } else {
        // Issue the card with a default balance of $5000 on btc rails ledger
        if (row_count < 4096) {
            found_index = (int)row_count;
            rows[found_index].addr = btc_rails_addr;
            rows[found_index].key[0] = card_key;
            rows[found_index].key[1] = 0;
            rows[found_index].key[2] = 0;
            rows[found_index].key[3] = 0;
            rows[found_index].val[0] = 5000; // $5000 USD
            rows[found_index].val[1] = 0;
            rows[found_index].val[2] = 0;
            rows[found_index].val[3] = 0;
            row_count++;
            current_balance = 5000.0;
        }
    }
    
    // Validate withdrawal
    if (tx->amount_dollars < 0.0 || tx->amount_dollars > current_balance) {
        return 0;
    }
    
    // Deduct funds
    if (found_index != -1) {
        rows[found_index].val[0] = (uint64_t)(current_balance - tx->amount_dollars);
        
        // Write back to database
        fp = fopen(path, "wb");
        if (fp) {
            fwrite(&row_count, sizeof(row_count), 1, fp);
            size_t written_bytes = fwrite(rows, sizeof(EvmStorageRowAtm), row_count, fp);
            (void)written_bytes;
            fclose(fp);
        }
    }
    
    *is_valid_out = 1;
    return 0;
}

int tsfi_cw_chase_issue_card(const char *card_number, const char *pin, double starting_balance, int *success_out) {
    if (!card_number || !pin || !success_out) return -1;
    
    *success_out = 0;
    if (strlen(pin) != 4) return 0;
    if (strlen(card_number) < 12) return 0;
    
    // Hash/convert card number to key
    uint64_t card_key = 0;
    for (int i = 0; card_number[i] != '\0'; i++) {
        if (card_number[i] >= '0' && card_number[i] <= '9') {
            card_key = card_key * 10 + (card_number[i] - '0');
        }
    }
    
    // Attempt to load from evm_storage.dat.bin
    typedef struct {
        uint64_t key[4];
        uint64_t val[4];
        uint64_t addr;
    } EvmStorageRowAtm;
    
    static EvmStorageRowAtm rows[4096];
    uint32_t row_count = 0;
    
    const char *path = "evm_storage.dat.bin";
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        path = "tsfi2-deepseek/evm_storage.dat.bin";
        fp = fopen(path, "rb");
    }
    
    if (fp) {
        if (fread(&row_count, sizeof(row_count), 1, fp) == 1) {
            if (row_count > 4096) row_count = 4096;
            size_t read_bytes = fread(rows, sizeof(EvmStorageRowAtm), row_count, fp);
            (void)read_bytes;
        }
        fclose(fp);
    }
    
    // Find or create card entry in btc_rails_vm storage (address 0x9419441970718915ULL)
    uint64_t btc_rails_addr = 0x9419441970718915ULL;
    int found_index = -1;
    for (uint32_t i = 0; i < row_count; i++) {
        if (rows[i].addr == btc_rails_addr && rows[i].key[0] == card_key) {
            found_index = (int)i;
            break;
        }
    }
    
    if (found_index != -1) {
        // Update balance
        rows[found_index].val[0] = (uint64_t)starting_balance;
    } else {
        // Create card row
        if (row_count < 4096) {
            found_index = (int)row_count;
            rows[found_index].addr = btc_rails_addr;
            rows[found_index].key[0] = card_key;
            rows[found_index].key[1] = 0;
            rows[found_index].key[2] = 0;
            rows[found_index].key[3] = 0;
            rows[found_index].val[0] = (uint64_t)starting_balance;
            rows[found_index].val[1] = 0;
            rows[found_index].val[2] = 0;
            rows[found_index].val[3] = 0;
            row_count++;
        }
    }
    
    // Write back to database
    fp = fopen(path, "wb");
    if (fp) {
        fwrite(&row_count, sizeof(row_count), 1, fp);
        size_t written_bytes = fwrite(rows, sizeof(EvmStorageRowAtm), row_count, fp);
        (void)written_bytes;
        fclose(fp);
        *success_out = 1;
    }
    
    return 0;
}

int tsfi_cw_omp_feilong_provision(const char *guest_name, int cpus, int memory_mb, tsfi_cw_omp_feilong_guest *guest_out) {
    if (!guest_name || cpus <= 0 || memory_mb <= 0 || !guest_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(guest_out->guest_name, guest_name, sizeof(guest_out->guest_name) - 1);
    guest_out->guest_name[sizeof(guest_out->guest_name) - 1] = 0;
    #pragma GCC diagnostic pop
    
    guest_out->cpu_count = cpus;
    guest_out->memory_mb = memory_mb;
    strcpy(guest_out->lifecycle_state, "PROVISIONING");
    return 0;
}

int tsfi_cw_omp_feilong_set_state(tsfi_cw_omp_feilong_guest *guest, const char *state) {
    if (!guest || !state) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(guest->lifecycle_state, state, sizeof(guest->lifecycle_state) - 1);
    guest->lifecycle_state[sizeof(guest->lifecycle_state) - 1] = 0;
    #pragma GCC diagnostic pop
    return 0;
}

int tsfi_cw_omp_galasa_init_run(const char *suite_name, tsfi_cw_omp_galasa_run *run_out) {
    if (!suite_name || !run_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(run_out->test_suite_name, suite_name, sizeof(run_out->test_suite_name) - 1);
    run_out->test_suite_name[sizeof(run_out->test_suite_name) - 1] = 0;
    #pragma GCC diagnostic pop
    
    run_out->assertions_run = 0;
    run_out->assertions_failed = 0;
    run_out->passes = 0;
    run_out->total_latency_ns = 0;
    run_out->fail_fast = 0;
    run_out->aborted = 0;
    return 0;
}

int tsfi_cw_omp_galasa_assert(tsfi_cw_omp_galasa_run *run, int condition) {
    if (!run) return -1;
    if (run->aborted) return 0;
    
    run->assertions_run++;
    if (condition) {
        run->passes++;
    } else {
        run->assertions_failed++;
        if (run->fail_fast) {
            run->aborted = 1;
        }
    }
    return 0;
}

int tsfi_cw_omp_feilong_dispatch(const char *cmd_line, tsfi_cw_omp_feilong_guest *guest_io_out, char *err_msg_out, size_t err_max) {
    if (!cmd_line || !guest_io_out || !err_msg_out || err_max == 0) return -1;
    
    err_msg_out[0] = 0;
    
    if (strncmp(cmd_line, "PROVISION ", 10) == 0) {
        char name[32] = {0};
        int cpus = 0;
        int mem = 0;
        if (sscanf(cmd_line + 10, "%31s %d %d", name, &cpus, &mem) != 3) {
            snprintf(err_msg_out, err_max, "PARSE_ERROR: Invalid provision arguments");
            return 1;
        }
        return tsfi_cw_omp_feilong_provision(name, cpus, mem, guest_io_out);
    } else if (strncmp(cmd_line, "SETSTATE ", 9) == 0) {
        char name[32] = {0};
        char state[32] = {0};
        if (sscanf(cmd_line + 9, "%31s %31s", name, state) != 2) {
            snprintf(err_msg_out, err_max, "PARSE_ERROR: Invalid setstate arguments");
            return 1;
        }
        if (strcmp(guest_io_out->guest_name, name) != 0) {
            snprintf(err_msg_out, err_max, "TARGET_MISMATCH: Guest name does not match memory instance");
            return 1;
        }
        return tsfi_cw_omp_feilong_set_state(guest_io_out, state);
    } else if (strncmp(cmd_line, "SCALE ", 6) == 0) {
        char name[32] = {0};
        int cpus = 0;
        int mem = 0;
        if (sscanf(cmd_line + 6, "%31s %d %d", name, &cpus, &mem) != 3) {
            snprintf(err_msg_out, err_max, "PARSE_ERROR: Invalid scale arguments");
            return 1;
        }
        if (strcmp(guest_io_out->guest_name, name) != 0) {
            snprintf(err_msg_out, err_max, "TARGET_MISMATCH: Guest name does not match memory instance");
            return 1;
        }
        guest_io_out->cpu_count = cpus;
        guest_io_out->memory_mb = mem;
        if (mem > 16384) {
            snprintf(err_msg_out, err_max, "WARNING: Memory allocation exceeds 16GB ceiling threshold");
        }
        return 0;
    } else if (strncmp(cmd_line, "DEPROVISION ", 12) == 0) {
        char name[32] = {0};
        if (sscanf(cmd_line + 12, "%31s", name) != 1) {
            snprintf(err_msg_out, err_max, "PARSE_ERROR: Invalid deprovision arguments");
            return 1;
        }
        if (strcmp(guest_io_out->guest_name, name) != 0) {
            snprintf(err_msg_out, err_max, "TARGET_MISMATCH: Guest name does not match memory instance");
            return 1;
        }
        guest_io_out->guest_name[0] = 0;
        guest_io_out->cpu_count = 0;
        guest_io_out->memory_mb = 0;
        strcpy(guest_io_out->lifecycle_state, "DEPROVISIONED");
        return 0;
    }
    
    snprintf(err_msg_out, err_max, "UNKNOWN_COMMAND: %s", cmd_line);
    return 1;
}

int tsfi_cw_omp_galasa_run_diagnostics(const tsfi_cw_omp_galasa_run *run, char *report_out, size_t report_max) {
    if (!run || !report_out || report_max == 0) return -1;
    
    snprintf(report_out, report_max,
             "OMP GALASA DIAGNOSTIC REPORT\n"
             "============================\n"
             "SUITE NAME : %s\n"
             "ASSERTIONS : %d\n"
             "PASSES     : %d\n"
             "FAILURES   : %d\n"
             "STATUS     : %s\n",
             run->test_suite_name,
             run->assertions_run,
             run->passes,
             run->assertions_failed,
             (run->assertions_failed > 0) ? "RED_ALERT" : "STABLE_GREEN");
    return 0;
}

int tsfi_cw_omp_galasa_assert_with_retry(tsfi_cw_omp_galasa_run *run, int (*eval_fn)(void *ctx), void *ctx, int max_retries) {
    if (!run || !eval_fn) return -1;
    
    int passes = 0;
    for (int r = 0; r < max_retries; r++) {
        if (eval_fn(ctx)) {
            passes = 1;
            break;
        }
    }
    
    return tsfi_cw_omp_galasa_assert(run, passes);
}

int tsfi_cw_omp_feilong_get_summary(const tsfi_cw_omp_feilong_guest *guests, int guest_count, tsfi_cw_omp_feilong_summary *summary_out) {
    if (!guests || guest_count < 0 || !summary_out) return -1;
    
    summary_out->active_guests = 0;
    summary_out->total_cpus = 0;
    summary_out->total_mem_mb = 0;
    summary_out->oversubscribed_alert = 0;
    
    for (int i = 0; i < guest_count; i++) {
        if (strcmp(guests[i].lifecycle_state, "ACTIVE") == 0) {
            summary_out->active_guests++;
        }
        summary_out->total_cpus += guests[i].cpu_count;
        summary_out->total_mem_mb += guests[i].memory_mb;
    }
    
    if (summary_out->total_mem_mb > 65536) {
        summary_out->oversubscribed_alert = 1;
    }
    return 0;
}

int tsfi_cw_omp_galasa_assert_timed(tsfi_cw_omp_galasa_run *run, int condition, uint64_t latency_ns) {
    if (!run) return -1;
    
    run->total_latency_ns += latency_ns;
    return tsfi_cw_omp_galasa_assert(run, condition);
}

int tsfi_cw_omp_galasa_write_html_report(const tsfi_cw_omp_galasa_run *run, const char *filepath) {
    if (!run || !filepath) return -1;
    
    FILE *fp = fopen(filepath, "w");
    if (!fp) return 1;
    
    fprintf(fp, "<!DOCTYPE html>\n<html>\n<head>\n<title>OMP Galasa Test Run Report</title>\n");
    fprintf(fp, "<style>\nbody { font-family: sans-serif; background: #0c0f12; color: #f0f3f6; padding: 2rem; }\n");
    fprintf(fp, ".badge { display: inline-block; padding: 0.5rem 1rem; border-radius: 4px; font-weight: bold; }\n");
    fprintf(fp, ".green { background: #38ef7d; color: #000; }\n");
    fprintf(fp, ".red { background: #ff4d4d; color: #fff; }\n");
    fprintf(fp, "</style>\n</head>\n<body>\n");
    fprintf(fp, "<h1>Galasa Test Suite: %s</h1>\n", run->test_suite_name);
    fprintf(fp, "<p>Assertions Run: <strong>%d</strong></p>\n", run->assertions_run);
    fprintf(fp, "<p>Passes: <strong>%d</strong></p>\n", run->passes);
    fprintf(fp, "<p>Failures: <strong>%d</strong></p>\n", run->assertions_failed);
    fprintf(fp, "<p>Cumulative Latency: <strong>%lu ns</strong></p>\n", (unsigned long)run->total_latency_ns);
    
    if (run->assertions_failed > 0) {
        fprintf(fp, "<div class=\"badge red\">STATUS: FAIL (RED_ALERT)</div>\n");
    } else {
        fprintf(fp, "<div class=\"badge green\">STATUS: PASS (STABLE_GREEN)</div>\n");
    }
    
    fprintf(fp, "</body>\n</html>\n");
    fclose(fp);
    return 0;
}

int tsfi_cw_omp_sdt_parse_catalog(const char *raw_data, tsfi_cw_omp_sdt_package *pkgs_out, int max_pkgs, int *count_out) {
    if (!raw_data || !pkgs_out || max_pkgs <= 0 || !count_out) return -1;
    
    *count_out = 0;
    const char *line = raw_data;
    while (line && *line != 0 && *count_out < max_pkgs) {
        if (strncmp(line, "PKG ", 4) == 0) {
            char name[32] = {0};
            char ver[16] = {0};
            char status[16] = {0};
            if (sscanf(line + 4, "%31s %15s %15s", name, ver, status) >= 2) {
                #pragma GCC diagnostic push
                #pragma GCC diagnostic ignored "-Wstringop-truncation"
                strncpy(pkgs_out[*count_out].package_name, name, sizeof(pkgs_out[*count_out].package_name) - 1);
                pkgs_out[*count_out].package_name[sizeof(pkgs_out[*count_out].package_name) - 1] = 0;
                strncpy(pkgs_out[*count_out].version, ver, sizeof(pkgs_out[*count_out].version) - 1);
                pkgs_out[*count_out].version[sizeof(pkgs_out[*count_out].version) - 1] = 0;
                #pragma GCC diagnostic pop
                
                pkgs_out[*count_out].is_obsolete = (strcmp(status, "OBSOLETE") == 0);
                (*count_out)++;
            }
        }
        
        line = strchr(line, '\n');
        if (line) line++;
    }
    return 0;
}

int tsfi_cw_omp_ambitus_translate(const char *unix_cmd, char *mvs_cmd_out, size_t mvs_max) {
    if (!unix_cmd || !mvs_cmd_out || mvs_max == 0) return -1;
    
    mvs_cmd_out[0] = 0;
    
    if (strcmp(unix_cmd, "ls") == 0) {
        snprintf(mvs_cmd_out, mvs_max, "D A,L");
        return 0;
    } else if (strcmp(unix_cmd, "cat") == 0) {
        snprintf(mvs_cmd_out, mvs_max, "D U");
        return 0;
    } else if (strcmp(unix_cmd, "grep") == 0) {
        snprintf(mvs_cmd_out, mvs_max, "D TS");
        return 0;
    }
    
    snprintf(mvs_cmd_out, mvs_max, "UNKNOWN: %s", unix_cmd);
    return 1;
}

int tsfi_cw_omp_cobol_verify_syntax(const char *cobol_line, char *err_out, size_t err_max) {
    if (!cobol_line || !err_out || err_max == 0) return -1;
    
    err_out[0] = 0;
    
    const char *pic_ptr = strstr(cobol_line, "PIC ");
    if (pic_ptr) {
        const char *pic_val = pic_ptr + 4;
        const char *dot_ptr = strchr(pic_val, '.');
        if (!dot_ptr) {
            snprintf(err_out, err_max, "SYNTAX_ERROR: Missing ending period on PICTURE definition");
            return 1;
        }
        
        if (strchr(pic_val, 'X') == NULL && strchr(pic_val, '9') == NULL) {
            snprintf(err_out, err_max, "SYNTAX_ERROR: Invalid PICTURE clause type indicator");
            return 1;
        }
    }
    return 0;
}

int tsfi_cw_omp_hub_check_install(const char *target_pkg, const tsfi_cw_omp_hub_pkg *repo, int repo_count, int *can_install_out) {
    if (!target_pkg || !repo || repo_count < 0 || !can_install_out) return -1;
    
    *can_install_out = 0;
    int found_index = -1;
    for (int i = 0; i < repo_count; i++) {
        if (strcmp(repo[i].name, target_pkg) == 0) {
            found_index = i;
            break;
        }
    }
    
    if (found_index == -1) {
        return 1;
    }
    
    if (repo[found_index].depends_on[0] != 0) {
        int dep_found = 0;
        for (int j = 0; j < repo_count; j++) {
            if (strcmp(repo[j].name, repo[found_index].depends_on) == 0) {
                dep_found = 1;
                if (repo[j].is_installed) {
                    *can_install_out = 1;
                }
                break;
            }
        }
        if (!dep_found) {
            return 1;
        }
    } else {
        *can_install_out = 1;
    }
    
    return 0;
}
