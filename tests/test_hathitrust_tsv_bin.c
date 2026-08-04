#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_tsv_bin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_invalid_extension(void) {
    printf("[Test] Running file extension validation checks...\n");
    uint8_t payload[4] = { 1, 2, 3, 4 };
    
    // Should fail because extension is not .dat.bin
    bool ok = hathitrust_tsv_bin_serialize("/tmp/test_bad.json", "Col1\tCol2\nVal1\tVal2", payload, 4);
    assert(ok == false);
    
    printf("[Test] File extension validation checks passed.\n");
}

static void test_serialize_deserialize(void) {
    printf("[Test] Running TSV binary block serialization & parsing checks...\n");
    
    const char *filepath = "/tmp/test_volume_block.dat.bin";
    const char *tsv = "HTID\tRights\tPubDate\nuc1.123\tpd\t1920\n";
    uint8_t payload[16] = { 0xAA, 0xBB, 0xCC, 0xDD, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    
    bool ok = hathitrust_tsv_bin_serialize(filepath, tsv, payload, 16);
    assert(ok == true);
    
    HtrcTsvBinBlock block;
    ok = hathitrust_tsv_bin_deserialize(filepath, &block);
    assert(ok == true);
    
    assert(strcmp(block.tsv_header, tsv) == 0);
    assert(block.binary_len == 16);
    assert(memcmp(block.binary_payload, payload, 16) == 0);
    
    printf("[Test] Generated Block Checksum: %lu\n", block.checksum);
    hathitrust_tsv_bin_free_block(&block);
    
    remove(filepath);
    printf("[Test] TSV binary block serialization & parsing checks passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust TSV Binary media storage tests...\n");
    test_invalid_extension();
    test_serialize_deserialize();
    printf("[Test] All TSV Binary media storage tests completed successfully.\n");
    return 0;
}
