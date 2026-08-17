#ifndef TSFI_PDF_READER_H
#define TSFI_PDF_READER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char *text;
    size_t length;
    size_t capacity;
} TsfiPdfTextBuffer;

typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} TsfiPdfByteBuffer;

typedef enum {
    TSFI_PDF_PATH_MOVE_TO,
    TSFI_PDF_PATH_LINE_TO,
    TSFI_PDF_PATH_CURVE_TO,
    TSFI_PDF_PATH_RECT,
    TSFI_PDF_PATH_CLOSE
} TsfiPdfPathOpType;

typedef struct {
    TsfiPdfPathOpType op;
    double x[3];
    double y[3];
} TsfiPdfPathSegment;

typedef struct {
    TsfiPdfPathSegment *segments;
    size_t count;
    size_t capacity;
    double fill_color[4];
    double stroke_color[4];
    double line_width;
} TsfiPdfVectorPath;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t bits_per_component;
    uint32_t color_space_channels;
    uint8_t *pixel_data;
    size_t data_size;
} TsfiPdfImageObject;

typedef struct {
    size_t total_streams;
    size_t text_bytes_extracted;
    size_t vector_path_count;
    size_t image_object_count;
    size_t chart_element_count;

    TsfiPdfTextBuffer *text_buffer;
    TsfiPdfVectorPath *vector_paths;
    TsfiPdfImageObject *images;
} TsfiPdfDocumentFeatures;

TsfiPdfTextBuffer *tsfi_pdf_text_buffer_create(void);
void tsfi_pdf_text_buffer_free(TsfiPdfTextBuffer *buf);
void tsfi_pdf_text_buffer_append(TsfiPdfTextBuffer *buf, const char *str, size_t len);

int tsfi_pdf_inflate_raw(const uint8_t *src, size_t src_len, TsfiPdfByteBuffer *out);
int tsfi_pdf_zlib_decompress(const uint8_t *src, size_t src_len, TsfiPdfByteBuffer *out);

char *tsfi_pdf_extract_text(const char *filepath, size_t *out_length);
TsfiPdfDocumentFeatures *tsfi_pdf_extract_all_features(const char *filepath);
void tsfi_pdf_document_features_free(TsfiPdfDocumentFeatures *feats);

#endif /* TSFI_PDF_READER_H */
