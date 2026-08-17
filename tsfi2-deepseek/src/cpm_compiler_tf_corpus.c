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
    uint32_t opcode_signature;             // "TFCG" (0x54464347)
    uint16_t strat_one_device;             // Strat 1: tf.distribute.OneDeviceStrategy
    uint16_t strat_mirrored;               // Strat 2: tf.distribute.MirroredStrategy
    uint16_t strat_multi_worker_mirrored;  // Strat 3: tf.distribute.MultiWorkerMirroredStrategy
    uint16_t strat_parameter_server;       // Strat 4: tf.distribute.ParameterServerStrategy
    uint16_t strat_tpu_strategy;           // Strat 5: tf.distribute.TPUStrategy
    uint16_t strat_central_storage;        // Strat 6: tf.distribute.experimental.CentralStorageStrategy
    uint16_t strat_collective_all_reduce;  // Strat 7: tf.distribute.experimental.CollectiveAllReduceStrategy
    uint16_t strat_dtensor_mesh_sharding;  // Strat 8: tf.experimental.dtensor DTensor Spatial Sharding
    char corpus_title[64];                 // "TF-ALL-DISTRIBUTED-STRATEGIES-ZORSE-CORPUS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmTFCorpusBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "tf_corpus_proving.bin";

    CpmTFCorpusBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x54464347; // "TFCG"

    bin.strat_one_device = 1;
    bin.strat_mirrored = 1;
    bin.strat_multi_worker_mirrored = 1;
    bin.strat_parameter_server = 1;
    bin.strat_tpu_strategy = 1;
    bin.strat_central_storage = 1;
    bin.strat_collective_all_reduce = 1;
    bin.strat_dtensor_mesh_sharding = 1;

    snprintf(bin.corpus_title, sizeof(bin.corpus_title),
             "TF-ALL-DISTRIBUTED-STRATEGIES-ZORSE-CORPUS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized TensorFlow Total Strategy Corpus Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
