#include "tsfi_mainframe_decnet.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


int tsfi_vtam_serialize_piu(const tsfi_vtam_piu_header *piu, const uint8_t *payload, size_t pay_len, uint8_t *buf_out, size_t *len_out) {
    if (!piu || !buf_out || !len_out) return -1;
    if (pay_len > 256) return -2;
    
    buf_out[0] = piu->th_fid;
    buf_out[1] = piu->th_daf;
    buf_out[2] = piu->th_saf;
    buf_out[3] = (piu->th_snf >> 8) & 0xFF;
    buf_out[4] = piu->th_snf & 0xFF;
    buf_out[5] = piu->rh_type;
    
    if (payload && pay_len > 0) {
        memcpy(buf_out + 6, payload, pay_len);
    }
    *len_out = 6 + pay_len;
    return 0;
}

int tsfi_vtam_deserialize_piu(const uint8_t *buf, size_t len, tsfi_vtam_piu_header *piu_out, uint8_t *pay_out, size_t *pay_len_out) {
    if (!buf || !piu_out || len < 6) return -1;
    
    piu_out->th_fid = buf[0];
    piu_out->th_daf = buf[1];
    piu_out->th_saf = buf[2];
    piu_out->th_snf = (buf[3] << 8) | buf[4];
    piu_out->rh_type = buf[5];
    
    if (pay_out && pay_len_out) {
        *pay_len_out = len - 6;
        if (*pay_len_out > 0) {
            memcpy(pay_out, buf + 6, *pay_len_out);
        }
    }
    return 0;
}

void tsfi_sna_route_init(tsfi_sna_route *route, uint8_t vr, uint8_t er, int prio) {
    if (!route) return;
    route->virtual_route_id = vr;
    route->explicit_route_id = er;
    route->transmission_priority = prio;
}

int tsfi_sna_route_prioritize(const tsfi_sna_route *route, int load_factor) {
    if (!route) return -1;
    return route->transmission_priority * 10 - load_factor;
}

void tsfi_sna_session_init(tsfi_sna_session *sess) {
    if (!sess) return;
    sess->session_active = 0;
    sess->traffic_started = 0;
}

int tsfi_sna_session_handshake(tsfi_sna_session *sess, uint8_t cmd) {
    if (!sess) return -1;
    if (cmd == SNA_CMD_BIND) {
        sess->session_active = 1;
        sess->traffic_started = 0;
        return 0;
    } else if (cmd == SNA_CMD_UNBIND) {
        sess->session_active = 0;
        sess->traffic_started = 0;
        return 0;
    } else if (cmd == SNA_CMD_SDT) {
        if (!sess->session_active) return -2;
        sess->traffic_started = 1;
        return 0;
    }
    return -3;
}

void tsfi_sna_crypto_init(tsfi_sna_crypto *crypto, const uint8_t *key) {
    if (!crypto) return;
    if (key) {
        memcpy(crypto->session_key, key, 8);
        crypto->encryption_enabled = 1;
    } else {
        memset(crypto->session_key, 0, 8);
        crypto->encryption_enabled = 0;
    }
}

int tsfi_sna_encrypt(tsfi_sna_crypto *crypto, const uint8_t *plain, size_t len, uint8_t *cipher) {
    if (!crypto || !plain || !cipher) return -1;
    if (!crypto->encryption_enabled) {
        memcpy(cipher, plain, len);
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        cipher[i] = plain[i] ^ crypto->session_key[i % 8];
    }
    return 0;
}

int tsfi_sna_decrypt(tsfi_sna_crypto *crypto, const uint8_t *cipher, size_t len, uint8_t *plain) {
    if (!crypto || !cipher || !plain) return -1;
    if (!crypto->encryption_enabled) {
        memcpy(plain, cipher, len);
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        plain[i] = cipher[i] ^ crypto->session_key[i % 8];
    }
    return 0;
}

void tsfi_vtam_nto_init(tsfi_vtam_nto *nto) {
    if (!nto) return;
    nto->active = 1;
}

int tsfi_vtam_nto_translate(tsfi_vtam_nto *nto, const char *tty_in, size_t len, uint8_t *lu_out, size_t *out_len) {
    if (!nto || !tty_in || !lu_out || !out_len) return -1;
    if (!nto->active) return -2;
    if (len > 250) return -3;
    
    lu_out[0] = 0x11;
    lu_out[1] = len & 0xFF;
    memcpy(lu_out + 2, tty_in, len);
    *out_len = 2 + len;
    return 0;
}

void tsfi_sna_pacing_init(tsfi_sna_pacing *pacing, int initial_window) {
    if (!pacing) return;
    pacing->window_size = initial_window;
    pacing->max_window = initial_window * 2;
    pacing->congestion_detected = 0;
}

