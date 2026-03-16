#include "vulkan_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <errno.h>

#define MAX_EVENTS 64

// Simple mapping for now
static void handle_event(VulkanSystem *s, struct input_event *ev) {
    // Update Telemetry (Mouse Microscope)
    if (s->telem) {
        uint64_t now = (uint64_t)ev->time.tv_sec * 1000000 + ev->time.tv_usec;
        uint64_t delta = now - s->telem->mouse_scope.last_shutter_ts;
        
        if (delta > 0) {
            float dt = (float)delta / 1000000.0f;
            // Estimated FPS based on event density (USB polling rate proxy)
            uint32_t fps = 1000000 / delta;
            if (fps > 30000) fps = 30000; // Cap
            // Exponential moving average for stability
            s->telem->mouse_scope.shutter_fps = (s->telem->mouse_scope.shutter_fps * 7 + fps) / 8;

            if (ev->type == EV_REL) {
                float val = (float)ev->value;
                // Instantaneous Velocity
                float vel = val / dt;
                // Velocity Flux (Acceleration)
                float accel = (vel - s->telem->mouse_scope.velocity_flux) / dt;
                // Jerk (Rate of change of acceleration)
                float jerk = (accel - s->telem->mouse_scope.jerk_metric) / dt;

                // Smooth into telemetry
                s->telem->mouse_scope.velocity_flux = (s->telem->mouse_scope.velocity_flux * 0.9f) + (vel * 0.1f);
                s->telem->mouse_scope.jerk_metric = (s->telem->mouse_scope.jerk_metric * 0.9f) + (jerk * 0.1f);

                // Simple SQUAL based on reporting stability
                // High jitter in delta => Low quality
                float jitter = (delta > 1000) ? (float)(delta % 1000) / 1000.0f : 0.0f;
                s->telem->mouse_scope.surface_quality = 1.0f - jitter;

                // Fingerprint: XOR hash of time-series variations
                s->telem->mouse_scope.surface_hash ^= (uint64_t)(val * 1000.0f) ^ now;
            }
        }
        s->telem->mouse_scope.last_shutter_ts = now;
    }

    if (ev->type == EV_KEY) {
        // value 0=release, 1=press, 2=repeat
        bool pressed = (ev->value > 0);
        
        // Update Modifier State
        if (ev->code == KEY_LEFTSHIFT || ev->code == KEY_RIGHTSHIFT) s->input_state.shift = pressed;
        if (ev->code == KEY_LEFTCTRL || ev->code == KEY_RIGHTCTRL) s->input_state.ctrl = pressed;
        if (ev->code == KEY_LEFTALT || ev->code == KEY_RIGHTALT) s->input_state.alt = pressed;
        
        if (pressed) s->last_key_scancode = ev->code;
    }
    else if (ev->type == EV_REL) {
        if (ev->code == REL_X) s->mouse_x += ev->value;
        if (ev->code == REL_Y) s->mouse_y += ev->value;
        
        // Clamp
        if (s->mouse_x < 0) s->mouse_x = 0;
        if (s->mouse_y < 0) s->mouse_y = 0;
        if (s->mouse_x >= s->width) s->mouse_x = s->width - 1;
        if (s->mouse_y >= s->height) s->mouse_y = s->height - 1;
    }
}

void vty_input_poll(VulkanSystem *s) {
    struct input_event ev[MAX_EVENTS];
    
    for (int i = 0; i < s->input_state.keyboard_count; i++) {
        int fd = s->input_state.keyboards[i].fd;
        if (fd <= 0) continue;
        
        int rd = read(fd, ev, sizeof(struct input_event) * MAX_EVENTS);
        if (rd > 0) {
            int count = rd / sizeof(struct input_event);
            for (int j = 0; j < count; j++) {
                handle_event(s, &ev[j]);
            }
        }
    }
}

void init_vty_input(VulkanSystem *s) {
    printf("[VTY] Scanning /dev/input/event*...\n");
    
    s->input_state.keyboard_count = 0;
    
    char path[64];
    for (int i = 0; i < 10; i++) {
        snprintf(path, 64, "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;
        
        char name[256] = "Unknown";
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        
        // Check capabilities (is it a keyboard?)
        unsigned long ev_bit[EV_MAX/8 + 1];
        ioctl(fd, EVIOCGBIT(0, sizeof(ev_bit)), ev_bit);
        
        // Crude check: Has EV_KEY?
        if ( (ev_bit[EV_KEY/8] & (1<<(EV_KEY%8))) ) {
            printf("[VTY] Found Input: %s (%s)\n", path, name);
            // Add to list
            if (s->input_state.keyboard_count < 4) {
                int idx = s->input_state.keyboard_count++;
                s->input_state.keyboards[idx].fd = fd;
                strncpy(s->input_state.keyboards[idx].device_path, path, 255);
                s->input_state.keyboards[idx].active = true;
            } else {
                close(fd);
            }
        } else {
            close(fd);
        }
    }
    printf("[VTY] Input Init Complete. Keyboards: %d\n", s->input_state.keyboard_count);
}