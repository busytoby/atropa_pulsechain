#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_oai.h"
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== HATHITRUST DIRECT C INGESTION AUDIT: DEEL 01 ===\n\n");

    const char *htid = "njp.32101074872001";

    // 1. Bibliographic Full Record
    printf("[1] Querying HathiTrust Bibliographic API for HTID: %s...\n", htid);
    char *full_bib = hathitrust_query_full("htid", htid);
    if (full_bib) {
        printf("    -> Status: SUCCESS (%zu bytes received)\n", strlen(full_bib));
        printf("    -> Snippet: %.250s...\n\n", full_bib);
        free(full_bib);
    } else {
        printf("    -> Status: FAILED (NULL response)\n\n");
    }

    // 2. OAI-PMH GetRecord
    printf("[2] Querying HathiTrust OAI-PMH GetRecord for oai:bdr:%s...\n", htid);
    char oai_id[128];
    snprintf(oai_id, sizeof(oai_id), "oai:bdr:%s", htid);
    char *oai_rec = hathitrust_oai_query("GetRecord", "oai_dc", NULL, oai_id, NULL, NULL, NULL);
    if (oai_rec) {
        printf("    -> Status: SUCCESS (%zu bytes received)\n", strlen(oai_rec));
        printf("    -> Snippet: %.250s...\n\n", oai_rec);
        free(oai_rec);
    } else {
        printf("    -> Status: FAILED (NULL response)\n\n");
    }

    return 0;
}
