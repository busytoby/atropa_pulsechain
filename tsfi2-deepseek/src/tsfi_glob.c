#include "tsfi_glob.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>

static char *tsfi_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *d = (char *)lau_malloc(len);
    if (d) memcpy(d, s, len);
    return d;
}

static void push_path(TSFiGlobResult *res, const char *path) {
    if (res->count >= res->capacity) {
        size_t new_cap = res->capacity == 0 ? 128 : res->capacity * 2;
        char **new_paths = (char **)lau_malloc(new_cap * sizeof(char *));
        if (!new_paths) return;
        if (res->paths) {
            memcpy(new_paths, res->paths, res->count * sizeof(char *));
            lau_free(res->paths);
        }
        res->paths = new_paths;
        res->capacity = new_cap;
    }
    res->paths[res->count++] = tsfi_strdup(path);
}

static void walk_dir(const char *dir, const char *pattern, TSFiGlobResult *res) {
    DIR *d = opendir(dir);
    if (!d) return;

    struct dirent *entry;
    char path[4096];

    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        struct stat st;
        if (lstat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Ignore large known garbage directories
                if (strcmp(entry->d_name, ".git") != 0 && strcmp(entry->d_name, "node_modules") != 0 && strcmp(entry->d_name, "obj") != 0 && strcmp(entry->d_name, "bin") != 0) {
                    walk_dir(path, pattern, res);
                }
            } else if (S_ISREG(st.st_mode)) {
                // Remove the "./" prefix if base_dir was "."
                const char *match_path = path;
                if (strncmp(match_path, "./", 2) == 0) {
                    match_path += 2;
                }
                
                // Allow * to cross directories without FNM_PATHNAME so `**/*.c` isn't strictly required
                if (fnmatch(pattern, match_path, 0) == 0) {
                    push_path(res, path);
                }
            }
        }
    }
    closedir(d);
}

TSFiGlobResult* tsfi_glob(const char *base_dir, const char *pattern) {
    TSFiGlobResult *res = (TSFiGlobResult *)lau_malloc(sizeof(TSFiGlobResult));
    if (!res) return NULL;
    res->paths = NULL;
    res->count = 0;
    res->capacity = 0;

    walk_dir(base_dir, pattern, res);
    return res;
}

void tsfi_glob_free(TSFiGlobResult *res) {
    if (!res) return;
    for (size_t i = 0; i < res->count; i++) {
        lau_free(res->paths[i]);
    }
    if (res->paths) lau_free(res->paths);
    lau_free(res);
}
