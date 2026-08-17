#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Standalone Pure C RFC 1951 / RFC 1952 Inflate Engine */

typedef struct {
    const uint8_t *src;
    size_t src_len;
    size_t bit_pos;
} BitReader;

static inline uint32_t read_bits(BitReader *br, int num_bits) {
    uint32_t val = 0;
    for (int i = 0; i < num_bits; ++i) {
        size_t byte_idx = br->bit_pos >> 3;
        int bit_idx = (int)(br->bit_pos & 7);
        if (byte_idx < br->src_len) {
            val |= ((uint32_t)((br->src[byte_idx] >> bit_idx) & 1)) << i;
        }
        br->bit_pos++;
    }
    return val;
}

typedef struct {
    uint16_t counts[16];
    uint16_t symbols[288];
} HuffmanTable;

static void build_huffman_table(HuffmanTable *ht, const uint8_t *lengths, int count) {
    memset(ht->counts, 0, sizeof(ht->counts));
    for (int i = 0; i < count; ++i) {
        if (lengths[i] > 0 && lengths[i] < 16) {
            ht->counts[lengths[i]]++;
        }
    }
    
    uint16_t offsets[16];
    offsets[0] = 0;
    offsets[1] = 0;
    for (int i = 1; i < 15; ++i) {
        offsets[i + 1] = offsets[i] + ht->counts[i];
    }
    
    for (int i = 0; i < count; ++i) {
        if (lengths[i] > 0 && lengths[i] < 16) {
            ht->symbols[offsets[lengths[i]]++] = (uint16_t)i;
        }
    }
}

static int decode_symbol(BitReader *br, const HuffmanTable *ht) {
    uint32_t code = 0;
    int first = 0;
    int index = 0;
    for (int len = 1; len <= 15; ++len) {
        code = (code << 1) | read_bits(br, 1);
        int count = ht->counts[len];
        if (code < (uint32_t)(first + count)) {
            return (int)ht->symbols[index + (code - first)];
        }
        index += count;
        first = (first + count) << 1;
    }
    return -1;
}

static const int LENGTH_BASES[29] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
static const int LENGTH_EXTRA_BITS[29] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};
static const int DIST_BASES[30] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};
static const int DIST_EXTRA_BITS[30] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};
static const uint8_t CLEN_ORDER[19] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static void byte_buf_push(TsfiPdfByteBuffer *b, uint8_t byte) {
    if (b->size >= b->capacity) {
        b->capacity = b->capacity ? b->capacity * 2 : 4096;
        b->data = (uint8_t *)realloc(b->data, b->capacity);
    }
    b->data[b->size++] = byte;
}

static int inflate_block(BitReader *br, TsfiPdfByteBuffer *out,
                         const HuffmanTable *lit_table,
                         const HuffmanTable *dist_table) {
    while (1) {
        int sym = decode_symbol(br, lit_table);
        if (sym < 0) return -1;
        if (sym < 256) {
            byte_buf_push(out, (uint8_t)sym);
        } else if (sym == 256) {
            break;
        } else if (sym <= 285) {
            int len_idx = sym - 257;
            int length = LENGTH_BASES[len_idx] + (int)read_bits(br, LENGTH_EXTRA_BITS[len_idx]);
            int dist_sym = decode_symbol(br, dist_table);
            if (dist_sym < 0 || dist_sym >= 30) return -1;
            int distance = DIST_BASES[dist_sym] + (int)read_bits(br, DIST_EXTRA_BITS[dist_sym]);
            if ((size_t)distance > out->size) return -1;
            size_t start = out->size - (size_t)distance;
            for (int i = 0; i < length; ++i) {
                byte_buf_push(out, out->data[start + (size_t)i]);
            }
        } else {
            return -1;
        }
    }
    return 0;
}

