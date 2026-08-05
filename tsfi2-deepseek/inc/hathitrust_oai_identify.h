#ifndef HATHITRUST_OAI_IDENTIFY_H
#define HATHITRUST_OAI_IDENTIFY_H

#include <stdbool.h>

typedef struct {
    char *repository_name;
    char *admin_email;
    char *granularity;
    char *compression;
} HtrcOaiIdentity;

/*
 * Parses OAI-PMH Identify XML response.
 * Parameters:
 *   xml: Raw XML string response.
 *   ident: Struct to populate on success.
 * Returns:
 *   true on success, false on parsing error.
 *   Caller is responsible for freeing allocated strings inside ident.
 */
bool hathitrust_oai_parse_identify(const char *xml, HtrcOaiIdentity *ident);

/*
 * Frees memory allocated inside HtrcOaiIdentity.
 */
void hathitrust_oai_free_identity(HtrcOaiIdentity *ident);

#endif /* HATHITRUST_OAI_IDENTIFY_H */
