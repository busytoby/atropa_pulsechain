#ifndef HATHITRUST_TSV_BIN_H
#define HATHITRUST_TSV_BIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    char *tsv_header;
    size_t header_len;
    uint8_t *binary_payload;
    size_t binary_len;
    uint64_t checksum;
} HtrcTsvBinBlock;

/*
 * Serializes a TSV header and binary payload into a aligned .dat.bin structure.
 * Parameters:
 *   filepath: Destination path ending in .dat.bin.
 *   tsv_header: Raw TSV metadata lines.
 *   payload: Raw binary payload array.
 *   payload_len: Length of the raw payload.
 * Returns:
 *   true on success, false on serialization error.
 */
bool hathitrust_tsv_bin_serialize(
    const char *filepath,
    const char *tsv_header,
    const uint8_t *payload,
    size_t payload_len
);

/*
 * Parses a aligned .dat.bin file containing a TSV header and binary payload.
 * Parameters:
 *   filepath: Path to the .dat.bin file.
 *   out_block: Block structure to populate.
 * Returns:
 *   true on success, false on format or validation failure.
 */
bool hathitrust_tsv_bin_deserialize(const char *filepath, HtrcTsvBinBlock *out_block);

/*
 * Frees memory allocated inside HtrcTsvBinBlock.
 */
void hathitrust_tsv_bin_free_block(HtrcTsvBinBlock *block);

#endif /* HATHITRUST_TSV_BIN_H */
