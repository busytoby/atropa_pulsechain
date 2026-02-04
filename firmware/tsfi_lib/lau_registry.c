#include "lau_registry.h"
#include <pthread.h>
#include <stddef.h>

static LauMetadata *g_head = NULL;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

void lau_registry_insert(LauMetadata *m) {
    pthread_mutex_lock(&g_lock);
    m->next = g_head;
    m->prev = NULL;
    if (g_head) g_head->prev = m;
    g_head = m;
    pthread_mutex_unlock(&g_lock);
}

void lau_registry_remove(LauMetadata *m) {
    pthread_mutex_lock(&g_lock);
    if (m->prev) m->prev->next = m->next;
    else g_head = m->next;
    
    if (m->next) m->next->prev = m->prev;
    pthread_mutex_unlock(&g_lock);
}

LauMetadata* lau_registry_get_head(void) {
    return g_head;
}

void lau_registry_lock(void) {
    pthread_mutex_lock(&g_lock);
}

void lau_registry_unlock(void) {
    pthread_mutex_unlock(&g_lock);
}
