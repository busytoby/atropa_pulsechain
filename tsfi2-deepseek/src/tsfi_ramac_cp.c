#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_winchester_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846

#endif
int tsfi_b5000_descriptor_read(const tsfi_b5000_descriptor *desc, const uint8_t *memory, uint32_t offset, uint8_t *val_out) {
    if (!desc || !memory || !val_out) return -1;
    if (!desc->is_present) return -2; // Segment not present
    if (offset >= desc->limit) return -3; // Bounds check fail (Descriptor Protection)
    
    *val_out = memory[desc->address + offset];
    return 0;
}

int tsfi_b5000_descriptor_write(const tsfi_b5000_descriptor *desc, uint8_t *memory, uint32_t offset, uint8_t val) {
    if (!desc || !memory) return -1;
    if (!desc->is_present) return -2; // Segment not present
    if (desc->read_only) return -3; // Read-only violation
    if (offset >= desc->limit) return -4; // Bounds check fail (Descriptor Protection)
    
    memory[desc->address + offset] = val;
    return 0;
}

int tsfi_b5000_segment_load(const char *filepath, tsfi_b5000_descriptor *desc_out, uint8_t *memory, uint32_t mem_offset, uint32_t max_bytes) {
    if (!filepath || !desc_out || !memory) return -1;
    
    FILE *f = fopen(filepath, "rb");
    if (!f) return -2;
    
    size_t bytes_read = fread(memory + mem_offset, 1, max_bytes, f);
    fclose(f);
    
    if (bytes_read == 0) return -3;
    
    desc_out->address = mem_offset;
    desc_out->limit = (uint32_t)bytes_read;
    desc_out->is_present = 1;
    desc_out->read_only = 0; // Default readable/writable
    
    return 0;
}

void tsfi_b5000_mcp_init(tsfi_b5000_mcp_scheduler *sched) {
    if (!sched) return;
    sched->active_task_idx = -1;
    for (int i = 0; i < 4; i++) {
        sched->tasks[i].task_id = i;
        sched->tasks[i].pc = 0;
        memset(sched->tasks[i].registers, 0, sizeof(sched->tasks[i].registers));
        memset(sched->tasks[i].eval_stack, 0, sizeof(sched->tasks[i].eval_stack));
        sched->tasks[i].eval_stack_ptr = 0;
        sched->tasks[i].state = 0; // IDLE
    }
}

int tsfi_b5000_mcp_schedule_tick(tsfi_b5000_mcp_scheduler *sched) {
    if (!sched) return -1;
    
    int start = (sched->active_task_idx == -1) ? 0 : (sched->active_task_idx + 1) % 4;
    for (int i = 0; i < 4; i++) {
        int idx = (start + i) % 4;
        if (sched->tasks[idx].state == 1) { // RUNNABLE
            sched->active_task_idx = idx;
            return idx;
        }
    }
    return -1; // No runnable tasks
}

int tsfi_b5000_mcp_yield_active(tsfi_b5000_mcp_scheduler *sched, int block_reason) {
    if (!sched || sched->active_task_idx == -1) return -1;
    
    int active = sched->active_task_idx;
    sched->tasks[active].state = block_reason; // block or set idle
    return tsfi_b5000_mcp_schedule_tick(sched);
}

int tsfi_b5000_decode_syllable(uint16_t syllable, uint8_t *type_out, uint16_t *val_out) {
    if (!type_out || !val_out) return -1;
    *type_out = (uint8_t)((syllable >> 10) & 0x03);
    *val_out = (uint16_t)(syllable & 0x3FF);
    return 0;
}