int tsfi_sna_pacing_adjust(tsfi_sna_pacing *pacing, int congestion_flag) {
    if (!pacing) return -1;
    pacing->congestion_detected = congestion_flag;
    if (congestion_flag) {
        pacing->window_size /= 2;
        if (pacing->window_size < 1) pacing->window_size = 1;
    } else {
        pacing->window_size++;
        if (pacing->window_size > pacing->max_window) {
            pacing->window_size = pacing->max_window;
        }
    }
    return pacing->window_size;
}

void tsfi_vtam_lu_registry_init(tsfi_vtam_lu_registry *reg) {
    if (!reg) return;
    reg->count = 0;
    for (int i = 0; i < 16; i++) {
        reg->lus[i].active = 0;
        reg->lus[i].lu_address = 0;
        reg->lus[i].lu_type = 0;
        reg->lus[i].resource_name[0] = '\0';
    }
}

int tsfi_vtam_lu_registry_add(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t type, const char *name) {
    if (!reg || !name) return -1;
    if (reg->count >= 16) return -2;
    for (int i = 0; i < reg->count; i++) {
        if (reg->lus[i].lu_address == addr) return -3;
    }
    tsfi_vtam_lu *lu = &reg->lus[reg->count];
    lu->lu_address = addr;
    lu->lu_type = type;
    lu->active = 1;
    strncpy(lu->resource_name, name, 31);
    lu->resource_name[31] = '\0';
    reg->count++;
    return 0;
}

int tsfi_vtam_lu_registry_route(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t *data, size_t len) {
    if (!reg || !data) return -1;
    for (int i = 0; i < reg->count; i++) {
        if (reg->lus[i].lu_address == addr && reg->lus[i].active) {
            if (len > 0) {
                data[0] = reg->lus[i].lu_type;
            }
            return i;
        }
    }
    return -2;
}

int tsfi_vtam_lu_bridge_winchester(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t *scsi_status, uint8_t *data_reg, uint8_t *keycode_reg) {
    if (!reg || !scsi_status || !data_reg || !keycode_reg) return -1;
    for (int i = 0; i < reg->count; i++) {
        if (reg->lus[i].lu_address == addr && reg->lus[i].active) {
            *scsi_status = 0x03;
            *data_reg = reg->lus[i].lu_type;
            *keycode_reg = 32;
            return i;
        }
    }
    return -2;
}

int tsfi_vtam_lu_bridge_coaxial(tsfi_vtam_lu_registry *reg, uint16_t addr, int *coax_phase, int *coax_signal) {
    if (!reg || !coax_phase || !coax_signal) return -1;
    for (int i = 0; i < reg->count; i++) {
        if (reg->lus[i].lu_address == addr && reg->lus[i].active) {
            *coax_phase = (int)(reg->lus[i].lu_address % 360);
            *coax_signal = (int)reg->lus[i].lu_type;
            return i;
        }
    }
    return -2;
}

void tsfi_sna_er_init(tsfi_sna_er_route *er, uint8_t route_num) {
    if (!er) return;
    er->route_number = route_num;
    er->active = 0;
}

int tsfi_sna_er_activate(tsfi_sna_er_route *er) {
    if (!er) return -1;
    er->active = 1;
    return 0;
}

int tsfi_sna_er_deactivate(tsfi_sna_er_route *er) {
    if (!er) return -1;
    er->active = 0;
    return 0;
}

void tsfi_sna_stage_pacing_init(tsfi_sna_stage_pacing *pacing, int window) {
    if (!pacing) return;
    pacing->stage_window = window;
    pacing->credits_left = window;
}

int tsfi_sna_stage_pacing_consume(tsfi_sna_stage_pacing *pacing) {
    if (!pacing) return -1;
    if (pacing->credits_left <= 0) return -2;
    pacing->credits_left--;
    return 0;
}

void tsfi_sna_stage_pacing_response(tsfi_sna_stage_pacing *pacing) {
    if (!pacing) return;
    pacing->credits_left = pacing->stage_window;
}

int tsfi_sna_serialize_fmh(const tsfi_sna_fmh *fmh, uint8_t *buf, size_t *len_out) {
    if (!fmh || !buf || !len_out) return -1;
    buf[0] = fmh->fmh_type;
    buf[1] = fmh->fmh_len;
    buf[2] = (fmh->destination_id >> 8) & 0xFF;
    buf[3] = fmh->destination_id & 0xFF;
    *len_out = 4;
    return 0;
}

int tsfi_sna_deserialize_fmh(const uint8_t *buf, size_t len, tsfi_sna_fmh *fmh_out) {
    if (!buf || !fmh_out || len < 4) return -1;
    fmh_out->fmh_type = buf[0];
    fmh_out->fmh_len = buf[1];
    fmh_out->destination_id = (buf[2] << 8) | buf[3];
    return 0;
}

void tsfi_sna_chain_init(tsfi_sna_chain_assembler *assembler) {
    if (!assembler) return;
    assembler->len = 0;
    assembler->assembly_complete = 0;
}

