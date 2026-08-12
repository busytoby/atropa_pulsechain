#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_hathifile.h"
#include "tsfi2-deepseek/inc/tsfi_mainframe_computerworld.h"
#include "tsfi2-deepseek/inc/tsfi_quadtree_ksds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    
    // We use strsep to handle consecutive tab characters efficiently
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

bool hathifile_export_to_vsam(const char *hathifile_path, const char *vsam_path) {
    if (!hathifile_path || !vsam_path) return false;

    FILE *hf = fopen(hathifile_path, "r");
    if (!hf) return false;

    remove(vsam_path);
    tsfi_cw_vsam_ksds ksds;
    memset(&ksds, 0, sizeof(ksds));
    if (tsfi_cw_vsam_open(&ksds, vsam_path) != 0) {
        fclose(hf);
        return false;
    }

    bool start_collecting = false;
    char line[4096];
    while (fgets(line, sizeof(line), hf)) {
        char line_copy[4096];
        strcpy(line_copy, line);

        HathifileRow row;
        if (hathifile_parse_line(line, &row)) {
            if (!start_collecting) {
                if (row.bib_fmt && (strcmp(row.bib_fmt, "BK") == 0 || strcmp(row.bib_fmt, "BK ") == 0)) {
                    start_collecting = true;
                } else {
                    hathifile_free_row(&row);
                    continue;
                }
            }

            char key[256];
            snprintf(key, sizeof(key), "ht.%s", row.htid);

            size_t copy_len = strlen(line_copy);
            while (copy_len > 0 && (line_copy[copy_len - 1] == '\n' || line_copy[copy_len - 1] == '\r')) {
                line_copy[copy_len - 1] = '\0';
                copy_len--;
            }

            tsfi_cw_vsam_write(&ksds, key, (const uint8_t *)line_copy, copy_len);
            hathifile_free_row(&row);
        }
    }

    fclose(hf);
    return true;
}

uint64_t hash_string(const char *str) {
    uint64_t hash = 14695981039346656037ULL;
    if (!str) return hash;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 1099511628211ULL;
    }
    return hash;
}

