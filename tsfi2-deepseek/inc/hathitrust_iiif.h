#ifndef HATHITRUST_IIIF_H
#define HATHITRUST_IIIF_H

#include "hathitrust_mets.h"

/*
 * Transcodes parsed METS structural metadata to a IIIF Presentation Manifest JSON string.
 * Parameters:
 *   mets: Pre-parsed METS structure.
 *   title: Title of the volume.
 * Returns:
 *   Allocated string containing IIIF Manifest representation, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_mets_to_iiif(const HtrcMetsStructure *mets, const char *title);

#endif /* HATHITRUST_IIIF_H */
