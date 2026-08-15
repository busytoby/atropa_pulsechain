#define _GNU_SOURCE
#include "tsfi_web_security_storage.h"
#include "tsfi_dom.h"
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

// -----------------------------------------------------------------------------
// 1. Live HTTPS URL Navigation & Vulkan Paint Pipeline
// -----------------------------------------------------------------------------
void tsfi_browser_nav_init(TsfiBrowserNavigationState *state) {
    if (!state) return;
    memset(state, 0, sizeof(TsfiBrowserNavigationState));
}

bool tsfi_browser_navigate_url(TsfiBrowserNavigationState *state, const char *url_str, int viewport_w, int viewport_h, uint32_t *out_pixels) {
    if (!state || !url_str || !out_pixels || viewport_w <= 0 || viewport_h <= 0) return false;

    strncpy(state->current_url, url_str, sizeof(state->current_url) - 1);
    state->is_rendering = false;

    if (state->active_layout) {
        tsfi_layout_box_free(state->active_layout);
        state->active_layout = NULL;
    }
    if (state->active_dom) {
        tsfi_node_free(state->active_dom);
        state->active_dom = NULL;
    }

    // 1. Download HTML over pure C HTTPS
    TsfiHttpResponse resp;
    if (!tsfi_http_get(url_str, &resp)) {
        state->last_http_status = 502; // Bad Gateway
        return false;
    }

    state->last_http_status = resp.status_code;
    state->page_bytes_downloaded = resp.body_len;

    // 2. Parse DOM & Solve Layout Box Tree
    const char *html_source = resp.body ? resp.body : "<div>Empty Page</div>";
    state->active_dom = tsfi_dom_parse_html(html_source);
    if (!state->active_dom) {
        tsfi_http_response_free(&resp);
        return false;
    }

    state->active_layout = tsfi_layout_tree_build(state->active_dom);
    if (!state->active_layout) {
        tsfi_http_response_free(&resp);
        return false;
    }

    tsfi_layout_solve(state->active_layout, 0.0f, 0.0f, (float)viewport_w, (float)viewport_h);

    // 3. Rasterize Layout to Vulkan Scanout Framebuffer
    tsfi_paint_tree(state->active_layout, out_pixels, viewport_w, viewport_h);
    state->is_rendering = true;

    tsfi_http_response_free(&resp);
    return true;
}

void tsfi_browser_nav_free(TsfiBrowserNavigationState *state) {
    if (!state) return;
    if (state->active_layout) {
        tsfi_layout_box_free(state->active_layout);
        state->active_layout = NULL;
    }
    if (state->active_dom) {
        tsfi_node_free(state->active_dom);
        state->active_dom = NULL;
    }
    state->is_rendering = false;
}

// -----------------------------------------------------------------------------
// 2. Form Data Submission & Interactive URL-Encoded POST & Redirects
// -----------------------------------------------------------------------------
void tsfi_form_init(TsfiHtmlForm *form, const char *action_url, const char *method) {
    if (!form) return;
    memset(form, 0, sizeof(TsfiHtmlForm));
    if (action_url) strncpy(form->action_url, action_url, sizeof(form->action_url) - 1);
    if (method) strncpy(form->method, method, sizeof(form->method) - 1);
    else strncpy(form->method, "POST", sizeof(form->method) - 1);
}

bool tsfi_form_add_field(TsfiHtmlForm *form, const char *key, const char *value) {
    if (!form || !key || !value || form->count >= 32) return false;
    strncpy(form->fields[form->count].key, key, sizeof(form->fields[0].key) - 1);
    strncpy(form->fields[form->count].value, value, sizeof(form->fields[0].value) - 1);
    form->count++;
    return true;
}

size_t tsfi_form_encode_url_encoded(const TsfiHtmlForm *form, char *out_buf, size_t max_len) {
    if (!form || !out_buf || max_len == 0) return 0;
    out_buf[0] = '\0';
    size_t offset = 0;

    for (int i = 0; i < form->count; i++) {
        if (offset > 0 && offset < max_len - 1) {
            out_buf[offset++] = '&';
            out_buf[offset] = '\0';
        }
        offset += snprintf(out_buf + offset, max_len - offset, "%s=%s", form->fields[i].key, form->fields[i].value);
    }
    return offset;
}

