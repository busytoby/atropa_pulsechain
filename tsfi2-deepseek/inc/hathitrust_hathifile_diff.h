#ifndef HATHITRUST_HATHIFILE_DIFF_H
#define HATHITRUST_HATHIFILE_DIFF_H

#include <stdbool.h>

/*
 * Compares two versions of the Hathifiles TSV datasets to find modifications.
 * Parameters:
 *   old_hathifile: Path to the old TSV.
 *   new_hathifile: Path to the new TSV.
 *   diff_output: Filepath to write the diff results to.
 * Returns:
 *   true on success, false on read/write failure.
 */
bool hathitrust_hathifile_diff(
    const char *old_hathifile,
    const char *new_hathifile,
    const char *diff_output
);

#endif /* HATHITRUST_HATHIFILE_DIFF_H */
