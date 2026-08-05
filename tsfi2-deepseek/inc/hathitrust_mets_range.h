#ifndef HATHITRUST_METS_RANGE_H
#define HATHITRUST_METS_RANGE_H

#include <stdbool.h>

/*
 * Maps a METS logical division range directly into a IIIF Presentation Manifest Range JSON block.
 * Parameters:
 *   label: The range title (e.g., "Chapter 1").
 *   start_page: First page canvas index.
 *   end_page: Last page canvas index.
 * Returns:
 *   Allocated JSON string representing the IIIF range, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_mets_map_range(const char *label, int start_page, int end_page);

#endif /* HATHITRUST_METS_RANGE_H */
