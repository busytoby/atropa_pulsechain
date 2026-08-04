#ifndef HATHITRUST_CLIENT_H
#define HATHITRUST_CLIENT_H

#include <stdbool.h>

/*
 * Queries the HathiTrust Bibliographic API for a brief volume record.
 * Parameters:
 *   id_type: The type of identifier (e.g. "oclc", "isbn", "lccn", "htid").
 *   id_val: The value of the identifier.
 * Returns:
 *   An allocated JSON string containing the response on success, or NULL on failure.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_query_brief(const char *id_type, const char *id_val);

/*
 * Queries the HathiTrust Bibliographic API for a full volume record.
 */
char* hathitrust_query_full(const char *id_type, const char *id_val);

#endif /* HATHITRUST_CLIENT_H */