int tsfi_sna_chain_add(tsfi_sna_chain_assembler *assembler, uint8_t chain_indicator, const uint8_t *data, size_t len) {
    if (!assembler || !data) return -1;
    if (assembler->len + len > 1024) return -2;
    memcpy(assembler->buffer + assembler->len, data, len);
    assembler->len += len;
    if (chain_indicator == SNA_CHAIN_LIC || chain_indicator == SNA_CHAIN_OIC) {
        assembler->assembly_complete = 1;
        return 1;
    }
    assembler->assembly_complete = 0;
    return 0;
}

void tsfi_sna_bind_profile_init(tsfi_sna_bind_profile *profile) {
    if (!profile) return;
    profile->profile_id = 1;
    profile->pacing_in = 2;
    profile->pacing_out = 2;
    profile->duplex_mode = 0;
}

int tsfi_sna_bind_profile_negotiate(tsfi_sna_bind_profile *local, const tsfi_sna_bind_profile *requested) {
    if (!local || !requested) return -1;
    local->profile_id = requested->profile_id;
    if (requested->pacing_in < local->pacing_in) {
        local->pacing_in = requested->pacing_in;
    }
    if (requested->pacing_out < local->pacing_out) {
        local->pacing_out = requested->pacing_out;
    }
    local->duplex_mode = requested->duplex_mode;
    return 0;
}

void tsfi_sna_tg_failover_init(tsfi_sna_tg_failover *failover) {
    if (!failover) return;
    failover->active_links = 3;
    failover->backup_route_active = 0;
}

int tsfi_sna_tg_link_fail(tsfi_sna_tg_failover *failover, int link_id) {
    if (!failover) return -1;
    (void)link_id;
    if (failover->active_links > 0) {
        failover->active_links--;
    }
    if (failover->active_links == 0) {
        failover->backup_route_active = 1;
        return 1;
    }
    return 0;
}

void tsfi_sna_bracket_init(tsfi_sna_bracket *b) {
    if (!b) return;
    b->bracket_active = 0;
}

int tsfi_sna_bracket_process(tsfi_sna_bracket *b, int begin_bracket, int end_bracket) {
    if (!b) return -1;
    if (begin_bracket) {
        b->bracket_active = 1;
    }
    if (end_bracket) {
        b->bracket_active = 0;
    }
    return 0;
}

void tsfi_sna_hdx_init(tsfi_sna_hdx *hdx, int initial_turn) {
    if (!hdx) return;
    hdx->my_turn = initial_turn;
}

int tsfi_sna_hdx_process(tsfi_sna_hdx *hdx, int receive_cd, int send_cd) {
    if (!hdx) return -1;
    if (receive_cd) {
        hdx->my_turn = 1;
    }
    if (send_cd) {
        hdx->my_turn = 0;
    }
    return 0;
}

void tsfi_sna_response_init(tsfi_sna_response_tracker *tracker) {
    if (!tracker) return;
    tracker->dr1_requested = 0;
    tracker->dr2_requested = 0;
    tracker->exception_response_only = 0;
}

int tsfi_sna_response_parse(tsfi_sna_response_tracker *tracker, uint8_t rh_byte) {
    if (!tracker) return -1;
    tracker->dr1_requested = (rh_byte & 0x01) ? 1 : 0;
    tracker->dr2_requested = (rh_byte & 0x02) ? 1 : 0;
    tracker->exception_response_only = (rh_byte & 0x04) ? 1 : 0;
    return 0;
}

uint8_t tsfi_sna_piu_bcc(const uint8_t *data, size_t len) {
    if (!data) return 0;
    uint8_t bcc = 0;
    for (size_t i = 0; i < len; i++) {
        bcc ^= data[i];
    }
    return bcc;
}

int tsfi_sna_serialize_session_cmd(const tsfi_sna_session_cmd *cmd, uint8_t *buf, size_t *len_out) {
    if (!cmd || !buf || !len_out) return -1;
    buf[0] = cmd->command_code;
    buf[1] = (cmd->local_addr >> 8) & 0xFF;
    buf[2] = cmd->local_addr & 0xFF;
    buf[3] = (cmd->remote_addr >> 8) & 0xFF;
    buf[4] = cmd->remote_addr & 0xFF;
    *len_out = 5;
    return 0;
}

int tsfi_sna_deserialize_session_cmd(const uint8_t *buf, size_t len, tsfi_sna_session_cmd *cmd_out) {
    if (!buf || !cmd_out || len < 5) return -1;
    cmd_out->command_code = buf[0];
    cmd_out->local_addr = (buf[1] << 8) | buf[2];
    cmd_out->remote_addr = (buf[3] << 8) | buf[4];
    return 0;
}

void tsfi_sna_key_rotation_init(tsfi_sna_key_rotation *rot, const uint8_t *dist_key) {
    if (!rot || !dist_key) return;
    memcpy(rot->distribution_key, dist_key, 8);
    memset(rot->current_session_key, 0, 8);
}

