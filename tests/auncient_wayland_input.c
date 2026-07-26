// Wayland Markdown Editor input and keyboard event listener routines
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <wayland-client.h>

static void keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
    (void)data; (void)wl_keyboard; (void)format; (void)size;
    close(fd);
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    (void)data; (void)wl_keyboard; (void)serial; (void)surface; (void)keys;
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
    (void)data; (void)wl_keyboard; (void)serial; (void)surface;
}

static void keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)wl_keyboard; (void)serial; (void)time;
    if (state == 0) return;

    if (key == 1) {
        running = false;
        printf("[INFO] Escape key pressed. Editor shutting down...\n");
        return;
    }

    if (key == 2) {
        voice_active[0] = !voice_active[0];
        printf("[AUDIO] Voice 1 toggled: %s\n", voice_active[0] ? "ON" : "OFF");
        redraw_screen();
        return;
    } else if (key == 3) {
        voice_active[1] = !voice_active[1];
        printf("[AUDIO] Voice 2 toggled: %s\n", voice_active[1] ? "ON" : "OFF");
        redraw_screen();
        return;
    } else if (key == 50) {
        raymarch_mode = (raymarch_mode + 1) % 3;
        printf("[RAY] Ray Marching Mode changed: %d\n", raymarch_mode);
        redraw_screen();
        return;
    } else if (key == 48) {
        material_variant = (material_variant + 1) % 3;
        printf("[USD] Material Variant Set changed: %d\n", material_variant);
        redraw_screen();
        return;
    } else if (key == 31) {
        USDStageRecord stage = {
            .active_model = (uint32_t)raymarch_mode,
            .material_variant = (uint32_t)material_variant,
            .rotation_angle = active_ubo.rotation_angle,
            .camera_y = active_ubo.camera_y,
            .starfield_count = 15
        };
        FILE *f_usd = fopen("assets/usd_stage.dat.bin", "wb");
        if (f_usd) {
            fwrite(&stage, sizeof(stage), 1, f_usd);
            fclose(f_usd);
            printf("[USD] Scene stage layer exported to assets/usd_stage.dat.bin\n");
            loader_flash_time = 0.5f;
        }
        redraw_screen();
        return;
    } else if (key == 22) {
        USDStageRecord stage = {
            .active_model = (uint32_t)raymarch_mode,
            .material_variant = (uint32_t)material_variant,
            .rotation_angle = active_ubo.rotation_angle,
            .camera_y = active_ubo.camera_y,
            .starfield_count = 15
        };
        FILE *f_usd = fopen("assets/usd_special.dat.bin", "wb");
        if (f_usd) {
            fwrite(&stage, sizeof(stage), 1, f_usd);
            fclose(f_usd);
            printf("[USD] Specialized template layer exported to assets/usd_special.dat.bin\n");
            loader_flash_time = 0.5f;
        }
        redraw_screen();
        return;
    } else if (key == 4) {
        voice_active[2] = !voice_active[2];
        printf("[AUDIO] Voice 3 toggled: %s\n", voice_active[2] ? "ON" : "OFF");
        redraw_screen();
        return;
    }

    type_activity += 1.2f;
    if (type_activity > 5.0f) type_activity = 5.0f;

    char typed_char = '\0';
    if (key == 28) {
        uint32_t hash = 2166136261U;
        for (int i = 0; i < doc_len; i++) {
            hash ^= (uint8_t)doc_buf[i];
            hash *= 16777619U;
        }
        
        sdk_cics_context_t cics_ctx;
        memset(&cics_ctx, 0, sizeof(cics_ctx));
        cics_ctx.writer_id = 0x4001;
        cics_ctx.security_clearance = 1;
        cics_ctx.state = SDK_STATE_UNLOCKED;
        bool approvals[4] = {true, true, true, true};
        auncient_sdk_cics_exec(&cics_ctx, (uint32_t)doc_len, approvals);

        EditorHistoryRecord rec = {
            .transaction_id = 0x4001,
            .state_code = doc_len,
            .action = 'E',
            .hash = hash
        };
        
        FILE *f = fopen("assets/editor_history.dat.bin", "ab");
        if (f) {
            fwrite(&rec, sizeof(rec), 1, f);
            fclose(f);
            printf("[LEDGER] Transaction committed to assets/editor_history.dat.bin via CICS exec\n");
            loader_flash_time = 0.5f;
        }
        
        typed_char = '\n';
    } else if (key == 57) {
        typed_char = ' ';
    } else if (key == 14) {
        if (doc_len > 0) {
            doc_len--;
            doc_buf[doc_len] = '\0';
            redraw_screen();
        }
        return;
    } else {
        switch (key) {
            case 30: typed_char = 'a'; break;
            case 48: typed_char = 'b'; break;
            case 46: typed_char = 'c'; break;
            case 32: typed_char = 'd'; break;
            case 18: typed_char = 'e'; break;
            case 33: typed_char = 'f'; break;
            case 34: typed_char = 'g'; break;
            case 35: typed_char = 'h'; break;
            case 23: typed_char = 'i'; break;
            case 36: typed_char = 'j'; break;
            case 37: typed_char = 'k'; break;
            case 38: typed_char = 'l'; break;
            case 50: typed_char = 'm'; break;
            case 49: typed_char = 'n'; break;
            case 24: typed_char = 'o'; break;
            case 25: typed_char = 'p'; break;
            case 16: typed_char = 'q'; break;
            case 19: typed_char = 'r'; break;
            case 31: typed_char = 's'; break;
            case 20: typed_char = 't'; break;
            case 22: typed_char = 'u'; break;
            case 47: typed_char = 'v'; break;
            case 17: typed_char = 'w'; break;
            case 45: typed_char = 'x'; break;
            case 21: typed_char = 'y'; break;
            case 44: typed_char = 'z'; break;
            case 41: typed_char = '#'; break;
            case 52: typed_char = '>'; break;
            default: typed_char = '\0'; break;
        }
    }

    if (typed_char >= 'a' && typed_char <= 'z') {
        key_history[0] = key_history[1];
        key_history[1] = key_history[2];
        key_history[2] = typed_char;
        if (key_history[0] == 't' && key_history[1] == 's' && key_history[2] == 'n') {
            hidden_unlocked = true;
            active_tune = 3;
            printf("[INFO] Hidden SID Tune 3 Unlocked! Warp Scroll Enabled.\n");
        }
    }

    if (typed_char == '#') {
        active_tune = (active_tune + 1) % 3;
        redraw_screen();
        return;
    }

    if (typed_char && doc_len < (int)sizeof(doc_buf) - 1) {
        doc_buf[doc_len++] = typed_char;
        doc_buf[doc_len] = '\0';
        if (voice_active[0]) {
            sid_chip.voices[0].freq = 0x2200 + (doc_len * 32);
            sid_chip.voices[0].ctrl = 0x21;
        }
        redraw_screen();
    }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    (void)data; (void)wl_keyboard; (void)serial; (void)mods_depressed; (void)mods_latched; (void)mods_locked; (void)group;
}

static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
    (void)data; (void)wl_keyboard; (void)rate; (void)delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_handle_keymap,
    .enter = keyboard_handle_enter,
    .leave = keyboard_handle_leave,
    .key = keyboard_handle_key,
    .modifiers = keyboard_handle_modifiers,
    .repeat_info = keyboard_handle_repeat_info,
};
