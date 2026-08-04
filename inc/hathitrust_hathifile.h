#ifndef HATHITRUST_HATHIFILE_H
#define HATHITRUST_HATHIFILE_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Structure representing a parsed row from a Hathifile (TSV format).
 */
typedef struct {
    char *htid;
    char *access;
    char *rights;
    char *ht_bib_key;
    char *description;
    char *source;
    char *source_inst_status;
    char *oclc;
    char *isbn;
    char *issn;
    char *lccn;
    char *title;
    char *imprint;
    char *rights_reason_code;
    char *rights_timestamp;
    char *us_gov_doc_flag;
    char *rights_date_used;
    char *pub_place;
    char *lang;
    char *bib_fmt;
} HathifileRow;

/*
 * Parses a single tab-separated line from a Hathifile.
 * Parameters:
 *   line: The raw input line string (will be modified during tokenization).
 *   row: Pointer to the HathifileRow struct to populate.
 * Returns:
 *   true if the row was successfully parsed, false otherwise.
 * Note: The caller must call hathifile_free_row to clean up strings.
 */
bool hathifile_parse_line(char *line, HathifileRow *row);

/*
 * Frees all allocated memory within a HathifileRow.
 */
void hathifile_free_row(HathifileRow *row);

/*
 * Exports Hathifile records to a VSAM KSDS database.
 */
bool hathifile_export_to_vsam(const char *hathifile_path, const char *vsam_path);

#endif /* HATHITRUST_HATHIFILE_H */