int tsfi_sna_rotate_key(tsfi_sna_key_rotation *rot, const uint8_t *encrypted_new_key) {
    if (!rot || !encrypted_new_key) return -1;
    for (int i = 0; i < 8; i++) {
        rot->current_session_key[i] = encrypted_new_key[i] ^ rot->distribution_key[i];
    }
    return 0;
}

int tsfi_sna_serialize_th(const tsfi_sna_th *th, uint8_t *buf, size_t *len_out) {
    if (!th || !buf || !len_out) return -1;
    if (th->fid_type == SNA_FID_TYPE0 || th->fid_type == SNA_FID_TYPE1) {
        buf[0] = (th->fid_type << 4) | (th->mpf & 0x0F);
        buf[1] = 0;
        buf[2] = (th->daf >> 8) & 0xFF;
        buf[3] = th->daf & 0xFF;
        buf[4] = (th->oaf >> 8) & 0xFF;
        buf[5] = th->oaf & 0xFF;
        buf[6] = (th->sn >> 8) & 0xFF;
        buf[7] = th->sn & 0xFF;
        buf[8] = 0;
        buf[9] = 0;
        *len_out = 10;
    } else if (th->fid_type == SNA_FID_TYPE2) {
        buf[0] = (SNA_FID_TYPE2 << 4) | (th->mpf & 0x0F);
        buf[1] = 0;
        buf[2] = th->daf & 0xFF;
        buf[3] = th->oaf & 0xFF;
        buf[4] = (th->sn >> 8) & 0xFF;
        buf[5] = th->sn & 0xFF;
        *len_out = 6;
    } else if (th->fid_type == SNA_FID_TYPE3 || th->fid_type == SNA_FID_TYPEF) {
        buf[0] = (th->fid_type << 4) | (th->mpf & 0x0F);
        buf[1] = th->daf & 0xFF;
        *len_out = 2;
    } else {
        buf[0] = (SNA_FID_TYPE4 << 4) | (th->mpf & 0x0F);
        buf[1] = 0;
        buf[2] = (th->daf >> 8) & 0xFF;
        buf[3] = th->daf & 0xFF;
        buf[4] = (th->oaf >> 8) & 0xFF;
        buf[5] = th->oaf & 0xFF;
        buf[6] = (th->sn >> 8) & 0xFF;
        buf[7] = th->sn & 0xFF;
        *len_out = 8;
    }
    return 0;
}

int tsfi_sna_deserialize_th(const uint8_t *buf, size_t len, tsfi_sna_th *th_out) {
    if (!buf || !th_out || len < 2) return -1;
    th_out->fid_type = (buf[0] >> 4) & 0x0F;
    th_out->mpf = buf[0] & 0x0F;
    if (th_out->fid_type == SNA_FID_TYPE0 || th_out->fid_type == SNA_FID_TYPE1) {
        if (len < 10) return -2;
        th_out->daf = (buf[2] << 8) | buf[3];
        th_out->oaf = (buf[4] << 8) | buf[5];
        th_out->sn = (buf[6] << 8) | buf[7];
    } else if (th_out->fid_type == SNA_FID_TYPE2) {
        if (len < 6) return -3;
        th_out->daf = buf[2];
        th_out->oaf = buf[3];
        th_out->sn = (buf[4] << 8) | buf[5];
    } else if (th_out->fid_type == SNA_FID_TYPE3 || th_out->fid_type == SNA_FID_TYPEF) {
        th_out->daf = buf[1];
        th_out->oaf = 0;
        th_out->sn = 0;
    } else {
        if (len < 8) return -4;
        th_out->daf = (buf[2] << 8) | buf[3];
        th_out->oaf = (buf[4] << 8) | buf[5];
        th_out->sn = (buf[6] << 8) | buf[7];
    }
    return 0;
}

int tsfi_sna_serialize_rh(const tsfi_sna_rh *rh, uint8_t *buf, size_t *len_out) {
    if (!rh || !buf || !len_out) return -1;
    buf[0] = ((rh->ru_category & 0x03) << 6) |
             ((rh->is_response & 0x01) << 5) |
             ((rh->format_indicator & 0x01) << 4) |
             ((rh->sense_data_included & 0x01) << 3);
    buf[1] = ((rh->begin_chain & 0x01) << 7) |
             ((rh->end_chain & 0x01) << 6) |
             ((rh->dr1_indicator & 0x01) << 5) |
             ((rh->dr2_indicator & 0x01) << 4) |
             ((rh->exception_response & 0x01) << 3);
    buf[2] = ((rh->change_direction & 0x01) << 7) |
             ((rh->begin_bracket & 0x01) << 6) |
             ((rh->end_bracket & 0x01) << 5);
    *len_out = 3;
    return 0;
}

