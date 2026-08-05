#include "../tsfi2-deepseek/src/auncient_edsac_firewall.h"
#include "../tsfi2-deepseek/src/auncient_timeline_autodin.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// Simulated System ABI interface routed through EDSAC Firewall
typedef struct {
    uint32_t user_id;
    uint32_t parameter_value;
    char instruction_opcode;
} AuncientSystemABI;

bool auncient_abi_execute(const AuncientSystemABI *abi, const AuncientAnalyzer *analyzer) {
    // Convert ABI action to a virtual instruction stream for Initial Orders 1
    uint32_t instruction = ((uint32_t)abi->instruction_opcode & 0xFF) << 24;
    instruction |= (abi->parameter_value & 0x3FFFFF) << 2;

    return auncient_analyzer_classify(analyzer, &instruction, 1);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT EDSAC SYSTEM WIDE FIREWALL TEST SUITE\n");
    printf("=============================================================\n");

    // Initialize Analyzer to prohibit 'X' (Execution/Override) opcodes system-wide
    AuncientAnalyzer system_analyzer;
    auncient_analyzer_init(&system_analyzer, 1 << ('X' - 'A'));

    // Grant AUTODIN authorization for tests
    auncient_autodin_edsac_authorize(true);

    // 1. Test ABI Gating with Permissible Opcodes
    printf("[TEST] Routing permissible ABI parameter update...\n");
    AuncientSystemABI normal_abi = { .user_id = 100, .parameter_value = 80, .instruction_opcode = 'A' };
    bool normal_pass = auncient_abi_execute(&normal_abi, &system_analyzer);
    assert(normal_pass == true);
    printf("   ✓ Permissible ABI operation allowed.\n");

    // 2. Test ABI Gating with Impermissible Opcodes (expected reject)
    printf("[TEST] Routing prohibited ABI override instruction (expected reject)...\n");
    AuncientSystemABI bad_abi = { .user_id = 101, .parameter_value = 999, .instruction_opcode = 'X' };
    bool bad_fail = auncient_abi_execute(&bad_abi, &system_analyzer);
    assert(bad_fail == false);
    printf("   ✓ Prohibited ABI operation blocked.\n");

    // 3. Test WinchesterMQ Gating over EDSAC Firewall
    // Setup firewall with valid rules tape and PKI keys
    const char *rules_tape = "A 80 F\n";
    uint32_t pki_keys[4] = { 1, 2, 3, 4 };
    bool init_ok = auncient_firewall_init(rules_tape, 600, pki_keys, 4, &system_analyzer);
    assert(init_ok);

    AuncientPacket normal_pkt = {
        .source_ip = 0x01020304,
        .dest_port = 80,
        .payload_checksum = 0x1111,
        .key_count = 4
    };

    printf("[TEST] Evaluating packet over WinchesterMQ filter (expected ALLOW)...\n");
    assert(auncient_firewall_eval_packet(&normal_pkt) == true);

    AuncientPacket bad_pkt = {
        .source_ip = 0x01020304,
        .dest_port = 22,
        .payload_checksum = 0x2222,
        .key_count = 4
    };

    printf("[TEST] Evaluating packet with unmapped port over WinchesterMQ filter (expected DENY)...\n");
    assert(auncient_firewall_eval_packet(&bad_pkt) == false);

    printf("=============================================================\n");
    printf("ALL SYSTEM WIDE FIREWALL TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
