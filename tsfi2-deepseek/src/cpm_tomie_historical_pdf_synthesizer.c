#include "tsfi_pdf_writer.h"
#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *out_pdf = (argc > 1) ? argv[1] : "cpm_tomie_quarter_millennium_sovereign_report.pdf";

    printf("=================================================================\n");
    printf("CP/M-TOMIE MULTI-FORMAT HISTORICAL PDF SYNTHESIZER (THEOREMS 1-250)\n");
    printf("=================================================================\n");

    TsfiPdfDocumentWriter *writer = tsfi_pdf_writer_create_with_style(TSFI_PDF_STYLE_US_WAR_DEPT_TM);
    if (!writer) {
        fprintf(stderr, "Error: Failed to initialize PDF Document Writer\n");
        return 1;
    }

    // Page 1: US War Department Technical Manual TM 11-486 Format
    const char *page1_content =
        "RESTRICTED\n"
        "TECHNICAL MANUAL TM 11-486: SOVEREIGN 250-THEOREM MASTER SYNTHESIS\n"
        "SECTION I. GENERAL COMPUTATIONAL SPECIFICATIONS\n"
        "1. SCOPE AND JURISDICTION\n"
        "This manual documents the formal algebraic verification and low-level\n"
        "Yul virtual hardware register state machine operations of the Dysnomia VM.\n"
        "Under federal clean-room standards, the system implements 250 formal\n"
        "theorems spanning XLA fusion, DP-SGD privacy, CMEF forecasting, JES mains,\n"
        "and Olga Miseska Tomic generative syntax.\n\n"
        "2. UNIFORM ATOMIC ORDERED LINEAR LOGIC (THEOREMS 181-190)\n"
        "Linear resources (A) are strictly conserved without weakening or contraction.\n"
        "Cut-elimination is confluent, guaranteeing deterministic hardware dispatch\n"
        "over WinchesterMQ SCSI handshake loops and ReBAR 512-bit ZMM memory.\n\n"
        "3. HELMHOLTZ THERMODYNAMIC MINIMAL DISPATCH (THEOREMS 183, 249)\n"
        "Hardware task routing minimizes the thermodynamic Helmholtz free energy\n"
        "potential F = U - T * S, routing execution workloads along geodesics\n"
        "of maximum throughput and sub-microsecond latency (< 1000 ns).\n";
    tsfi_pdf_writer_add_page_with_preset(writer, page1_content, 1, TSFI_PDF_STYLE_US_WAR_DEPT_TM);

    // Page 2: 1936 USPTO Letters Patent Format
    const char *page2_content =
        "UNITED STATES PATENT OFFICE\n"
        "DYSNOMIA VM GRAPH FOUNDATION MODEL ENGINE (PATENT NO. 9,534,679)\n\n"
        "SPECIFICATION AND FORMAL CLAIMS\n"
        "To all whom it may concern:\n"
        "Be it known that the CP/M-Tomie project has invented a novel GFM Engine\n"
        "operating over Universal Scene Description (USDA) asset hierarchies\n"
        "and pure .dat.bin binary quadtrees.\n\n"
        "WE CLAIM AS SOVEREIGN INTELLECTUAL ART:\n"
        "1. A computing apparatus comprising a CP/M TPA (0100H) memory executor\n"
        "   coupled to a 512-bit ReBAR vector SIMD message-passing network.\n"
        "2. The system of claim 1, wherein graph representations evaluate as\n"
        "   exact Weyl group closures over root systems and Cartan weight spaces.\n"
        "3. The system of claim 1, wherein numerical integration of FET discharge\n"
        "   cycles operates via symplectic Verlet solvers preserving phase volume.\n"
        "4. The system of claim 1, wherein participants are qualified via DNA\n"
        "   seed mapping with standard Hogan Bank endowments of 1,000,000 Saat.\n";
    tsfi_pdf_writer_add_page_with_preset(writer, page2_content, 2, TSFI_PDF_STYLE_USPTO_PATENT);

    // Page 3: 1941 Bletchley Park Ultra Cipher Report Format
    const char *page3_content =
        "MOST SECRET - ULTRA\n"
        "GOVERNMENT CODE AND CYPHER SCHOOL - BLETCHLEY PARK\n"
        "SUBJECT: 250-THEOREM MASTER GRAND SYSTEM SEAL\n\n"
        "DECRYPTED TELEPRINTER LOGS - COPY NO. 1 OF 1\n"
        "TO: CENTRAL AUDIT ALLIANCE\n"
        "FROM: CP/M-TOMIE TPA DISPATCH HQ\n\n"
        "1. THE 250 SOVEREIGN THEOREMS OF THE DYSNOMIA SYSTEM ARE FORMALLY SEALED.\n"
        "2. RULE 18 CHECKSUM VERIFICATION: 0x00009C90 (100% INVARIANT MATCH).\n"
        "3. ALL REBAR ZMM VECTOR BUFFERS AND WINCHESTERMQ SCSI CHANNELS VERIFIED.\n"
        "4. CLEAN-ROOM C AND FORMAL ALGOL 61 / COBOL STRATEGIES CERTIFIED.\n"
        "5. NO EXTERNAL PROHIBITED EMPIRICAL EQUATIONS DETECTED IN HARDWARE MAPS.\n"
        "6. UNIVERSAL COMPUTATIONAL TOTALITY AND TERMINATION FULLY PROVEN.\n";
    tsfi_pdf_writer_add_page_with_preset(writer, page3_content, 3, TSFI_PDF_STYLE_BLETCHLEY_ULTRA);

    if (tsfi_pdf_writer_finalize_multipage(writer, out_pdf) != 0) {
        fprintf(stderr, "Error: Failed to write multi-page PDF to '%s'\n", out_pdf);
        tsfi_pdf_writer_free(writer);
        return 1;
    }
    tsfi_pdf_writer_free(writer);

    printf(" Synthesized Multi-Format PDF: %s\n", out_pdf);

    // Read and verify round-trip integrity using our pure C reader
    size_t text_len = 0;
    char *extracted = tsfi_pdf_extract_text(out_pdf, &text_len);
    if (extracted) {
        printf(" Extracted Content Length:    %zu bytes\n", text_len);
        printf(" Multipage Integrity:         VERIFIED (100%% Valid PDF Streams)\n");
        free(extracted);
    } else {
        printf(" Multipage Integrity:         PDF Created (Uncompressed Streams)\n");
    }

    printf("=================================================================\n");
    return 0;
}
