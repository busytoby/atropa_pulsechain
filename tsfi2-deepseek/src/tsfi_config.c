#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include "tsfi_io.h"
#include "lau_memory.h"

#define MAX_CONFIG_ENTRIES 100

typedef struct {
    char *key;
    char *value;
} ConfigEntry;

static ConfigEntry config_cache[MAX_CONFIG_ENTRIES];
static int config_count = 0;
static int config_loaded = 0;

static void tsfi_config_load() {
    if (config_loaded) return;
    
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
    
    if (!home) return;
    
    char path[1024];
    snprintf(path, sizeof(path), "%s/.gemini/config", home);
    
    FILE *f = fopen(path, "r");
    if (!f) return;
    
    char line[4096];
    while (fgets(line, sizeof(line), f) && config_count < MAX_CONFIG_ENTRIES) {
        char *l = line;
        while (*l == ' ' || *l == '\t') l++;
        if (*l == '#' || *l == '\n' || *l == '\0') continue;
        
        char *eq = strchr(l, '=');
        if (eq) {
            *eq = '\0';
            char *key = l;
            char *val = eq + 1;
            
            // Trim key
            char *key_end = key + (key[0] ? strlen(key) - 1 : 0);
            while (key_end > key && (*key_end == ' ' || *key_end == '\t')) *key_end-- = '\0';
            
            // Trim value
            while (*val == ' ' || *val == '\t') val++;
            size_t vlen = strlen(val);
            char *val_end = val + (vlen > 0 ? vlen - 1 : 0);
            while (val_end >= val && (*val_end == '\n' || *val_end == '\r' || *val_end == ' ' || *val_end == '\t')) *val_end-- = '\0';
            
            // Handle quotes
            if (val_end > val) {
                if ((*val == '"' && *val_end == '"') || (*val == '\'' && *val_end == '\'')) {
                    val++;
                    *val_end = '\0';
                }
            }
            
            config_cache[config_count].key = strdup(key);
            config_cache[config_count].value = strdup(val);
            config_count++;
        }
    }
    fclose(f);
    config_loaded = 1;
}

const char* tsfi_config_get(const char *key) {
    const char *env = getenv(key);
    if (env) return env;
    
    tsfi_config_load();
    for (int i = 0; i < config_count; i++) {
        if (strcmp(config_cache[i].key, key) == 0) {
            return config_cache[i].value;
        }
    }
    return NULL;
}

void tsfi_config_teardown() {
    for (int i = 0; i < config_count; i++) {
        free(config_cache[i].key);
        free(config_cache[i].value);
    }
    config_count = 0;
    config_loaded = 0;
}
