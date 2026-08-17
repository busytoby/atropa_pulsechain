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
    TsfiPdfStreamWriter out;
    TsfiPdfXrefTable xref;
    size_t object_count;
    size_t current_page_obj;
    size_t catalog_obj;
    size_t pages_root_obj;
} TsfiPdfDocumentWriter;

TsfiPdfDocumentWriter *tsfi_pdf_writer_create(void);
size_t tsfi_pdf_writer_add_object(TsfiPdfDocumentWriter *writer);
void tsfi_pdf_writer_write_raw(TsfiPdfDocumentWriter *writer, const char *str, size_t len);
void tsfi_pdf_writer_add_page(TsfiPdfDocumentWriter *writer, const char *text_content, size_t text_len);
int tsfi_pdf_writer_finalize_file(TsfiPdfDocumentWriter *writer, const char *output_filepath);
void tsfi_pdf_writer_free(TsfiPdfDocumentWriter *writer);

#endif /* TSFI_PDF_WRITER_H */
