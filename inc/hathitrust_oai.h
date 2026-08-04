#ifndef HATHITRUST_OAI_H
#define HATHITRUST_OAI_H

#include <stdbool.h>

/*
 * Queries the HathiTrust OAI-PMH provider.
 * Parameters:
 *   verb: The OAI verb (e.g. "Identify", "ListMetadataFormats", "ListRecords").
 *   metadata_prefix: The metadata prefix (e.g. "oai_dc" or "marc21"), can be NULL.
 *   set: Optional set filter (e.g. "hathitrust:pdus"), can be NULL.
 *   identifier: The item identifier for GetRecord, can be NULL.
 *   resumption_token: The pagination resumption token, can be NULL.
 *   from_date: Start date boundary (YYYY-MM-DD), can be NULL.
 *   until_date: End date boundary (YYYY-MM-DD), can be NULL.
 * Returns:
 *   An allocated XML string containing the response on success, or NULL on failure.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_oai_query(const char *verb,
                           const char *metadata_prefix,
                           const char *set,
                           const char *identifier,
                           const char *resumption_token,
                           const char *from_date,
                           const char *until_date);

#endif /* HATHITRUST_OAI_H */
