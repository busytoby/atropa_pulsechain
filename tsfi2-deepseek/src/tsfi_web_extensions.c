#define _GNU_SOURCE
#include "tsfi_web_extensions.h"
#include "tsfi_http_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

// -----------------------------------------------------------------------------
// 1. Pure C Cookie Jar Implementation
// -----------------------------------------------------------------------------
void tsfi_cookie_jar_init(TsfiCookieJar *jar) {
    if (!jar) return;
    memset(jar, 0, sizeof(TsfiCookieJar));
}

bool tsfi_cookie_jar_parse_set_cookie(TsfiCookieJar *jar, const char *set_cookie_str, const char *default_domain) {
    if (!jar || !set_cookie_str || jar->count >= MAX_COOKIES) return false;

    char buf[1024];
    strncpy(buf, set_cookie_str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    TsfiCookie cookie;
    memset(&cookie, 0, sizeof(TsfiCookie));
    strncpy(cookie.path, "/", sizeof(cookie.path) - 1);
    if (default_domain) strncpy(cookie.domain, default_domain, sizeof(cookie.domain) - 1);

    char *token = strtok(buf, ";");
    if (!token) return false;

    char *eq = strchr(token, '=');
    if (!eq) return false;

    size_t n_len = (size_t)(eq - token);
    if (n_len >= sizeof(cookie.name)) n_len = sizeof(cookie.name) - 1;
    strncpy(cookie.name, token, n_len);

    char *val = eq + 1;
    while (*val == ' ') val++;
    strncpy(cookie.value, val, sizeof(cookie.value) - 1);

    while ((token = strtok(NULL, ";")) != NULL) {
        while (*token == ' ') token++;
        char *attr_eq = strchr(token, '=');
        if (attr_eq) {
            *attr_eq = '\0';
            char *attr_name = token;
            char *attr_val = attr_eq + 1;
            while (*attr_val == ' ') attr_val++;

            if (strcasecmp(attr_name, "Domain") == 0) {
                strncpy(cookie.domain, attr_val, sizeof(cookie.domain) - 1);
            } else if (strcasecmp(attr_name, "Path") == 0) {
                strncpy(cookie.path, attr_val, sizeof(cookie.path) - 1);
            } else if (strcasecmp(attr_name, "Max-Age") == 0) {
                cookie.expires_at = time(NULL) + atol(attr_val);
            } else if (strcasecmp(attr_name, "SameSite") == 0) {
                if (strcasecmp(attr_val, "Strict") == 0) cookie.samesite = 2;
                else if (strcasecmp(attr_val, "Lax") == 0) cookie.samesite = 1;
            }
        } else {
            if (strcasecmp(token, "Secure") == 0) cookie.is_secure = true;
            else if (strcasecmp(token, "HttpOnly") == 0) cookie.is_httponly = true;
        }
    }

    // Update existing cookie or append
    for (int i = 0; i < jar->count; i++) {
        if (strcmp(jar->cookies[i].name, cookie.name) == 0 &&
            strcmp(jar->cookies[i].domain, cookie.domain) == 0) {
            jar->cookies[i] = cookie;
            return true;
        }
    }

    jar->cookies[jar->count++] = cookie;
    return true;
}

int tsfi_cookie_jar_format_cookie_header(const TsfiCookieJar *jar, const char *domain, const char *path, bool is_secure, char *out_buf, size_t max_len) {
    if (!jar || !domain || !out_buf || max_len == 0) return 0;
    out_buf[0] = '\0';

    size_t offset = 0;
    time_t now = time(NULL);

    for (int i = 0; i < jar->count; i++) {
        const TsfiCookie *c = &jar->cookies[i];
        if (c->expires_at > 0 && c->expires_at < now) continue;
        if (c->is_secure && !is_secure) continue;
        if (strstr(domain, c->domain) == NULL) continue;
        if (path && strncmp(path, c->path, strlen(c->path)) != 0) continue;

        if (offset > 0 && offset < max_len - 2) {
            offset += snprintf(out_buf + offset, max_len - offset, "; ");
        }
        offset += snprintf(out_buf + offset, max_len - offset, "%s=%s", c->name, c->value);
    }
    return (int)offset;
}

bool tsfi_cookie_jar_save_dat_bin(const TsfiCookieJar *jar, const char *filepath) {
    if (!jar || !filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;

    uint32_t magic = 0x434F4F4B; // 'C''O''O''K'
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(&jar->count, sizeof(int), 1, f);
    fwrite(jar->cookies, sizeof(TsfiCookie), (size_t)jar->count, f);
    fclose(f);
    return true;
}

bool tsfi_cookie_jar_load_dat_bin(TsfiCookieJar *jar, const char *filepath) {
    if (!jar || !filepath) return false;
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    uint32_t magic = 0;
    if (fread(&magic, sizeof(magic), 1, f) != 1 || magic != 0x434F4F4B) {
        fclose(f);
        return false;
    }

    if (fread(&jar->count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return false;
    }

    if (jar->count > MAX_COOKIES) jar->count = MAX_COOKIES;
    size_t rd = fread(jar->cookies, sizeof(TsfiCookie), (size_t)jar->count, f);
    fclose(f);
    return (rd == (size_t)jar->count);
}

// -----------------------------------------------------------------------------
// 2. HTTP Caching Engine
// -----------------------------------------------------------------------------
void tsfi_http_cache_init(TsfiHttpCache *cache) {
    if (!cache) return;
    memset(cache, 0, sizeof(TsfiHttpCache));
}

bool tsfi_http_cache_put(TsfiHttpCache *cache, const char *url, const char *etag, const char *last_modified, int max_age_sec, const uint8_t *data, size_t len) {
    if (!cache || !url || !data || len == 0 || cache->count >= MAX_CACHE_ENTRIES) return false;

    for (int i = 0; i < cache->count; i++) {
        if (strcmp(cache->entries[i].url, url) == 0) {
            free(cache->entries[i].data);
            cache->entries[i].data = (uint8_t *)malloc(len);
            if (!cache->entries[i].data) return false;
            memcpy(cache->entries[i].data, data, len);
            cache->entries[i].data_len = len;
            if (etag) strncpy(cache->entries[i].etag, etag, sizeof(cache->entries[i].etag) - 1);
            if (last_modified) strncpy(cache->entries[i].last_modified, last_modified, sizeof(cache->entries[i].last_modified) - 1);
            cache->entries[i].expires_at = (max_age_sec > 0) ? (time(NULL) + max_age_sec) : 0;
            return true;
        }
    }

    TsfiCacheEntry *entry = &cache->entries[cache->count];
    memset(entry, 0, sizeof(TsfiCacheEntry));
    strncpy(entry->url, url, sizeof(entry->url) - 1);
    if (etag) strncpy(entry->etag, etag, sizeof(entry->etag) - 1);
    if (last_modified) strncpy(entry->last_modified, last_modified, sizeof(entry->last_modified) - 1);
    entry->expires_at = (max_age_sec > 0) ? (time(NULL) + max_age_sec) : 0;

    entry->data = (uint8_t *)malloc(len);
    if (!entry->data) return false;
    memcpy(entry->data, data, len);
    entry->data_len = len;
    cache->count++;
    return true;
}

const TsfiCacheEntry* tsfi_http_cache_get(const TsfiHttpCache *cache, const char *url) {
    if (!cache || !url) return NULL;
    time_t now = time(NULL);
    for (int i = 0; i < cache->count; i++) {
        if (strcmp(cache->entries[i].url, url) == 0) {
            if (cache->entries[i].expires_at > 0 && cache->entries[i].expires_at < now) {
                return NULL; // Expired
            }
            return &cache->entries[i];
        }
    }
    return NULL;
}

void tsfi_http_cache_free(TsfiHttpCache *cache) {
    if (!cache) return;
    for (int i = 0; i < cache->count; i++) {
        if (cache->entries[i].data) {
            free(cache->entries[i].data);
            cache->entries[i].data = NULL;
        }
    }
    cache->count = 0;
}

// -----------------------------------------------------------------------------
// 3. Server-Sent Events (SSE) Stream Decoder
// -----------------------------------------------------------------------------
void tsfi_sse_decoder_init(TsfiSseDecoder *decoder, TsfiSseCallback cb, void *user_data) {
    if (!decoder) return;
    memset(decoder, 0, sizeof(TsfiSseDecoder));
    decoder->on_message = cb;
    decoder->user_data = user_data;
}

size_t tsfi_sse_decode_chunk(TsfiSseDecoder *decoder, const char *chunk, size_t chunk_len) {
    if (!decoder || !chunk || chunk_len == 0) return 0;

    size_t processed = 0;
    for (size_t i = 0; i < chunk_len; i++) {
        char c = chunk[i];
        processed++;

        if (c == '\n') {
            decoder->line_buffer[decoder->line_len] = '\0';
            if (decoder->line_len == 0) {
                // Dispatch event on empty line
                if (decoder->current_msg.data_len > 0) {
                    if (decoder->on_message) {
                        decoder->on_message(&decoder->current_msg, decoder->user_data);
                    }
                    memset(&decoder->current_msg, 0, sizeof(TsfiSseMessage));
                }
            } else {
                // Parse field: value
                if (strncmp(decoder->line_buffer, "data:", 5) == 0) {
                    const char *val = decoder->line_buffer + 5;
                    while (*val == ' ') val++;
                    size_t v_len = strlen(val);
                    if (decoder->current_msg.data_len + v_len < MAX_SSE_BUFFER) {
                        memcpy(decoder->current_msg.data + decoder->current_msg.data_len, val, v_len);
                        decoder->current_msg.data_len += v_len;
                    }
                } else if (strncmp(decoder->line_buffer, "event:", 6) == 0) {
                    const char *val = decoder->line_buffer + 6;
                    while (*val == ' ') val++;
                    strncpy(decoder->current_msg.event_name, val, sizeof(decoder->current_msg.event_name) - 1);
                } else if (strncmp(decoder->line_buffer, "id:", 3) == 0) {
                    const char *val = decoder->line_buffer + 3;
                    while (*val == ' ') val++;
                    strncpy(decoder->current_msg.event_id, val, sizeof(decoder->current_msg.event_id) - 1);
                }
            }
            decoder->line_len = 0;
        } else if (c != '\r') {
            if (decoder->line_len < MAX_SSE_BUFFER - 1) {
                decoder->line_buffer[decoder->line_len++] = c;
            }
        }
    }
    return processed;
}

// -----------------------------------------------------------------------------
// 4. Asynchronous Multi-Asset Fetcher
// -----------------------------------------------------------------------------
void tsfi_asset_queue_init(TsfiAssetQueue *queue) {
    if (!queue) return;
    memset(queue, 0, sizeof(TsfiAssetQueue));
}

bool tsfi_asset_queue_enqueue(TsfiAssetQueue *queue, const char *url, const char *asset_type) {
    if (!queue || !url || queue->count >= 32) return false;
    TsfiAssetTask *task = &queue->tasks[queue->count++];
    memset(task, 0, sizeof(TsfiAssetTask));
    strncpy(task->url, url, sizeof(task->url) - 1);
    strncpy(task->asset_type, asset_type ? asset_type : "unknown", sizeof(task->asset_type) - 1);
    task->is_completed = false;
    return true;
}

int tsfi_asset_queue_process_all(TsfiAssetQueue *queue) {
    if (!queue) return 0;
    int success_count = 0;

    for (int i = 0; i < queue->count; i++) {
        TsfiAssetTask *task = &queue->tasks[i];
        if (task->is_completed) {
            success_count++;
            continue;
        }

        TsfiHttpResponse resp;
        if (tsfi_http_get(task->url, &resp)) {
            task->data = (uint8_t *)resp.body;
            task->data_len = resp.body_len;
            task->is_completed = true;
            success_count++;
            // Note: Transfer ownership of body to task.data, do not free resp.body here
        }
    }
    return success_count;
}

void tsfi_asset_queue_free(TsfiAssetQueue *queue) {
    if (!queue) return;
    for (int i = 0; i < queue->count; i++) {
        if (queue->tasks[i].data) {
            free(queue->tasks[i].data);
            queue->tasks[i].data = NULL;
        }
    }
    queue->count = 0;
}
