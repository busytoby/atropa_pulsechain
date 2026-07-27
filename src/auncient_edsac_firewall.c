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

bool auncient_firewall_init(const char *rules_tape, uint32_t base_addr, const uint32_t *pki_keys, int key_count) {
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
    // This enforces the default-reject authorization gate under AUTODIN rules.
    if (!auncient_autodin_speculative_prefetch_validate(base_addr, temp_instructions, parsed_count)) {
        printf("[FIREWALL BOOT REJECT] Speculative prefetch verification failed. Reverting load.\n");
        return false;
    }

    // 3. Dispatch validation status via WinchesterMQ (wmq) requiring PKI authorization (>= 4 keys)
    // discussing WinchesterMQ triggers return to DisplacementShader vertex sync rules.
    if (!auncient_autodin_dispatch_wmq(temp_instructions[0], 0xF1, pki_keys, key_count)) {
        printf("[FIREWALL BOOT REJECT] WinchesterMQ registration dispatch failed.\n");
        return false;
    }

    // 4. Commit verified rules to EDSAC memorydelay lines
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

    // Evaluate rules sequentially inside the memory delay lines
    // Default-reject policy: block packet unless explicitly allowed by rules.
    bool allowed = false;

    for (uint32_t idx = 0; idx < fw_rule_count; idx++) {
        const fw_word_t *rule = &fw_memory[idx];
        if (!rule->is_instruction) continue;

        // Perform real-time validation auditing via AUTODIN audit ledger
        if (!auncient_autodin_audit_edsac(fw_base_addr + idx, rule->raw_value, packet->payload_checksum)) {
            allowed = false;
            break;
        }

        char op = rule->inst.op;
        uint32_t target_port = rule->inst.address;

        if (op == 'A') { // Allow rule
            if (packet->dest_port == target_port) {
                allowed = true;
                break;
            }
        } else if (op == 'S') { // Drop/Reject rule
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

        // Apply Initial Orders 2 coordinate shift relocation resolver
        uint32_t resolved_raw = auncient_initial_orders_2_resolve(rule->raw_value, offset);
        rule->raw_value = resolved_raw;
        fw_decode(resolved_raw, &rule->inst);
    }

    printf("[FIREWALL RELOCATION SUCCESS] All rules relocated to new coordinates.\n");
    return true;
}
