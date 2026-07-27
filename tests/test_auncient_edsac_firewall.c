#include "../src/auncient_edsac_firewall.h"
#include "../src/auncient_timeline_autodin.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT EDSAC-AUTODIN COMPILER FIREWALL TEST SUITE\n");
    printf("=============================================================\n");

    const char *firewall_rules_tape = 
        "A 80 F\n"    // Allow HTTP (port 80) - short word modifier
        "A 443 D\n"   // Allow HTTPS (port 443) - long/relocatable modifier
        "S 22 F\n";   // Drop SSH (port 22) - short word modifier

    uint32_t valid_pki[4] = { 0x1111, 0x2222, 0x3333, 0x4444 };
    uint32_t invalid_pki[3] = { 0x1111, 0x2222, 0x3333 };

    // 1. Verify default-reject initialization without authorization
    printf("[TEST] Booting firewall without AUTODIN authorization (expected block)...\n");
    bool boot_fail = auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, NULL);
    assert(boot_fail == false);
    printf("   ✓ Blocked unauthorized boot successful.\n");

    // Grant AUTODIN authorization
    auncient_autodin_edsac_authorize(true);

    // 2. Verify boot rejection on insufficient PKI keys (< 4 keys)
    printf("[TEST] Booting firewall with insufficient PKI keys (expected block)...\n");
    bool boot_fail_pki = auncient_firewall_init(firewall_rules_tape, 500, invalid_pki, 3, NULL);
    assert(boot_fail_pki == false);
    printf("   ✓ Blocked insufficient PKI boot successful.\n");

    // 3. Test analyzer classification at Initial Orders 1
    // Setup analyzer that prohibits 'S' (Drop/Reject) opcodes
    AuncientAnalyzer analyzer;
    auncient_analyzer_init(&analyzer, 1 << ('S' - 'A'));

    printf("[TEST] Booting firewall with prohibited opcode in tape (expected analyzer block)...\n");
    bool boot_fail_analyzer = auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, &analyzer);
    assert(boot_fail_analyzer == false);
    printf("   ✓ Analyzer classification block verified.\n");

    // 4. Boot successfully with valid credentials and no prohibited opcodes (using NULL or empty analyzer)
    printf("[TEST] Booting firewall with valid credentials (expected pass)...\n");
    bool boot_pass = auncient_firewall_init(firewall_rules_tape, 500, valid_pki, 4, NULL);
    assert(boot_pass == true);
    printf("   ✓ Firewall successfully booted.\n");

    // 5. Evaluate packets
    AuncientPacket p1 = { .source_ip = 0x0A000001, .dest_port = 80, .payload_checksum = 0xAA55, .key_count = 4 };
    AuncientPacket p2 = { .source_ip = 0x0A000001, .dest_port = 22, .payload_checksum = 0xBB66, .key_count = 4 };
    AuncientPacket p3 = { .source_ip = 0x0A000001, .dest_port = 8080, .payload_checksum = 0xCC77, .key_count = 4 }; 
    AuncientPacket p4 = { .source_ip = 0x0A000001, .dest_port = 443, .payload_checksum = 0xDD88, .key_count = 4 };

    printf("[TEST] Evaluating HTTP packet (expected ALLOW)...\n");
    assert(auncient_firewall_eval_packet(&p1) == true);

    printf("[TEST] Evaluating SSH packet (expected DENY)...\n");
    assert(auncient_firewall_eval_packet(&p2) == false);

    printf("[TEST] Evaluating untracked HTTP-ALT packet (expected default DENY)...\n");
    assert(auncient_firewall_eval_packet(&p3) == false);

    printf("[TEST] Evaluating HTTPS packet (expected ALLOW)...\n");
    assert(auncient_firewall_eval_packet(&p4) == true);
    printf("   ✓ Packet evaluation controls verified.\n");

    // 6. Relocate rules using Initial Orders 2 coordinate shifts
    printf("[TEST] Testing Initial Orders 2 coordinate shift rule relocation...\n");
    auncient_firewall_relocate_rules(1000);

    // Verify HTTPS allowed target relocated to 1443
    AuncientPacket p4_old = { .source_ip = 0x0A000001, .dest_port = 443, .payload_checksum = 0xDD88, .key_count = 4 };
    AuncientPacket p4_new = { .source_ip = 0x0A000001, .dest_port = 1443, .payload_checksum = 0xDD88, .key_count = 4 };

    printf("[TEST] Evaluating old HTTPS port after relocation (expected default DENY)...\n");
    assert(auncient_firewall_eval_packet(&p4_old) == false);

    printf("[TEST] Evaluating relocated HTTPS port (expected ALLOW)...\n");
    assert(auncient_firewall_eval_packet(&p4_new) == true);

    printf("[TEST] Evaluating HTTP port after relocation (expected ALLOW as 'F' modifier does not shift)...\n");
    assert(auncient_firewall_eval_packet(&p1) == true);
    printf("   ✓ Relocatable rule shifting verified.\n");

    // 7. Test evaluation block upon revoking authorization
    auncient_autodin_edsac_authorize(false);
    printf("[TEST] Evaluating HTTP packet after revoking authorization (expected DENY due to audit block)...\n");
    assert(auncient_firewall_eval_packet(&p1) == false);
    printf("   ✓ Revocation auditing block verified.\n");

    printf("=============================================================\n");
    printf("ALL EDSAC-AUTODIN COMPILER FIREWALL TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
