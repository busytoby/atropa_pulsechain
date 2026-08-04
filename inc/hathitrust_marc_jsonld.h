#ifndef HATHITRUST_MARC_JSONLD_H
#define HATHITRUST_MARC_JSONLD_H

#include "hathitrust_marc.h"

/*
 * Transcodes MARC bibliographic record struct to JSON-LD formatted string.
 * Parameters:
 *   bib: Pre-parsed MARC bibliographic metadata.
 * Returns:
 *   Allocated string containing JSON-LD representation, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_marc_to_jsonld(const HtrcMarcBibliographic *bib);

#endif /* HATHITRUST_MARC_JSONLD_H */