bool tsfi_form_submit(const TsfiHtmlForm *form, TsfiHttpResponse *response_out, int max_redirects) {
    if (!form || !response_out) return false;
    char encoded_payload[2048];
    size_t payload_len = tsfi_form_encode_url_encoded(form, encoded_payload, sizeof(encoded_payload));

    char target_url[1024];
    strncpy(target_url, form->action_url, sizeof(target_url) - 1);

    for (int redir = 0; redir <= max_redirects; redir++) {
        bool ok = false;
        if (strcasecmp(form->method, "GET") == 0) {
            char full_get_url[4096];
            snprintf(full_get_url, sizeof(full_get_url), "%s?%s", target_url, encoded_payload);
            ok = tsfi_http_get(full_get_url, response_out);
        } else {

            ok = tsfi_http_post(target_url, "application/x-www-form-urlencoded", encoded_payload, payload_len, response_out);
        }

        if (!ok) return false;

        // Check for 301/302/303 Redirects
        if (response_out->status_code >= 301 && response_out->status_code <= 303) {
            bool found_loc = false;
            for (int h = 0; h < response_out->header_count; h++) {
                if (strcasecmp(response_out->headers[h].name, "Location") == 0) {
                    strncpy(target_url, response_out->headers[h].value, sizeof(target_url) - 1);
                    found_loc = true;
                    break;
                }
            }
            tsfi_http_response_free(response_out);
            if (!found_loc) return false;
        } else {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// 3. CORS & Content Security Policy (CSP) Policy Engine
// -----------------------------------------------------------------------------
void tsfi_security_policy_init(TsfiSecurityPolicy *policy, const char *origin) {
    if (!policy) return;
    memset(policy, 0, sizeof(TsfiSecurityPolicy));
    if (origin) strncpy(policy->origin, origin, sizeof(policy->origin) - 1);
    strncpy(policy->allowed_methods, "GET, POST, OPTIONS", sizeof(policy->allowed_methods) - 1);
    strncpy(policy->csp_default_src, "'self'", sizeof(policy->csp_default_src) - 1);
}

bool tsfi_security_parse_csp_header(TsfiSecurityPolicy *policy, const char *csp_header_val) {
    if (!policy || !csp_header_val) return false;
    char buf[1024];
    strncpy(buf, csp_header_val, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *directive = strtok(buf, ";");
    while (directive) {
        while (*directive == ' ') directive++;
        if (strncmp(directive, "default-src", 11) == 0) {
            strncpy(policy->csp_default_src, directive + 11, sizeof(policy->csp_default_src) - 1);
        } else if (strncmp(directive, "script-src", 10) == 0) {
            strncpy(policy->csp_script_src, directive + 10, sizeof(policy->csp_script_src) - 1);
        } else if (strncmp(directive, "img-src", 7) == 0) {
            strncpy(policy->csp_img_src, directive + 7, sizeof(policy->csp_img_src) - 1);
        }
        directive = strtok(NULL, ";");
    }
    return true;
}

bool tsfi_security_verify_cors_request(const TsfiSecurityPolicy *policy, const char *request_origin, const char *method) {
    if (!policy || !request_origin || !method) return false;
    if (policy->allow_all_origins || strcmp(policy->origin, "*") == 0) return true;
    if (strcmp(policy->origin, request_origin) == 0) return true;
    return false;
}

bool tsfi_security_check_csp_resource(const TsfiSecurityPolicy *policy, const char *resource_type, const char *target_domain) {
    if (!policy || !resource_type || !target_domain) return false;

    const char *rule = policy->csp_default_src;
    if (strcasecmp(resource_type, "script") == 0 && strlen(policy->csp_script_src) > 0) {
        rule = policy->csp_script_src;
    } else if (strcasecmp(resource_type, "img") == 0 && strlen(policy->csp_img_src) > 0) {
        rule = policy->csp_img_src;
    }

    if (strstr(rule, "*") != NULL) return true;
    if (strstr(rule, "'self'") != NULL && strstr(target_domain, policy->origin) != NULL) return true;
    if (strstr(rule, target_domain) != NULL) return true;
    return false;
}

// -----------------------------------------------------------------------------
// 4. DOM LocalStorage Key-Value Store
// -----------------------------------------------------------------------------
void tsfi_local_storage_init(TsfiLocalStorage *store, const char *origin) {
    if (!store) return;
    memset(store, 0, sizeof(TsfiLocalStorage));
    if (origin) strncpy(store->origin, origin, sizeof(store->origin) - 1);
}

bool tsfi_local_storage_set_item(TsfiLocalStorage *store, const char *key, const char *val) {
    if (!store || !key || !val || store->count >= MAX_STORAGE_ITEMS) return false;

    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->items[i].key, key) == 0) {
            strncpy(store->items[i].value, val, sizeof(store->items[i].value) - 1);
            return true;
        }
    }

    strncpy(store->items[store->count].key, key, sizeof(store->items[0].key) - 1);
    strncpy(store->items[store->count].value, val, sizeof(store->items[0].value) - 1);
    store->count++;
    return true;
}

const char* tsfi_local_storage_get_item(const TsfiLocalStorage *store, const char *key) {
    if (!store || !key) return NULL;
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->items[i].key, key) == 0) {
            return store->items[i].value;
        }
    }
    return NULL;
}

bool tsfi_local_storage_remove_item(TsfiLocalStorage *store, const char *key) {
    if (!store || !key) return false;
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->items[i].key, key) == 0) {
            for (int j = i; j < store->count - 1; j++) {
                store->items[j] = store->items[j + 1];
            }
            store->count--;
            return true;
        }
    }
    return false;
}

void tsfi_local_storage_clear(TsfiLocalStorage *store) {
    if (!store) return;
    store->count = 0;
}

bool tsfi_local_storage_save_dat_bin(const TsfiLocalStorage *store, const char *filepath) {
    if (!store || !filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;

    uint32_t magic = 0x4C535452; // 'L''S''T''R'
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(store->origin, 1, sizeof(store->origin), f);
    fwrite(&store->count, sizeof(int), 1, f);
    fwrite(store->items, sizeof(TsfiStorageItem), (size_t)store->count, f);
    fclose(f);
    return true;
}

bool tsfi_local_storage_load_dat_bin(TsfiLocalStorage *store, const char *filepath) {
    if (!store || !filepath) return false;
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    uint32_t magic = 0;
    if (fread(&magic, sizeof(magic), 1, f) != 1 || magic != 0x4C535452) {
        fclose(f);
        return false;
    }

    if (fread(store->origin, 1, sizeof(store->origin), f) != sizeof(store->origin)) {
        fclose(f);
        return false;
    }

    if (fread(&store->count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return false;
    }

    if (store->count > MAX_STORAGE_ITEMS) store->count = MAX_STORAGE_ITEMS;
    size_t rd = fread(store->items, sizeof(TsfiStorageItem), (size_t)store->count, f);
    fclose(f);
    return (rd == (size_t)store->count);
}
