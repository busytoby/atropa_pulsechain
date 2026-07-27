#ifndef AUNCIENT_EDSAC_FIREWALL_H
#define AUNCIENT_EDSAC_FIREWALL_H

#include <stdint.h>
#include <stdbool.h>

// Represents an active packet traversing the network firewall
typedef struct {
    uint32_t source_ip;
    uint32_t dest_port;
    uint32_t payload_checksum;
    uint32_t pki_keys[4];
    int key_count;
} AuncientPacket;

// The Analyzer layout for classifying permissible and impermissible operations at Initial Orders 1
typedef struct {
    uint32_t prohibited_opcodes; // Bitmask of prohibited ASCII opcodes
} AuncientAnalyzer;

// Initializes an analyzer instance
void auncient_analyzer_init(AuncientAnalyzer *analyzer, uint32_t prohibited_opcodes);

// Evaluates compiled instruction tape against classification rules. 
// Returns true if permissible, false if impermissible (default-reject).
bool auncient_analyzer_classify(const AuncientAnalyzer *analyzer, const uint32_t *instructions, int count);

// Initializes the firewall environment and loads rules from the paper tape input using an analyzer at Initial Orders 1
bool auncient_firewall_init(const char *rules_tape, uint32_t base_addr, const uint32_t *pki_keys, int key_count, const AuncientAnalyzer *analyzer);

// Evaluates an incoming packet against the compiled rules inside the EDSAC memory lines
bool auncient_firewall_eval_packet(const AuncientPacket *packet);

// Relocates the active firewall rules to a new port offset dynamically using Initial Orders 2 coordinate shifts
bool auncient_firewall_relocate_rules(uint32_t offset);

#endif // AUNCIENT_EDSAC_FIREWALL_H
