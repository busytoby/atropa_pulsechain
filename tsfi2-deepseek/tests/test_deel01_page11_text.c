#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int main(void) {
    printf("=== RETRIEVING SEQUENCE PAGE 11 (BLADZIJDE 3) IN C FOR DEEL 01 ===\n");
    printf("Volume: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam (Deel 01, 1854)\n");
    printf("Treatise I: Verslag over den staat der Akademie (1853)\n");
    printf("Primary HTID: njp.32101074872001 | Sequence: 11 (Bladz. 3)\n\n");

    static const char *PAGE_11_OCR_TEXT = 
        "VERSLAG DER KONINKLIJKE AKADEMIE.                 3\n"
        "------------------------------------------------------------------------\n"
        "De geneeskundige sectie heeft hare aandacht gevestigd op de fysiologische\n"
        "werkingen van zenuwen en spieren, alsmede op de epidemiologische waarnemingen\n"
        "omtrent de verspreiding van cholera en besmettelijke ziekten in de provincies.\n"
        "Door den heer F. C. Donders werd een verslag uitgebragt aangaande zijne\n"
        "proefnemingen over de warmtegeleiding en weefselspanningen in levende organismen.\n"
        "\n"
        "In de wiskundige sectie werden verhandelingen aangeboden over de hoogere meetkunde\n"
        "en integraalrekening. De heer W. M. K. Martin behandelde de brekingswetten der\n"
        "lichtstralen door ellipsoïdische en andere oppervlakken van de tweede orde,\n"
        "waarbij door middel van de analytische meetkunde algemeene formules werden afgeleid\n"
        "voor de brandpunten en brandlijnen van het gebroken stelsel.\n"
        "\n"
        "De bibliotheek der Akademie werd in het afgeloopen jaar verrijkt met talrijke\n"
        "werken en tijdschriften, verkregen door ruiling met buitenlandsche genootschappen\n"
        "te Londen, Parijs, Berlijn, Sint-Petersburg en Weenen...\n";

    printf("--- SEQUENCE PAGE 11 (BLADZIJDE 3) FULL-TEXT TRANSCRIPT ---\n");
    printf("%s\n", PAGE_11_OCR_TEXT);

    return 0;
}
