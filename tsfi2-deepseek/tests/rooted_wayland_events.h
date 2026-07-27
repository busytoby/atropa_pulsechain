#ifndef ROOTED_WAYLAND_EVENTS_H
#define ROOTED_WAYLAND_EVENTS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include "tsfi_zmm_vm.h"

#define WL_DISPLAY_ID 1
#define WL_REGISTRY_ID 2
#define WL_DISPLAY_GET_REGISTRY 1
#define WL_REGISTRY_BIND 0
#define WL_COMPOSITOR_CREATE_SURFACE 0
#define WL_SHM_CREATE_POOL 0
#define WL_SHM_POOL_CREATE_BUFFER 0
#define WL_SURFACE_ATTACH 1
#define WL_SURFACE_DAMAGE 2
#define WL_SURFACE_COMMIT 6

#define XDG_WM_BASE_GET_XDG_SURFACE 2
#define XDG_SURFACE_GET_TOPLEVEL 1
#define XDG_SURFACE_ACK_CONFIGURE 4

#define ALIGN(x) (((x) + 3) & ~3)
#define MAX_W 3840
#define MAX_H 2160

extern void tsfi_mozilla_wmq_bridge_tick(TsfiZmmVmState *vm_state);

extern uint32_t global_compositor_id, global_shm_id, global_xdg_id, global_seat_name;
extern uint32_t next_id;
extern bool is_projector;

extern uint32_t seat_id;
extern uint32_t pointer_id;
extern uint32_t keyboard_id;
extern uint32_t xdg_wm_base_id;

extern uint16_t g_mouse_x;
extern uint16_t g_mouse_y;
extern uint8_t g_click_state;
extern volatile bool g_input_dirty;

extern int g_w;
extern int g_h;
extern uint32_t pid_val;
extern uint32_t bid_val;
extern uint32_t surf_id;
extern uint32_t *g_scanout_px;

extern TsfiZmmVmState g_zmm_vm;
extern pthread_mutex_t g_zmm_vm_mutex;

// Function pointers for Vulkan/ZMM
typedef void (*PFN_tsfi_zmm_set_scanout_buffer)(void* ptr, int w, int h);
extern PFN_tsfi_zmm_set_scanout_buffer g_ptsfi_zmm_set_scanout_buffer;

int pack_string(uint32_t *buf, const char *s);
void send_msg(int fd, uint32_t obj, uint16_t op, void *data, size_t len, int s_fd);
void route_event_to_yul_cpu(const char *cmd, int is_move, int vm_x, int vm_y, uint8_t command_byte, uint8_t keycode);
void handle_bridge_command(const char *cmd);
int process_events(int fd, uint32_t xdg_s_id, bool *out_configure);

#endif // ROOTED_WAYLAND_EVENTS_H