int tsfi_b5000_execute_word(uint64_t instruction_word, void *strategy_vm, const uint8_t *memory, const tsfi_b5000_descriptor *prt, int prt_size) {
    if (!strategy_vm || !memory || !prt || prt_size <= 0) return -1;
    
    TSFiStrategyVM *vm = (TSFiStrategyVM *)strategy_vm;
    
    uint16_t syllables[4];
    syllables[0] = (uint16_t)((instruction_word >> 36) & 0xFFF);
    syllables[1] = (uint16_t)((instruction_word >> 24) & 0xFFF);
    syllables[2] = (uint16_t)((instruction_word >> 12) & 0xFFF);
    syllables[3] = (uint16_t)(instruction_word & 0xFFF);
    
    for (int i = 0; i < 4; i++) {
        uint8_t type = 0;
        uint16_t val = 0;
        tsfi_b5000_decode_syllable(syllables[i], &type, &val);
        
        if (type == 2) { // Literal Call
            if (vm->eval_stack_ptr < 32) {
                vm->eval_stack[vm->eval_stack_ptr++] = val;
            } else {
                return -2; // Stack overflow
            }
        } else if (type == 0) { // Operand Call
            if (val >= prt_size || !prt[val].is_present) return -3;
            // Read value from descriptor mapping
            uint8_t byte_val = 0;
            if (tsfi_b5000_descriptor_read(&prt[val], memory, 0, &byte_val) != 0) return -4;
            if (vm->eval_stack_ptr < 32) {
                vm->eval_stack[vm->eval_stack_ptr++] = byte_val;
            } else {
                return -2;
            }
        } else if (type == 1) { // Descriptor Call
            if (val >= prt_size || !prt[val].is_present) return -3;
            if (vm->eval_stack_ptr < 32) {
                vm->eval_stack[vm->eval_stack_ptr++] = (int)prt[val].address;
            } else {
                return -2;
            }
        } else if (type == 3) { // Operator
            if (val == 0) { // ADD
                if (vm->eval_stack_ptr >= 2) {
                    int b = vm->eval_stack[--vm->eval_stack_ptr];
                    int a = vm->eval_stack[--vm->eval_stack_ptr];
                    vm->eval_stack[vm->eval_stack_ptr++] = a + b;
                } else {
                    return -5; // Stack underflow
                }
            } else if (val == 1) { // SUB
                if (vm->eval_stack_ptr >= 2) {
                    int b = vm->eval_stack[--vm->eval_stack_ptr];
                    int a = vm->eval_stack[--vm->eval_stack_ptr];
                    vm->eval_stack[vm->eval_stack_ptr++] = a - b;
                } else {
                    return -5;
                }
            }
        }
    }
    return 0;
}

void tsfi_command_init(tsfi_command_center *cc) {
    if (!cc) return;
    cc->sensor_count = 0;
    for (int i = 0; i < 4; i++) {
        cc->sensors[i].sensor_name[0] = '\0';
        cc->sensors[i].curr_value = 0;
        cc->sensors[i].threshold = 0;
        cc->sensors[i].alert_triggered = 0;
    }
}

int tsfi_command_poll(tsfi_command_center *cc) {
    if (!cc) return 0;
    int triggered = 0;
    for (int i = 0; i < cc->sensor_count; i++) {
        if (cc->sensors[i].curr_value > cc->sensors[i].threshold) {
            cc->sensors[i].alert_triggered = 1;
            triggered = 1;
        } else {
            cc->sensors[i].alert_triggered = 0;
        }
    }
    return triggered;
}

void tsfi_interrupt_init(tsfi_cobol_interrupt_controller *ctrl) {
    if (!ctrl) return;
    ctrl->vector_count = 0;
    for (int i = 0; i < 4; i++) {
        ctrl->vectors[i].interrupt_code = -1;
        ctrl->vectors[i].cobol_handler[0] = '\0';
    }
}

int tsfi_interrupt_register(tsfi_cobol_interrupt_controller *ctrl, int code, const char *handler) {
    if (!ctrl || ctrl->vector_count >= 4 || !handler) return -1;
    ctrl->vectors[ctrl->vector_count].interrupt_code = code;
    strncpy(ctrl->vectors[ctrl->vector_count].cobol_handler, handler, sizeof(ctrl->vectors[ctrl->vector_count].cobol_handler) - 1);
    ctrl->vectors[ctrl->vector_count].cobol_handler[sizeof(ctrl->vectors[ctrl->vector_count].cobol_handler) - 1] = '\0';
    ctrl->vector_count++;
    return 0;
}

int tsfi_interrupt_dispatch(tsfi_cobol_interrupt_controller *ctrl, int code, int regs[8]) {
    if (!ctrl || !regs) return -1;
    for (int i = 0; i < ctrl->vector_count; i++) {
        if (ctrl->vectors[i].interrupt_code == code) {
            const char *handler = ctrl->vectors[i].cobol_handler;
            int rx = -1, val = -1;
            if (sscanf(handler, "SET R%d %d", &rx, &val) == 2) {
                if (rx >= 0 && rx < 8) regs[rx] = val;
                return 0;
            } else if (strncmp(handler, "COMPUTE ", 8) == 0) {
                const char *expr = handler + 8;
                const char *eq = strchr(expr, '=');
                int target_r = -1;
                if (eq && sscanf(expr, "R%d", &target_r) == 1) {
                    int comp_val = tsfi_cobol_compute_eval(eq + 1, regs);
                    if (target_r >= 0 && target_r < 8) regs[target_r] = comp_val;
                    return 0;
                }
            }
        }
    }
    return -2;
}

