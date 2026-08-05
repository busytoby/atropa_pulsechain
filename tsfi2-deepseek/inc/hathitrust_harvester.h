#ifndef HATHITRUST_HARVESTER_H
#define HATHITRUST_HARVESTER_H

#include <stdbool.h>

/*
 * Runs a single incremental harvest cycle using the saved state file.
 * Parameters:
 *   state_file: The path to the file storing the last sync timestamp.
 *   set: Optional OAI-PMH set filter, can be NULL.
 *   record_callback: Function called for each individual parsed XML record.
 * Returns:
 *   true if the harvest cycle completed and state was updated, false on error.
 */
bool hathitrust_run_incremental_harvest(const char *state_file,
                                        const char *set,
                                        void (*record_callback)(const char *xml_record));

#endif /* HATHITRUST_HARVESTER_H */