int tsfi_pdf_inflate_raw(const uint8_t *src, size_t src_len, TsfiPdfByteBuffer *out) {
    BitReader br = { src, src_len, 0 };
    int bfinal = 0;
    while (!bfinal) {
        bfinal = (int)read_bits(&br, 1);
        int btype = (int)read_bits(&br, 2);
        if (btype == 0) {
            br.bit_pos = (br.bit_pos + 7) & ~7ULL;
            uint16_t len = (uint16_t)read_bits(&br, 16);
            uint16_t nlen = (uint16_t)read_bits(&br, 16);
            if ((len ^ 0xFFFF) != nlen) return -1;
            for (int i = 0; i < len; ++i) {
                byte_buf_push(out, (uint8_t)read_bits(&br, 8));
            }
        } else if (btype == 1) {
            uint8_t lit_lengths[288];
            for (int i = 0; i <= 143; ++i) lit_lengths[i] = 8;
            for (int i = 144; i <= 255; ++i) lit_lengths[i] = 9;
            for (int i = 256; i <= 279; ++i) lit_lengths[i] = 7;
            for (int i = 280; i <= 287; ++i) lit_lengths[i] = 8;
            HuffmanTable lit_ht;
            build_huffman_table(&lit_ht, lit_lengths, 288);

            uint8_t dist_lengths[32];
            for (int i = 0; i < 32; ++i) dist_lengths[i] = 5;
            HuffmanTable dist_ht;
            build_huffman_table(&dist_ht, dist_lengths, 32);

            if (inflate_block(&br, out, &lit_ht, &dist_ht) != 0) return -1;
        } else if (btype == 2) {
            int hlit = (int)read_bits(&br, 5) + 257;
            int hdist = (int)read_bits(&br, 5) + 1;
            int hclen = (int)read_bits(&br, 4) + 4;

            uint8_t clen_lengths[19] = {0};
            for (int i = 0; i < hclen; ++i) {
                clen_lengths[CLEN_ORDER[i]] = (uint8_t)read_bits(&br, 3);
            }
            HuffmanTable clen_ht;
            build_huffman_table(&clen_ht, clen_lengths, 19);

            uint8_t dyn_lengths[320] = {0};
            int total_codes = hlit + hdist;
            int idx = 0;
            while (idx < total_codes) {
                int sym = decode_symbol(&br, &clen_ht);
                if (sym < 16) {
                    dyn_lengths[idx++] = (uint8_t)sym;
                } else if (sym == 16) {
                    int repeat = (int)read_bits(&br, 2) + 3;
                    uint8_t prev = idx > 0 ? dyn_lengths[idx - 1] : 0;
                    while (repeat-- > 0 && idx < total_codes) dyn_lengths[idx++] = prev;
                } else if (sym == 17) {
                    int repeat = (int)read_bits(&br, 3) + 3;
                    while (repeat-- > 0 && idx < total_codes) dyn_lengths[idx++] = 0;
                } else if (sym == 18) {
                    int repeat = (int)read_bits(&br, 7) + 11;
                    while (repeat-- > 0 && idx < total_codes) dyn_lengths[idx++] = 0;
                } else {
                    return -1;
                }
            }

            HuffmanTable lit_ht;
            build_huffman_table(&lit_ht, dyn_lengths, hlit);

            HuffmanTable dist_ht;
            build_huffman_table(&dist_ht, &dyn_lengths[hlit], hdist);

            if (inflate_block(&br, out, &lit_ht, &dist_ht) != 0) return -1;
        } else {
            return -1;
        }
    }
    return 0;
}

int tsfi_pdf_zlib_decompress(const uint8_t *src, size_t src_len, TsfiPdfByteBuffer *out) {
    if (src_len < 2) return -1;
    size_t offset = 2;
    if ((src[1] & 0x20) != 0) {
        if (src_len < 6) return -1;
        offset += 4;
    }
    return tsfi_pdf_inflate_raw(src + offset, src_len - offset, out);
}

TsfiPdfTextBuffer *tsfi_pdf_text_buffer_create(void) {
    TsfiPdfTextBuffer *buf = (TsfiPdfTextBuffer *)malloc(sizeof(TsfiPdfTextBuffer));
    if (!buf) return NULL;
    buf->capacity = 4096;
    buf->length = 0;
    buf->text = (char *)malloc(buf->capacity);
    if (buf->text) buf->text[0] = '\0';
    return buf;
}

void tsfi_pdf_text_buffer_free(TsfiPdfTextBuffer *buf) {
    if (!buf) return;
    if (buf->text) free(buf->text);
    free(buf);
}

void tsfi_pdf_text_buffer_append(TsfiPdfTextBuffer *buf, const char *str, size_t len) {
    if (!buf || !str || len == 0) return;
    if (buf->length + len + 1 >= buf->capacity) {
        buf->capacity = (buf->length + len + 1) * 2;
        buf->text = (char *)realloc(buf->text, buf->capacity);
    }
    memcpy(buf->text + buf->length, str, len);
    buf->length += len;
    buf->text[buf->length] = '\0';
}

static void *memmem_fast(const void *l, size_t l_len, const void *s, size_t s_len) {
    if (!l || !s || s_len == 0 || l_len < s_len) return NULL;
    const uint8_t *cl = (const uint8_t *)l;
    const uint8_t *cs = (const uint8_t *)s;
    for (size_t i = 0; i <= l_len - s_len; i++) {
        if (memcmp(cl + i, cs, s_len) == 0) return (void *)(cl + i);
    }
    return NULL;
}