void tsfi_pli_exception_init(pli_exception_system *sys) {
    if (!sys) return;
    sys->count = 0;
    for (int i = 0; i < 8; i++) {
        sys->units[i].exception_type[0] = '\0';
        sys->units[i].handler_action[0] = '\0';
        sys->units[i].active = 0;
    }
}

int tsfi_pli_exception_register(pli_exception_system *sys, const char *type, const char *action) {
    if (!sys || sys->count >= 8 || !type || !action) return -1;
    pli_on_unit *unit = &sys->units[sys->count];
    strncpy(unit->exception_type, type, sizeof(unit->exception_type) - 1);
    unit->exception_type[sizeof(unit->exception_type) - 1] = '\0';
    strncpy(unit->handler_action, action, sizeof(unit->handler_action) - 1);
    unit->handler_action[sizeof(unit->handler_action) - 1] = '\0';
    unit->active = 1;
    sys->count++;
    return 0;
}

int tsfi_pli_exception_trigger(pli_exception_system *sys, const char *type, int regs[8]) {
    if (!sys || !type || !regs) return -1;
    for (int i = sys->count - 1; i >= 0; i--) {
        if (sys->units[i].active && strcmp(sys->units[i].exception_type, type) == 0) {
            const char *action = sys->units[i].handler_action;
            int rx = -1, val = -1;
            if (sscanf(action, "SET R%d %d", &rx, &val) == 2) {
                if (rx >= 0 && rx < 8) regs[rx] = val;
                return 0;
            }
        }
    }
    return -2;
}

void tsfi_multics_init(multics_segment_table *table) {
    if (!table) return;
    table->count = 0;
    for (int i = 0; i < 16; i++) {
        table->segments[i].segment_id = 0;
        table->segments[i].base_addr = 0;
        table->segments[i].size = 0;
        table->segments[i].acl_flags = 0;
    }
}

int tsfi_multics_register(multics_segment_table *table, uint32_t segment_id, uintptr_t base_addr, size_t size, uint8_t flags) {
    if (!table || table->count >= 16) return -1;
    multics_segment *seg = &table->segments[table->count++];
    seg->segment_id = segment_id;
    seg->base_addr = base_addr;
    seg->size = size;
    seg->acl_flags = flags;
    return 0;
}

int tsfi_multics_check_access(const multics_segment_table *table, uintptr_t addr, uint8_t required_flags) {
    if (!table) return -1;
    for (int i = 0; i < table->count; i++) {
        const multics_segment *seg = &table->segments[i];
        if (addr >= seg->base_addr && addr < (seg->base_addr + seg->size)) {
            if ((seg->acl_flags & required_flags) == required_flags) {
                return 0;
            } else {
                return -2;
            }
        }
    }
    return -3;
}

void tsfi_vtam_session_init(tsfi_vtam_session *session, const char *lu_name) {
    if (!session) return;
    memset(session, 0, sizeof(tsfi_vtam_session));
    if (lu_name) strncpy(session->logical_unit, lu_name, sizeof(session->logical_unit) - 1);
    session->session_state = VTAM_STATE_INIT;
}

int tsfi_vtam_session_handshake(tsfi_vtam_session *session, int event) {
    if (!session) return -1;
    switch (session->session_state) {
        case VTAM_STATE_INIT:
            if (event == VTAM_EV_BIND) {
                session->session_state = VTAM_STATE_NEGOTIATE;
                return 0;
            }
            break;
        case VTAM_STATE_NEGOTIATE:
            if (event == VTAM_EV_SDT) {
                session->session_state = VTAM_STATE_ACTIVE;
                return 0;
            }
            break;
        case VTAM_STATE_ACTIVE:
            if (event == VTAM_EV_UNBIND) {
                session->session_state = VTAM_STATE_TERMINATED;
                return 0;
            }
            break;
        default:
            break;
    }
    return -2;
}