int tsfi_sna_deserialize_rh(const uint8_t *buf, size_t len, tsfi_sna_rh *rh_out) {
    if (!buf || !rh_out || len < 3) return -1;
    rh_out->ru_category = (buf[0] >> 6) & 0x03;
    rh_out->is_response = (buf[0] >> 5) & 0x01;
    rh_out->format_indicator = (buf[0] >> 4) & 0x01;
    rh_out->sense_data_included = (buf[0] >> 3) & 0x01;
    rh_out->begin_chain = (buf[1] >> 7) & 0x01;
    rh_out->end_chain = (buf[1] >> 6) & 0x01;
    rh_out->dr1_indicator = (buf[1] >> 5) & 0x01;
    rh_out->dr2_indicator = (buf[1] >> 4) & 0x01;
    rh_out->exception_response = (buf[1] >> 3) & 0x01;
    rh_out->change_direction = (buf[2] >> 7) & 0x01;
    rh_out->begin_bracket = (buf[2] >> 6) & 0x01;
    rh_out->end_bracket = (buf[2] >> 5) & 0x01;
    return 0;
}

int tsfi_sna_map_lu_type(uint8_t unified_type) {
    switch (unified_type) {
        case LU_TYPE_FILE:     return SNA_LU_TYPE1;
        case LU_TYPE_SOCKET:   return SNA_LU_TYPE62;
        case LU_TYPE_TERMINAL: return SNA_LU_TYPE2;
        case LU_TYPE_DISK:     return SNA_LU_TYPE0;
        default:               return SNA_LU_TYPE0;
    }
}

const char *tsfi_sna_resolve_sense(uint16_t sense_code) {
    switch (sense_code) {
        case SNA_SENSE_RESOURCE_UNAVAILABLE: return "Resource Not Available";
        case SNA_SENSE_SESSION_LIMIT:        return "Session Limit Exceeded";
        case SNA_SENSE_END_USER_UNAVAILABLE: return "End-User Not Available";
        case SNA_SENSE_PATH_OUT_OF_ORDER:    return "Path Out of Order";
        default:                             return "Unknown/General Protocol Error";
    }
}

int tsfi_sna_package_piu(const tsfi_sna_th *th, const tsfi_sna_rh *rh, const uint8_t *ru_data, size_t ru_len, uint8_t *piu_out, size_t *piu_len) {
    if (!th || !rh || !piu_out || !piu_len) return -1;
    size_t th_len = 0;
    if (tsfi_sna_serialize_th(th, piu_out, &th_len) != 0) return -2;
    size_t rh_len = 0;
    if (tsfi_sna_serialize_rh(rh, piu_out + th_len, &rh_len) != 0) return -3;
    if (ru_data && ru_len > 0) {
        memcpy(piu_out + th_len + rh_len, ru_data, ru_len);
    } else {
        ru_len = 0;
    }
    size_t payload_len = th_len + rh_len + ru_len;
    uint8_t bcc = tsfi_sna_piu_bcc(piu_out, payload_len);
    piu_out[payload_len] = bcc;
    *piu_len = payload_len + 1;
    return 0;
}

int tsfi_sna_parse_piu(const uint8_t *piu, size_t piu_len, tsfi_sna_th *th_out, tsfi_sna_rh *rh_out, uint8_t *ru_out, size_t *ru_len) {
    if (!piu || piu_len < 10 || !th_out || !rh_out || !ru_out || !ru_len) return -1;
    uint8_t rx_bcc = piu[piu_len - 1];
    if (tsfi_sna_piu_bcc(piu, piu_len - 1) != rx_bcc) return -2;
    if (tsfi_sna_deserialize_th(piu, piu_len - 1, th_out) != 0) return -3;
    size_t th_len = (th_out->fid_type == SNA_FID_TYPE2) ? 6 : 8;
    if (tsfi_sna_deserialize_rh(piu + th_len, piu_len - th_len - 1, rh_out) != 0) return -4;
    size_t rh_len = 3;
    *ru_len = piu_len - th_len - rh_len - 1;
    if (*ru_len > 0) {
        memcpy(ru_out, piu + th_len + rh_len, *ru_len);
    }
    return 0;
}

void tsfi_sna_map_th_mpf_to_rh_chain(uint8_t mpf, tsfi_sna_rh *rh) {
    if (!rh) return;
    switch (mpf & 0x03) {
        case 0x02:
            rh->begin_chain = 1;
            rh->end_chain = 0;
            break;
        case 0x00:
            rh->begin_chain = 0;
            rh->end_chain = 0;
            break;
        case 0x01:
            rh->begin_chain = 0;
            rh->end_chain = 1;
            break;
        case 0x03:
        default:
            rh->begin_chain = 1;
            rh->end_chain = 1;
            break;
    }
}

void tsfi_zvm_gcs_set_vtam(tsfi_zvm_gcs *gcs, int active) {
    if (gcs) {
        gcs->vtam_active = active;
    }
}

int tsfi_appc_allocate(tsfi_appc_conversation *conv, int local_lu, int partner_lu) {
    if (!conv) return -1;
    conv->conversation_id = (local_lu << 16) | (partner_lu & 0xFFFF);
    conv->state = 0; // Initial allocated state
    conv->security_active = 0;
    conv->pacing_window = 4;
    conv->sync_state = 0;
    conv->crypto_session = NULL;
    conv->conversation_type = 0;
    conv->tp_name[0] = '\0';
    return 0;
}

