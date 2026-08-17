#include "tsfi_pdf_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

TsfiPdfDocumentWriter *tsfi_pdf_writer_create(void) {
    TsfiPdfDocumentWriter *w = (TsfiPdfDocumentWriter *)malloc(sizeof(TsfiPdfDocumentWriter));
    if (!w) return NULL;
    memset(w, 0, sizeof(*w));

    w->out.capacity = 65536;
    w->out.data = (char *)malloc(w->out.capacity);
    if (!w->out.data) {
        free(w);
        return NULL;
    }
    w->out.length = 0;

    w->xref.capacity = 256;
    w->xref.offsets = (size_t *)malloc(w->xref.capacity * sizeof(size_t));
    if (!w->xref.offsets) {
        free(w->out.data);
        free(w);
        return NULL;
    }
    w->xref.count = 0;

    // Standard PDF 1.7 Header
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

void tsfi_pdf_writer_add_page_styled(TsfiPdfDocumentWriter *writer, const char *text_content, int page_number) {
    if (!writer || writer->page_count >= 16) return;

    TsfiPdfPageEntry *entry = &writer->pages[writer->page_count++];
    entry->text_content = strdup(text_content ? text_content : "");
    entry->page_number = page_number;
}

static void render_coling_page_stream(TsfiPdfStreamWriter *content, const char *text, int page_number) {
    // Begin Text block: Courier font, 10pt with 13.5pt leading, authentic margins
    const char *bt = "BT\n/F1 10 Tf\n13.5 TL\n54 770 Td\n";
    memcpy(content->data + content->length, bt, strlen(bt));
    content->length += strlen(bt);

    const char *p = text;
    char line[128];
    size_t line_len = 0;
    bool in_body = false;

    while (*p) {
        if (*p == '\n' || line_len >= 72) {
            if (line_len > 0) {
                line[line_len] = '\0';
                char op[256];
                int olen = 0;
                if (!in_body) {
                    olen = snprintf(op, sizeof(op), "(%s) Tj\n", line);
                    in_body = true;
                } else {
                    olen = snprintf(op, sizeof(op), "T* (%s) Tj\n", line);
                }
                if (content->length + olen + 512 >= content->capacity) {
                    content->capacity = (content->length + olen + 4096) * 2;
                    content->data = (char *)realloc(content->data, content->capacity);
                }
                memcpy(content->data + content->length, op, olen);
                content->length += olen;
                line_len = 0;
            }
            if (*p == '\n') {
                p++;
                continue;
            }
        }

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
        int olen = snprintf(op, sizeof(op), "T* (%s) Tj\n", line);
        if (content->length + olen + 512 >= content->capacity) {
            content->capacity = (content->length + olen + 4096) * 2;
            content->data = (char *)realloc(content->data, content->capacity);
        }
        memcpy(content->data + content->length, op, olen);
        content->length += olen;
    }

    // Format page footer: e.g. "- 292 -" centered at bottom
    char footer[128];
    int f_len = snprintf(footer, sizeof(footer),
                         "T* T* (                                -  %d  -                                ) Tj\nET\n",
                         page_number);
    if (content->length + f_len + 64 >= content->capacity) {
        content->capacity = (content->length + f_len + 512) * 2;
        content->data = (char *)realloc(content->data, content->capacity);
    }
    memcpy(content->data + content->length, footer, f_len);
    content->length += f_len;
    content->data[content->length] = '\0';
}

int tsfi_pdf_writer_finalize_multipage(TsfiPdfDocumentWriter *writer, const char *output_filepath) {
    if (!writer || !output_filepath || writer->page_count == 0) return -1;

    // Object 1: Catalog
    writer->catalog_obj = tsfi_pdf_writer_add_object(writer);
    const char *cat_dict = "<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";
    tsfi_pdf_writer_write_raw(writer, cat_dict, strlen(cat_dict));

    // Object 2: Pages Root
    writer->pages_root_obj = tsfi_pdf_writer_add_object(writer);
    char pgs_dict[512];
    char kids_buf[256] = "[";
    
    // Page objects start at Object 4 (after Catalog, Pages, and Font)
    // Font Object = Object 3
    size_t start_page_obj = 4;
    for (size_t i = 0; i < writer->page_count; ++i) {
        char k_entry[32];
        snprintf(k_entry, sizeof(k_entry), "%zu 0 R ", start_page_obj + i * 2);
        strcat(kids_buf, k_entry);
    }
    strcat(kids_buf, "]");

    snprintf(pgs_dict, sizeof(pgs_dict),
             "<< /Type /Pages /Kids %s /Count %zu >>\nendobj\n",
             kids_buf, writer->page_count);
    tsfi_pdf_writer_write_raw(writer, pgs_dict, strlen(pgs_dict));

    // Object 3: Font Object
    size_t font_obj = tsfi_pdf_writer_add_object(writer);
    const char *font_dict = "<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>\nendobj\n";
    tsfi_pdf_writer_write_raw(writer, font_dict, strlen(font_dict));

    // Now write each Page Object and its Content Stream Object
    for (size_t i = 0; i < writer->page_count; ++i) {
        size_t page_num = start_page_obj + i * 2;
        size_t stream_num = page_num + 1;

        // Page Object
        size_t p_obj = tsfi_pdf_writer_add_object(writer);
        char page_dict[512];
        int pd_len = snprintf(page_dict, sizeof(page_dict),
                              "<< /Type /Page\n"
                              "   /Parent 2 0 R\n"
                              "   /MediaBox [0 0 595 842]\n"
                              "   /Contents %zu 0 R\n"
                              "   /Resources << /Font << /F1 3 0 R >> >>\n"
                              ">>\nendobj\n",
                              stream_num);
        tsfi_pdf_writer_write_raw(writer, page_dict, (size_t)pd_len);

        // Content Stream Object
        size_t s_obj = tsfi_pdf_writer_add_object(writer);
        TsfiPdfStreamWriter strm = {0};
        strm.capacity = 16384;
        strm.data = (char *)malloc(strm.capacity);
        strm.length = 0;

        render_coling_page_stream(&strm, writer->pages[i].text_content, writer->pages[i].page_number);

        char shdr[128];
        int sh_len = snprintf(shdr, sizeof(shdr), "<< /Length %zu >>\nstream\n", strm.length);
        tsfi_pdf_writer_write_raw(writer, shdr, (size_t)sh_len);
        tsfi_pdf_writer_write_raw(writer, strm.data, strm.length);
        tsfi_pdf_writer_write_raw(writer, "\nendstream\nendobj\n", 18);

        free(strm.data);
        (void)font_obj;
        (void)p_obj;
        (void)s_obj;
    }

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
    for (size_t i = 0; i < writer->page_count; ++i) {
        if (writer->pages[i].text_content) free(writer->pages[i].text_content);
    }
    if (writer->out.data) free(writer->out.data);
    if (writer->xref.offsets) free(writer->xref.offsets);
    free(writer);
}