bool hathifile_export_to_quadtree_ksds(
    const char *hathifile_path,
    const char *primary_path,
    const char *aix_isbn_path,
    const char *aix_oclc_path
) {
    if (!hathifile_path || !primary_path || !aix_isbn_path || !aix_oclc_path) return false;

    FILE *hf = fopen(hathifile_path, "r");
    if (!hf) return false;

    // We write primary book record texts to a flat file first so we can seek by offset
    FILE *pf = fopen(primary_path, "wb");
    if (!pf) {
        fclose(hf);
        return false;
    }

    // Write aligned spacing at the start
    uint8_t spacing[512] = {0};
    fwrite(spacing, 1, 512, pf);

    // Track up to 4 parsed rows to put into our 4 quadrants
    HathifileRow rows[4];
    uint32_t offsets[4] = {0};
    int row_count = 0;

    bool start_collecting = false;
    char line[4096];
    while (fgets(line, sizeof(line), hf) && row_count < 4) {
        char line_copy[4096];
        strcpy(line_copy, line);

        HathifileRow row;
        if (hathifile_parse_line(line, &row)) {
            if (!start_collecting) {
                if (row.bib_fmt && (strcmp(row.bib_fmt, "BK") == 0 || strcmp(row.bib_fmt, "BK ") == 0)) {
                    start_collecting = true;
                } else {
                    hathifile_free_row(&row);
                    continue;
                }
            }

            // Seek and record the offset of the raw line text
            offsets[row_count] = (uint32_t)ftell(pf);
            size_t copy_len = strlen(line_copy);
            fwrite(line_copy, 1, copy_len, pf);

            rows[row_count] = row;
            row_count++;
        }
    }
    fclose(pf);
    fclose(hf);

    if (row_count == 0) return false;

    // Define 5-node Quadtrees: 1 root, 4 quadrants
    InteropQuadNode primary_nodes[5] = {
        { 0, 0, 100, 100, 0, { 1, 2, 3, 4 } },
        { 0, 0, 50, 50, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 50, 0, 100, 50, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 0, 50, 50, 100, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 50, 50, 100, 100, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } }
    };

    InteropQuadNode isbn_nodes[5] = {
        { 0, 0, 100, 100, 0, { 1, 2, 3, 4 } },
        { 0, 0, 50, 50, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 50, 0, 100, 50, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 0, 50, 50, 100, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 50, 50, 100, 100, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } }
    };

    InteropQuadNode oclc_nodes[5] = {
        { 0, 0, 100, 100, 0, { 1, 2, 3, 4 } },
        { 0, 0, 50, 50, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 50, 0, 100, 50, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 0, 50, 50, 100, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },
        { 50, 50, 100, 100, 0xFFFFFFFF, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } }
    };

    // Distribute records to quadrants based on their key hashes
    for (int i = 0; i < row_count; i++) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        uint32_t val = offsets[i];

        // 1. Primary Index Mapping (Key: htid)
        uint64_t h_primary = hash_string(rows[i].htid);
        uint32_t x_p = (h_primary & 0xFFFFFFFF) % 100;
        uint32_t y_p = ((h_primary >> 32) & 0xFFFFFFFF) % 100;
        int q_p = 1;
        if (x_p > 50) q_p += 1;
        if (y_p > 50) q_p += 2;
        primary_nodes[q_p].value = val;

        // 2. Secondary ISBN Index Mapping (Key: isbn)
        if (rows[i].isbn && strlen(rows[i].isbn) > 0) {
            uint64_t h_isbn = hash_string(rows[i].isbn);
            uint32_t x_i = (h_isbn & 0xFFFFFFFF) % 100;
            uint32_t y_i = ((h_isbn >> 32) & 0xFFFFFFFF) % 100;
            int q_i = 1;
            if (x_i > 50) q_i += 1;
            if (y_i > 50) q_i += 2;
            isbn_nodes[q_i].value = val;
        }

        // 3. Secondary OCLC Index Mapping (Key: oclc)
        if (rows[i].oclc && strlen(rows[i].oclc) > 0) {
            uint64_t h_oclc = hash_string(rows[i].oclc);
            uint32_t x_o = (h_oclc & 0xFFFFFFFF) % 100;
            uint32_t y_o = ((h_oclc >> 32) & 0xFFFFFFFF) % 100;
            int q_o = 1;
            if (x_o > 50) q_o += 1;
            if (y_o > 50) q_o += 2;
            oclc_nodes[q_o].value = val;
        }

        hathifile_free_row(&rows[i]);

        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
        if (elapsed_ns > 1000.0) {
            fprintf(stderr, "[ANALYZER] Latency warning: HathiTrust record distribution took %.2f ns (> 1000 ns limit)\n", elapsed_ns);
        }
    }

    // Write primary index Quadtree to a separate file (or package it inside primary path)
    char primary_qt_path[512];
    snprintf(primary_qt_path, sizeof(primary_qt_path), "%s.qt.bin", primary_path);
    tsfi_qt_ksds_write(primary_qt_path, "AUNCIENT_PRIMARY\nQuadtreeCount:\t5\nRecordCount:\t1", primary_nodes, 5, (const uint8_t *)"OK", 2);

    // Write AIX files
    tsfi_qt_ksds_aix_write(aix_isbn_path, "AUNCIENT_ISBN_AIX\nQuadtreeCount:\t5\nRecordCount:\t1", isbn_nodes, 5, offsets, row_count);
    tsfi_qt_ksds_aix_write(aix_oclc_path, "AUNCIENT_OCLC_AIX\nQuadtreeCount:\t5\nRecordCount:\t1", oclc_nodes, 5, offsets, row_count);

    return true;
}
