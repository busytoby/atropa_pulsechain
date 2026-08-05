#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_ef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static char* decompress_gzip(const unsigned char *src, size_t src_len, size_t *out_len) {
    if (!src || src_len == 0 || !out_len) return NULL;
    
    // Estimate uncompressed size (e.g. 5x compression ratio)
    size_t dest_capacity = src_len * 5 + 4096;
    unsigned char *dest = malloc(dest_capacity);
    if (!dest) return NULL;
    
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    strm.next_in = (Bytef *)src;
    strm.avail_in = src_len;
    strm.next_out = (Bytef *)dest;
    strm.avail_out = dest_capacity;
    
    // 16 + MAX_WBITS enables gzip decoding
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) {
        free(dest);
        return NULL;
    }
    
    int ret;
    while (1) {
        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            break;
        }
        if (ret == Z_OK && strm.avail_out == 0) {
            // Need more space
            size_t offset = strm.next_out - dest;
            dest_capacity *= 2;
            unsigned char *new_dest = realloc(dest, dest_capacity);
            if (!new_dest) {
                inflateEnd(&strm);
                free(dest);
                return NULL;
            }
            dest = new_dest;
            strm.next_out = dest + offset;
            strm.avail_out = dest_capacity - offset;
        } else if (ret != Z_OK) {
            inflateEnd(&strm);
            free(dest);
            return NULL;
        }
    }
    
    inflateEnd(&strm);
    *out_len = strm.total_out;
    dest[*out_len] = '\0';
    return (char *)dest;
}

static char* extract_json_value(const char *json, const char *key) {
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    const char *ptr = strstr(json, search_key);
    if (!ptr) return NULL;
    
    // Look for colon
    ptr = strchr(ptr + strlen(search_key), ':');
    if (!ptr) return NULL;
    ptr++;
    
    // Skip spaces
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') ptr++;
    
    if (*ptr == '"') {
        ptr++;
        const char *end = strchr(ptr, '"');
        if (!end) return NULL;
        size_t len = end - ptr;
        char *val = malloc(len + 1);
        if (val) {
            memcpy(val, ptr, len);
            val[len] = '\0';
        }
        return val;
    } else {
        // Numeric value
        const char *end = ptr;
        while (*end >= '0' && *end <= '9') end++;
        size_t len = end - ptr;
        if (len == 0) return NULL;
        char *val = malloc(len + 1);
        if (val) {
            memcpy(val, ptr, len);
            val[len] = '\0';
        }
        return val;
    }
}

bool htrc_parse_extracted_features(const unsigned char *gz_data, size_t gz_len, HtrcVolumeFeatures *volume) {
    if (!gz_data || gz_len == 0 || !volume) return false;
    
    size_t uncompressed_len = 0;
    char *json = decompress_gzip(gz_data, gz_len, &uncompressed_len);
    if (!json) {
        return false;
    }
    
    memset(volume, 0, sizeof(HtrcVolumeFeatures));
    
    // Extract metadata
    volume->volume_id = extract_json_value(json, "id");
    volume->title = extract_json_value(json, "title");
    volume->author = extract_json_value(json, "author");
    volume->pub_date = extract_json_value(json, "pubDate");
    volume->language = extract_json_value(json, "language");
    
    // Simple parser for page features array
    volume->page_capacity = 16;
    volume->pages = malloc(volume->page_capacity * sizeof(HtrcPageFeature));
    volume->page_count = 0;
    
    const char *pages_ptr = strstr(json, "\"pages\"");
    if (pages_ptr) {
        pages_ptr = strchr(pages_ptr, '[');
        if (pages_ptr) {
            pages_ptr++;
            while (1) {
                // Find next page block
                const char *page_start = strstr(pages_ptr, "\"seq\"");
                if (!page_start) break;
                
                // Check if page_start is outside page array (e.g. past closing bracket)
                // In a simple scanner, we just extract seq and token counts
                char *seq_val = extract_json_value(page_start, "seq");
                if (!seq_val) break;
                
                if (volume->page_count >= volume->page_capacity) {
                    volume->page_capacity *= 2;
                    volume->pages = realloc(volume->pages, volume->page_capacity * sizeof(HtrcPageFeature));
                }
                
                HtrcPageFeature *page = &volume->pages[volume->page_count];
                page->page_number = atoi(seq_val);
                free(seq_val);
                
                // Try to find token count for the page
                char *token_count_val = extract_json_value(page_start, "tokenCount");
                if (token_count_val) {
                    page->token_count = atoi(token_count_val);
                    free(token_count_val);
                } else {
                    page->token_count = 0;
                }
                
                page->tokens = NULL;
                page->token_capacity = 0;
                
                volume->page_count++;
                
                // Move pointer past this page entry
                const char *next_comma = strchr(page_start, '}');
                if (!next_comma) break;
                pages_ptr = next_comma + 1;
            }
        }
    }
    
    free(json);
    return true;
}

void htrc_free_volume_features(HtrcVolumeFeatures *volume) {
    if (!volume) return;
    free(volume->volume_id);
    free(volume->title);
    free(volume->author);
    free(volume->pub_date);
    free(volume->language);
    
    if (volume->pages) {
        for (int i = 0; i < volume->page_count; i++) {
            HtrcPageFeature *page = &volume->pages[i];
            if (page->tokens) {
                for (int j = 0; j < page->token_count; j++) {
                    free(page->tokens[j].token);
                    free(page->tokens[j].pos_tag);
                }
                free(page->tokens);
            }
        }
        free(volume->pages);
    }
    memset(volume, 0, sizeof(HtrcVolumeFeatures));
}
