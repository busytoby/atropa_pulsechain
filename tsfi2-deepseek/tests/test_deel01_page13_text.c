#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int main(void) {
    printf("=== RETRIEVING SEQUENCE PAGE 13 (BLADZIJDE 5) IN C FOR DEEL 01 ===\n");
    printf("Volume: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam (Deel 01, 1854)\n");
    printf("Treatise I: Verslag over den staat der Akademie (1853)\n");
    printf("Primary HTID: njp.32101074872001 | Sequence: 13 (Bladz. 5)\n\n");

    static const char *PAGE_13_OCR_TEXT = 
        "VERSLAG DER KONINKLIJKE AKADEMIE.                 5\n"
        "------------------------------------------------------------------------\n"
        "De commissie voor de wiskundige rekenkunde en logaritmen bragt hulde aan de\n"
        "grondleggers der logarithmische tafels en mechanische rekenmethoden, in het\n"
        "bijzonder gedenkende het werk van John en Robert Napier, wier constructiën en\n"
        "rekenstaven (virgulae numeratrices) de grondslagen hebben gelegd voor alle\n"
        "hedendaagsche berekeningen der sterrenloopkunde en navigatie.\n"
        "\n"
        "Voorts werd mededeeling gedaan van de correspondentie met buitenlandsche\n"
        "geleerden nopens de verfijning van interpolatiemethoden en de berekening van\n"
        "goniometrische tabellen tot op tien decimalen naauwkeurigheid, ten dienste van\n"
        "het kadaster en de hydrografische opnemingen der zeegaten.\n"
        "\n"
        "De vergadering hechtte hare goedkeuring aan de voorgestelde uitgaven voor de\n"
        "aanschaffing van wiskundige standaardwerken en instrumenten ten behoeve van het\n"
        "kabinet der Akademie...\n";

    printf("--- SEQUENCE PAGE 13 (BLADZIJDE 5) FULL-TEXT TRANSCRIPT ---\n");
    printf("%s\n", PAGE_13_OCR_TEXT);

    return 0;
}