int tsfi_appc_send_data(tsfi_appc_conversation *conv, const uint8_t *data, size_t len) {
    if (!conv || !data || len == 0) return -1;
    conv->state = 1; // Transition to SEND state
    return 0;
}

int tsfi_appc_receive_data(tsfi_appc_conversation *conv, uint8_t *buf, size_t *len_out) {
    if (!conv || !buf || !len_out) return -1;
    conv->state = 2; // Transition to RECEIVE state
    *len_out = 0;
    return 0;
}

int tsfi_appc_deallocate(tsfi_appc_conversation *conv) {
    if (!conv) return -1;
    conv->state = 3; // DEALLOCATED
    return 0;
}

int tsfi_appc_confirm(tsfi_appc_conversation *conv) {
    if (!conv) return -1;
    if (conv->state != 1) return -2; // Can only confirm from SEND state
    conv->state = 4; // Transition to CONFIRM state
    return 0;
}

int tsfi_appc_confirmed(tsfi_appc_conversation *conv) {
    if (!conv) return -1;
    if (conv->state != 4) return -2; // Can only respond to CONFIRM
    conv->state = 1; // Return back to SEND state after confirmation
    return 0;
}

int tsfi_appc_bridge_winchester(tsfi_appc_conversation *conv, uint8_t *scsi_status, uint8_t *data_reg, uint8_t *keycode_reg) {
    if (!conv || !scsi_status || !data_reg || !keycode_reg) return -1;
    if (conv->state == 3) return -2; // Deallocated
    
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
    
    uint8_t cd[36] = {0};
    cd[0] = 0x90; // APPC request signature
    cd[1] = (uint8_t)conv->state;
    cd[2] = (uint8_t)(conv->conversation_id >> 8);
    cd[3] = (uint8_t)(conv->conversation_id & 0xFF);
    
    uint8_t ret[32];
    size_t ret_len = 32;
    if (lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len)) {
        *scsi_status = ret[0];
        *data_reg = ret[1];
        *keycode_reg = ret[2];
        return 0;
    }
    return -3; // Handshake loop failure
}

int tsfi_appc_bridge_decnet(tsfi_appc_conversation *conv, tsfi_decnet_router *router, uint16_t dest_node) {
    if (!conv || !router) return -1;
    if (conv->state == 3) return -2; // Deallocated
    
    uint16_t next_hop;
    if (tsfi_decnet_route_packet(router, dest_node, &next_hop) == 0) {
        conv->state = 2; // Transition to RECEIVE mode waiting for hop response
        return 0;
    }
    return -3; // DECnet routing lookup failure
}

int tsfi_appc_bridge_terminal(tsfi_appc_conversation *conv, tsfi_ibm3270_terminal *term) {
    if (!conv || !term) return -1;
    if (conv->state == 3) return -2; // Deallocated
    
    char status_msg[64];
    snprintf(status_msg, sizeof(status_msg), "APPC CONV %08X STATE %d", conv->conversation_id, conv->state);
    
    for (size_t i = 0; i < strlen(status_msg); i++) {
        tsfi_ibm3270_write_char(term, status_msg[i], i);
    }
    term->buffer_updated = 1;
    return 0;
}

int tsfi_appc_security_validate(tsfi_appc_conversation *conv, const char *username, const char *password) {
    if (!conv || !username || !password) return -1;
    if (conv->state == 3) return -2; // Deallocated
    
    if (strcmp(username, password) != 0 && strlen(password) >= 4) {
        conv->security_active = 1;
        return 0;
    }
    conv->security_active = 0;
    return -3; // Security validation failed
}

int tsfi_appc_spawn_tp(tsfi_appc_conversation *conv, const char *tp_name) {
    if (!conv || !tp_name) return -1;
    if (conv->state == 3) return -2; // Deallocated
    if (conv->security_active == 0) return -4; // Security check required
    
    return 0;
}

int tsfi_appc_pacing_adjust(tsfi_appc_conversation *conv, int congestion_flag) {
    if (!conv) return -1;
    if (conv->state == 3) return -2; // Deallocated
    
    if (congestion_flag) {
        if (conv->pacing_window > 1) {
            conv->pacing_window--;
        }
    } else {
        if (conv->pacing_window < 16) {
            conv->pacing_window++;
        }
    }
    return 0;
}

int tsfi_appc_syncpoint_commit(tsfi_appc_conversation *conv, int phase) {
    if (!conv) return -1;
    if (conv->state == 3) return -2; // Deallocated
    
    if (phase == 1) {
        conv->sync_state = 1; // PREPARED
        return 0;
    } else if (phase == 2) {
        if (conv->sync_state == 1) {
            conv->sync_state = 2; // COMMITTED
            return 0;
        }
        return -3;
    }
    return -4;
}

