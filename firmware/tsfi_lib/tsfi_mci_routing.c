#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    uint32_t active_lun;
    bool is_authorized;
    uint32_t auth_code_seed;
    pthread_mutex_t mutex;
} MCIRouter;

void mci_init(MCIRouter *router, uint32_t seed) {
    router->active_lun = 0;
    router->is_authorized = false;
    router->auth_code_seed = seed;
    pthread_mutex_init(&router->mutex, NULL);
}

bool mci_authorize(MCIRouter *router, uint32_t token) {
    pthread_mutex_lock(&router->mutex);
    uint32_t expected = (router->auth_code_seed ^ 0x2600) & 0xFFFF;
    if (token == expected) {
        router->is_authorized = true;
        pthread_mutex_unlock(&router->mutex);
        return true;
    }
    router->is_authorized = false;
    pthread_mutex_unlock(&router->mutex);
    return false;
}

bool mci_set_lun(MCIRouter *router, uint32_t lun) {
    pthread_mutex_lock(&router->mutex);
    if (lun <= 2) {
        router->active_lun = lun;
        pthread_mutex_unlock(&router->mutex);
        return true;
    }
    pthread_mutex_unlock(&router->mutex);
    return false;
}

const char* mci_route_write(MCIRouter *router, uint8_t val) {
    (void)val;
    pthread_mutex_lock(&router->mutex);
    if (!router->is_authorized) {
        pthread_mutex_unlock(&router->mutex);
        return "BLOCKED";
    }
    
    const char* target = "UNKNOWN";
    switch(router->active_lun) {
        case 0: target = "AVX_THUNK"; break;
        case 1: target = "ALSA_THUNK"; break;
        case 2: target = "VULKAN_THUNK"; break;
    }
    pthread_mutex_unlock(&router->mutex);
    return target;
}
