#ifndef HATHITRUST_ALTO_H
#define HATHITRUST_ALTO_H

#include <stdbool.h>

typedef struct {
    char *content;
    int hpos;
    int vpos;
    int width;
    int height;
} HtrcAltoToken;

typedef struct {
    int token_count;
    HtrcAltoToken *tokens;
    int token_capacity;
} HtrcAltoPage;

/*
 * Parses ALTO XML string to extract all word-level layout tokens and coordinates.
 * Parameters:
 *   xml: The raw ALTO XML layout string.
 *   page: Pointer to HtrcAltoPage to populate.
 * Returns:
 *   true on success, false on parsing failure.
 */
bool hathitrust_alto_parse(const char *xml, HtrcAltoPage *page);

/*
 * Frees all allocated memory in HtrcAltoPage.
 */
void hathitrust_alto_free(HtrcAltoPage *page);

#endif /* HATHITRUST_ALTO_H */
