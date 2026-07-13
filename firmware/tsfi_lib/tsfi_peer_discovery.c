#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#define MAX_PEERS 64
#define URI_LEN 64

typedef enum {
    PEER_TYPE_MCP,
    PEER_TYPE_THUNK,
    PEER_TYPE_REGISTER
} PeerType;

typedef struct {
    char uri[URI_LEN];
    PeerType type;
    void *target_handle;
    uint32_t metadata; // Stores port or auxiliary index
} RegisteredPeer;

typedef struct {
    RegisteredPeer list[MAX_PEERS];
    uint32_t count;
    pthread_mutex_t mutex;
} PeerDiscoveryRegistry;

void peer_registry_init(PeerDiscoveryRegistry *reg) {
    reg->count = 0;
    pthread_mutex_init(&reg->mutex, NULL);
}

// Register a new unified peer endpoint
bool peer_registry_register(PeerDiscoveryRegistry *reg, const char *uri, PeerType type, void *handle, uint32_t meta) {
    pthread_mutex_lock(&reg->mutex);
    if (reg->count >= MAX_PEERS) {
        pthread_mutex_unlock(&reg->mutex);
        return false; // Registry full
    }
    
    // Avoid duplicate URIs
    for (uint32_t i = 0; i < reg->count; i++) {
        if (strcmp(reg->list[i].uri, uri) == 0) {
            pthread_mutex_unlock(&reg->mutex);
            return false;
        }
    }
    
    RegisteredPeer *p = &reg->list[reg->count];
    strncpy(p->uri, uri, URI_LEN - 1);
    p->uri[URI_LEN - 1] = '\0';
    p->type = type;
    p->target_handle = handle;
    p->metadata = meta;
    reg->count++;
    
    pthread_mutex_unlock(&reg->mutex);
    return true;
}

// Discover/Query a peer by URI lookup
RegisteredPeer* peer_registry_discover(PeerDiscoveryRegistry *reg, const char *uri) {
    pthread_mutex_lock(&reg->mutex);
    for (uint32_t i = 0; i < reg->count; i++) {
        if (strcmp(reg->list[i].uri, uri) == 0) {
            pthread_mutex_unlock(&reg->mutex);
            return &reg->list[i];
        }
    }
    pthread_mutex_unlock(&reg->mutex);
    return NULL; // Not found
}

void peer_registry_destroy(PeerDiscoveryRegistry *reg) {
    pthread_mutex_destroy(&reg->mutex);
}
