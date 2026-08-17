#ifndef TSFI_PDF_WRITER_H
#define TSFI_PDF_WRITER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TSFI_PDF_STYLE_COLING_1976,       // 1976 COLING IBM Selectric typewriter (Courier 10pt)
    TSFI_PDF_STYLE_US_WAR_DEPT_TM,    // 1944 US War Dept Technical Manual (RESTRICTED banner, Helvetica Bold, Courier body)
    TSFI_PDF_STYLE_USPTO_PATENT,      // 1936 USPTO Patent (Dual column, numbered line gutters, formal Claims)
    TSFI_PDF_STYLE_BLETCHLEY_ULTRA,   // 1941 Bletchley Park Ultra Secret Report (Red security box stamps, teleprinter text)
    TSFI_PDF_STYLE_ALGOL60_REPORT,    // 1960 IFIP ALGOL 60 Report (BNF grammar tables, bold keywords, serif body)
    TSFI_PDF_STYLE_CHOMSKY_1957,      // 1957 Mouton Syntactic Structures (Numbered derivation steps, small caps headers)
    TSFI_PDF_STYLE_CODASYL_1960       // 1960 DoD CODASYL COBOL (4-division hierarchy, margin A & B 80-col card rulings)
} TsfiPdfDocumentStyle;

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} TsfiPdfStreamWriter;

typedef struct {
    size_t *offsets;
    size_t count;
    size_t capacity;
} TsfiPdfXrefTable;

typedef struct {
    size_t stream_obj;
    size_t page_obj;
    char *text_content;
    int page_number;
    TsfiPdfDocumentStyle style;
} TsfiPdfPageEntry;

typedef struct {
    TsfiPdfStreamWriter out;
    TsfiPdfXrefTable xref;
    size_t object_count;
    size_t catalog_obj;
    size_t pages_root_obj;
    size_t font_courier;
    size_t font_courier_bold;
    size_t font_times_roman;
    size_t font_times_bold;
    size_t font_helvetica_bold;
    TsfiPdfPageEntry pages[16];
    size_t page_count;
    TsfiPdfDocumentStyle default_style;
} TsfiPdfDocumentWriter;

TsfiPdfDocumentWriter *tsfi_pdf_writer_create(void);
TsfiPdfDocumentWriter *tsfi_pdf_writer_create_with_style(TsfiPdfDocumentStyle style);
size_t tsfi_pdf_writer_add_object(TsfiPdfDocumentWriter *writer);
void tsfi_pdf_writer_write_raw(TsfiPdfDocumentWriter *writer, const char *str, size_t len);
void tsfi_pdf_writer_add_page_styled(TsfiPdfDocumentWriter *writer, const char *text_content, int page_number);
void tsfi_pdf_writer_add_page_with_preset(TsfiPdfDocumentWriter *writer, const char *text_content, int page_number, TsfiPdfDocumentStyle style);
int tsfi_pdf_writer_finalize_multipage(TsfiPdfDocumentWriter *writer, const char *output_filepath);
void tsfi_pdf_writer_free(TsfiPdfDocumentWriter *writer);

#endif /* TSFI_PDF_WRITER_H */
