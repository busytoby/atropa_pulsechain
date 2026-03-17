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
    size_t b64_len = strlen(b64_img);
    size_t p_len = strlen(prompt);
    char *json_payload = (char *)malloc(b64_len + p_len + 256);
    if (!json_payload) return -1;
    
    snprintf(json_payload, b64_len + p_len + 256, 
             "{\"model\": \"moondream\", \"prompt\": \"%s\", \"images\": [\"%s\"], \"stream\": false}", 
             prompt, b64_img);
    
    char *resp = exec_raw_http_post("127.0.0.1", "11435", "/api/generate", json_payload);
    free(json_payload);
    
    if (!resp) return -1;
    
    char *body = strstr(resp, "\r\n\r\n");
    if (body) {
        char *resp_start = strstr(body, "\"response\":\"");
        if (resp_start) {
            resp_start += 12;
            // Handle escaped strings generically (simplified for standard VLM output)
            char *resp_end = resp_start;
            while (*resp_end != '\0') {
                if (*resp_end == '"' && *(resp_end - 1) != '\\') break;
                resp_end++;
            }
            
            if (*resp_end == '"') {
                size_t len = resp_end - resp_start;
                if (len >= out_max) len = out_max - 1;
                strncpy(output, resp_start, len);
                output[len] = '\0';
            }
        }
    }
    
    free(resp);
    return 0;
}
