#define _GNU_SOURCE
#include "rooted_wayland_events.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include "tsfi_raw.h"
#include "lau_yul_thunk.h"


// Define globals declared in the header (so they are allocated in this compilation unit)
uint32_t global_compositor_id = 0;
uint32_t global_shm_id = 0;
uint32_t global_xdg_id = 0;
uint32_t global_seat_name = 0;
uint32_t next_id = 3;
bool is_projector = false;

uint32_t seat_id = 0;
uint32_t pointer_id = 0;
uint32_t keyboard_id = 0;
uint32_t xdg_wm_base_id = 0;

uint16_t g_mouse_x = 0;
uint16_t g_mouse_y = 0;
uint8_t g_click_state = 0;
volatile bool g_input_dirty = false;

int g_w = 1024;
int g_h = 768;
uint32_t pid_val = 0;
uint32_t bid_val = 0;
uint32_t surf_id = 0;
uint32_t *g_scanout_px = NULL;

PFN_tsfi_zmm_set_scanout_buffer g_ptsfi_zmm_set_scanout_buffer = NULL;

extern void push_input_event(const char *cmd);
static void send_input_command(const char *cmd) {
    push_input_event(cmd);
}

extern bool decode_jpeg_zero_copy(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height);
extern bool update_and_present(int fd_wl, uint32_t surf, uint32_t bid, bool force_redraw);
extern size_t last_jpeg_sz;
extern uint8_t last_jpeg_buf[];

int pack_string(uint32_t *buf, const char *s) {
    uint32_t len = strlen(s) + 1;
    buf[0] = len;
    memcpy(&buf[1], s, len);
    return 1 + (ALIGN(len) / 4);
}

void send_msg(int fd, uint32_t obj, uint16_t op, void *data, size_t len, int s_fd) {
    uint32_t head[2] = {obj, ((uint32_t)(8 + len) << 16) | op};
    struct iovec iov[2] = {{head, 8}, {data, len}};
    struct msghdr msg = {0};
    msg.msg_iov = iov;
    msg.msg_iovlen = len ? 2 : 1;
    union {
        char buf[CMSG_SPACE(sizeof(int))];
        struct cmsghdr align;
    } u;
    if (s_fd != -1) {
        memset(&u, 0, sizeof(u));
        msg.msg_control = u.buf;
        msg.msg_controllen = sizeof(u.buf);
        struct cmsghdr *cm = CMSG_FIRSTHDR(&msg);
        cm->cmsg_level = SOL_SOCKET;
        cm->cmsg_type = SCM_RIGHTS;
        cm->cmsg_len = CMSG_LEN(sizeof(int));
        *((int *)CMSG_DATA(cm)) = s_fd;
    }
    ssize_t ret = sendmsg(fd, &msg, MSG_NOSIGNAL);
    if (ret < 0) {
        fprintf(stderr, "[Auncient Presenter ERR] sendmsg failed: obj=%u, op=%u, err=%s\n", obj, op, strerror(errno));
    }
}

void route_event_to_yul_cpu(const char *cmd, int is_move, int vm_x, int vm_y, uint8_t command_byte, uint8_t keycode) {
    pthread_mutex_lock(&g_zmm_vm_mutex);
    if (is_move) {
        if (g_zmm_vm.reu_ram && g_zmm_vm.reu_size > 0xF004) {
            g_zmm_vm.reu_ram[0xF000] = (uint8_t)(vm_x & 0xFF);
            g_zmm_vm.reu_ram[0xF001] = (uint8_t)(vm_y & 0xFF);
            g_zmm_vm.reu_ram[0xF003] = (uint8_t)((vm_x >> 8) & 0xFF);
            g_zmm_vm.reu_ram[0xF004] = (uint8_t)((vm_y >> 8) & 0xFF);
        }
    } else {
        if (g_zmm_vm.reu_ram && g_zmm_vm.reu_size > 0xF002) {
            g_zmm_vm.reu_ram[0xF002] = keycode;
        }
    }
    
    uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
    cd[35] = command_byte;
    
    uint8_t ret[32];
    size_t ret_len = 32;
    lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

    uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
    char cmd_str[33] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "Y:%s", cmd);
    char processed[33] = {0};
    char *src = cmd_str;
    char *dst = processed;
    while (*src && (dst - processed) < 30) {
        if (strncmp(src, "MOUSE_MOVE", 10) == 0) {
            strcpy(dst, "MM"); dst += 2; src += 10;
        } else if (strncmp(src, "MOUSE_DOWN", 10) == 0) {
            strcpy(dst, "MD"); dst += 2; src += 10;
        } else if (strncmp(src, "MOUSE_UP", 8) == 0) {
            strcpy(dst, "MU"); dst += 2; src += 8;
        } else if (strncmp(src, "MOUSE_SCROLL", 12) == 0) {
            strcpy(dst, "MS"); dst += 2; src += 12;
        } else if (strncmp(src, "KEY_DOWN", 8) == 0) {
            strcpy(dst, "KD"); dst += 2; src += 8;
        } else if (strncmp(src, "KEY_UP", 6) == 0) {
            strcpy(dst, "KU"); dst += 2; src += 6;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    memcpy(cd_post + 4, processed, 32);
    
    lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);
    pthread_mutex_unlock(&g_zmm_vm_mutex);
}

