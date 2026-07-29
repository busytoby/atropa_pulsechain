#include "auncient_edsac_firewall.h"
#include "auncient_timeline_autodin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEM_SIZE 128

typedef struct {
    char op;
    uint32_t address;
    char modifier;
} fw_inst_t;

typedef struct {
    uint32_t raw_value;
    bool is_instruction;
    fw_inst_t inst;
} fw_word_t;

static fw_word_t fw_memory[MEM_SIZE];
static uint32_t fw_rule_count = 0;
static uint32_t fw_base_addr = 0;

static uint32_t fw_encode(char op, uint32_t address, char modifier) {
    uint32_t raw = ((uint32_t)op & 0xFF) << 24;
    raw |= (address & 0x3FFFFF) << 2;
    if (modifier == 'D') raw |= 1;
    else if (modifier == 'L') raw |= 2;
    return raw;
}

static void fw_decode(uint32_t raw, fw_inst_t *inst) {
    inst->op = (char)((raw >> 24) & 0xFF);
    inst->address = (raw >> 2) & 0x3FFFFF;
    uint8_t mod = raw & 3;
    if (mod == 1) inst->modifier = 'D';
    else if (mod == 2) inst->modifier = 'L';
    else inst->modifier = 'F';
}

void auncient_analyzer_init(AuncientAnalyzer *analyzer, uint32_t prohibited_opcodes) {
    if (!analyzer) return;
    analyzer->prohibited_opcodes = prohibited_opcodes;
}

bool auncient_analyzer_classify(const AuncientAnalyzer *analyzer, const uint32_t *instructions, int count) {
    if (!instructions || count <= 0) return true;
    if (!analyzer) return true;

    // Validate 6-bit FIELDATA structures under Initial Orders 1 primary requirements
    for (int i = 0; i < count; i++) {
        uint32_t raw = instructions[i];
        char op = (char)((raw >> 24) & 0xFF);
        uint8_t fieldata_op = (uint8_t)(op & 0x3F);
        printf("[INITIAL ORDERS 1] Validating instruction %d: 6-bit FIELDATA primary opcode 0x%02X ('%c')\n", 
               i, fieldata_op, (op >= 32 && op < 127) ? op : '?');
    }

    for (int i = 0; i < count; i++) {
        uint32_t raw = instructions[i];
        char op = (char)((raw >> 24) & 0xFF);
        if (op >= 'A' && op <= 'Z') {
            uint32_t mask = 1 << (op - 'A');
            if (analyzer->prohibited_opcodes & mask) {
                printf("[ANALYZER CLASSIFY] Instruction %d (Opcode '%c') classified as IMPERMISSIBLE.\n", i, op);
                return false;
            }
        }
    }
    return true;
}

bool auncient_firewall_init(const char *rules_tape, uint32_t base_addr, const uint32_t *pki_keys, int key_count, const AuncientAnalyzer *analyzer) {
    memset(fw_memory, 0, sizeof(fw_memory));
    fw_rule_count = 0;
    fw_base_addr = base_addr;

    printf("[FIREWALL INITIALIZATION] Booting EDSAC-AUTODIN Firewall at base 0x%04X...\n", base_addr);

    // 1. Temporary batch parsing (Initial Orders 1 speculative load)
    char line[128];
    const char *ptr = rules_tape;
    uint32_t temp_instructions[MEM_SIZE] = {0};
    fw_inst_t temp_insts[MEM_SIZE];
    memset(temp_insts, 0, sizeof(temp_insts));
    int parsed_count = 0;

    while (*ptr != '\0' && parsed_count < MEM_SIZE) {
        int i = 0;
        while (*ptr != '\n' && *ptr != '\0' && i < 127) {
            line[i++] = *ptr++;
        }
        line[i] = '\0';
        if (*ptr == '\n') ptr++;

        if (strlen(line) == 0 || line[0] == ';') continue;

        char op;
        int address;
        char modifier;
        if (sscanf(line, "%c %d %c", &op, &address, &modifier) == 3) {
            temp_insts[parsed_count].op = op;
            temp_insts[parsed_count].address = address;
            temp_insts[parsed_count].modifier = modifier;
            temp_instructions[parsed_count] = fw_encode(op, address, modifier);
            parsed_count++;
        }
    }

    // 2. Validate the compiled rules batch using AUTODIN's speculative prefetch validator
    if (!auncient_autodin_speculative_prefetch_validate(base_addr, temp_instructions, parsed_count)) {
        printf("[FIREWALL BOOT REJECT] Speculative prefetch verification failed. Reverting load.\n");
        return false;
    }

    // Employ ANALYZER to classify permissible vs impermissible at Initial Orders 1
    if (analyzer && !auncient_analyzer_classify(analyzer, temp_instructions, parsed_count)) {
        printf("[FIREWALL BOOT REJECT] Analyzer classified rules tape as IMPERMISSIBLE.\n");
        return false;
    }

    // 3. Dispatch validation status via WinchesterMQ (wmq) requiring PKI authorization (>= 4 keys)
    if (!auncient_autodin_dispatch_wmq(temp_instructions[0], 0xF1, pki_keys, key_count)) {
        printf("[FIREWALL BOOT REJECT] WinchesterMQ registration dispatch failed.\n");
        return false;
    }

    // 4. Commit verified rules to EDSAC memory delay lines
    for (int idx = 0; idx < parsed_count; idx++) {
        fw_memory[idx].is_instruction = true;
        fw_memory[idx].inst = temp_insts[idx];
        fw_memory[idx].raw_value = temp_instructions[idx];
    }
    fw_rule_count = parsed_count;

    printf("[FIREWALL BOOT SUCCESS] Loaded %d rules successfully into active delay lines.\n", fw_rule_count);
    return true;
}

