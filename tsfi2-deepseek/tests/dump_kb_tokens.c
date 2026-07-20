#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_block_monitor.h"

int main(void) {
    FILE *f = fopen("tsfi2-deepseek/assets/contract_metadata.dat.bin", "rb");
    if (!f) {
        f = fopen("assets/contract_metadata.dat.bin", "rb");
    }
    if (!f) {
        printf("Error: contract_metadata.dat.bin not found.\n");
        return 1;
    }
    LauRdbmsTable *table = malloc(sizeof(LauRdbmsTable));
    if (!table) {
        fclose(f);
        return 1;
    }
    size_t bytes_read = fread(table, sizeof(LauRdbmsTable), 1, f);
    fclose(f);
    if (bytes_read == 0) {
        printf("Warning: zero bytes read.\n");
    }
    printf("Searching for FDIC and TLRz addresses in KB...\n");
    for (uint32_t i = 0; i < table->count; i++) {
        if (strcasestr(table->rows[i].address, "812571a") || strcasestr(table->rows[i].address, "c7145e1")) {
            printf("Found at index %u: Address: %s, Symbol: %s, Name: %s\n", i, table->rows[i].address, table->rows[i].symbol, table->rows[i].name);
        }
    }
    free(table);
    return 0;
}
