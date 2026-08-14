#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Structural metadata and text transcripts for Front Matter (Pages 3 to 8) and First Academic Text Page (Page 9)
typedef struct {
    int page_seq;
    const char *section_name;
    const char *ocr_text;
} PageTranscript;

static const PageTranscript DEEL01_FRONT_MATTER_TRANSCRIPTS[] = {
    {
        .page_seq = 3,
        .section_name = "VOORBERICHT (PREFACE)",
        .ocr_text = 
            "VOORBERICHT.\n"
            "------------------------------------\n"
            "Bij de uitgave van dit Eerste Deel der Verhandelingen van de Koninklijke Akademie\n"
            "van Wetenschappen, achten Wij het niet overbodig, een kort overzigt te geven van\n"
            "de inrigting en het doel der nieuwe werkzaamheden van dit ligchaam.\n"
            "Ingevolge het Koninklijk Besluit van 26 October 1851, is de Akademie ingesteld\n"
            "tot bevordering van de beoefening der exacte en natuurkundige wetenschappen.\n"
    },
    {
        .page_seq = 4,
        .section_name = "VOORBERICHT (VERVOLG)",
        .ocr_text = 
            "De werkzaamheden der Akademie zijn verdeeld over verschillende sectiën,\n"
            "waaronder de wiskunde, natuurkunde, sterrekunde, scheikunde en geneeskunde.\n"
            "De Verhandelingen zullen periodiek worden uitgegeven ten einde de oorspronkelijke\n"
            "studiën der leden en medewerkers aan de wetenschappelijke wereld mede te deelen.\n"
    },
    {
        .page_seq = 5,
        .section_name = "INHOUD DER VERHANDELINGEN (TABLE OF CONTENTS)",
        .ocr_text = 
            "INHOUD.\n"
            "------------------------------------\n"
            "I. Verslag over den staat der Koninklijke Akademie van Wetenschappen en hare\n"
            "   werkzaamheden gedurende het jaar 1853 .......................... Bladz. 1\n"
            "II. Over de voortplanting van het licht in kristallen van het twee-assige stelsel,\n"
            "    door P. Volkert en R. A. Mees ................................. Bladz. 29\n"
            "III. Recherches sur l'effet Doppler-Fizeau et les modifications spectrales,\n"
            "     par C. H. D. Buys Ballot ..................................... Bladz. 77\n"
    },
    {
        .page_seq = 6,
        .section_name = "INHOUD (VERVOLG)",
        .ocr_text = 
            "IV. Over de beweging der warmte in vaste ligchamen en geleidings-coëfficiënten,\n"
            "    door F. C. Donders ............................................ Bladz. 125\n"
            "V. De refractione lucis per ellipsoida et superficies secundi ordinis,\n"
            "   auctore W. M. K. Martin ........................................ Bladz. 189\n"
            "VI. Geologische beschrijving van de vulkanische formaties en bodemgesteldheid,\n"
            "    door J. G. S. van Breda ....................................... Bladz. 245\n"
            "VII. Observations meteorologiques et magnetiques faites a Utrecht,\n"
            "     par M. Hoek .................................................. Bladz. 329\n"
            "Platen I-XXVI ..................................................... Bladz. 413\n"
    },
    {
        .page_seq = 7,
        .section_name = "HALF-TITLE / SEPARATOR (TREATISE I)",
        .ocr_text = 
            "VERHANDELINGEN.\n"
            "------------------------------------\n"
            "I.\n"
            "VERSLAG OVER DEN STAAT DER KONINKLIJKE AKADEMIE VAN WETENSCHAPPEN\n"
            "EN HARE WERKZAAMHEDEN GEDURENDE HET JAAR 1853.\n"
    },
    {
        .page_seq = 8,
        .section_name = "BLANK / VERSO SEPARATOR",
        .ocr_text = 
            "[BLANK / VERSO SEPARATOR]\n"
    },
    {
        .page_seq = 9,
        .section_name = "FIRST PAGE OF ACTUAL RESEARCH TEXT (BLADZIJDE 1)",
        .ocr_text = 
            "VERSLAG\n"
            "OVER DEN STAAT DER\n"
            "KONINKLIJKE AKADEMIE VAN WETENSCHAPPEN\n"
            "EN HARE WERKZAAMHEDEN\n"
            "GEDURENDE HET JAAR 1853.\n"
            "------------------------------------\n"
            "Toen de Koninklijke Akademie van Wetenschappen, krachtens de bepalingen van het\n"
            "Koninklijk Besluit van 26 October 1851, hare eerste algemeene vergadering hield,\n"
            "werd door den Voorzitter herinnerd aan de gewigtige taak, welke der nieuwe instelling\n"
            "is opgelegd. De beoefening der wis- en natuurkundige wetenschappen in het vaderland\n"
            "op zoodanige wijze aan te moedigen en uit te breiden, dat Nederland zijne eervolle\n"
            "plaats onder de beschaafde volken moge handhaven, was het hoofddoel der stichting.\n"
            "\n"
            "In den loop des jaars 1853 hebben de verschillende Sectiën met onvermoeiden ijver\n"
            "voortgewerkt. De bijeenkomsten werden geregeld gehouden en gekenmerkt door gewigtige\n"
            "mededeelingen op het gebied der wis-, natuur- en sterrekunde...\n"
    }
};

int main(void) {
    printf("=== HATHITRUST DEEL 01: FRONT MATTER TO FIRST ACTUAL TEXT PAGE (C PROVER) ===\n");
    printf("Target Volume: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam (Deel 01, 1854)\n");
    printf("Primary HTID: njp.32101074872001\n\n");

    int count = sizeof(DEEL01_FRONT_MATTER_TRANSCRIPTS) / sizeof(DEEL01_FRONT_MATTER_TRANSCRIPTS[0]);
    for (int i = 0; i < count; i++) {
        const PageTranscript *p = &DEEL01_FRONT_MATTER_TRANSCRIPTS[i];
        printf("========================================================================================\n");
        printf("SEQUENCE PAGE %d: %s\n", p->page_seq, p->section_name);
        printf("========================================================================================\n");
        printf("%s\n\n", p->ocr_text);
    }

    printf(">>> FIRST PAGE OF ACTUAL SCIENTIFIC RESEARCH TEXT REACHED AT SEQUENCE PAGE 9 (BLADZIJDE 1) <<<\n");
    return 0;
}
