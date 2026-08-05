#ifndef HATHITRUST_OSL_GILDING_H
#define HATHITRUST_OSL_GILDING_H

#include <stdbool.h>

/*
 * Generates an OSL shader representing gilded book edges or striped stacked paper block edges.
 * Parameters:
 *   gilded: Boolean flag, true for gold leaf gilding, false for stacked paper sheets.
 * Returns:
 *   Allocated string containing the OSL shader code, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_gilding_osl(bool gilded);

#endif /* HATHITRUST_OSL_GILDING_H */
