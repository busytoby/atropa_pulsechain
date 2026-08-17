#include "tsfi_pdf_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TsfiPdfDocumentWriter *tsfi_pdf_writer_create(void) {
    TsfiPdfDocumentWriter *w = (TsfiPdfDocumentWriter *)malloc(sizeof(TsfiPdfDocumentWriter));
    if (!w) return NULL;
    memset(w, 0, sizeof(*w));

    w->out.capacity = 16384;
    w->out.data = (char *)malloc(w->out.capacity);
    if (!w->out.data) {
        free(w);
        return NULL;
    }
    w->out.length = 0;

    w->xref.capacity = 64;
    w->xref.offsets = (size_t *)malloc(w->xref.capacity * sizeof(size_t));
    if (!w->xref.offsets) {
        free(w->out.data);
        free(w);
        return NULL;
    }
    w->xref.count = 0;

    // Write PDF header
    const char *hdr = "%PDF-1.7\n%\xE2\xE3\xCF\xD3\n";
    tsfi_pdf_writer_write_raw(w, hdr, strlen(hdr));

    return w;
}

void tsfi_pdf_writer_write_raw(TsfiPdfDocumentWriter *writer, const char *str, size_t len) {
    if (!writer || !str || len == 0) return;
    if (writer->out.length + len + 1 >= writer->out.capacity) {
        writer->out.capacity = (writer->out.length + len + 1) * 2;
        writer->out.data = (char *)realloc(writer->out.data, writer->out.capacity);
    }
    memcpy(writer->out.data + writer->out.length, str, len);
    writer->out.length += len;
    writer->out.data[writer->out.length] = '\0';
}

size_t tsfi_pdf_writer_add_object(TsfiPdfDocumentWriter *writer) {
    if (!writer) return 0;
    writer->object_count++;
    if (writer->xref.count >= writer->xref.capacity) {
        writer->xref.capacity *= 2;
        writer->xref.offsets = (size_t *)realloc(writer->xref.offsets, writer->xref.capacity * sizeof(size_t));
    }
    writer->xref.offsets[writer->xref.count++] = writer->out.length;

    char obj_hdr[64];
    int len = snprintf(obj_hdr, sizeof(obj_hdr), "%zu 0 obj\n", writer->object_count);
    tsfi_pdf_writer_write_raw(writer, obj_hdr, (size_t)len);
    return writer->object_count;
}

void tsfi_pdf_writer_add_page(TsfiPdfDocumentWriter *writer, const char *text_content, size_t text_len) {
    if (!writer) return;

    // Content Stream Object
    size_t stream_obj = tsfi_pdf_writer_add_object(writer);
    char stream_hdr[256];
    
    // Format text stream operations
    char content_buf[8192];
    int c_len = snprintf(content_buf, sizeof(content_buf),
                         "BT\n"
                         "/F1 12 Tf\n"
                         "50 750 Td\n"
                         "14 TL\n"
                         "(%s) Tj\n"
                         "ET\n",
                         text_content ? text_content : "Auncient CP/M-Tomie Pure C Document");

    int s_len = snprintf(stream_hdr, sizeof(stream_hdr),
                         "<< /Length %d >>\nstream\n", c_len);
    tsfi_pdf_writer_write_raw(writer, stream_hdr, (size_t)s_len);
    tsfi_pdf_writer_write_raw(writer, content_buf, (size_t)c_len);
    tsfi_pdf_writer_write_raw(writer, "\nendstream\nendobj\n", 18);

    // Page Object
    size_t page_obj = tsfi_pdf_writer_add_object(writer);
    char page_dict[512];
    int p_len = snprintf(page_dict, sizeof(page_dict),
                         "<< /Type /Page\n"
                         "   /Parent 1 0 R\n"
                         "   /MediaBox [0 0 595 842]\n"
                         "   /Contents %zu 0 R\n"
                         "   /Resources << /Font << /F1 2 0 R >> >>\n"
                         ">>\nendobj\n",
                         stream_obj);
    tsfi_pdf_writer_write_raw(writer, page_dict, (size_t)p_len);
    (void)text_len;
    (void)page_obj;
}

int tsfi_pdf_writer_finalize_file(TsfiPdfDocumentWriter *writer, const char *output_filepath) {
    if (!writer || !output_filepath) return -1;

    // Standard Font Object (Object 2)
    size_t font_obj = tsfi_pdf_writer_add_object(writer);
    const char *font_dict = "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>\nendobj\n";
    tsfi_pdf_writer_write_raw(writer, font_dict, strlen(font_dict));

    // Pages Root Object (Object 1)
    size_t pages_obj = tsfi_pdf_writer_add_object(writer);
    char pages_dict[256];
    int pg_len = snprintf(pages_dict, sizeof(pages_dict),
                          "<< /Type /Pages /Kids [4 0 R] /Count 1 >>\nendobj\n");
    tsfi_pdf_writer_write_raw(writer, pages_dict, (size_t)pg_len);

    // Catalog Object (Object 5)
    size_t catalog_obj = tsfi_pdf_writer_add_object(writer);
    char cat_dict[256];
    int cat_len = snprintf(cat_dict, sizeof(cat_dict),
                           "<< /Type /Catalog /Pages %zu 0 R >>\nendobj\n", pages_obj);
    tsfi_pdf_writer_write_raw(writer, cat_dict, (size_t)cat_len);

    // XREF Table
    size_t xref_start = writer->out.length;
    char xref_hdr[64];
    int x_len = snprintf(xref_hdr, sizeof(xref_hdr), "xref\n0 %zu\n0000000000 65535 f \n", writer->xref.count + 1);
    tsfi_pdf_writer_write_raw(writer, xref_hdr, (size_t)x_len);

    for (size_t i = 0; i < writer->xref.count; ++i) {
        char entry[32];
        snprintf(entry, sizeof(entry), "%010zu 00000 n \n", writer->xref.offsets[i]);
        tsfi_pdf_writer_write_raw(writer, entry, strlen(entry));
    }

    // Trailer
    char trailer[256];
    int tr_len = snprintf(trailer, sizeof(trailer),
                          "trailer\n<< /Size %zu /Root %zu 0 R >>\nstartxref\n%zu\n%%%%EOF\n",
                          writer->xref.count + 1, catalog_obj, xref_start);
    tsfi_pdf_writer_write_raw(writer, trailer, (size_t)tr_len);

    FILE *f = fopen(output_filepath, "wb");
    if (!f) return -1;
    fwrite(writer->out.data, 1, writer->out.length, f);
    fclose(f);

    (void)font_obj;
    return 0;
}

void tsfi_pdf_writer_free(TsfiPdfDocumentWriter *writer) {
    if (!writer) return;
    if (writer->out.data) free(writer->out.data);
    if (writer->xref.offsets) free(writer->xref.offsets);
    free(writer);
}
