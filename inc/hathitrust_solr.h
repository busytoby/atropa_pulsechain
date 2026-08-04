#ifndef HATHITRUST_SOLR_H
#define HATHITRUST_SOLR_H

/*
 * Performs a Solr brief search query against HathiTrust Catalog API.
 * Parameters:
 *   field: The search field to target (e.g. "title", "author", or "all").
 *   query: The raw query term.
 * Returns:
 *   Allocated string containing the JSON response, or NULL on network/API failure.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_solr_search(const char *field, const char *query);

#endif /* HATHITRUST_SOLR_H */
