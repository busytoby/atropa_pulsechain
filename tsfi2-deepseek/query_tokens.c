#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <unistd.h>

extern bool tsfi_pulse_rpc_call(const char *address, const char *data, char *result, size_t result_max);
extern int usleep(unsigned int useconds);

typedef struct {
    char address[64];
    char symbol[64];
    char owner[64];
    bool ignored;
    bool is_debenture;
} TokenInfo;

#define MAX_TOKENS 1000
TokenInfo tokens[MAX_TOKENS];
int token_count = 0;

void parse_json_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open JSON file");
        exit(1);
    }

    char line[1024];
    TokenInfo current = {0};
    bool inside_token = false;

    while (fgets(line, sizeof(line), f)) {
        if (strchr(line, '{') && !inside_token) {
            inside_token = true;
            memset(&current, 0, sizeof(current));
            continue;
        }
        if (strstr(line, "}")) {
            if (inside_token && current.address[0] != '\0' && current.owner[0] != '\0') {
                if (token_count < MAX_TOKENS) {
                    tokens[token_count++] = current;
                }
            }
            inside_token = false;
            continue;
        }

        char *addr_ptr = strstr(line, "\"address\":");
        if (addr_ptr) {
            char *start = strchr(addr_ptr + 10, '"');
            if (start) {
                char *end = strchr(start + 1, '"');
                if (end) {
                    size_t len = end - (start + 1);
                    if (len < 64) {
                        memcpy(current.address, start + 1, len);
                        current.address[len] = '\0';
                    }
                }
            }
        }

        char *sym_ptr = strstr(line, "\"symbol\":");
        if (sym_ptr) {
            char *start = strchr(sym_ptr + 9, '"');
            if (start) {
                char *end = strchr(start + 1, '"');
                if (end) {
                    size_t len = end - (start + 1);
                    if (len < 64) {
                        memcpy(current.symbol, start + 1, len);
                        current.symbol[len] = '\0';
                    }
                }
            }
        }

        char *owner_ptr = strstr(line, "\"owner\":");
        if (owner_ptr) {
            char *start = strchr(owner_ptr + 8, '"');
            if (start) {
                char *end = strchr(start + 1, '"');
                if (end) {
                    size_t len = end - (start + 1);
                    if (len < 64) {
                        memcpy(current.owner, start + 1, len);
                        current.owner[len] = '\0';
                    }
                }
            }
        }

        char *ig_ptr = strstr(line, "\"ignored\":");
        if (ig_ptr) {
            if (strstr(ig_ptr, "true")) {
                current.ignored = true;
            }
        }
    }
    fclose(f);
}

int main(void) {
    printf("[QUERY] Parsing treasury tokens registry...\n");
    parse_json_file("../assets/treasury_tokens_federalminter.json");
    printf("[QUERY] Parsed %d tokens.\n", token_count);

    printf("[QUERY] Executing RPC queries to determine Debenture status...\n");
    for (int i = 0; i < token_count; i++) {
        if (tokens[i].ignored) continue;
        char result[256];
        // Query Debenture() state: selector 0x3574a50b
        if (tsfi_pulse_rpc_call(tokens[i].address, "0x3574a50b", result, sizeof(result))) {
            // Check if result ends with 1 (true)
            unsigned long val = strtoul(result, NULL, 16);
            tokens[i].is_debenture = (val != 0);
            fprintf(stderr, "[RPC] Query %s (%s): Debenture=%s\n", 
                    tokens[i].address, tokens[i].symbol, tokens[i].is_debenture ? "true" : "false");
        } else {
            fprintf(stderr, "[RPC] Query %s (%s) failed, assuming published\n", 
                    tokens[i].address, tokens[i].symbol);
            tokens[i].is_debenture = false;
        }
        // Small delay to avoid RPC rate limit
        usleep(50000); 
    }

    // Print grouped report by Owner
    printf("\n==================================================\n");
    printf("     TREASURY SECRETARY ASSETS STATUS REPORT\n");
    printf("==================================================\n");

    // Get unique owners
    char owners[200][64];
    int owner_count = 0;
    for (int i = 0; i < token_count; i++) {
        if (tokens[i].ignored) continue;
        bool exists = false;
        for (int j = 0; j < owner_count; j++) {
            if (strcasecmp(tokens[i].owner, owners[j]) == 0) {
                exists = true;
                break;
            }
        }
        if (!exists && owner_count < 200) {
            strcpy(owners[owner_count++], tokens[i].owner);
        }
    }

    for (int o = 0; o < owner_count; o++) {
        printf("\nSecretary: %s\n", owners[o]);
        printf("  Debentures (Retained Collateral):\n");
        int deb_cnt = 0;
        for (int i = 0; i < token_count; i++) {
            if (tokens[i].ignored) continue;
            if (strcasecmp(tokens[i].owner, owners[o]) == 0 && tokens[i].is_debenture) {
                printf("    - [%s] %s\n", tokens[i].symbol, tokens[i].address);
                deb_cnt++;
            }
        }
        if (deb_cnt == 0) printf("    (none)\n");

        printf("  Published Articles (Released to Public):\n");
        int pub_cnt = 0;
        for (int i = 0; i < token_count; i++) {
            if (tokens[i].ignored) continue;
            if (strcasecmp(tokens[i].owner, owners[o]) == 0 && !tokens[i].is_debenture) {
                printf("    - [%s] %s\n", tokens[i].symbol, tokens[i].address);
                pub_cnt++;
            }
        }
        if (pub_cnt == 0) printf("    (none)\n");
    }
    printf("\n==================================================\n");
    return 0;
}