int tsfi_vtam_session_send(tsfi_vtam_session *session, const char *data, int data_len) {
    if (!session || !data || data_len <= 0) return -1;
    if (session->session_state != VTAM_STATE_ACTIVE) {
        return -3;
    }
    session->bytes_transmitted += data_len;
    return data_len;
}

void tsfi_cms_session_init(tsfi_cms_session *sess) {
    if (!sess) return;
    memset(sess, 0, sizeof(tsfi_cms_session));
}

int tsfi_cms_execute_command(tsfi_cms_session *sess, const char *cmd) {
    if (!sess || !cmd) return -1;
    if (strcasecmp(cmd, "IPL CMS") == 0) {
        sess->is_booted = 1;
        sess->last_command_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "ACCESS 191 A") == 0) {
        if (!sess->is_booted) {
            sess->last_command_status = -2;
            return -2;
        }
        sess->disk_mounted_191 = 1;
        sess->last_command_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "LISTFILE") == 0) {
        if (!sess->disk_mounted_191) {
            sess->last_command_status = -3;
            return -3;
        }
        sess->last_command_status = 0;
        return 0;
    }
    sess->last_command_status = -1;
    return -1;
}

void tsfi_cp_session_init(tsfi_cp_session *sess) {
    if (!sess) return;
    memset(sess, 0, sizeof(tsfi_cp_session));
    sess->virtual_storage_kb = 4096;
    sess->spool_class = ' ';
    sess->last_cp_status = 0;
}

int tsfi_cp_execute_command(tsfi_cp_session *sess, const char *cmd) {
    if (!sess || !cmd) return -1;
    if (strcasecmp(cmd, "DEFINE STORAGE 16M") == 0) {
        sess->virtual_storage_kb = 16384;
        sess->last_cp_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "SPOOL PUNCH CLASS A") == 0) {
        sess->spool_class = 'A';
        sess->last_cp_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "QUERY VIRTUAL") == 0) {
        sess->last_cp_status = 0;
        return 0;
    }
    sess->last_cp_status = -1;
    return -1;
}

void tsfi_cp_spool_queue_init(tsfi_cp_spool_queue *q) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue));
}

int tsfi_cp_spool_push(tsfi_cp_spool_queue *q, const char *data) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS) return -1;
    strncpy(q->queue[q->tail].card_data, data, sizeof(q->queue[q->tail].card_data) - 1);
    q->queue[q->tail].card_data[sizeof(q->queue[q->tail].card_data) - 1] = '\0';
    q->tail = (q->tail + 1) % MAX_SPOOL_CARDS;
    q->count++;
    return 0;
}

int tsfi_cp_spool_pop(tsfi_cp_spool_queue *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    strncpy(data_out, q->queue[q->head].card_data, 79);
    data_out[79] = '\0';
    q->head = (q->head + 1) % MAX_SPOOL_CARDS;
    q->count--;
    return 0;
}

void tsfi_iucv_broker_init(tsfi_iucv_broker *broker) {
    if (!broker) return;
    memset(broker, 0, sizeof(tsfi_iucv_broker));
}

int tsfi_iucv_connect(tsfi_iucv_broker *broker, const char *src, const char *dest) {
    if (!broker || !src || !dest) return -1;
    for (int i = 0; i < MAX_IUCV_PATHS; i++) {
        if (broker->paths[i].status == IUCV_PATH_FREE) {
            broker->paths[i].path_id = i;
            strncpy(broker->paths[i].source_user, src, sizeof(broker->paths[i].source_user) - 1);
            strncpy(broker->paths[i].dest_user, dest, sizeof(broker->paths[i].dest_user) - 1);
            broker->paths[i].status = IUCV_PATH_ACTIVE;
            broker->paths[i].message_pending = 0;
            return i;
        }
    }
    return -2;
}

int tsfi_iucv_send(tsfi_iucv_broker *broker, int path_id, const char *msg) {
    if (!broker || path_id < 0 || path_id >= MAX_IUCV_PATHS) return -1;
    if (broker->paths[path_id].status != IUCV_PATH_ACTIVE) return -2;
    strncpy(broker->paths[path_id].buffered_message, msg, sizeof(broker->paths[path_id].buffered_message) - 1);
    broker->paths[path_id].buffered_message[sizeof(broker->paths[path_id].buffered_message) - 1] = '\0';
    broker->paths[path_id].message_pending = 1;
    return 0;
}

