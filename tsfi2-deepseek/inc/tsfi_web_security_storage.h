#ifndef TSFI_WEB_SECURITY_STORAGE_H
#define TSFI_WEB_SECURITY_STORAGE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "tsfi_http_client.h"
#include "tsfi_layout.h"

#define MAX_STORAGE_ITEMS 128
#define MAX_STORAGE_KEY_LEN 64
#define MAX_STORAGE_VAL_LEN 512

// -----------------------------------------------------------------------------
// 1. Live HTTPS URL Navigation & Vulkan Paint Pipeline
// -----------------------------------------------------------------------------
typedef struct {
    char current_url[1024];
    int last_http_status;
    size_t page_bytes_downloaded;
    bool is_rendering;
    TsfiNode *active_dom;
    TsfiLayoutBox *active_layout;
} TsfiBrowserNavigationState;

void tsfi_browser_nav_init(TsfiBrowserNavigationState *state);
bool tsfi_browser_navigate_url(TsfiBrowserNavigationState *state, const char *url_str, int viewport_w, int viewport_h, uint32_t *out_pixels);
void tsfi_browser_nav_free(TsfiBrowserNavigationState *state);

// -----------------------------------------------------------------------------
// 2. Form Data Submission & Interactive URL-Encoded POST & Redirects
// -----------------------------------------------------------------------------
typedef struct {
    char key[64];
    char value[256];
} TsfiFormField;

typedef struct {
    TsfiFormField fields[32];
    int count;
    char action_url[512];
    char method[16]; // "GET" or "POST"
} TsfiHtmlForm;

void tsfi_form_init(TsfiHtmlForm *form, const char *action_url, const char *method);
bool tsfi_form_add_field(TsfiHtmlForm *form, const char *key, const char *value);
size_t tsfi_form_encode_url_encoded(const TsfiHtmlForm *form, char *out_buf, size_t max_len);
bool tsfi_form_submit(const TsfiHtmlForm *form, TsfiHttpResponse *response_out, int max_redirects);

// -----------------------------------------------------------------------------
// 3. CORS & Content Security Policy (CSP) Policy Engine
// -----------------------------------------------------------------------------
typedef struct {
    char origin[256];
    bool allow_all_origins;
    char allowed_methods[128];
    char csp_default_src[128];
    char csp_script_src[128];
    char csp_img_src[128];
} TsfiSecurityPolicy;

void tsfi_security_policy_init(TsfiSecurityPolicy *policy, const char *origin);
bool tsfi_security_parse_csp_header(TsfiSecurityPolicy *policy, const char *csp_header_val);
bool tsfi_security_verify_cors_request(const TsfiSecurityPolicy *policy, const char *request_origin, const char *method);
bool tsfi_security_check_csp_resource(const TsfiSecurityPolicy *policy, const char *resource_type, const char *target_domain);

// -----------------------------------------------------------------------------
// 4. DOM LocalStorage & SessionStorage Key-Value Store
// -----------------------------------------------------------------------------
typedef struct {
    char key[MAX_STORAGE_KEY_LEN];
    char value[MAX_STORAGE_VAL_LEN];
} TsfiStorageItem;

typedef struct {
    char origin[256];
    TsfiStorageItem items[MAX_STORAGE_ITEMS];
    int count;
} TsfiLocalStorage;

void tsfi_local_storage_init(TsfiLocalStorage *store, const char *origin);
bool tsfi_local_storage_set_item(TsfiLocalStorage *store, const char *key, const char *val);
const char* tsfi_local_storage_get_item(const TsfiLocalStorage *store, const char *key);
bool tsfi_local_storage_remove_item(TsfiLocalStorage *store, const char *key);
void tsfi_local_storage_clear(TsfiLocalStorage *store);
bool tsfi_local_storage_save_dat_bin(const TsfiLocalStorage *store, const char *filepath);
bool tsfi_local_storage_load_dat_bin(TsfiLocalStorage *store, const char *filepath);

#endif // TSFI_WEB_SECURITY_STORAGE_H
