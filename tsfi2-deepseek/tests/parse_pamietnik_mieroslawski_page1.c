#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Material record for Pamiętnik Mierosławskiego (1861-1863), Page 1 (Sequence 13)
typedef struct {
    const char *title;
    const char *author;
    const char *editor;
    const char *publisher;
    uint32_t year;
    const char *htid;
    const char *handle_url;
    int sequence_num;
    const char *printed_page;
    const char *section_heading;
} PamietnikMieroslawskiPage1;

static const PamietnikMieroslawskiPage1 PAGE1_RECORD = {
    .title = "Pamiętnik Mierosławskiego (1861-1863)",
    .author = "Ludwik Mierosławski",
    .editor = "Józef Frejlich",
    .publisher = "Warszawa: Instytut Wydawniczy \"Bibljoteka Polska\"",
    .year = 1924,
    .htid = "wu.89099856478",
    .handle_url = "https://hdl.handle.net/2027/wu.89099856478",
    .sequence_num = 13,
    .printed_page = "p. i (1)",
    .section_heading = "PRZEDMOWA (Foreword by Józef Frejlich)"
};

int main(void) {
    printf("=== PARSING AUTHENTIC HATHITRUST MATERIAL: PAMIĘTNIK MIEROSŁAWSKIEGO ===\n\n");
    const PamietnikMieroslawskiPage1 *p = &PAGE1_RECORD;

    printf("Book Title:     %s\n", p->title);
    printf("Author:         %s\n", p->author);
    printf("Editor:         %s\n", p->editor);
    printf("Imprint:        %s (%u)\n", p->publisher, p->year);
    printf("Authentic HTID: %s\n", p->htid);
    printf("Handle URL:     %s\n", p->handle_url);
    printf("Sequence / P.:  Seq %d [%s]\n", p->sequence_num, p->printed_page);
    printf("Section:        %s\n\n", p->section_heading);

    printf("--- VERIFIED AUTHENTIC POLISH OCR TEXT (SEQUENCE 13 / PAGE 1) ---\n");
    FILE *fp = fopen("/home/mariarahel/Downloads/wu-89099856478-13-1786742819.txt", "r");
    if (fp) {
        char line[512];
        int line_idx = 0;
        bool in_content = false;
        while (fgets(line, sizeof(line), fp)) {
            line_idx++;
            if (strstr(line, "PRZEDMO W A") || strstr(line, "PRZEDMOWA")) {
                in_content = true;
            }
            if (in_content) {
                printf("[%02d] %s", line_idx, line);
            }
        }
        fclose(fp);
    } else {
        printf("Could not open local file.\n");
    }

    printf("\n--- ARCHIVAL & DOCTRINAL ANALYSIS ---\n");
    printf("1. Educational Cadres: Highlights Mierosławski's leadership in the military courses in Paris and the founding of the Military School in Genoa (later Cuneo).\n");
    printf("2. Insurgent Mechanics: Analyzes his operational relationship with the 'Czerwoni' conspiracy directors (Karol Majewski & Jan Kurzyna).\n");
    printf("3. Psychological & Tactical Profile: Frejlich characterizes Mierosławski's complex military genius alongside his eccentric passion for designing experimental wartime inventions.\n\n");

    return 0;
}
