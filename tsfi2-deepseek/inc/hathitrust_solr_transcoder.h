#ifndef HATHITRUST_SOLR_TRANSCODER_H
#define HATHITRUST_SOLR_TRANSCODER_H

/*
 * Transcodes a Solr search response record into a valid MARC-XML string template.
 * Parameters:
 *   htid: Volume Identifier.
 *   title: Title string.
 *   author: Author string.
 *   pub_date: Publication year string.
 * Returns:
 *   Allocated string containing MARC-XML structure, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_solr_to_marcxml(
    const char *htid,
    const char *title,
    const char *author,
    const char *pub_date
);

#endif /* HATHITRUST_SOLR_TRANSCODER_H */
