#ifndef HATHITRUST_RIB_TO_IIIF_H
#define HATHITRUST_RIB_TO_IIIF_H

/*
 * Parses a RenderMan RIB stream to compile back a IIIF Presentation Manifest.
 * Parameters:
 *   rib: The raw input RIB bytestream string.
 * Returns:
 *   Allocated string containing IIIF Manifest representation, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_rib_to_iiif(const char *rib);

#endif /* HATHITRUST_RIB_TO_IIIF_H */