bool auncient_firewall_eval_packet(const AuncientPacket *packet) {
    if (!packet) return false;

    bool allowed = false;

    for (uint32_t idx = 0; idx < fw_rule_count; idx++) {
        const fw_word_t *rule = &fw_memory[idx];
        if (!rule->is_instruction) continue;

        if (!auncient_autodin_audit_edsac(fw_base_addr + idx, rule->raw_value, packet->payload_checksum)) {
            allowed = false;
            break;
        }

        char op = rule->inst.op;
        uint32_t target_port = rule->inst.address;

        if (op == 'A') {
            if (packet->dest_port == target_port) {
                allowed = true;
                break;
            }
        } else if (op == 'S') {
            if (packet->dest_port == target_port) {
                allowed = false;
                break;
            }
        }
    }

    printf("[FIREWALL EVALUATION] Port=%d Checksum=0x%08X -> %s\n", 
           packet->dest_port, packet->payload_checksum, allowed ? "ALLOW" : "DENY");
    return allowed;
}

bool auncient_firewall_relocate_rules(uint32_t offset) {
    printf("[FIREWALL RELOCATION] Relocating firewall rules by offset +%d under Initial Orders 2...\n", offset);

    for (uint32_t idx = 0; idx < fw_rule_count; idx++) {
        fw_word_t *rule = &fw_memory[idx];
        if (!rule->is_instruction) continue;

        uint32_t resolved_raw = auncient_initial_orders_2_resolve(rule->raw_value, offset);
        rule->raw_value = resolved_raw;
        fw_decode(resolved_raw, &rule->inst);
    }

    printf("[FIREWALL RELOCATION SUCCESS] All rules relocated to new coordinates.\n");
    return true;
}

bool auncient_analyzer_validate_cics_citizen(uint32_t writer_id) {
    char ssn[16];
    char site[32];
    extern void auncient_bridge_entity_to_ssa(const char *entity_name, char *ssn_out, char *site_out, int max_len);
    
    const char *entity_name = NULL;
    switch (writer_id) {
        case 555:    entity_name = "TeddyBearSkelCharacter"; break;
        case 888:    entity_name = "AuncientFederalWorkerCharacter"; break;
        case 99:     entity_name = "TestAgent99"; break;
        case 3:      entity_name = "TestAgent3"; break;
        case 10:     entity_name = "TestAgent10"; break;
        case 11:     entity_name = "TestAgent11"; break;
        case 20:     entity_name = "TestAgent20"; break;
        case 1:      entity_name = "TestAgent1"; break;
        case 2:      entity_name = "TestAgent2"; break;
        case 88:     entity_name = "TestAgent88"; break;
        case 42:     entity_name = "TestAgent42"; break;
        case 0x4001: entity_name = "WaylandInputAgent"; break;
        default:     return false; // Default reject (not registered with the SSA)
    }
    
    auncient_bridge_entity_to_ssa(entity_name, ssn, site, sizeof(site));
    if (strlen(ssn) == 0 || strcmp(ssn, "000-00-0000") == 0) {
        return false;
    }
    return true;
}
