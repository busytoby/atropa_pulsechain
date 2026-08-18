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
                         const HuffmanTable *lt, const HuffmanTable *dt) {
    while (1) {
        int sym = decode_symbol(br, lt);
        if (sym < 0) return -1;
        if (sym < 256) {
            byte_buf_push(out, (uint8_t)sym);
        } else if (sym == 256) {
            return 0; // End of block
        } else if (sym <= 285) {
            int len_idx = sym - 257;
            int length = LENGTH_BASES[len_idx];
            int extra_bits = LENGTH_EXTRA_BITS[len_idx];
            if (extra_bits > 0) {
                length += (int)read_bits(br, extra_bits);
            }

            int dist_sym = decode_symbol(br, dt);
            if (dist_sym < 0 || dist_sym >= 30) return -1;
            int distance = DIST_BASES[dist_sym];
            int dist_extra = DIST_EXTRA_BITS[dist_sym];
            if (dist_extra > 0) {
                distance += (int)read_bits(br, dist_extra);
            }

            if ((size_t)distance > out->size) return -1;
            size_t start = out->size - (size_t)distance;
            for (int i = 0; i < length; ++i) {
                uint8_t b = out->data[start + i];
                byte_buf_push(out, b);
            }
        } else {
            return -1;
        }
    }
}

int tsfi_pdf_zlib_decompress(const uint8_t *src, size_t src_len, TsfiPdfByteBuffer *out) {
    if (!src || src_len < 2 || !out) return -1;
    
    // Check zlib header (CMF/FLG)
    uint8_t cmf = src[0];
    uint8_t flg = src[1];
    if ((cmf & 0x0F) != 8) {
        return -1;
    }
    if (((cmf * 256 + flg) % 31) != 0) {
        return -1;
    }

    size_t offset = 2;
    if (flg & 0x20) { // FDICT
        if (src_len < 6) return -1;
        offset += 4;
    }

    BitReader br = { src + offset, src_len - offset, 0 };
    int bfinal = 0;

    while (!bfinal) {
        bfinal = (int)read_bits(&br, 1);
        int btype = (int)read_bits(&br, 2);

        if (btype == 0) { // Uncompressed
            br.bit_pos = (br.bit_pos + 7) & ~7ULL; // Byte align
            size_t byte_idx = br.bit_pos >> 3;
            if (byte_idx + 4 > br.src_len) return -1;
            uint16_t len = (uint16_t)(br.src[byte_idx] | (br.src[byte_idx + 1] << 8));
            uint16_t nlen = (uint16_t)(br.src[byte_idx + 2] | (br.src[byte_idx + 3] << 8));
            if ((uint16_t)len != (uint16_t)~nlen) return -1;
            byte_idx += 4;
            if (byte_idx + len > br.src_len) return -1;
            for (uint16_t i = 0; i < len; ++i) {
                byte_buf_push(out, br.src[byte_idx + i]);
            }
            br.bit_pos = (byte_idx + len) << 3;
        } else if (btype == 1) { // Fixed Huffman
            uint8_t l_lens[288];
            for (int i = 0; i <= 143; ++i) l_lens[i] = 8;
            for (int i = 144; i <= 255; ++i) l_lens[i] = 9;
            for (int i = 256; i <= 279; ++i) l_lens[i] = 7;
            for (int i = 280; i <= 287; ++i) l_lens[i] = 8;
            HuffmanTable lt;
            build_huffman_table(&lt, l_lens, 288);

            uint8_t d_lens[32];
            for (int i = 0; i < 32; ++i) d_lens[i] = 5;
            HuffmanTable dt;
            build_huffman_table(&dt, d_lens, 32);

            if (inflate_block(&br, out, &lt, &dt) != 0) return -1;
        } else if (btype == 2) { // Dynamic Huffman
            int hlit = (int)read_bits(&br, 5) + 257;
            int hdist = (int)read_bits(&br, 5) + 1;
            int hclen = (int)read_bits(&br, 4) + 4;

            uint8_t cl_lens[19] = {0};
            for (int i = 0; i < hclen; ++i) {
                cl_lens[CLEN_ORDER[i]] = (uint8_t)read_bits(&br, 3);
            }
            HuffmanTable cl_table;
            build_huffman_table(&cl_table, cl_lens, 19);

            uint8_t total_lens[320];
            int total_codes = hlit + hdist;
            int idx = 0;
            while (idx < total_codes) {
                int sym = decode_symbol(&br, &cl_table);
                if (sym < 0) return -1;
                if (sym < 16) {
                    total_lens[idx++] = (uint8_t)sym;
                } else if (sym == 16) {
                    if (idx == 0) return -1;
                    uint8_t prev = total_lens[idx - 1];
                    int rep = (int)read_bits(&br, 2) + 3;
                    while (rep-- > 0 && idx < total_codes) total_lens[idx++] = prev;
                } else if (sym == 17) {
                    int rep = (int)read_bits(&br, 3) + 3;
                    while (rep-- > 0 && idx < total_codes) total_lens[idx++] = 0;
                } else if (sym == 18) {
                    int rep = (int)read_bits(&br, 7) + 11;
                    while (rep-- > 0 && idx < total_codes) total_lens[idx++] = 0;
                }
            }

            HuffmanTable lt, dt;
            build_huffman_table(&lt, total_lens, hlit);
            build_huffman_table(&dt, total_lens + hlit, hdist);

            if (inflate_block(&br, out, &lt, &dt) != 0) return -1;
        } else {
            return -1;
        }
    }
    return 0;
}

