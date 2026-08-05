#ifndef HATHITRUST_ALTO_CONSOLIDATE_H
#define HATHITRUST_ALTO_CONSOLIDATE_H

#include "hathitrust_alto.h"

/*
 * Consolidates parsed ALTO tokens into a single cohesive string.
 * Reconstructs lines and handles hyphenation adjustments.
 * Parameters:
 *   page: The parsed HtrcAltoPage structures.
 * Returns:
 *   Allocated string containing consolidated page text, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_alto_consolidate_text(const HtrcAltoPage *page);

#endif /* HATHITRUST_ALTO_CONSOLIDATE_H */
