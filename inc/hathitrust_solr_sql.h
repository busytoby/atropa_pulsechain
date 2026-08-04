#ifndef HATHITRUST_SOLR_SQL_H
#define HATHITRUST_SOLR_SQL_H

/*
 * Translates HathiTrust Solr search results into normalized SQL INSERT statement strings.
 * Parameters:
 *   htid: Volume ID.
 *   title: Title string.
 *   author: Author string.
 *   pub_date: Publication date.
 * Returns:
 *   Allocated string containing SQL insert command, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_solr_to_sql(
    const char *htid,
    const char *title,
    const char *author,
    const char *pub_date
);

#endif /* HATHITRUST_SOLR_SQL_H */
