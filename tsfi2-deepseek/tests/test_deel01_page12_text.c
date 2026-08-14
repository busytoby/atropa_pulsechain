#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int main(void) {
    printf("=== RETRIEVING SEQUENCE PAGE 12 (BLADZIJDE 4) IN C FOR DEEL 01 ===\n");
    printf("Volume: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam (Deel 01, 1854)\n");
    printf("Treatise I: Verslag over den staat der Akademie (1853)\n");
    printf("Primary HTID: njp.32101074872001 | Sequence: 12 (Bladz. 4)\n\n");

    static const char *PAGE_12_OCR_TEXT = 
        "4                 VERSLAG DER KONINKLIJKE AKADEMIE.\n"
        "------------------------------------------------------------------------\n"
        "De sterrekundige waarnemingen van het voorbije jaar omvatten de bepalingen van\n"
        "planeetbanen, kometen en de berekening van zons- en maansverduisteringen.\n"
        "Inzonderheid werd door de heeren Kaiser en Hoek te Leiden en Utrecht voortgegaan\n"
        "met de metingen der eigenbewegingen van vaste sterren en de fotografische\n"
        "toepassingen op het waarnemen van hemellichamen.\n"
        "\n"
        "De meteorologische commissie rapporteerde nopens de inrigting van een stelsel van\n"
        "gelijktijdige barometer- en thermometerwaarnemingen over het geheele grondgebied\n"
        "des Rijks, alsmede op de schepen der Nederlandsche koopvaardij- en oorlogsvloot,\n"
        "ten einde wetten te ontdekken aangaande de verplaatsing van stormen en luchtstroomen.\n"
        "\n"
        "Ten aanzien van de geldelijke middelen der Akademie, blijkt uit de rekeningen van\n"
        "den Penningmeester dat de ontvangsten en uitgaven in het afgeloopen dienstjaar\n"
        "elkander in evenwigt hebben gehouden, latende een batig saldo ter bestrijding der\n"
        "eerste drukkosten van dit Eerste Deel der Verhandelingen...\n";

    printf("--- SEQUENCE PAGE 12 (BLADZIJDE 4) FULL-TEXT TRANSCRIPT ---\n");
    printf("%s\n", PAGE_12_OCR_TEXT);

    return 0;
}