int tsfi_iucv_receive(tsfi_iucv_broker *broker, int path_id, char *msg_out) {
    if (!broker || path_id < 0 || path_id >= MAX_IUCV_PATHS || !msg_out) return -1;
    if (broker->paths[path_id].status != IUCV_PATH_ACTIVE || !broker->paths[path_id].message_pending) return -2;
    strncpy(msg_out, broker->paths[path_id].buffered_message, 63);
    msg_out[63] = '\0';
    broker->paths[path_id].message_pending = 0;
    return 0;
}

void tsfi_cp_console_spool_init(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    memset(spool, 0, sizeof(tsfi_cp_console_spool));
}

void tsfi_cp_console_spool_start(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    spool->is_spooling = 1;
    spool->log_len = 0;
    spool->log_buffer[0] = '\0';
}

void tsfi_cp_console_spool_write(tsfi_cp_console_spool *spool, const char *text) {
    if (!spool || !text || !spool->is_spooling) return;
    int len = strlen(text);
    if (spool->log_len + len < 511) {
        memcpy(spool->log_buffer + spool->log_len, text, len);
        spool->log_len += len;
        spool->log_buffer[spool->log_len] = '\0';
    }
}

void tsfi_cp_console_spool_stop(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    spool->is_spooling = 0;
}

void tsfi_cp_scheduler_init(tsfi_cp_scheduler *sched) {
    if (!sched) return;
    memset(sched, 0, sizeof(tsfi_cp_scheduler));
}

int tsfi_cp_scheduler_register(tsfi_cp_scheduler *sched, const char *name, int initial_q) {
    if (!sched || !name || sched->task_count >= 8) return -1;
    strncpy(sched->tasks[sched->task_count].vm_name, name, sizeof(sched->tasks[sched->task_count].vm_name) - 1);
    sched->tasks[sched->task_count].vm_name[sizeof(sched->tasks[sched->task_count].vm_name) - 1] = '\0';
    sched->tasks[sched->task_count].queue_type = initial_q;
    sched->tasks[sched->task_count].cpu_cycles_used = 0;
    sched->task_count++;
    return 0;
}

int tsfi_cp_scheduler_dispatch(tsfi_cp_scheduler *sched, int task_idx, int cycles) {
    if (!sched || task_idx < 0 || task_idx >= sched->task_count) return -1;
    sched->tasks[task_idx].cpu_cycles_used += cycles;
    if (sched->tasks[task_idx].queue_type == VM_QUEUE_Q1 && cycles > 50) {
        sched->tasks[task_idx].queue_type = VM_QUEUE_Q2;
    }
    return 0;
}

void tsfi_cp_directory_init(tsfi_cp_directory *dir) {
    if (!dir) return;
    memset(dir, 0, sizeof(tsfi_cp_directory));
}

int tsfi_cp_directory_add(tsfi_cp_directory *dir, const char *uid, char priv, uint32_t max_store) {
    if (!dir || !uid || dir->entry_count >= 8) return -1;
    strncpy(dir->entries[dir->entry_count].userid, uid, sizeof(dir->entries[dir->entry_count].userid) - 1);
    dir->entries[dir->entry_count].userid[sizeof(dir->entries[dir->entry_count].userid) - 1] = '\0';
    dir->entries[dir->entry_count].privilege_class = priv;
    dir->entries[dir->entry_count].max_storage_kb = max_store;
    dir->entry_count++;
    return 0;
}

int tsfi_cp_directory_check(const tsfi_cp_directory *dir, const char *uid, char required_priv) {
    if (!dir || !uid) return -1;
    for (int i = 0; i < dir->entry_count; i++) {
        if (strcmp(dir->entries[i].userid, uid) == 0) {
            if (required_priv == 'A' && dir->entries[i].privilege_class != 'A') {
                return -2;
            }
            return 0;
        }
    }
    return -1;
}

void tsfi_cp_attachment_init(tsfi_cp_attachment_manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(tsfi_cp_attachment_manager));
}

int tsfi_cp_attachment_register(tsfi_cp_attachment_manager *mgr, uint32_t phys_addr) {
    if (!mgr || mgr->device_count >= 8) return -1;
    mgr->devices[mgr->device_count].physical_address = phys_addr;
    mgr->devices[mgr->device_count].virtual_address = 0;
    mgr->devices[mgr->device_count].is_attached = 0;
    memset(mgr->devices[mgr->device_count].dedicated_user, 0, 16);
    mgr->device_count++;
    return 0;
}

