#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                      // "ANKH"
    uint16_t load_address;                 // 0x0100
    uint16_t entry_point;                  // 0x0100
    uint32_t opcode_signature;             // "SYNQ" (0x53594E51)
    uint16_t pre_synaptic_neuron_id;       // Pre-synaptic neuron node ID
    uint16_t post_synaptic_neuron_id;      // Post-synaptic neuron node ID
    uint16_t proof_graph_edge_morphism;    // Theorem 1: Synapse as strict Morphism Hom(N_i, N_j)
    uint16_t proof_neural_net_containment; // Theorem 2: Neural Net Graph Closure (No isolated existence)
    uint16_t proof_skyrmion_as_weight;     // Theorem 3: Skyrmionic Potential strictly as Synaptic Weight Conductance
    char closure_id[64];                   // "NEURAL-SYNAPSE-STRICT-CLOSURE-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmSynapseClosureBinary;

static uint32_t compute_rule18_checksum(const uint8_t *data, size_t len) {
    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}

int main(int argc, char **argv) {
    const char *out_path = (argc > 1) ? argv[1] : "synapse_closure.bin";

    CpmSynapseClosureBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x53594E51; // "SYNQ"

    bin.pre_synaptic_neuron_id = 1;
    bin.post_synaptic_neuron_id = 2;

    bin.proof_graph_edge_morphism = 1;
    bin.proof_neural_net_containment = 1;
    bin.proof_skyrmion_as_weight = 1;

    snprintf(bin.closure_id, sizeof(bin.closure_id),
             "NEURAL-SYNAPSE-STRICT-CLOSURE-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Synapse Closure Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
