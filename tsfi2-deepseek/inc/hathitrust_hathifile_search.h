#ifndef HATHITRUST_HATHIFILE_SEARCH_H
#define HATHITRUST_HATHIFILE_SEARCH_H

#include "hathitrust_hathifile.h"

/*
 * Performs a binary search directly on a sorted Hathifiles TSV database file.
 * Parameters:
 *   filepath: The absolute path to the TSV file.
 *   htid: The target volume ID (HTID) key.
 *   record: Pointer to HathifileRow to populate on success.
 * Returns:
 *   true if the record was found and successfully parsed, false otherwise.
 */
bool hathitrust_hathifile_binary_search(
    const char *filepath,
    const char *htid,
    HathifileRow *record
);

#endif /* HATHITRUST_HATHIFILE_SEARCH_H */
