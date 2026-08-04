#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_hathifile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* safe_strdup(const char *s) {
    if (!s) return NULL;
    return strdup(s);
}

bool hathifile_parse_line(char *line, HathifileRow *row) {
    if (!line || !row) return false;
    
    memset(row, 0, sizeof(HathifileRow));
    
    // Remove trailing newline character
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
    
    char *curr = line;
    char *token;
    int index = 0;
    
    // We use strsep to handle consecutive tab characters correctly
    while ((token = strsep(&curr, "\t")) != NULL) {
        switch (index) {
            case 0:  row->htid = safe_strdup(token); break;
            case 1:  row->access = safe_strdup(token); break;
            case 2:  row->rights = safe_strdup(token); break;
            case 3:  row->ht_bib_key = safe_strdup(token); break;
            case 4:  row->description = safe_strdup(token); break;
            case 5:  row->source = safe_strdup(token); break;
            case 6:  row->source_inst_status = safe_strdup(token); break;
            case 7:  row->oclc = safe_strdup(token); break;
            case 8:  row->isbn = safe_strdup(token); break;
            case 9:  row->issn = safe_strdup(token); break;
            case 10: row->lccn = safe_strdup(token); break;
            case 11: row->title = safe_strdup(token); break;
            case 12: row->imprint = safe_strdup(token); break;
            case 13: row->rights_reason_code = safe_strdup(token); break;
            case 14: row->rights_timestamp = safe_strdup(token); break;
            case 15: row->us_gov_doc_flag = safe_strdup(token); break;
            case 16: row->rights_date_used = safe_strdup(token); break;
            case 17: row->pub_place = safe_strdup(token); break;
            case 18: row->lang = safe_strdup(token); break;
            case 19: row->bib_fmt = safe_strdup(token); break;
            default:
                break;
        }
        index++;
    }
    
    // A valid Hathifile line must have at least the basic metadata fields (e.g. 20 fields)
    if (index < 20) {
        hathifile_free_row(row);
        return false;
    }
    
    return true;
}

void hathifile_free_row(HathifileRow *row) {
    if (!row) return;
    free(row->htid);
    free(row->access);
    free(row->rights);
    free(row->ht_bib_key);
    free(row->description);
    free(row->source);
    free(row->source_inst_status);
    free(row->oclc);
    free(row->isbn);
    free(row->issn);
    free(row->lccn);
    free(row->title);
    free(row->imprint);
    free(row->rights_reason_code);
    free(row->rights_timestamp);
    free(row->us_gov_doc_flag);
    free(row->rights_date_used);
    free(row->pub_place);
    free(row->lang);
    free(row->bib_fmt);
    memset(row, 0, sizeof(HathifileRow));
}
