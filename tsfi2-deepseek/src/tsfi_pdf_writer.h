#ifndef TSFI_PDF_WRITER_H
#define TSFI_PDF_WRITER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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
} TsfiPdfPageEntry;

typedef struct {
    TsfiPdfStreamWriter out;
    TsfiPdfXrefTable xref;
    size_t object_count;
    size_t catalog_obj;
    size_t pages_root_obj;
    size_t font_times_roman;
    size_t font_times_bold;
    size_t font_times_italic;
    size_t font_courier;
    TsfiPdfPageEntry pages[16];
    size_t page_count;
} TsfiPdfDocumentWriter;

TsfiPdfDocumentWriter *tsfi_pdf_writer_create(void);
size_t tsfi_pdf_writer_add_object(TsfiPdfDocumentWriter *writer);
void tsfi_pdf_writer_write_raw(TsfiPdfDocumentWriter *writer, const char *str, size_t len);
void tsfi_pdf_writer_add_page_styled(TsfiPdfDocumentWriter *writer, const char *text_content, int page_number);
int tsfi_pdf_writer_finalize_multipage(TsfiPdfDocumentWriter *writer, const char *output_filepath);
void tsfi_pdf_writer_free(TsfiPdfDocumentWriter *writer);

#endif /* TSFI_PDF_WRITER_H */