static bool is_content_stream(const uint8_t *data, size_t len) {
    if (len < 4) return false;
    if (memmem_fast(data, len < 64 ? len : 64, "%!PS-Adobe", 10) != NULL) return false;
    if (memmem_fast(data, len < 128 ? len : 128, "/CIDInit", 8) != NULL) return false;
    if (memmem_fast(data, len, "BT", 2) != NULL ||
        memmem_fast(data, len, "ET", 2) != NULL ||
        memmem_fast(data, len, "Tj", 2) != NULL ||
        memmem_fast(data, len, "TJ", 2) != NULL ||
        memmem_fast(data, len, "Td", 2) != NULL) {
        return true;
    }
    return false;
}

static void parse_multimodal_content_stream(const uint8_t *data, size_t len,
                                           TsfiPdfTextBuffer *text_out,
                                           TsfiPdfDocumentFeatures *feats) {
    if (!is_content_stream(data, len)) {
        return;
    }

    size_t i = 0;
    while (i < len) {
        if (data[i] == '[') {
            i++;
            while (i < len && data[i] != ']') {
                if (data[i] == '(') {
                    i++;
                    size_t start = i;
                    int depth = 1;
                    while (i < len && depth > 0) {
                        if (data[i] == '\\' && i + 1 < len) {
                            i += 2;
                        } else if (data[i] == '(') {
                            depth++;
                            i++;
                        } else if (data[i] == ')') {
                            depth--;
                            if (depth == 0) break;
                            i++;
                        } else {
                            i++;
                        }
                    }
                    if (i >= start) {
                        char temp[2048];
                        size_t out_idx = 0;
                        for (size_t k = start; k < i && out_idx + 1 < sizeof(temp); ++k) {
                            if (data[k] == '\\' && k + 1 < i) {
                                k++;
                                if (data[k] == 'n') temp[out_idx++] = '\n';
                                else if (data[k] == 'r') temp[out_idx++] = '\r';
                                else if (data[k] == 't') temp[out_idx++] = '\t';
                                else temp[out_idx++] = (char)data[k];
                            } else {
                                temp[out_idx++] = (char)data[k];
                            }
                        }
                        temp[out_idx] = '\0';
                        if (out_idx > 0 && text_out) {
                            tsfi_pdf_text_buffer_append(text_out, temp, out_idx);
                        }
                    }
                    if (i < len && data[i] == ')') i++;
                } else if (data[i] == '-' && i + 1 < len && isdigit(data[i+1])) {
                    long kerning = strtol((const char *)(data + i), NULL, 10);
                    if (kerning < -100 && text_out) {
                        tsfi_pdf_text_buffer_append(text_out, " ", 1);
                    }
                    while (i < len && (isdigit(data[i]) || data[i] == '-' || data[i] == '.')) i++;
                } else {
                    i++;
                }
            }
            if (text_out) tsfi_pdf_text_buffer_append(text_out, " ", 1);
            if (i < len && data[i] == ']') i++;
        } else if (data[i] == '(') {
            i++;
            size_t start = i;
            int depth = 1;
            while (i < len && depth > 0) {
                if (data[i] == '\\' && i + 1 < len) {
                    i += 2;
                } else if (data[i] == '(') {
                    depth++;
                    i++;
                } else if (data[i] == ')') {
                    depth--;
                    if (depth == 0) break;
                    i++;
                } else {
                    i++;
                }
            }
            if (i >= start) {
                char temp[2048];
                size_t out_idx = 0;
                for (size_t k = start; k < i && out_idx + 1 < sizeof(temp); ++k) {
                    if (data[k] == '\\' && k + 1 < i) {
                        k++;
                        if (data[k] == 'n') temp[out_idx++] = '\n';
                        else if (data[k] == 'r') temp[out_idx++] = '\r';
                        else if (data[k] == 't') temp[out_idx++] = '\t';
                        else temp[out_idx++] = (char)data[k];
                    } else {
                        temp[out_idx++] = (char)data[k];
                    }
                }
                temp[out_idx] = '\0';
                if (out_idx > 0 && text_out) {
                    tsfi_pdf_text_buffer_append(text_out, temp, out_idx);
                    tsfi_pdf_text_buffer_append(text_out, " ", 1);
                }
            }
            if (i < len && data[i] == ')') i++;
        } else if (data[i] == 'm' || data[i] == 'l' || data[i] == 'c' || data[i] == 'r' || data[i] == 'f' || data[i] == 'S') {
            if (feats) {
                feats->vector_path_count++;
                if (data[i] == 'f' || data[i] == 'c') {
                    feats->chart_element_count++;
                }
            }
            i++;
        } else if (data[i] == 'E' && i + 1 < len && data[i+1] == 'T') {
            if (text_out) tsfi_pdf_text_buffer_append(text_out, "\n", 1);
            i += 2;
        } else {
            i++;
        }
    }
}

