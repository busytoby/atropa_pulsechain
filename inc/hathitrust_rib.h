#ifndef HATHITRUST_RIB_H
#define HATHITRUST_RIB_H

#include "hathitrust_mets.h"

/*
 * Compiles a METS structural definition into a RenderMan RIB scene stream.
 * Parameters:
 *   mets: Pre-parsed METS structure.
 * Returns:
 *   Allocated string containing the RIB scene, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_mets_to_rib(const HtrcMetsStructure *mets);

#endif /* HATHITRUST_RIB_H */
