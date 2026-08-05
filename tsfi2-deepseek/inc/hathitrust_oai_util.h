#ifndef HATHITRUST_OAI_UTIL_H
#define HATHITRUST_OAI_UTIL_H

#include <stdbool.h>

/*
 * Extracts the contents of a specific XML tag from an XML string.
 * Supports standard tags (e.g., "dc:title", "resumptionToken", etc.).
 * Parameters:
 *   xml: The raw XML source string.
 *   tag: The name of the XML tag to extract (without brackets).
 * Returns:
 *   An allocated string containing the tag contents, or NULL if not found.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_oai_extract_tag(const char *xml, const char *tag);

/*
 * Automatically harvests multiple pages of records using resumption tokens.
 * Callback function 'record_callback' is invoked with each page response payload.
 * Returns:
 *   true on successful completion, false on network or parsing failure.
 */
bool hathitrust_oai_harvest_all(const char *set, void (*page_callback)(const char *xml_page));

/*
 * Saves the latest datestamp state to a local file.
 */
bool hathitrust_oai_save_state(const char *filepath, const char *datestamp);

/*
 * Loads the latest saved datestamp state from a local file.
 * Returns:
 *   An allocated datestamp string, or NULL if not found/error.
 */
char* hathitrust_oai_load_state(const char *filepath);

#endif /* HATHITRUST_OAI_UTIL_H */