TsfiPdfDocumentFeatures *tsfi_pdf_extract_all_features(const char *filepath) {
    if (!filepath) return NULL;
    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    size_t fsize = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize == 0) {
        fclose(f);
        return NULL;
    }

    uint8_t *pdf_data = (uint8_t *)malloc(fsize);
    if (!pdf_data) {
        fclose(f);
        return NULL;
    }

    if (fread(pdf_data, 1, fsize, f) != fsize) {
        free(pdf_data);
        fclose(f);
        return NULL;
    }
    fclose(f);

    TsfiPdfDocumentFeatures *feats = (TsfiPdfDocumentFeatures *)malloc(sizeof(TsfiPdfDocumentFeatures));
    if (!feats) {
        free(pdf_data);
        return NULL;
    }
    memset(feats, 0, sizeof(*feats));
    feats->text_buffer = tsfi_pdf_text_buffer_create();

    size_t pos = 0;
    while (pos < fsize) {
        uint8_t *p = (uint8_t *)memmem_fast(pdf_data + pos, fsize - pos, "stream", 6);
        if (!p) break;

        size_t s_pos = (size_t)(p - pdf_data) + 6;
        while (s_pos < fsize && (pdf_data[s_pos] == '\r' || pdf_data[s_pos] == '\n')) {
            s_pos++;
        }

        uint8_t *end_p = (uint8_t *)memmem_fast(pdf_data + s_pos, fsize - s_pos, "endstream", 9);
        if (!end_p) break;

        size_t stream_len = (size_t)(end_p - (pdf_data + s_pos));
        while (stream_len > 0 && (pdf_data[s_pos + stream_len - 1] == '\r' || pdf_data[s_pos + stream_len - 1] == '\n')) {
            stream_len--;
        }

        feats->total_streams++;

        size_t dict_back = (size_t)(p - pdf_data);
        size_t dict_start = dict_back > 300 ? dict_back - 300 : 0;
        if (memmem_fast(pdf_data + dict_start, dict_back - dict_start, "/Image", 6) != NULL ||
            memmem_fast(pdf_data + dict_start, dict_back - dict_start, "/DCTDecode", 10) != NULL) {
            feats->image_object_count++;
        }

        TsfiPdfByteBuffer decompressed = {0};
        if (tsfi_pdf_zlib_decompress(pdf_data + s_pos, stream_len, &decompressed) == 0 && decompressed.size > 0) {
            parse_multimodal_content_stream(decompressed.data, decompressed.size, feats->text_buffer, feats);
            free(decompressed.data);
        } else {
            parse_multimodal_content_stream(pdf_data + s_pos, stream_len, feats->text_buffer, feats);
        }

        pos = (size_t)(end_p - pdf_data) + 9;
    }

    if (feats->text_buffer) {
        feats->text_bytes_extracted = feats->text_buffer->length;
    }

    free(pdf_data);
    return feats;
}

void tsfi_pdf_document_features_free(TsfiPdfDocumentFeatures *feats) {
    if (!feats) return;
    if (feats->text_buffer) tsfi_pdf_text_buffer_free(feats->text_buffer);
    if (feats->vector_paths) free(feats->vector_paths);
    if (feats->images) free(feats->images);
    free(feats);
}

char *tsfi_pdf_extract_text(const char *filepath, size_t *out_length) {
    TsfiPdfDocumentFeatures *feats = tsfi_pdf_extract_all_features(filepath);
    if (!feats) return NULL;
    if (!feats->text_buffer) {
        tsfi_pdf_document_features_free(feats);
        return NULL;
    }

    if (out_length) *out_length = feats->text_buffer->length;
    char *result = feats->text_buffer->text;
    feats->text_buffer->text = NULL;
    tsfi_pdf_document_features_free(feats);
    return result;
}

#ifdef TSFI_PDF_READER_STANDALONE
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_pdf>\n", argv[0]);
        return 1;
    }
    TsfiPdfDocumentFeatures *feats = tsfi_pdf_extract_all_features(argv[1]);
    if (!feats) {
        fprintf(stderr, "Failed to analyze %s\n", argv[1]);
        return 1;
    }
    printf("=== TSFI2 PDF Multimodal Extraction Proof Report ===\n");
    printf("Document: %s\n", argv[1]);
    printf("Total Compressed/Uncompressed Streams: %zu\n", feats->total_streams);
    printf("Extracted Text Stream Length: %zu bytes\n", feats->text_bytes_extracted);
    printf("Vector Graphics Path Operators: %zu\n", feats->vector_path_count);
    printf("Chart & Diagram Polygonal Elements: %zu\n", feats->chart_element_count);
    printf("Raster Image XObjects: %zu\n", feats->image_object_count);
    printf("====================================================\n");

    tsfi_pdf_document_features_free(feats);
    return 0;
}
#endif