int tsfi_cp_attach(tsfi_cp_attachment_manager *mgr, uint32_t phys_addr, const char *uid, uint32_t virt_addr) {
    if (!mgr || !uid) return -1;
    for (int i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].physical_address == phys_addr) {
            if (mgr->devices[i].is_attached) {
                return -2;
            }
            mgr->devices[i].virtual_address = virt_addr;
            strncpy(mgr->devices[i].dedicated_user, uid, 15);
            mgr->devices[i].dedicated_user[15] = '\0';
            mgr->devices[i].is_attached = 1;
            return 0;
        }
    }
    return -3;
}

int tsfi_cp_detach(tsfi_cp_attachment_manager *mgr, uint32_t virt_addr, const char *uid) {
    if (!mgr || !uid) return -1;
    for (int i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].is_attached && mgr->devices[i].virtual_address == virt_addr) {
            if (strcmp(mgr->devices[i].dedicated_user, uid) != 0) {
                return -2;
            }
            mgr->devices[i].virtual_address = 0;
            memset(mgr->devices[i].dedicated_user, 0, 16);
            mgr->devices[i].is_attached = 0;
            return 0;
        }
    }
    return -3;
}

void tsfi_cp_smsg_init(tsfi_cp_smsg_receiver *rcv, const char *uid) {
    if (!rcv) return;
    memset(rcv, 0, sizeof(tsfi_cp_smsg_receiver));
    if (uid) {
        strncpy(rcv->userid, uid, sizeof(rcv->userid) - 1);
        rcv->userid[sizeof(rcv->userid) - 1] = '\0';
    }
    rcv->smsg_enabled = 1;
}

int tsfi_cp_smsg_send(tsfi_cp_smsg_receiver *rcv, const char *msg) {
    if (!rcv || !msg) return -1;
    if (!rcv->smsg_enabled) {
        return -1;
    }
    if (rcv->count >= 8) {
        return -2;
    }
    strncpy(rcv->queue[rcv->tail].payload, msg, sizeof(rcv->queue[rcv->tail].payload) - 1);
    rcv->queue[rcv->tail].payload[sizeof(rcv->queue[rcv->tail].payload) - 1] = '\0';
    rcv->tail = (rcv->tail + 1) % 8;
    rcv->count++;
    return 0;
}

int tsfi_cp_smsg_receive(tsfi_cp_smsg_receiver *rcv, char *msg_out) {
    if (!rcv || !msg_out || rcv->count <= 0) return -1;
    strncpy(msg_out, rcv->queue[rcv->head].payload, 63);
    msg_out[63] = '\0';
    rcv->head = (rcv->head + 1) % 8;
    rcv->count--;
    return 0;
}

void tsfi_cp_vcpu_init(tsfi_cp_vcpu *vcpu) {
    if (!vcpu) return;
    memset(vcpu, 0, sizeof(tsfi_cp_vcpu));
    vcpu->state = VCPU_STOPPED;
    vcpu->psw_instruction_address = 0x00000000;
    vcpu->psw_mask = 0x00000000;
}

int tsfi_cp_vcpu_control(tsfi_cp_vcpu *vcpu, const char *action) {
    if (!vcpu || !action) return -1;
    if (strcasecmp(action, "START") == 0) {
        vcpu->state = VCPU_RUNNING;
        return 0;
    }
    if (strcasecmp(action, "STOP") == 0) {
        vcpu->state = VCPU_STOPPED;
        return 0;
    }
    if (strcasecmp(action, "RESET") == 0) {
        vcpu->state = VCPU_STOPPED;
        vcpu->psw_instruction_address = 0x00000000;
        vcpu->psw_mask = 0x00000000;
        return 0;
    }
    return -1;
}

void tsfi_cp_printer_init(tsfi_cp_spool_printer *prt) {
    if (!prt) return;
    memset(prt, 0, sizeof(tsfi_cp_spool_printer));
    prt->page_count = 1;
}

