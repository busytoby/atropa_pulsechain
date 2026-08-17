#include "tsfi_pdf_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

    // Write standard PDF 1.7 header
    const char *hdr = "%PDF-1.7\n%\xE2\xE3\xCF\xD3\n";
    tsfi_pdf_writer_write_raw(w, hdr, strlen(hdr));

    // Object 1: Catalog
    w->catalog_obj = tsfi_pdf_writer_add_object(w);
    const char *cat_dict = "<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";
    tsfi_pdf_writer_write_raw(w, cat_dict, strlen(cat_dict));

    // Object 2: Pages Root
    w->pages_root_obj = tsfi_pdf_writer_add_object(w);
    const char *pgs_dict = "<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n";
    tsfi_pdf_writer_write_raw(w, pgs_dict, strlen(pgs_dict));

    // Object 4: Font Object (referenced before page, but numbered as 4)
    // We will reserve Object 3 for Page, Object 4 for Font, Object 5 for Content Stream

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

static void format_wrapped_text_stream(TsfiPdfStreamWriter *stream_buf, const char *text) {
    if (!text) return;

    // Start Text object, set font to Helvetica 11pt, leading 15pt
    const char *bt = "BT\n/F1 11 Tf\n15 TL\n50 780 Td\n";
    if (stream_buf->length + strlen(bt) >= stream_buf->capacity) {
        stream_buf->capacity = (stream_buf->length + strlen(bt) + 4096) * 2;
        stream_buf->data = (char *)realloc(stream_buf->data, stream_buf->capacity);
    }
    memcpy(stream_buf->data + stream_buf->length, bt, strlen(bt));
    stream_buf->length += strlen(bt);

    // Split text into ~75 character wrapped lines
    const char *p = text;
    char line[128];
    size_t line_len = 0;
    bool first_line = true;

    while (*p) {
        if (*p == '\n' || line_len >= 75) {
            if (line_len > 0) {
                line[line_len] = '\0';
                char op[256];
                int olen = 0;
                if (first_line) {
                    olen = snprintf(op, sizeof(op), "(%s) Tj\n", line);
                    first_line = false;
                } else {
                    olen = snprintf(op, sizeof(op), "T* (%s) Tj\n", line);
                }
                if (stream_buf->length + olen >= stream_buf->capacity) {
                    stream_buf->capacity = (stream_buf->length + olen + 4096) * 2;
                    stream_buf->data = (char *)realloc(stream_buf->data, stream_buf->capacity);
                }
                memcpy(stream_buf->data + stream_buf->length, op, olen);
                stream_buf->length += olen;
                line_len = 0;
            }
            if (*p == '\n') {
                p++;
                continue;
            }
        }

        // Handle parentheses escaping
        if (*p == '(' || *p == ')' || *p == '\\') {
            if (line_len + 2 < sizeof(line)) {
                line[line_len++] = '\\';
                line[line_len++] = *p;
            }
        } else if ((unsigned char)*p >= 32 && (unsigned char)*p <= 126) {
            if (line_len + 1 < sizeof(line)) {
                line[line_len++] = *p;
            }
        }
        p++;
    }

    if (line_len > 0) {
        line[line_len] = '\0';
        char op[256];
        int olen = 0;
        if (first_line) {
            olen = snprintf(op, sizeof(op), "(%s) Tj\n", line);
        } else {
            olen = snprintf(op, sizeof(op), "T* (%s) Tj\n", line);
        }
        if (stream_buf->length + olen >= stream_buf->capacity) {
            stream_buf->capacity = (stream_buf->length + olen + 4096) * 2;
            stream_buf->data = (char *)realloc(stream_buf->data, stream_buf->capacity);
        }
        memcpy(stream_buf->data + stream_buf->length, op, olen);
        stream_buf->length += olen;
    }

    const char *et = "ET\n";
    if (stream_buf->length + strlen(et) >= stream_buf->capacity) {
        stream_buf->capacity = (stream_buf->length + strlen(et) + 128) * 2;
        stream_buf->data = (char *)realloc(stream_buf->data, stream_buf->capacity);
    }
    memcpy(stream_buf->data + stream_buf->length, et, strlen(et));
    stream_buf->length += strlen(et);
    stream_buf->data[stream_buf->length] = '\0';
}

void tsfi_pdf_writer_add_page(TsfiPdfDocumentWriter *writer, const char *text_content, size_t text_len) {
    if (!writer) return;

    // Object 3: Page Object
    writer->current_page_obj = tsfi_pdf_writer_add_object(writer);
    const char *page_dict = 
        "<< /Type /Page\n"
        "   /Parent 2 0 R\n"
        "   /MediaBox [0 0 595 842]\n"
        "   /Contents 5 0 R\n"
        "   /Resources << /Font << /F1 4 0 R >> >>\n"
        ">>\nendobj\n";
    tsfi_pdf_writer_write_raw(writer, page_dict, strlen(page_dict));

    // Object 4: Font Object
    size_t font_obj = tsfi_pdf_writer_add_object(writer);
    const char *font_dict = "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>\nendobj\n";
    tsfi_pdf_writer_write_raw(writer, font_dict, strlen(font_dict));

    // Object 5: Content Stream
    size_t stream_obj = tsfi_pdf_writer_add_object(writer);
    TsfiPdfStreamWriter content = {0};
    content.capacity = 4096;
    content.data = (char *)malloc(content.capacity);
    content.length = 0;

    format_wrapped_text_stream(&content, text_content);

    char s_hdr[128];
    int sh_len = snprintf(s_hdr, sizeof(s_hdr), "<< /Length %zu >>\nstream\n", content.length);
    tsfi_pdf_writer_write_raw(writer, s_hdr, (size_t)sh_len);
    tsfi_pdf_writer_write_raw(writer, content.data, content.length);
    tsfi_pdf_writer_write_raw(writer, "\nendstream\nendobj\n", 18);

    free(content.data);
    (void)font_obj;
    (void)stream_obj;
    (void)text_len;
}

int tsfi_pdf_writer_finalize_file(TsfiPdfDocumentWriter *writer, const char *output_filepath) {
    if (!writer || !output_filepath) return -1;

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
                          "trailer\n<< /Size %zu /Root 1 0 R >>\nstartxref\n%zu\n%%%%EOF\n",
                          writer->xref.count + 1, xref_start);
    tsfi_pdf_writer_write_raw(writer, trailer, (size_t)tr_len);

    FILE *f = fopen(output_filepath, "wb");
    if (!f) return -1;
    fwrite(writer->out.data, 1, writer->out.length, f);
    fclose(f);

    return 0;
}

void tsfi_pdf_writer_free(TsfiPdfDocumentWriter *writer) {
    if (!writer) return;
    if (writer->out.data) free(writer->out.data);
    if (writer->xref.offsets) free(writer->xref.offsets);
    free(writer);
}
