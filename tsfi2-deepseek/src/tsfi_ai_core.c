#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int tsfi_ai_fetch_sd(const char *prompt, unsigned char **out_b64, size_t *out_len) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = (char *)malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if(curl) {
        char json_payload[2048];
        snprintf(json_payload, sizeof(json_payload), 
                 "{\"prompt\": \"%s\", \"width\": 256, \"height\": 256, \"steps\": 20, \"cfg_scale\": 7.0, \"seed\": 42}", prompt);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080/sdapi/v1/txt2img");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            char *img_start = strstr(chunk.memory, "\"images\":[\"");
            if (img_start) {
                img_start += 11;
                char *img_end = strchr(img_start, '"');
                if (img_end) {
                    size_t b64_len = img_end - img_start;
                    *out_b64 = (unsigned char *)malloc(b64_len + 1);
                    memcpy(*out_b64, img_start, b64_len);
                    (*out_b64)[b64_len] = '\0';
                    *out_len = b64_len;
                }
            }
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    free(chunk.memory);
    return (*out_b64 != NULL) ? 0 : -1;
}

int tsfi_ai_evaluate_vlm(const char *b64_img, const char *prompt, char *output, size_t out_max) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = (char *)malloc(1);  
    chunk.size = 0;    
    
    curl = curl_easy_init();
    if(curl) {
        size_t b64_len = strlen(b64_img);
        size_t p_len = strlen(prompt);
        char *json_payload = (char *)malloc(b64_len + p_len + 256);
        
        sprintf(json_payload, "{\"model\": \"moondream\", \"prompt\": \"%s\", \"images\": [\"%s\"], \"stream\": false}", prompt, b64_img);
        
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        // Using Ollama on port 11435 per TSFi environment configs
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:11435/api/generate");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        
        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            char *resp_start = strstr(chunk.memory, "\"response\":\"");
            if (resp_start) {
                resp_start += 12;
                char *resp_end = strchr(resp_start, '"');
                if (resp_end) {
                    size_t len = resp_end - resp_start;
                    if (len >= out_max) len = out_max - 1;
                    strncpy(output, resp_start, len);
                    output[len] = '\0';
                }
            }
        }
        free(json_payload);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    free(chunk.memory);
    return 0;
}