int tsfi_appc_cnos_negotiate(int local_lu, int partner_lu, int session_limit) {
    if (local_lu == 0 || partner_lu == 0 || session_limit < 0) return -1;
    return 0;
}

int tsfi_appc_rollback(tsfi_appc_conversation *conv) {
    if (!conv) return -1;
    if (conv->state == 3) return -2; // Deallocated
    conv->sync_state = 0; // Roll back to NONE
    conv->state = 2; // Receive mode
    return 0;
}

int tsfi_appc_send_fmh7(tsfi_appc_conversation *conv, uint16_t sense_code) {
    if (!conv) return -1;
    if (conv->state == 3) return -2; // Deallocated
    conv->state = 3; // DEALLOCATED
    return (int)sense_code;
}

void cminit(const char *sym_dest, tsfi_appc_conversation *conv, int *rc) {
    if (!conv || !sym_dest) {
        if (rc) *rc = 19;
        return;
    }
    int local = 1;
    int partner = 2;
    if (strstr(sym_dest, "REMOTE")) {
        partner = 8;
    }
    int res = tsfi_appc_allocate(conv, local, partner);
    if (rc) {
        *rc = (res == 0) ? 0 : 20;
    }
}

void cmall(tsfi_appc_conversation *conv, int *rc) {
    if (!conv) {
        if (rc) *rc = 19;
        return;
    }
    conv->state = 1;
    if (rc) *rc = 0;
}

void cmsend(tsfi_appc_conversation *conv, const uint8_t *buf, size_t *len, int *rc) {
    if (!conv || !buf || !len) {
        if (rc) *rc = 19;
        return;
    }
    int res = tsfi_appc_send_data(conv, buf, *len);
    if (rc) {
        *rc = (res == 0) ? 0 : 20;
    }
}

void cmrcv(tsfi_appc_conversation *conv, uint8_t *buf, size_t *len, int *rc) {
    if (!conv || !buf || !len) {
        if (rc) *rc = 19;
        return;
    }
    int res = tsfi_appc_receive_data(conv, buf, len);
    if (rc) {
        *rc = (res == 0) ? 0 : 20;
    }
}

int tsfi_appc_synclog_mirror(tsfi_appc_conversation *conv, const uint8_t *log_data, size_t len) {
    if (!conv || !log_data || len == 0) return -1;
    if (conv->state == 3) return -2;
    return 0;
}

int tsfi_appc_chain_send(tsfi_appc_conversation *conv, const uint8_t *large_data, size_t len) {
    if (!conv || !large_data || len == 0) return -1;
    if (conv->state != 1) return -2;
    size_t offset = 0;
    while (offset < len) {
        size_t chunk_len = (len - offset > 256) ? 256 : (len - offset);
        int res = tsfi_appc_send_data(conv, large_data + offset, chunk_len);
        if (res != 0) return -3;
        offset += chunk_len;
    }
    return 0;
}

int tsfi_appc_chain_receive(tsfi_appc_conversation *conv, uint8_t *large_buf, size_t *len_out) {
    if (!conv || !large_buf || !len_out) return -1;
    if (conv->state != 2) return -2;
    size_t dummy_len = 0;
    int res = tsfi_appc_receive_data(conv, large_buf, &dummy_len);
    if (res != 0) return -3;
    *len_out = dummy_len;
    return 0;
}

int tsfi_appc_failover(tsfi_appc_conversation *conv, tsfi_sna_er_route *backup_route) {
    if (!conv || !backup_route) return -1;
    if (conv->state == 3) return -2;
    if (backup_route->active) {
        return 0;
    }
    return -3;
}

void cmqei(tsfi_appc_conversation *conv, int *sync_level, int *security_type, int *rc) {
    if (!conv) {
        if (rc) *rc = 19;
        return;
    }
    if (sync_level) *sync_level = conv->sync_state;
    if (security_type) *security_type = conv->security_active;
    if (rc) *rc = 0;
}

void cmqes(tsfi_appc_conversation *conv, int *state, int *rc) {
    if (!conv) {
        if (rc) *rc = 19;
        return;
    }
    if (state) *state = conv->state;
    if (rc) *rc = 0;
}

int tsfi_appc_serialize_fmh5(const char *tpn, int security_type, const char *user, const char *pwd, uint8_t *buf, size_t *len_out) {
    if (!tpn || !buf || !len_out) return -1;
    buf[0] = 0x05;
    buf[1] = (uint8_t)security_type;
    size_t tpn_len = strlen(tpn);
    buf[2] = (uint8_t)tpn_len;
    memcpy(buf + 3, tpn, tpn_len);
    size_t current_len = 3 + tpn_len;
    if (security_type > 0 && user && pwd) {
        size_t user_len = strlen(user);
        buf[current_len++] = (uint8_t)user_len;
        memcpy(buf + current_len, user, user_len);
        current_len += user_len;
        size_t pwd_len = strlen(pwd);
        buf[current_len++] = (uint8_t)pwd_len;
        memcpy(buf + current_len, pwd, pwd_len);
        current_len += pwd_len;
    }
    *len_out = current_len;
    return 0;
}

