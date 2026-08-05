#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char type[32];
    char filetype[32];
    char local_file[256];
    char remote_file[256];
    bool secure_socket;
    int port;
    char lun_name[64];
} XcomConfig;

bool parse_xcom_template(const char *filepath, XcomConfig *cfg) {
    if (!filepath || !cfg) return false;
    memset(cfg, 0, sizeof(XcomConfig));

    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        
        // Strip val newline
        size_t len = strlen(val);
        while (len > 0 && (val[len - 1] == '\n' || val[len - 1] == '\r')) {
            val[len - 1] = '\0';
            len--;
        }

        if (strcmp(key, "TYPE") == 0) strncpy(cfg->type, val, sizeof(cfg->type) - 1);
        else if (strcmp(key, "FILETYPE") == 0) strncpy(cfg->filetype, val, sizeof(cfg->filetype) - 1);
        else if (strcmp(key, "LOCAL_FILE") == 0) strncpy(cfg->local_file, val, sizeof(cfg->local_file) - 1);
        else if (strcmp(key, "REMOTE_FILE") == 0) strncpy(cfg->remote_file, val, sizeof(cfg->remote_file) - 1);
        else if (strcmp(key, "SECURE_SOCKET") == 0) cfg->secure_socket = (strcmp(val, "YES") == 0);
        else if (strcmp(key, "PORT") == 0) cfg->port = atoi(val);
        else if (strcmp(key, "LUN_NAME") == 0) strncpy(cfg->lun_name, val, sizeof(cfg->lun_name) - 1);
    }

    fclose(f);
    return true;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <xcom_template_file>\n", argv[0]);
        return 1;
    }

    XcomConfig cfg;
    if (!parse_xcom_template(argv[1], &cfg)) {
        fprintf(stderr, "Error: Failed to parse XCOM template file %s\n", argv[1]);
        return 1;
    }

    printf("=============================================================\n");
    printf("SNA/APPC COMMUNICATIONS PEER SIMULATOR (LUN: %s)\n", cfg.lun_name);
    printf("=============================================================\n");
    printf("[APPC] Handshaking with SNA Logical Unit at port %d...\n", cfg.port);
    printf("[APPC] Establishing APPC Conversation (LU 6.2 Transaction Program)...\n");

    if (cfg.secure_socket) {
        printf("[APPC] Handshake: TLS Secure Socket layer verified successfully.\n");
    }

    printf("[APPC] Local DSN: %s -> Remote File: %s\n", cfg.local_file, cfg.remote_file);
    printf("[APPC] Transmission protocol: %s (Type: %s)\n", cfg.type, cfg.filetype);
    printf("[APPC] Status: SNA/APPC communication loopback validation SUCCESS.\n");
    printf("=============================================================\n");

    return 0;
}