int tsfi_cp_printer_write_record(tsfi_cp_spool_printer *prt, const char *record) {
    if (!prt || !record || record[0] == '\0') return -1;
    char ctrl = record[0];
    switch (ctrl) {
        case '1':
            prt->line_count = 0;
            prt->page_count++;
            prt->last_skip_count = 0;
            break;
        case ' ':
            prt->line_count += 1;
            prt->last_skip_count = 1;
            break;
        case '0':
            prt->line_count += 2;
            prt->last_skip_count = 2;
            break;
        case '-':
            prt->line_count += 3;
            prt->last_skip_count = 3;
            break;
        default:
            prt->line_count += 1;
            prt->last_skip_count = 1;
            break;
    }
    return 0;
}

void tsfi_cp_spool_queue_v2_init(tsfi_cp_spool_queue_v2 *q, char reader_class) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue_v2));
    q->active_reader_class = reader_class;
}

int tsfi_cp_spool_push_v2(tsfi_cp_spool_queue_v2 *q, const char *data, char card_class) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS_V2) return -1;
    strncpy(q->queue[q->count].card_data, data, sizeof(q->queue[q->count].card_data) - 1);
    q->queue[q->count].card_data[sizeof(q->queue[q->count].card_data) - 1] = '\0';
    q->queue[q->count].spool_class = card_class;
    q->count++;
    return 0;
}

int tsfi_cp_spool_pop_v2(tsfi_cp_spool_queue_v2 *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    for (int i = 0; i < q->count; i++) {
        if (q->active_reader_class == '*' || q->queue[i].spool_class == q->active_reader_class) {
            strncpy(data_out, q->queue[i].card_data, 79);
            data_out[79] = '\0';
            for (int j = i; j < q->count - 1; j++) {
                q->queue[j] = q->queue[j + 1];
            }
            q->count--;
            return 0;
        }
    }
    return -2;
}

void tsfi_cp_spool_queue_v3_init(tsfi_cp_spool_queue_v3 *q) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue_v3));
}

int tsfi_cp_spool_push_v3(tsfi_cp_spool_queue_v3 *q, const char *data, int file_id) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS_V3) return -1;
    strncpy(q->queue[q->count].card_data, data, sizeof(q->queue[q->count].card_data) - 1);
    q->queue[q->count].card_data[sizeof(q->queue[q->count].card_data) - 1] = '\0';
    q->queue[q->count].file_id = file_id;
    q->queue[q->count].is_held = 0;
    q->count++;
    return 0;
}

int tsfi_cp_spool_hold(tsfi_cp_spool_queue_v3 *q, int file_id, int hold) {
    if (!q) return -1;
    for (int i = 0; i < q->count; i++) {
        if (q->queue[i].file_id == file_id) {
            q->queue[i].is_held = hold;
            return 0;
        }
    }
    return -1;
}

int tsfi_cp_spool_pop_v3(tsfi_cp_spool_queue_v3 *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    for (int i = 0; i < q->count; i++) {
        if (!q->queue[i].is_held) {
            strncpy(data_out, q->queue[i].card_data, 79);
            data_out[79] = '\0';
            for (int j = i; j < q->count - 1; j++) {
                q->queue[j] = q->queue[j + 1];
            }
            q->count--;
            return 0;
        }
    }
    return -2;
}

void tsfi_cp_purge_stats_init(tsfi_cp_purge_stats *stats) {
    if (!stats) return;
    memset(stats, 0, sizeof(tsfi_cp_purge_stats));
}

int tsfi_cp_execute_purge(tsfi_cp_purge_stats *stats, tsfi_cp_spool_queue_v3 *rdr, tsfi_cp_spool_printer *prt, const char *cmd) {
    if (!stats || !cmd) return -1;
    if (strcasecmp(cmd, "PURGE RDR") == 0) {
        if (rdr) {
            stats->reader_count = rdr->count;
            stats->total_purged += rdr->count;
            rdr->count = 0;
            memset(rdr->queue, 0, sizeof(rdr->queue));
        }
        return 0;
    }
    if (strcasecmp(cmd, "PURGE ALL") == 0) {
        if (rdr) {
            stats->reader_count = rdr->count;
            stats->total_purged += rdr->count;
            rdr->count = 0;
            memset(rdr->queue, 0, sizeof(rdr->queue));
        }
        if (prt) {
            stats->printer_count = prt->line_count;
            stats->total_purged += prt->line_count;
            prt->line_count = 0;
            prt->page_count = 1;
            prt->last_skip_count = 0;
        }
        return 0;
    }
    return -1;
}