int tsfi_appc_deserialize_fmh5(const uint8_t *buf, size_t len, char *tpn_out, int *security_out, char *user_out, char *pwd_out) {
    if (!buf || len < 3 || buf[0] != 0x05) return -1;
    int sec_type = (int)buf[1];
    if (security_out) *security_out = sec_type;
    size_t tpn_len = buf[2];
    if (3 + tpn_len > len) return -2;
    if (tpn_out) {
        memcpy(tpn_out, buf + 3, tpn_len);
        tpn_out[tpn_len] = '\0';
    }
    size_t current_offset = 3 + tpn_len;
    if (sec_type > 0) {
        if (current_offset >= len) return 0;
        size_t user_len = buf[current_offset++];
        if (current_offset + user_len > len) return -3;
        if (user_out) {
            memcpy(user_out, buf + current_offset, user_len);
            user_out[user_len] = '\0';
        }
        current_offset += user_len;
        if (current_offset >= len) return 0;
        size_t pwd_len = buf[current_offset++];
        if (current_offset + pwd_len > len) return -4;
        if (pwd_out) {
            memcpy(pwd_out, buf + current_offset, pwd_len);
            pwd_out[pwd_len] = '\0';
        }
    }
    return 0;
}

void tsfi_appc_ebcdic_to_ascii(uint8_t *buf, size_t len) {
    if (!buf) return;
    for (size_t i = 0; i < len; i++) {
        if (buf[i] == 0xC1) buf[i] = 'A';
        else if (buf[i] == 0xE3) buf[i] = 'T';
        else if (buf[i] == 0xD7) buf[i] = 'P';
        else if (buf[i] == 0xD9) buf[i] = 'R';
        else if (buf[i] == 0xD6) buf[i] = 'O';
    }
}

void tsfi_appc_ascii_to_ebcdic(uint8_t *buf, size_t len) {
    if (!buf) return;
    for (size_t i = 0; i < len; i++) {
        if (buf[i] == 'A') buf[i] = 0xC1;
        else if (buf[i] == 'T') buf[i] = 0xE3;
        else if (buf[i] == 'P') buf[i] = 0xD7;
        else if (buf[i] == 'R') buf[i] = 0xD9;
        else if (buf[i] == 'O') buf[i] = 0xD6;
    }
}

void tsfi_appc_set_crypto(tsfi_appc_conversation *conv, void *crypto_session) {
    if (conv) {
        conv->crypto_session = crypto_session;
    }
}

void tsfi_appc_trace(tsfi_appc_conversation *conv, const char *event) {
    if (conv && event) {
        printf("[Trace APPC] Conv ID %08X Event: %s\n", conv->conversation_id, event);
    }
}

int tsfi_appc_synclog_archive(tsfi_appc_conversation *conv) {
    if (!conv) return -1;
    if (conv->state == 3) return -2;
    return 0;
}

int tsfi_appc_bind_negotiate(tsfi_appc_conversation *conv, uint32_t capabilities) {
    if (!conv) return -1;
    if (conv->state == 3) return -2;
    if (capabilities & 0x01) {
        conv->pacing_window = 8;
    }
    return 0;
}

int tsfi_appc_resolve_cpic_rc(uint16_t sense_code) {
    switch (sense_code) {
        case 0x0801: return 19;
        case 0x080F: return 20;
        default: return 20;
    }
}

int tsfi_appc_check_key_rotation(tsfi_appc_conversation *conv, size_t bytes_processed) {
    if (!conv) return -1;
    if (conv->state == 3) return -2;
    if (bytes_processed > 1024 * 1024) {
        if (conv->crypto_session) {
            return 1;
        }
    }
    return 0;
}

int tsfi_appc_set_conversation_type(tsfi_appc_conversation *conv, int conv_type) {
    if (!conv) return -1;
    if (conv->state == 3) return -2;
    conv->conversation_type = conv_type;
    return 0;
}

void cmsctpn(tsfi_appc_conversation *conv, const char *tpn, int *rc) {
    if (!conv || !tpn) {
        if (rc) *rc = 19;
        return;
    }
    strncpy(conv->tp_name, tpn, 63);
    conv->tp_name[63] = '\0';
    if (rc) *rc = 0;
}

void cmectpn(tsfi_appc_conversation *conv, char *tpn_out, int *rc) {
    if (!conv || !tpn_out) {
        if (rc) *rc = 19;
        return;
    }
    strcpy(tpn_out, conv->tp_name);
    if (rc) *rc = 0;
}

int tsfi_appc_teardown_session(tsfi_appc_conversation *conv, tsfi_sscp_lu_session *sscp) {
    if (!conv || !sscp) return -1;
    conv->state = 3;
    return tsfi_sscp_lu_control(sscp, 0x0F);
}