void handle_bridge_command(const char *cmd) {
    if (strncmp(cmd, "Y:MM ", 5) == 0) {
        int vm_x = 0, vm_y = 0;
        if (sscanf(cmd + 5, "%d %d", &vm_x, &vm_y) == 2) {
            g_mouse_x = (uint16_t)((float)vm_x * (float)g_w / 320.0f);
            g_mouse_y = (uint16_t)((float)vm_y * (float)g_h / 300.0f);
            uint8_t cmd_byte = (0x00 << 6) | 0x01;
            route_event_to_yul_cpu(cmd + 2, 1, vm_x, vm_y, cmd_byte, 0);
            g_input_dirty = true;
        }
    } else if (strncmp(cmd, "Y:MD ", 5) == 0) {
        int btn = 0;
        if (sscanf(cmd + 5, "%d", &btn) == 1) {
            if (btn == 272) g_click_state = 1;
            uint8_t btn_idx = 0;
            if (btn == 273) btn_idx = 1;
            if (btn == 274) btn_idx = 2;
            uint8_t cmd_byte = (0x01 << 6) | 0x04 | (btn_idx & 0x03);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, 0);
            g_input_dirty = true;
        }
    } else if (strncmp(cmd, "Y:MU ", 5) == 0) {
        int btn = 0;
        if (sscanf(cmd + 5, "%d", &btn) == 1) {
            if (btn == 272) g_click_state = 0;
            uint8_t btn_idx = 0;
            if (btn == 273) btn_idx = 1;
            if (btn == 274) btn_idx = 2;
            uint8_t cmd_byte = (0x01 << 6) | (btn_idx & 0x03);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, 0);
            g_input_dirty = true;
        }
    } else if (strncmp(cmd, "Y:MS ", 5) == 0) {
        int axis = 0;
        float value = 0.0f;
        if (sscanf(cmd + 5, "%d %f", &axis, &value) == 2) {
            int val_i = (int)value;
            if (val_i < 0) val_i = -val_i;
            if (val_i > 15) val_i = 15;
            uint8_t cmd_byte = (0x03 << 6) | ((axis & 1) << 5) | (value < 0 ? 0x10 : 0) | (val_i & 0x0F);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, 0);
            g_input_dirty = true;
        }
    } else if (strncmp(cmd, "Y:KD ", 5) == 0) {
        int key = 0;
        if (sscanf(cmd + 5, "%d", &key) == 1) {
            uint8_t cmd_byte = (0x02 << 6) | 0x20 | (key & 0x1F);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, (uint8_t)key);
            g_input_dirty = true;
            if (key == 1) {
                printf("[Auncient Presenter] ESC key via bridge. Exiting.\n");
                exit(0);
            }
        }
    } else if (strncmp(cmd, "Y:KU ", 5) == 0) {
        int key = 0;
        if (sscanf(cmd + 5, "%d", &key) == 1) {
            uint8_t cmd_byte = (0x02 << 6) | (key & 0x1F);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, (uint8_t)key);
            g_input_dirty = true;
        }
    }

    pthread_mutex_lock(&g_zmm_vm_mutex);
    extern int g_cached_contracts_count;
    if (g_cached_contracts_count > 0) {
        tsfi_mozilla_wmq_bridge_tick(&g_zmm_vm);
    }
    pthread_mutex_unlock(&g_zmm_vm_mutex);
}