TsfiPdfTextBuffer *tsfi_pdf_text_buffer_create(void) {
    TsfiPdfTextBuffer *buf = (TsfiPdfTextBuffer *)malloc(sizeof(TsfiPdfTextBuffer));
    if (!buf) return NULL;
    buf->capacity = 8192;
    buf->length = 0;
    buf->text = (char *)malloc(buf->capacity);
    if (!buf->text) {
        free(buf);
        return NULL;
    }
    buf->text[0] = '\0';
    return buf;
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

void tsfi_pdf_text_buffer_free(TsfiPdfTextBuffer *buf) {
    if (!buf) return;
    if (buf->text) free(buf->text);
    free(buf);
}

static inline void *memmem_fast(const void *haystack, size_t haystacklen,
                                const void *needle, size_t needlelen) {
    if (needlelen == 0) return (void *)haystack;
    if (haystacklen < needlelen) return NULL;
    const uint8_t *h = (const uint8_t *)haystack;
    const uint8_t *n = (const uint8_t *)needle;
    for (size_t i = 0; i <= haystacklen - needlelen; ++i) {
        if (h[i] == n[0] && memcmp(h + i, n, needlelen) == 0) {
            return (void *)(h + i);
        }
    }
    return NULL;
}

static void parse_multimodal_content_stream(const uint8_t *data, size_t len, 
                                            TsfiPdfTextBuffer *text_out,
                                            TsfiPdfDocumentFeatures *feats) {
    if (!data || len == 0) return;
    
    size_t i = 0;
    while (i < len) {
        if (data[i] == '[') { // Array TJ operator
            i++;
            while (i < len && data[i] != ']') {
                while (i < len && isspace(data[i])) i++;
                if (i >= len || data[i] == ']') break;
                
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
                                if (data[k] >= '0' && data[k] <= '7') {
                                    int oct_val = data[k] - '0';
                                    int count = 1;
                                    while (count < 3 && k + 1 < i && data[k+1] >= '0' && data[k+1] <= '7') {
                                        k++;
                                        oct_val = (oct_val << 3) | (data[k] - '0');
                                        count++;
                                    }
                                    if (oct_val == 033) { temp[out_idx++] = 'f'; temp[out_idx++] = 'f'; }
                                    else if (oct_val == 034) { temp[out_idx++] = 'f'; temp[out_idx++] = 'i'; }
                                    else if (oct_val == 035) { temp[out_idx++] = 'f'; temp[out_idx++] = 'l'; }
                                    else if (oct_val == 036) { temp[out_idx++] = 'f'; temp[out_idx++] = 'f'; temp[out_idx++] = 'i'; }
                                    else if (oct_val == 025 || oct_val == 026) temp[out_idx++] = '-';
                                    else if (oct_val == 050) temp[out_idx++] = '(';
                                    else if (oct_val == 051) temp[out_idx++] = ')';
                                    else if (oct_val == 002) temp[out_idx++] = 'x';
                                    else if (oct_val >= 32 && oct_val <= 126) temp[out_idx++] = (char)oct_val;
                                    else temp[out_idx++] = ' ';
                                } else if (data[k] == 'n') temp[out_idx++] = '\n';
                                else if (data[k] == 'r') temp[out_idx++] = '\r';
                                else if (data[k] == 't') temp[out_idx++] = '\t';
                                else temp[out_idx++] = (char)data[k];
                            } else {
                                if (data[k] >= 32 && data[k] <= 126) {
                                    temp[out_idx++] = (char)data[k];
                                } else if (data[k] == '\n' || data[k] == '\t') {
                                    temp[out_idx++] = (char)data[k];
                                }
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
                        if (data[k] >= '0' && data[k] <= '7') {
                            int oct_val = data[k] - '0';
                            int count = 1;
                            while (count < 3 && k + 1 < i && data[k+1] >= '0' && data[k+1] <= '7') {
                                k++;
                                oct_val = (oct_val << 3) | (data[k] - '0');
                                count++;
                            }
                            if (oct_val == 033) { temp[out_idx++] = 'f'; temp[out_idx++] = 'f'; }
                            else if (oct_val == 034) { temp[out_idx++] = 'f'; temp[out_idx++] = 'i'; }
                            else if (oct_val == 035) { temp[out_idx++] = 'f'; temp[out_idx++] = 'l'; }
                            else if (oct_val == 036) { temp[out_idx++] = 'f'; temp[out_idx++] = 'f'; temp[out_idx++] = 'i'; }
                            else if (oct_val == 025 || oct_val == 026) temp[out_idx++] = '-';
                            else if (oct_val == 050) temp[out_idx++] = '(';
                            else if (oct_val == 051) temp[out_idx++] = ')';
                            else if (oct_val == 002) temp[out_idx++] = 'x';
                            else if (oct_val >= 32 && oct_val <= 126) temp[out_idx++] = (char)oct_val;
                            else temp[out_idx++] = ' ';
                        } else if (data[k] == 'n') temp[out_idx++] = '\n';
                        else if (data[k] == 'r') temp[out_idx++] = '\r';
                        else if (data[k] == 't') temp[out_idx++] = '\t';
                        else temp[out_idx++] = (char)data[k];
                    } else {
                        if (data[k] >= 32 && data[k] <= 126) {
                            temp[out_idx++] = (char)data[k];
                        } else if (data[k] == '\n' || data[k] == '\t') {
                            temp[out_idx++] = (char)data[k];
                        }
                    }
                }
                temp[out_idx] = '\0';
                if (out_idx > 0 && text_out) {
                    tsfi_pdf_text_buffer_append(text_out, temp, out_idx);
                    if (out_idx > 1 && temp[out_idx - 1] != ' ') {
                        tsfi_pdf_text_buffer_append(text_out, " ", 1);
                    }
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
        size_t dict_start = dict_back > 500 ? dict_back - 500 : 0;
        size_t dict_len = dict_back - dict_start;

        bool is_image = (memmem_fast(pdf_data + dict_start, dict_len, "/Image", 6) != NULL ||
                         memmem_fast(pdf_data + dict_start, dict_len, "/DCTDecode", 10) != NULL);
        if (is_image) {
            feats->image_object_count++;
        }

        bool is_font_or_meta = (memmem_fast(pdf_data + dict_start, dict_len, "/FontFile", 9) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/FontFile2", 10) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/FontFile3", 10) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Type/XRef", 10) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Type /XRef", 11) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Type/ObjStm", 12) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Type /ObjStm", 13) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Type/Font", 10) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Type /Font", 11) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Subtype/Type1C", 15) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Subtype /Type1C", 16) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Subtype/CIDFontType0C", 22) != NULL ||
                                memmem_fast(pdf_data + dict_start, dict_len, "/Subtype /CIDFontType0C", 23) != NULL ||
                                is_image);

        TsfiPdfByteBuffer decompressed = {0};
        if (tsfi_pdf_zlib_decompress(pdf_data + s_pos, stream_len, &decompressed) == 0 && decompressed.size > 0) {
            bool is_decomp_font = (memmem_fast(decompressed.data, decompressed.size < 256 ? decompressed.size : 256, "%!PS-AdobeFont", 14) != NULL ||
                                   memmem_fast(decompressed.data, decompressed.size < 256 ? decompressed.size : 256, "%!FontType1", 10) != NULL ||
                                   memmem_fast(decompressed.data, decompressed.size < 256 ? decompressed.size : 256, "FontDirectory", 13) != NULL ||
                                   memmem_fast(decompressed.data, decompressed.size < 256 ? decompressed.size : 256, "CIDInit", 7) != NULL);
            if (!is_font_or_meta && !is_decomp_font) {
                parse_multimodal_content_stream(decompressed.data, decompressed.size, feats->text_buffer, feats);
            }
            free(decompressed.data);
        } else {
            bool is_raw_font = (memmem_fast(pdf_data + s_pos, stream_len < 256 ? stream_len : 256, "%!PS-AdobeFont", 14) != NULL ||
                                memmem_fast(pdf_data + s_pos, stream_len < 256 ? stream_len : 256, "CIDInit", 7) != NULL);
            if (!is_font_or_meta && !is_raw_font) {
                parse_multimodal_content_stream(pdf_data + s_pos, stream_len, feats->text_buffer, feats);
            }
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

    char *res = NULL;
    if (feats->text_buffer->length > 0) {
        res = (char *)malloc(feats->text_buffer->length + 1);
        if (res) {
            memcpy(res, feats->text_buffer->text, feats->text_buffer->length);
            res[feats->text_buffer->length] = '\0';
            if (out_length) *out_length = feats->text_buffer->length;
        }
    }
    tsfi_pdf_document_features_free(feats);
    return res;
}
