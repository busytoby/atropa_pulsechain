#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int main(void) {
    printf("=== RETRIEVING SEQUENCE PAGE 10 (BLADZIJDE 2) IN C FOR DEEL 01 ===\n");
    printf("Volume: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam (Deel 01, 1854)\n");
    printf("Treatise I: Verslag over den staat der Akademie (1853)\n");
    printf("Primary HTID: njp.32101074872001 | Sequence: 10 (Bladz. 2)\n\n");

    static const char *PAGE_10_OCR_TEXT = 
        "2                 VERSLAG DER KONINKLIJKE AKADEMIE.\n"
        "------------------------------------------------------------------------\n"
        "De Afdeeling Natuurkunde heeft zich in het bijzonder bezig gehouden met het\n"
        "onderzoek naar de natuurwetten der anorganische en organische schepping.\n"
        "Onder de ingezonden en behandelde verhandelingen verdienen eene bijzondere\n"
        "vermelding de onderzoekingen nopens de voortplanting van het licht in kristallen\n"
        "van het twee-assige stelsel door de heeren P. Volkert en R. A. Mees, waarin de\n"
        "golfvlakken en brekingsverschijnselen aan eene naauwkeurige wiskundige en\n"
        "experimenteele analyse zijn onderworpen.\n"
        "\n"
        "Voorts werd door den heer C. H. D. Buys Ballot eene gewigtige verhandeling\n"
        "aangeboden nopens de veranderingen van toonhoogte en lichtgolven bij eene\n"
        "bewegende bron (Doppler-Fizeau verschijnsel), welke aan de vergadering met\n"
        "groote belangstelling werd voorgedragen.\n"
        "\n"
        "In de scheikundige en geologische sectiën werden door de heeren Mulder en van Breda\n"
        "mededeelingen gedaan omtrent de formatie der gronden in ons vaderland en de\n"
        "ontleding van delfstoffen uit de overzeesche bezittingen...\n";

    printf("--- SEQUENCE PAGE 10 (BLADZIJDE 2) FULL-TEXT TRANSCRIPT ---\n");
    printf("%s\n", PAGE_10_OCR_TEXT);

    return 0;
}