int process_events(int fd, uint32_t xdg_s_id, bool *out_configure) {
    uint32_t h[2];
    ssize_t r = recv(fd, h, 8, MSG_DONTWAIT);
    if (r < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        return -1;
    }
    if (r == 0) return -1;

    uint32_t obj = h[0], sz = h[1] >> 16, op = h[1] & 0xFFFF;
    size_t plen = sz - 8;
    uint32_t *p = plen ? (uint32_t*)malloc(plen) : NULL;
    if (p) {
        if (recv(fd, p, plen, 0) <= 0) {
            free(p);
            return -1;
        }
    }

    if (obj == WL_REGISTRY_ID && op == 0) {
        char *iface = (char *)&p[2];
        if (!strcmp(iface, "wl_compositor")) global_compositor_id = p[0];
        else if (!strcmp(iface, "wl_shm")) global_shm_id = p[0];
        else if (!strcmp(iface, "xdg_wm_base")) global_xdg_id = p[0];
        else if (!strcmp(iface, "wl_seat")) global_seat_name = p[0];
    } else if (obj == xdg_s_id && op == 0) {
        uint32_t ack[] = {p[0]};
        send_msg(fd, xdg_s_id, XDG_SURFACE_ACK_CONFIGURE, ack, 4, -1);
        uint32_t geom[] = {0, 0, (uint32_t)g_w, (uint32_t)g_h};
        send_msg(fd, xdg_s_id, 3, geom, 16, -1);
        if (out_configure) *out_configure = true;
    } else if (xdg_wm_base_id && obj == xdg_wm_base_id && op == 0) {
        uint32_t pong[] = {p[0]};
        send_msg(fd, xdg_wm_base_id, 3, pong, 4, -1);
    } else if (obj == 1 && op == 0) {
        uint32_t err_obj = p[0];
        uint32_t code = p[1];
        uint32_t str_len = p[2];
        char *err_msg = (char *)&p[3];
        fprintf(stderr, "[Auncient Presenter ERR] Display Error: err_obj=%u, code=%u, msg=%.*s\n", err_obj, code, str_len, err_msg);
    } else if (xdg_s_id && (xdg_s_id + 1 == obj)) {
        if (op == 1) {
            printf("WINDOW_CLOSE\n");
            fflush(stdout);
            usleep(500000);
            exit(0);
        } else if (op == 0) {
            int32_t width = p[0];
            int32_t height = p[1];
            if (width > 0 && height > 0) {
                if (width <= MAX_W && height <= MAX_H) {
                    if (width != g_w || height != g_h) {
                        g_w = width;
                        g_h = height;
                        send_msg(fd, bid_val, 0, NULL, 0, -1);
                        bid_val = next_id++;
                        uint32_t bf_args[] = {bid_val, 0, g_w, g_h, g_w * 4, 1};
                        send_msg(fd, pid_val, 0, bf_args, 24, -1);
                        if (g_ptsfi_zmm_set_scanout_buffer && g_scanout_px) {
                            g_ptsfi_zmm_set_scanout_buffer(g_scanout_px, g_w, g_h);
                        }
                        printf("WINDOW_RESIZE %d %d\n", width, height);
                        fflush(stdout);
                    }
                }
            }
        }
    } else if (pointer_id && obj == pointer_id) {
        if (op == 2) {
            double x = ((int32_t)p[1]) / 256.0;
            double y = ((int32_t)p[2]) / 256.0;
            g_mouse_x = (uint16_t)x;
            g_mouse_y = (uint16_t)y;
            
            uint16_t vm_mx = (uint16_t)((float)x * 320.0f / (float)g_w);
            uint16_t vm_my = (uint16_t)((float)y * 300.0f / (float)g_h);
            char cmd_buf[128];
            snprintf(cmd_buf, sizeof(cmd_buf), "MOUSE_MOVE %d %d", vm_mx, vm_my);
            send_input_command(cmd_buf);
            
            if (p) free(p);
            return 2;
        } else if (op == 3) {
            uint32_t btn = p[2];
            uint32_t state = p[3];
            fprintf(stderr, "[DEBUG POINTER] Button: btn=%u, state=%u\n", btn, state);
            if (btn == 272) {
                g_click_state = (state == 1) ? 1 : 0;
            }
            
            char cmd_buf[128];
            if (state == 1) {
                snprintf(cmd_buf, sizeof(cmd_buf), "MOUSE_DOWN %u %d %d", btn, g_mouse_x, g_mouse_y);
            } else {
                snprintf(cmd_buf, sizeof(cmd_buf), "MOUSE_UP %u %d %d", btn, g_mouse_x, g_mouse_y);
            }
            send_input_command(cmd_buf);
            
            if (p) free(p);
            return 2;
        } else if (op == 4) {
            uint32_t axis = p[1];
            double value = ((int32_t)p[2]) / 256.0;
            fprintf(stderr, "[DEBUG POINTER] Axis: axis=%u, value=%f\n", axis, value);
            
            char cmd_buf[128];
            snprintf(cmd_buf, sizeof(cmd_buf), "MOUSE_SCROLL %u %f", axis, value);
            send_input_command(cmd_buf);
        }
    } else if (keyboard_id && obj == keyboard_id) {
        if (op == 3) {
            uint32_t key = p[2];
            uint32_t state = p[3];
            fprintf(stderr, "[DEBUG KEYBOARD] key=%u, state=%u\n", key, state);
            if (state == 1) {
                char cmd_buf[128];
                snprintf(cmd_buf, sizeof(cmd_buf), "KEY_DOWN %u", key + 8);
                send_input_command(cmd_buf);
                
                if (key == 1) {
                    printf("[Auncient Presenter] ESC pressed. Exiting gracefully.\n");
                    fflush(stdout);
                    exit(0);
                }
            } else {
                char cmd_buf[128];
                snprintf(cmd_buf, sizeof(cmd_buf), "KEY_UP %u", key + 8);
                send_input_command(cmd_buf);
            }
            if (p) free(p);
            return 2;
        }
    }
    if (p) free(p);
    return 1;
}
