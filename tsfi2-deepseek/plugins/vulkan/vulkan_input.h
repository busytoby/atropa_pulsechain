#ifndef VULKAN_INPUT_H
#define VULKAN_INPUT_H

#include "vulkan_system.h"

extern const struct wl_pointer_listener pointer_listener;
extern const struct wl_keyboard_listener keyboard_listener;
extern const struct wl_data_device_listener data_device_listener;
extern const struct wl_seat_listener_v10 seat_listener;

// Custom Listener Structs
struct wl_seat_listener_v10 {
    void (*capabilities)(void *data, struct wl_seat *wl_seat, uint32_t capabilities);
    void (*name)(void *data, struct wl_seat *wl_seat, const char *name);
    void (*release)(void *data, struct wl_seat *wl_seat);
};

void keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void tsfi_input_request_paste(VulkanSystem *s);

void tsfi_input_set_key_hook(void *hook_func);
void tsfi_input_set_resize_hook(void *hook_func);

#endif
