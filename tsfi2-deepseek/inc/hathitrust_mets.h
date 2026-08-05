#ifndef HATHITRUST_METS_H
#define HATHITRUST_METS_H

#include <stdbool.h>

typedef struct {
    int seq;
    char *label;
    char *file_id;
} HtrcMetsPage;

typedef struct {
    char *volume_id;
    int page_count;
    HtrcMetsPage *pages;
    int page_capacity;
} HtrcMetsStructure;

/*
 * Parses HathiTrust METS XML payload to extract structural mapping.
 * Parameters:
 *   xml: The raw METS XML string.
 *   mets: Pointer to HtrcMetsStructure to populate.
 * Returns:
 *   true on success, false on parser error.
 */
bool hathitrust_mets_parse(const char *xml, HtrcMetsStructure *mets);

/*
 * Frees all allocated memory in HtrcMetsStructure.
 */
void hathitrust_mets_free(HtrcMetsStructure *mets);

#endif /* HATHITRUST_METS_H */
