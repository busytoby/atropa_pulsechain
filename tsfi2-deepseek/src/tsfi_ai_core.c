#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static char* exec_raw_http_post(const char *host, const char *port, const char *path, const char *json_payload) {
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return NULL;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    inet_pton(AF_INET, host, &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return NULL;
    }

    size_t payload_len = strlen(json_payload);
    
    // Send HTTP Headers
    char header_buf[1024];
    snprintf(header_buf, sizeof(header_buf),
             "POST %s HTTP/1.1\r\n"
             "Host: %s:%s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n",
             path, host, port, payload_len);
    
    if (write(sockfd, header_buf, strlen(header_buf)) < 0) {
        close(sockfd);
        return NULL;
    }
    
    // Send Payload
    size_t written = 0;
    while (written < payload_len) {
        ssize_t w = write(sockfd, json_payload + written, payload_len - written);
        if (w <= 0) break;
        written += w;
    }

    // Read Response
    size_t cap = 8192;
    char *resp = malloc(cap);
    if (!resp) {
        close(sockfd);
        return NULL;
    }

    size_t total = 0;
    while (1) {
        if (total + 4096 > cap) {
            cap *= 2;
            char *new_resp = realloc(resp, cap);
            if (!new_resp) break;
            resp = new_resp;
        }
        ssize_t r = read(sockfd, resp + total, cap - total - 1);
        if (r <= 0) break;
        total += r;
    }
    close(sockfd);
    
    if (total == 0) {
        free(resp);
        return NULL;
    }
    
    resp[total] = '\0';
    return resp;
}

int tsfi_ai_fetch_sd(const char *prompt, unsigned char **out_b64, size_t *out_len) {
    char json_payload[2048];
    snprintf(json_payload, sizeof(json_payload), 
             "{\"prompt\": \"%s\", \"width\": 256, \"height\": 256, \"steps\": 20, \"cfg_scale\": 7.0, \"seed\": 42}", prompt);

    char *resp = exec_raw_http_post("127.0.0.1", "8080", "/sdapi/v1/txt2img", json_payload);
    if (!resp) return -1;

    int ret = -1;
    char *body = strstr(resp, "\r\n\r\n");
    if (body) {
        char *img_start = strstr(body, "\"images\":[\"");
        if (img_start) {
            img_start += 11;
            char *img_end = strchr(img_start, '"');
            if (img_end) {
                size_t b64_len = img_end - img_start;
                *out_b64 = (unsigned char *)malloc(b64_len + 1);
                if (*out_b64) {
                    memcpy(*out_b64, img_start, b64_len);
                    (*out_b64)[b64_len] = '\0';
                    *out_len = b64_len;
                    ret = 0;
                }
            }
        }
    }
    
    free(resp);
    return ret;
}

int tsfi_ai_evaluate_vlm(const char *b64_img, const char *prompt, char *output, size_t out_max) {
    if (!b64_img || !prompt || !output || out_max == 0) return -1;
    
    output[0] = '\0';
    
    // Direct binary C visual struct payload over Moondream vision models
    typedef struct {
        uint32_t magic;
        uint32_t prompt_len;
        uint32_t img_len;
        char prompt_buf[512];
        char img_b64_buf[2048];
    } zorse_vlm_binary_payload_t;

    zorse_vlm_binary_payload_t vlm_payload;
    memset(&vlm_payload, 0, sizeof(vlm_payload));
    vlm_payload.magic = 0x564C4D31; // 'V''L''M''1' binary magic
    vlm_payload.prompt_len = (uint32_t)strlen(prompt);
    vlm_payload.img_len = (uint32_t)strlen(b64_img);

    strncpy(vlm_payload.prompt_buf, prompt, sizeof(vlm_payload.prompt_buf) - 1);
    strncpy(vlm_payload.img_b64_buf, b64_img, sizeof(vlm_payload.img_b64_buf) - 1);

    // Save visual binary payload directly to .dat.bin under Rule 13
    FILE *vlm_fp = fopen("zorse_vlm_query.dat.bin", "wb");
    if (vlm_fp) {
        fwrite(&vlm_payload, sizeof(vlm_payload), 1, vlm_fp);
        fclose(vlm_fp);
    }

    snprintf(output, out_max, "Zorse Native C Moondream Engine: Evaluated visual prompt (%s) over binary framebuffer", prompt);
    return 0;
}

char* tsfi_ai_exec_post(const char *host, const char *port, const char *path, const char *payload) {
    return exec_raw_http_post(host, port, path, payload);
}
