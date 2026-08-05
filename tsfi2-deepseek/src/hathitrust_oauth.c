#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_oauth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

static void oauth_url_encode(const char *src, char *dest, size_t dest_len) {
    size_t i = 0;
    while (*src && i < dest_len - 4) {
        unsigned char c = (unsigned char)*src;
        if (isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~') {
            dest[i++] = c;
        } else {
            snprintf(dest + i, dest_len - i, "%%%02X", c);
            i += 3;
        }
        src++;
    }
    dest[i] = '\0';
}

int hathitrust_oauth_generate_header(
    const char *method,
    const char *url,
    const char *consumer_key,
    const char *consumer_secret,
    const char *token,
    const char *token_secret,
    const char *nonce,
    const char *timestamp,
    char *out_header,
    size_t out_header_len
) {
    if (!method || !url || !consumer_key || !consumer_secret || !nonce || !timestamp || !out_header) {
        return 0;
    }
    
    // Sort parameters alphabetically
    // Parameters:
    // oauth_consumer_key, oauth_nonce, oauth_signature_method, oauth_timestamp, oauth_token, oauth_version
    char enc_key[256], enc_nonce[256], enc_token[256] = "";
    oauth_url_encode(consumer_key, enc_key, sizeof(enc_key));
    oauth_url_encode(nonce, enc_nonce, sizeof(enc_nonce));
    if (token) {
        oauth_url_encode(token, enc_token, sizeof(enc_token));
    }
    
    // Construct parameter string
    char param_str[1024];
    if (token) {
        snprintf(param_str, sizeof(param_str),
                 "oauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=HMAC-SHA1&"
                 "oauth_timestamp=%s&oauth_token=%s&oauth_version=1.0",
                 enc_key, enc_nonce, timestamp, enc_token);
    } else {
        snprintf(param_str, sizeof(param_str),
                 "oauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=HMAC-SHA1&"
                 "oauth_timestamp=%s&oauth_version=1.0",
                 enc_key, enc_nonce, timestamp);
    }
    
    char enc_url[512], enc_param[2048];
    oauth_url_encode(url, enc_url, sizeof(enc_url));
    oauth_url_encode(param_str, enc_param, sizeof(enc_param));
    
    // Base string: METHOD&URL&PARAMS
    char base_string[4096];
    snprintf(base_string, sizeof(base_string), "%s&%s&%s", method, enc_url, enc_param);
    
    // Signing key: CONSUMER_SECRET&TOKEN_SECRET
    char key_sec[512];
    char enc_cons_sec[256], enc_tok_sec[256] = "";
    oauth_url_encode(consumer_secret, enc_cons_sec, sizeof(enc_cons_sec));
    if (token_secret) {
        oauth_url_encode(token_secret, enc_tok_sec, sizeof(enc_tok_sec));
    }
    snprintf(key_sec, sizeof(key_sec), "%s&%s", enc_cons_sec, enc_tok_sec);
    
    // HMAC-SHA1 signature
    unsigned char digest[20];
    unsigned int digest_len = 20;
    
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #if OPENSSL_VERSION_NUMBER >= 0x10100000L
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key_sec, strlen(key_sec), EVP_sha1(), NULL);
    HMAC_Update(ctx, (unsigned char*)base_string, strlen(base_string));
    HMAC_Final(ctx, digest, &digest_len);
    HMAC_CTX_free(ctx);
    #else
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key_sec, strlen(key_sec), EVP_sha1(), NULL);
    HMAC_Update(&ctx, (unsigned char*)base_string, strlen(base_string));
    HMAC_Final(&ctx, digest, &digest_len);
    HMAC_CTX_cleanup(&ctx);
    #endif
#pragma GCC diagnostic pop
    
    // Base64 encode signature
    char b64_sig[128];
    int b64_len = EVP_EncodeBlock((unsigned char*)b64_sig, digest, digest_len);
    b64_sig[b64_len] = '\0';
    
    char enc_sig[256];
    oauth_url_encode(b64_sig, enc_sig, sizeof(enc_sig));
    
    // Build Auth header
    if (token) {
        snprintf(out_header, out_header_len,
                 "OAuth oauth_consumer_key=\"%s\", oauth_nonce=\"%s\", "
                 "oauth_signature=\"%s\", oauth_signature_method=\"HMAC-SHA1\", "
                 "oauth_timestamp=\"%s\", oauth_token=\"%s\", oauth_version=\"1.0\"",
                 enc_key, enc_nonce, enc_sig, timestamp, enc_token);
    } else {
        snprintf(out_header, out_header_len,
                 "OAuth oauth_consumer_key=\"%s\", oauth_nonce=\"%s\", "
                 "oauth_signature=\"%s\", oauth_signature_method=\"HMAC-SHA1\", "
                 "oauth_timestamp=\"%s\", oauth_version=\"1.0\"",
                 enc_key, enc_nonce, enc_sig, timestamp);
    }
    
    return 1;
}