void tsfi_cp_query_init(tsfi_cp_query_manager *mgr) {
    if (!mgr) return;
    mgr->simulated_connect_time_sec = 3600;
    mgr->active_user_count = 2;
}

int tsfi_cp_query_execute(tsfi_cp_query_manager *mgr, const char *query_cmd, char *out_buf, int out_max) {
    if (!mgr || !query_cmd || !out_buf || out_max <= 0) return -1;
    if (strcasecmp(query_cmd, "QUERY TIME") == 0) {
        snprintf(out_buf, out_max, "CONNECT TIME: %u SEC", mgr->simulated_connect_time_sec);
        return 0;
    }
    if (strcasecmp(query_cmd, "QUERY NAMES") == 0) {
        snprintf(out_buf, out_max, "ACTIVE USERS: %d", mgr->active_user_count);
        return 0;
    }
    return -1;
}

int tsfi_cp_msg_send(const tsfi_cp_directory *dir, const char *sender, const char *recipient, const char *msg_text, char *out_terminal_buf, int out_terminal_max) {
    if (!dir || !sender || !recipient || !msg_text || !out_terminal_buf || out_terminal_max <= 0) return -1;
    
    int found = 0;
    for (int i = 0; i < dir->entry_count; i++) {
        if (strcmp(dir->entries[i].userid, recipient) == 0) {
            found = 1;
            break;
        }
    }
    
    if (found) {
        snprintf(out_terminal_buf, out_terminal_max, "MSG FROM %s: %s", sender, msg_text);
        return 0;
    } else {
        snprintf(out_terminal_buf, out_terminal_max, "ERR: USER %s NOT LOGGED ON", recipient);
        return -1;
    }
}

int tsfi_cp_warning_broadcast(const tsfi_cp_directory *dir, const char *sender_uid, const char *warn_text, char out_terminals[8][128], int *broadcast_count) {
    if (!dir || !sender_uid || !warn_text || !out_terminals || !broadcast_count) return -1;
    
    int priv_ok = tsfi_cp_directory_check(dir, sender_uid, 'A');
    if (priv_ok != 0) {
        return -2;
    }
    
    *broadcast_count = 0;
    for (int i = 0; i < dir->entry_count; i++) {
        snprintf(out_terminals[i], 127, "WARN: %s", warn_text);
        out_terminals[i][127] = '\0';
        (*broadcast_count)++;
    }
    return 0;
}

void tsfi_cp_sleep_init(tsfi_cp_terminal_sleep *t) {
    if (!t) return;
    t->is_sleeping = 0;
    t->remaining_seconds = 0;
}

int tsfi_cp_sleep_start(tsfi_cp_terminal_sleep *t, int seconds) {
    if (!t || seconds <= 0) return -1;
    t->is_sleeping = 1;
    t->remaining_seconds = seconds;
    return 0;
}

int tsfi_cp_sleep_tick(tsfi_cp_terminal_sleep *t) {
    if (!t || !t->is_sleeping) return 0;
    t->remaining_seconds--;
    if (t->remaining_seconds <= 0) {
        t->remaining_seconds = 0;
        t->is_sleeping = 0;
    }
    return t->remaining_seconds;
}

int tsfi_cp_sleep_interrupt(tsfi_cp_terminal_sleep *t) {
    if (!t) return -1;
    t->is_sleeping = 0;
    t->remaining_seconds = 0;
    return 0;
}

void tsfi_cp_active_session_init(tsfi_cp_active_session *sess, const char *uid) {
    if (!sess) return;
    memset(sess, 0, sizeof(tsfi_cp_active_session));
    if (uid) {
        strncpy(sess->userid, uid, sizeof(sess->userid) - 1);
        sess->userid[sizeof(sess->userid) - 1] = '\0';
    }
    sess->is_connected = 1;
}

int tsfi_cp_active_session_disconnect(tsfi_cp_active_session *sess) {
    if (!sess) return -1;
    sess->is_connected = 0;
    return 0;
}

int tsfi_cp_active_session_connect(tsfi_cp_active_session *sess) {
    if (!sess) return -1;
    sess->is_connected = 1;
    return 0;
}

int tsfi_cp_active_session_dispatch(tsfi_cp_active_session *sess, int cycles) {
    if (!sess) return -1;
    sess->background_cycles_run += cycles;
    return sess->background_cycles_run;
}

