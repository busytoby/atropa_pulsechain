#ifndef HATHITRUST_TSV_SCANNER_H
#define HATHITRUST_TSV_SCANNER_H

#include <stdbool.h>

/*
 * Scans a Hathifile TSV row, verifying column count and integrity.
 * Parameters:
 *   tsv_line: The raw TSV input line.
 * Returns:
 *   true if row format is valid, false if malformed.
 */
bool hathitrust_tsv_verify_integrity(const char *tsv_line);

#endif /* HATHITRUST_TSV_SCANNER_H */
