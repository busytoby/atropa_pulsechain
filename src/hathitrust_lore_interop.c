#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_lore_interop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_LORE_RECORDS 64

static HtrcLoreRecord g_lore_registry[MAX_LORE_RECORDS];
static int g_lore_count = 0;
static pthread_mutex_t g_lore_mutex = PTHREAD_MUTEX_INITIALIZER;

bool hathitrust_lore_interop_register(
    const char *lore_id,
    const char *title,
    const char *pub_date,
    const char *filepath
) {
    if (!lore_id || !title || !pub_date || !filepath) return false;
    
    pthread_mutex_lock(&g_lore_mutex);
    
    if (g_lore_count >= MAX_LORE_RECORDS) {
        pthread_mutex_unlock(&g_lore_mutex);
        return false;
    }
    
    // Check if already registered
    for (int i = 0; i < g_lore_count; i++) {
        if (strcmp(g_lore_registry[i].lore_id, lore_id) == 0) {
            pthread_mutex_unlock(&g_lore_mutex);
            return false;
        }
    }
    
    g_lore_registry[g_lore_count].lore_id = strdup(lore_id);
    g_lore_registry[g_lore_count].title = strdup(title);
    g_lore_registry[g_lore_count].pub_date = strdup(pub_date);
    g_lore_registry[g_lore_count].filepath = strdup(filepath);
    g_lore_count++;
    
    pthread_mutex_unlock(&g_lore_mutex);
    return true;
}

HtrcLoreRecord* hathitrust_lore_interop_lookup(const char *lore_id) {
    if (!lore_id) return NULL;
    
    pthread_mutex_lock(&g_lore_mutex);
    
    for (int i = 0; i < g_lore_count; i++) {
        if (strcmp(g_lore_registry[i].lore_id, lore_id) == 0) {
            pthread_mutex_unlock(&g_lore_mutex);
            return &g_lore_registry[i];
        }
    }
    
    pthread_mutex_unlock(&g_lore_mutex);
    return NULL;
}

void hathitrust_lore_interop_clear(void) {
    pthread_mutex_lock(&g_lore_mutex);
    
    for (int i = 0; i < g_lore_count; i++) {
        free(g_lore_registry[i].lore_id);
        free(g_lore_registry[i].title);
        free(g_lore_registry[i].pub_date);
        free(g_lore_registry[i].filepath);
        memset(&g_lore_registry[i], 0, sizeof(HtrcLoreRecord));
    }
    g_lore_count = 0;
    
    pthread_mutex_unlock(&g_lore_mutex);
}
