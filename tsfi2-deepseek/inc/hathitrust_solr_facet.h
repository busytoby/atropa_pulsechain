#ifndef HATHITRUST_SOLR_FACET_H
#define HATHITRUST_SOLR_FACET_H

/*
 * Compiles a Solr faceted search query URL.
 * Parameters:
 *   query: The raw query term (e.g. "carolingien").
 *   facet_field: The field to count facets for (e.g. "publisher").
 * Returns:
 *   Allocated query URL, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_solr_compile_facet_query(const char *query, const char *facet_field);

#endif /* HATHITRUST_SOLR_FACET_H */
