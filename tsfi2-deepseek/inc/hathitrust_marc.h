#ifndef HATHITRUST_MARC_H
#define HATHITRUST_MARC_H

#include <stdbool.h>

typedef struct {
    char *title;
    char *author;
    char *pub_date;
    char *isbn;
    char *oclc;
} HtrcMarcBibliographic;

/*
 * Transcodes MARC-XML record stream to unified Dublin Core style attributes.
 * Parameters:
 *   xml: Raw MARC-XML data.
 *   bib: Pointer to HtrcMarcBibliographic struct to populate.
 * Returns:
 *   true on success, false on parsing failure.
 */
bool hathitrust_marc_transcode(const char *xml, HtrcMarcBibliographic *bib);

/*
 * Frees all allocated memory in HtrcMarcBibliographic.
 */
void hathitrust_marc_free(HtrcMarcBibliographic *bib);

#endif /* HATHITRUST_MARC_H */
