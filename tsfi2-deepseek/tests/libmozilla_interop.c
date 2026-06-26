#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LAYOUT_ELEMENTS 256
#define MAX_EVENT_QUEUE 512

typedef struct {
    char element_id[64];
    int x;
    int y;
    int w;
    int h;
} LayoutElement;

typedef struct {
    uint32_t type;       // 0=Mouse Move, 1=Mouse Button, 2=Key, 3=Scroll
    int32_t param1;      // X coord, keycode, or axis
    int32_t param2;      // Y coord, state, or scroll delta
    int32_t param3;      // Button mask or modifier flags
} MozillaEventPacket;

typedef struct {
    pthread_mutex_t layout_mutex;
    LayoutElement elements[MAX_LAYOUT_ELEMENTS];
    int elements_count;

    pthread_mutex_t event_mutex;
    MozillaEventPacket event_queue[MAX_EVENT_QUEUE];
    int event_head;
    int event_tail;
} SharedInteropState;

static SharedInteropState *g_shared_state = NULL;
static pthread_once_t g_shared_once = PTHREAD_ONCE_INIT;

static void init_shared_state(void) {
    int fd = shm_open("/tsfi_mozilla_interop_shm", O_RDWR | O_CREAT, 0666);
    if (fd != -1) {
        struct stat st;
        fstat(fd, &st);
        bool is_creator = (st.st_size == 0);
        if (is_creator) {
            if (ftruncate(fd, sizeof(SharedInteropState)) != 0) {
                // handle error or proceed
            }
        }
        g_shared_state = (SharedInteropState*)mmap(NULL, sizeof(SharedInteropState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);

        if (g_shared_state && is_creator) {
            memset(g_shared_state, 0, sizeof(SharedInteropState));
            
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            
            pthread_mutex_init(&g_shared_state->layout_mutex, &attr);
            pthread_mutex_init(&g_shared_state->event_mutex, &attr);
            
            pthread_mutexattr_destroy(&attr);
        }
    }
}

static SharedInteropState* get_state(void) {
    pthread_once(&g_shared_once, init_shared_state);
    return g_shared_state;
}

/**
 * Register element boundaries from the DOM/Layout cycle.
 */
void tsfi_register_layout_element(const char* element_id, int x, int y, int w, int h) {
    if (!element_id) return;
    SharedInteropState *s = get_state();
    if (!s) return;

    pthread_mutex_lock(&s->layout_mutex);
    
    // Check if already exists, if so update coordinates
    for (int i = 0; i < s->elements_count; i++) {
        if (strcmp(s->elements[i].element_id, element_id) == 0) {
            s->elements[i].x = x;
            s->elements[i].y = y;
            s->elements[i].w = w;
            s->elements[i].h = h;
            pthread_mutex_unlock(&s->layout_mutex);
            return;
        }
    }
    
    // Insert new element mapping
    if (s->elements_count < MAX_LAYOUT_ELEMENTS) {
        strncpy(s->elements[s->elements_count].element_id, element_id, 63);
        s->elements[s->elements_count].x = x;
        s->elements[s->elements_count].y = y;
        s->elements[s->elements_count].w = w;
        s->elements[s->elements_count].h = h;
        s->elements_count++;
    }
    pthread_mutex_unlock(&s->layout_mutex);
}

/**
 * Lookup coordinates of registered elements.
 */
bool tsfi_lookup_layout_element(const char* element_id, int* x, int* y, int* w, int* h) {
    if (!element_id) return false;
    SharedInteropState *s = get_state();
    if (!s) return false;

    pthread_mutex_lock(&s->layout_mutex);
    for (int i = 0; i < s->elements_count; i++) {
        if (strcmp(s->elements[i].element_id, element_id) == 0) {
            *x = s->elements[i].x;
            *y = s->elements[i].y;
            *w = s->elements[i].w;
            *h = s->elements[i].h;
            pthread_mutex_unlock(&s->layout_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&s->layout_mutex);
    return false;
}

/**
 * Push events into the WebRender queue.
 */
void mozilla_web_render_push_event(const MozillaEventPacket *packet) {
    if (!packet) return;
    SharedInteropState *s = get_state();
    if (!s) return;

    pthread_mutex_lock(&s->event_mutex);
    int next = (s->event_head + 1) % MAX_EVENT_QUEUE;
    if (next != s->event_tail) {
        s->event_queue[s->event_head] = *packet;
        s->event_head = next;
    }
    pthread_mutex_unlock(&s->event_mutex);

    printf("[Auncient WebRender] Event Dispatched: type=%u, param1=%d, param2=%d, param3=%d\n",
           packet->type, packet->param1, packet->param2, packet->param3);
}

/**
 * Pop dispatched events from the queue for automated verification.
 */
bool tsfi_pop_webrender_event(MozillaEventPacket *packet) {
    if (!packet) return false;
    SharedInteropState *s = get_state();
    if (!s) return false;

    pthread_mutex_lock(&s->event_mutex);
    if (s->event_tail == s->event_head) {
        pthread_mutex_unlock(&s->event_mutex);
        return false;
    }
    *packet = s->event_queue[s->event_tail];
    s->event_tail = (s->event_tail + 1) % MAX_EVENT_QUEUE;
    pthread_mutex_unlock(&s->event_mutex);

    printf("[Auncient WebRender] Event Popped: type=%u, param1=%d, param2=%d, param3=%d\n",
           packet->type, packet->param1, packet->param2, packet->param3);
    return true;
}

#define AC_ALPHABET_SIZE 128
#define MAX_CACHE_ENTRIES 1024
#define MAX_AC_NODES 4096

typedef struct {
    int next[AC_ALPHABET_SIZE];
    int fail;
    int is_match;
    int cache_index;
} ACStateNode;

typedef struct {
    char shader_key[128];
    uint8_t pre_render_data[256];
    uint8_t post_render_data[256];
    size_t pre_size;
    size_t post_size;
    bool active;
} CompositorCacheEntry;

typedef struct {
    pthread_mutex_t cache_mutex;
    ACStateNode ac_nodes[MAX_AC_NODES];
    int ac_node_count;
    CompositorCacheEntry cache_entries[MAX_CACHE_ENTRIES];
    int cache_count;
} CompositorCacheState;

static CompositorCacheState g_cache_state;
static pthread_once_t g_cache_once = PTHREAD_ONCE_INIT;

static void init_cache_state(void) {
    memset(&g_cache_state, 0, sizeof(CompositorCacheState));
    pthread_mutex_init(&g_cache_state.cache_mutex, NULL);
    g_cache_state.ac_node_count = 0;
    int root = g_cache_state.ac_node_count++;
    for (int i = 0; i < AC_ALPHABET_SIZE; i++) {
        g_cache_state.ac_nodes[root].next[i] = -1;
    }
    g_cache_state.ac_nodes[root].fail = -1;
    g_cache_state.ac_nodes[root].is_match = 0;
    g_cache_state.ac_nodes[root].cache_index = -1;
}

static CompositorCacheState* get_cache(void) {
    pthread_once(&g_cache_once, init_cache_state);
    return &g_cache_state;
}

static void rebuild_ac_failure_links(CompositorCacheState *c) {
    int root = 0;
    int q[MAX_AC_NODES];
    int head = 0, tail = 0;
    
    c->ac_nodes[root].fail = root;
    for (int i = 0; i < AC_ALPHABET_SIZE; i++) {
        if (c->ac_nodes[root].next[i] != -1) {
            c->ac_nodes[c->ac_nodes[root].next[i]].fail = root;
            q[tail++] = c->ac_nodes[root].next[i];
        } else {
            c->ac_nodes[root].next[i] = root;
        }
    }
    
    while (head < tail) {
        int u = q[head++];
        for (int i = 0; i < AC_ALPHABET_SIZE; i++) {
            if (c->ac_nodes[u].next[i] != -1) {
                int v = c->ac_nodes[u].next[i];
                c->ac_nodes[v].fail = c->ac_nodes[c->ac_nodes[u].fail].next[i];
                if (c->ac_nodes[c->ac_nodes[v].fail].is_match) {
                    c->ac_nodes[v].is_match |= c->ac_nodes[c->ac_nodes[v].fail].is_match;
                    if (c->ac_nodes[v].cache_index == -1) {
                        c->ac_nodes[v].cache_index = c->ac_nodes[c->ac_nodes[v].fail].cache_index;
                    }
                }
                q[tail++] = v;
            } else {
                c->ac_nodes[u].next[i] = c->ac_nodes[c->ac_nodes[u].fail].next[i];
            }
        }
    }
}

void tsfi_compositor_cache_register(const char *key, const void *pre_data, size_t pre_size, const void *post_data, size_t post_size) {
    if (!key) return;
    CompositorCacheState *c = get_cache();
    if (!c) return;

    pthread_mutex_lock(&c->cache_mutex);

    int cache_idx = -1;
    for (int i = 0; i < c->cache_count; i++) {
        if (strcmp(c->cache_entries[i].shader_key, key) == 0) {
            cache_idx = i;
            break;
        }
    }
    if (cache_idx == -1) {
        if (c->cache_count < MAX_CACHE_ENTRIES) {
            cache_idx = c->cache_count++;
            strncpy(c->cache_entries[cache_idx].shader_key, key, 127);
            c->cache_entries[cache_idx].active = true;
        } else {
            pthread_mutex_unlock(&c->cache_mutex);
            return;
        }
    }

    if (pre_data && pre_size <= 256) {
        memcpy(c->cache_entries[cache_idx].pre_render_data, pre_data, pre_size);
        c->cache_entries[cache_idx].pre_size = pre_size;
    }
    if (post_data && post_size <= 256) {
        memcpy(c->cache_entries[cache_idx].post_render_data, post_data, post_size);
        c->cache_entries[cache_idx].post_size = post_size;
    }

    c->ac_node_count = 1;
    for (int i = 0; i < AC_ALPHABET_SIZE; i++) {
        c->ac_nodes[0].next[i] = -1;
    }
    c->ac_nodes[0].fail = -1;
    c->ac_nodes[0].is_match = 0;
    c->ac_nodes[0].cache_index = -1;

    for (int i = 0; i < c->cache_count; i++) {
        if (!c->cache_entries[i].active) continue;
        int curr = 0;
        const char *k = c->cache_entries[i].shader_key;
        for (int j = 0; k[j] != '\0'; j++) {
            int ch = (unsigned char)k[j];
            if (ch >= AC_ALPHABET_SIZE) ch = ch % AC_ALPHABET_SIZE;
            if (c->ac_nodes[curr].next[ch] == -1) {
                if (c->ac_node_count < MAX_AC_NODES) {
                    int next_node = c->ac_node_count++;
                    for (int m = 0; m < AC_ALPHABET_SIZE; m++) {
                        c->ac_nodes[next_node].next[m] = -1;
                    }
                    c->ac_nodes[next_node].fail = -1;
                    c->ac_nodes[next_node].is_match = 0;
                    c->ac_nodes[next_node].cache_index = -1;
                    c->ac_nodes[curr].next[ch] = next_node;
                } else {
                    break;
                }
            }
            curr = c->ac_nodes[curr].next[ch];
        }
        c->ac_nodes[curr].is_match = 1;
        c->ac_nodes[curr].cache_index = i;
    }

    rebuild_ac_failure_links(c);
    pthread_mutex_unlock(&c->cache_mutex);
    printf("[Auncient Compositor Cache] Registered pipeline layout signature: %s\n", key);
}

bool tsfi_compositor_cache_lookup(const char *pipeline_signature, void *pre_dest, size_t *pre_size_out, void *post_dest, size_t *post_size_out) {
    if (!pipeline_signature) return false;
    CompositorCacheState *c = get_cache();
    if (!c) return false;

    pthread_mutex_lock(&c->cache_mutex);
    int curr = 0;
    int matched_idx = -1;

    for (int i = 0; pipeline_signature[i] != '\0'; i++) {
        int ch = (unsigned char)pipeline_signature[i];
        if (ch >= AC_ALPHABET_SIZE) ch = ch % AC_ALPHABET_SIZE;
        
        curr = c->ac_nodes[curr].next[ch];
        if (curr == -1) {
            curr = 0;
        } else if (c->ac_nodes[curr].is_match) {
            matched_idx = c->ac_nodes[curr].cache_index;
            break;
        }
    }

    if (matched_idx != -1 && matched_idx < c->cache_count) {
        CompositorCacheEntry *entry = &c->cache_entries[matched_idx];
        if (pre_dest && pre_size_out) {
            memcpy(pre_dest, entry->pre_render_data, entry->pre_size);
            *pre_size_out = entry->pre_size;
        }
        if (post_dest && post_size_out) {
            memcpy(post_dest, entry->post_render_data, entry->post_size);
            *post_size_out = entry->post_size;
        }
        pthread_mutex_unlock(&c->cache_mutex);
        return true;
    }

    pthread_mutex_unlock(&c->cache_mutex);
    return false;
}
