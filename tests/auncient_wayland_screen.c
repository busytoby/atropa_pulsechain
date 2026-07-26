// Wayland Markdown Editor screen redraw and widget rendering component
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <wayland-client.h>

static void redraw_screen(void) {
    if (!surface) return;
    
    struct timespec t_start;
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    
    current_buffer_idx = 1 - current_buffer_idx;
    if (wl_buffers[current_buffer_idx]) {
        wl_buffer_destroy(wl_buffers[current_buffer_idx]);
    }
    
    uint32_t *pixels = NULL;
    wl_buffers[current_buffer_idx] = create_shm_buffer(win_width, win_height, &pixels);
    if (!wl_buffers[current_buffer_idx] || !pixels) return;
    
    static uint32_t *bg_cache = NULL;
    static int bg_cache_w = 0;
    static int bg_cache_h = 0;
    
    if (bg_cache == NULL || bg_cache_w != win_width || bg_cache_h != win_height) {
        if (bg_cache) free(bg_cache);
        bg_cache = malloc(win_width * win_height * sizeof(uint32_t));
        bg_cache_w = win_width;
        bg_cache_h = win_height;
        
        for (int y = 0; y < win_height; y++) {
            if (y >= 100 && y < 550 && (y % 24) == 0) {
                for (int x = 0; x < win_width; x++) {
                    bg_cache[y * win_width + x] = 0xFF000000;
                }
                continue;
            }
            
            uint32_t bg_color = 0xFF221100;
            float factor = (float)y / (float)win_height;
            uint8_t r_val, g_val, b_val;
            
            if (factor < 0.6f) {
                float t = factor / 0.6f;
                r_val = (uint8_t)(0x1A * (1.0f - t) + 0xFF * t);
                g_val = (uint8_t)(0x00 * (1.0f - t) + 0x55 * t);
                b_val = (uint8_t)(0x2A * (1.0f - t) + 0x00 * t);
            } else {
                float t = (factor - 0.6f) / 0.4f;
                r_val = (uint8_t)(0xFF * (1.0f - t) + 0x1C * t);
                g_val = (uint8_t)(0x55 * (1.0f - t) + 0x0B * t);
                b_val = (uint8_t)(0x00 * (1.0f - t) + 0x00 * t);
            }
            bg_color = (0xFF << 24) | (r_val << 16) | (g_val << 8) | b_val;
            
            for (int x = 0; x < win_width; x++) {
                bg_cache[y * win_width + x] = bg_color;
            }
        }
    }
    
    for (int y = 0; y < win_height; y++) {
        int b1_dist = abs(y - (120 + (int)(sinf(retro_time * 1.8f) * 90.0f)));
        int b2_dist = abs(y - (240 + (int)(sinf(retro_time * 2.5f) * 70.0f)));
        int b3_dist = abs(y - (360 + (int)(sinf(retro_time * 1.3f) * 100.0f)));
        
        uint32_t add_color = 0;
        if (b1_dist < 15) {
            uint8_t intensity = (15 - b1_dist) * 8;
            add_color = blend_color_add(add_color, (intensity << 16) | ((intensity / 3) << 8));
        }
        if (b2_dist < 20) {
            uint8_t intensity = (20 - b2_dist) * 6;
            add_color = blend_color_add(add_color, ((intensity / 2) << 16) | (intensity << 8));
        }
        if (b3_dist < 12) {
            uint8_t intensity = (12 - b3_dist) * 12;
            add_color = blend_color_add(add_color, intensity);
        }
        
        for (int x = 0; x < win_width; x++) {
            int idx = y * win_width + x;
            if (add_color != 0) {
                pixels[idx] = blend_color_add(bg_cache[idx], add_color);
            } else {
                pixels[idx] = bg_cache[idx];
            }
        }
    }
    
    for (int i = 0; i < 30; i++) {
        int px = (int)scroll_stars[i].x + glitch_x;
        int py = scroll_stars[i].y + glitch_y;
        if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
            draw_char(pixels, win_width, win_height, px, py, scroll_stars[i].glyph, scroll_stars[i].color, 1);
        }
    }

    static const HydraRenderDelegate hd_storm = { .render_scene = hd_storm_render };
    hd_storm.render_scene(pixels, win_width, win_height, &active_ubo);

    int tunnel_center_x = win_width / 2;
    int tunnel_center_y = win_height / 2;
    for (int i = 0; i < 4; i++) {
        float zoom_rad = fmodf(retro_time * 80.0f + i * 90.0f, 360.0f);
        int w_size = (int)(zoom_rad * 1.6f);
        int h_size = (int)(zoom_rad * 0.9f);
        
        for (int tx = -w_size; tx <= w_size; tx += 12) {
            draw_char(pixels, win_width, win_height, tunnel_center_x + tx + glitch_x, tunnel_center_y - h_size + glitch_y, '=', 0xFF331100, 1);
            draw_char(pixels, win_width, win_height, tunnel_center_x + tx + glitch_x, tunnel_center_y + h_size + glitch_y, '=', 0xFF331100, 1);
        }
        for (int ty = -h_size; ty <= h_size; ty += 12) {
            draw_char(pixels, win_width, win_height, tunnel_center_x - w_size + glitch_x, tunnel_center_y + ty + glitch_y, '|', 0xFF331100, 1);
            draw_char(pixels, win_width, win_height, tunnel_center_x + w_size + glitch_x, tunnel_center_y + ty + glitch_y, '|', 0xFF331100, 1);
        }
    }

    int base_scale = win_width / 280;
    if (base_scale < 1) base_scale = 1;
    if (base_scale > 6) base_scale = 6;
    
    int scale = base_scale;
    if (sid_chip.volume > 8) {
        scale = base_scale + 1;
    }
    
    int grid_w = 40 * 6 * scale;
    int start_x = (win_width - grid_w) / 2 + glitch_x;
    int start_y = win_height / 2 + 10 + glitch_y;
    if (start_x < 20) start_x = 20;
    if (start_y < win_height / 2) start_y = win_height / 2;
    
    char ansi_grid[280];
    memset(ansi_grid, ' ', sizeof(ansi_grid));
    auncient_parse_markdown_to_ansi(doc_buf, ansi_grid, 40, 7, 0);
    
    int cursor_r = doc_len / 40;
    int cursor_c = doc_len % 40;
    
    for (int r = 0; r < 7; r++) {
        int fine_shift = (vic_d016 & 0x07);
        int wave_idx = (int)(r * 15.0f + retro_time * 150.0f) & 0xFF;
        int row_displace = (int)(sine_lut[wave_idx] * 3.0f * scale) + fine_shift;
        
        for (int c = 0; c < 40; c++) {
            char ch = ansi_grid[r * 40 + c];
            
            float nx = ((float)c - 20.0f) / 10.0f;
            float ny = ((float)r - 3.5f) / 3.0f;
            float time_sin = sine_lut[(int)(retro_time * 120.0f) & 0xFF];
            float sphere_radius = 1.0f + time_sin * 0.2f;
            float dist_from_sphere = sqrtf(nx*nx + ny*ny) - sphere_radius;
            
            if (dist_from_sphere < 0.0f && ch == ' ') {
                char sphere_shades[] = "%#*+=-:. ";
                int shade_idx = (int)(fabsf(dist_from_sphere) * 8.0f);
                if (shade_idx < 0) shade_idx = 0;
                if (shade_idx > 8) shade_idx = 8;
                ch = sphere_shades[shade_idx];
            }
            
            int idx1 = (int)(c * 12.0f + retro_time * 180.0f) & 0xFF;
            int idx2 = (int)(r * 18.0f + retro_time * 120.0f) & 0xFF;
            float wave1 = sine_lut[idx1];
            float wave2 = sine_lut[idx2];
            float plasma = (wave1 + wave2) / 2.0f;
            
            if (ch == ' ') {
                char plasma_chars[] = " .:-=+*";
                int p_idx = (int)((plasma + 1.0f) * 3.0f);
                if (p_idx < 0) p_idx = 0;
                if (p_idx > 6) p_idx = 6;
                ch = plasma_chars[p_idx];
            } else {
                char mapped = charset_map_lut[(uint8_t)ch];
                if (mapped) ch = mapped;
            }
            
            int dist_r = r - cursor_r;
            int dist_c = c - cursor_c;
            float dist = sqrtf((float)(dist_r * dist_r + dist_c * dist_c));
            
            uint32_t color = 0xFFFFCC00;
            if (type_activity > 1.0f) {
                int type_color_idx = (int)(retro_time * 25.0f + c) & 0x0F;
                color = color_cycle_lut[type_color_idx];
            } else {
                if (dist < 4.0f) {
                    color = 0xFFFF8800;
                } else if (dist < 8.0f) {
                    color = 0xFFCC4400;
                } else {
                    color = 0xFF882200;
                }
            }
            
            if (ch == '|') color = 0xFFFFFF00;
            else if (ch == '*') color = 0xFFFF5500;
            
            int fld_stretch = (int)(sinf(retro_time * 2.5f) * 3.0f * scale);
            int row_y = start_y + r * (8 * scale + fld_stretch);
            
            draw_char(pixels, win_width, win_height, 
                      start_x + c * 6 * scale + row_displace, 
                      row_y, 
                      ch, color, scale);
        }
    }
    
    int art_start_x = (win_width - 24 * 12) / 2 + glitch_x;
    int art_start_y = 120 + glitch_y;
    for (int pr = 0; pr < 6; pr++) {
        for (int pc = 0; pc < 24; pc++) {
            char symbol = western_desert_art[pr][pc];
            if (symbol != ' ') {
                uint32_t art_color = 0xFF00FF00;
                if (symbol == '*') art_color = 0xFFFFCC00;
                else if (symbol == '.') art_color = 0xFFFF5500;
                else if (symbol == '=') art_color = 0xFF884400;
                
                draw_char(pixels, win_width, win_height, 
                          art_start_x + pc * 12, 
                          art_start_y + pr * 16, 
                          symbol, art_color, 2);
            }
        }
    }

    int logo_start_x = art_start_x - 50;
    int char_spacing = 64;
    
    static float sim_y[6] = {120.0f, 120.0f, 120.0f, 120.0f, 120.0f, 120.0f};
    static float sim_vy[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    static float sim_inflation[6] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    static float sim_scale_x[6] = {4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f};
    static float sim_scale_y[6] = {4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f};
    static float water_flood_height = 0.0f;
    static float last_sim_time = 0.0f;
    
    if (last_sim_time == 0.0f) {
        last_sim_time = retro_time;
    }
    float dt = retro_time - last_sim_time;
    if (dt > 0.1f) dt = 0.1f;
    last_sim_time = retro_time;
    
    int ground = 220;
    (void)sim_vy;
    
    static int prebaked_frame_idx = 0;
    static float last_frame_time = 0.0f;
    if (retro_time - last_frame_time > 0.12f) {
        last_frame_time = retro_time;
        prebaked_frame_idx = (prebaked_frame_idx + 1) % 5;
        
        PrebakedFrame f = prebaked_script[prebaked_frame_idx];
        water_flood_height += f.water_change * 0.12f;
        if (water_flood_height > 25.0f) water_flood_height = 25.0f;
        
        for (int i = 0; i < f.update_count; i++) {
            int id = f.updates[i].char_id;
            if (id < 6) {
                sim_y[id] = f.updates[i].y;
                sim_scale_x[id] = 4.2f;
                sim_scale_y[id] = 3.8f;
                sim_inflation[id] = 2.2f;
            }
        }
    }
    
    for (int i = 0; i < 6; i++) {
        sim_inflation[i] += (1.0f - sim_inflation[i]) * dt * 2.0f;
        sim_scale_x[i] += (4.0f - sim_scale_x[i]) * dt * 4.0f;
        sim_scale_y[i] += (4.0f - sim_scale_y[i]) * dt * 4.0f;
    }
    
    simulate_smc_remove_next_line(pixels, bg_cache, win_width, win_height, ground, water_flood_height);

    for (int char_idx = 0; char_idx < 6; char_idx++) {
        int logo_start_y = (int)sim_y[char_idx];
        float elastic_scale_x = sim_scale_x[char_idx];
        float elastic_scale_y = sim_scale_y[char_idx];
        for (int r = 0; r < 16; r++) {
            uint16_t row_bits = bubble_font_tsfi2[char_idx][r];
            for (int c = 0; c < 16; c++) {
                if (row_bits & (1 << (15 - c))) {
                    int wobble_idx_x = (int)(retro_time * 90.0f + r * 12 + c * 6 + char_idx * 24) & 0xFF;
                    int wobble_idx_y = (int)(retro_time * 75.0f + r * 8 + c * 10 + char_idx * 30) & 0xFF;
                    int wobble_x = (int)(sine_lut[wobble_idx_x] * 3.0f);
                    int wobble_y = (int)(sine_lut[wobble_idx_y] * 3.0f);
                    
                    int pixel_x = logo_start_x + char_idx * char_spacing + c * elastic_scale_x + wobble_x + 6;
                    int pixel_y = logo_start_y + r * elastic_scale_y + wobble_y + 6;
                    for (int sy = 0; sy < (int)elastic_scale_y; sy++) {
                        for (int sx = 0; sx < (int)elastic_scale_x; sx++) {
                            int px = pixel_x + sx;
                            int py = pixel_y + sy;
                            if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
                                pixels[py * win_width + px] = 0xFF1C0500;
                            }
                        }
                    }
                }
            }
        }
    }

    for (int char_idx = 0; char_idx < 6; char_idx++) {
        int logo_start_y = (int)sim_y[char_idx];
        float elastic_scale_x = sim_scale_x[char_idx];
        float elastic_scale_y = sim_scale_y[char_idx];
        
        int step_idx = (int)((sim_inflation[char_idx] - 1.0f) * 2.0f);
        if (step_idx < 0) step_idx = 0;
        if (step_idx > 4) step_idx = 4;
        
        MorphFrame *frame = &morph_cache[char_idx][step_idx];
        
        for (int r = 0; r < 16; r++) {
            for (int c = 0; c < 16; c++) {
                uint16_t mask = 1 << (15 - c);
                bool is_dilated = frame->dilated[r] & mask;
                if (!is_dilated) continue;
                
                int wobble_idx_x = (int)(retro_time * 90.0f + r * 12 + c * 6 + char_idx * 24) & 0xFF;
                int wobble_idx_y = (int)(retro_time * 75.0f + r * 8 + c * 10 + char_idx * 30) & 0xFF;
                int wobble_x = (int)(sine_lut[wobble_idx_x] * 3.0f);
                int wobble_y = (int)(sine_lut[wobble_idx_y] * 3.0f);
                
                int pixel_x = logo_start_x + char_idx * char_spacing + c * elastic_scale_x + wobble_x;
                int pixel_y = logo_start_y + r * elastic_scale_y + wobble_y;
                
                uint32_t pixel_color = 0x00000000;
                bool is_eroded = frame->eroded[r] & mask;
                bool is_source = bubble_font_tsfi2[char_idx][r] & mask;
                bool is_glint = frame->glints[r] & mask;
                
                if (is_glint) {
                    pixel_color = 0xFFFFFFFF;
                } else if (is_eroded) {
                    pixel_color = 0xFFFFCC00;
                } else if (is_source) {
                    pixel_color = 0xFFFF6600;
                } else {
                    int color_idx = (int)(retro_time * 15.0f + char_idx * 4 + c) & 0x0F;
                    pixel_color = color_cycle_lut[color_idx];
                }
                
                int sheen_pos = ((int)(retro_time * 200.0f)) % 900 - 200;
                int dist_to_sheen = abs((char_idx * char_spacing + c * 4 + r * 4) - sheen_pos);
                if (dist_to_sheen < 12) {
                    pixel_color = 0xFFFFFFFF;
                }
                
                int ground_val = 220;
                
                for (int sy = 0; sy < (int)elastic_scale_y; sy++) {
                    for (int sx = 0; sx < (int)elastic_scale_x; sx++) {
                        int px = pixel_x + sx;
                        int py = pixel_y + sy;
                        if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
                            uint32_t *dest_pixel = &pixels[py * win_width + px];
                            uint32_t bg_val = bg_cache[py * win_width + px];
                            uint32_t boundary_mask = (py >= ground_val + 30) ? 0x00FFFFFF : 0xFFFFFFFF;
                            
                            if (is_glint && sx < 2 && sy < 2) {
                                *dest_pixel = (*dest_pixel & ~boundary_mask) | (0xFFFFFFFF & boundary_mask);
                            } else {
                                uint32_t blended_color = pixel_color;
                                if (boundary_mask != 0xFFFFFFFF) {
                                    blended_color = bg_val;
                                }
                                *dest_pixel = blended_color;
                            }
                        }
                    }
                }
            }
        }
    }

    int frame_y_top = start_y - 12 * scale;
    int frame_y_bottom = start_y + 7 * 8 * scale + 4 * scale;
    for (int col = 0; col < 40; col++) {
        int frame_char_idx = (col + (int)(retro_time * 12.0f)) % 6;
        char frame_chars[] = "[=#=-]";
        char ch = frame_chars[frame_char_idx];
        draw_char(pixels, win_width, win_height, start_x + col * 6 * scale, frame_y_top, ch, 0xFFFF8800, scale);
        draw_char(pixels, win_width, win_height, start_x + col * 6 * scale, frame_y_bottom, ch, 0xFFFF8800, scale);
    }

    for (int i = 0; i < 5; i++) {
        sprites[i].x = 100 + i * 200 + (int)(retro_time * 60.0f) % (win_width - 200);
        int wave_idx = ((int)(retro_time * 80.0f) + i * 40) & 0xFF;
        sprites[i].y = win_height / 2 + (int)(sine_lut[wave_idx] * 80.0f);
    }
    
    for (int i = 0; i < 5; i++) sorted_sprite_indices[i] = i;
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (sprites[sorted_sprite_indices[i]].y > sprites[sorted_sprite_indices[j]].y) {
                int temp = sorted_sprite_indices[i];
                sorted_sprite_indices[i] = sorted_sprite_indices[j];
                sorted_sprite_indices[j] = temp;
            }
        }
    }
    
    for (int i = 0; i < 5; i++) {
        int idx = sorted_sprite_indices[i];
        draw_char(pixels, win_width, win_height, 
                  sprites[idx].x + glitch_x, 
                  sprites[idx].y + glitch_y, 
                  sprites[idx].glyph, 
                  sprites[idx].color, 3);
    }
    
    float p_scroll_speed = 60.0f;
    float p_scroll_x_total = retro_time * p_scroll_speed;
    int p_pixel_shift = (int)fmodf(p_scroll_x_total, 12.0f);
    int p_scroller_y = 60 + glitch_y;
    int len = strlen(parallax_scroller_text);
    int p_base_char_idx = len - ((int)(p_scroll_x_total / 12.0f) % len);
    
    for (int col = 0; col < 90; col++) {
        int char_idx = (p_base_char_idx + col) % len;
        char ch = parallax_scroller_text[char_idx];
        
        int dyn_scale = 2 + (int)(sinf(retro_time * 4.0f + col * 0.25f) * 0.6f);
        if (dyn_scale < 1) dyn_scale = 1;
        
        int px = 20 + col * 12 + p_pixel_shift + glitch_x;
        draw_char(pixels, win_width, win_height, px, p_scroller_y - (dyn_scale * 2), ch, 0xFFFFCC00, dyn_scale);
    }

    float scroll_x_speed = 30.0f;
    if (active_tune == 1) scroll_x_speed = 45.0f;
    else if (active_tune == 2) scroll_x_speed = 60.0f;
    else if (active_tune == 3) scroll_x_speed = 90.0f;
    
    float scroll_x_total = retro_time * scroll_x_speed;
    int base_char_idx = (int)(scroll_x_total / 24.0f) % strlen(decompressed_scroller);
    int pixel_shift = (int)fmodf(scroll_x_total, 24.0f);
    int scroller_y_base = win_height - 100 + glitch_y;
    
    int visible_cols = win_width / 24 + 2;
    for (int col = 0; col < visible_cols; col++) {
        int char_idx = (base_char_idx + col) % strlen(decompressed_scroller);
        char ch = decompressed_scroller[char_idx];
        
        int lut_step = (col * 10 + (int)(retro_time * 250.0f)) & 0xFF;
        int dy = (int)(sine_lut[lut_step] * 10.0f);
        
        uint32_t wash_color = color_cycle_lut[(col + (int)(retro_time * 20.0f)) & 0x0F];
        
        draw_char(pixels, win_width, win_height, 40 + col * 24 - pixel_shift + glitch_x, scroller_y_base + dy, ch, wash_color, 2);
    }
    
    draw_string(pixels, win_width, win_height, 100 + glitch_x, 30 + glitch_y, "AUNCIENT WAYLAND VULKAN MARKDOWN EDITOR", 0xFFFF8800, 2);
    
    char sid_buf[256];
    snprintf(sid_buf, sizeof(sid_buf), 
             "SID TUNE %d %s | FREQ=0x%04X PW=0x%04X ADSR=0x%02X%02X VOL=%d | SYNC:%s RM:%s | V1:%s V2:%s V3:%s", 
             active_tune, (active_tune == 3) ? "(HIDDEN UNLOCKED!)" : "(ACTIVE)",
             sid_chip.voices[0].freq, sid_chip.voices[1].pw,
             sid_chip.voices[0].adsr[0], sid_chip.voices[0].adsr[1],
             sid_chip.volume,
             sid_chip.hard_sync_enabled ? "ON" : "OFF",
             sid_chip.ring_mod_enabled ? "ON" : "OFF",
             voice_active[0] ? "ON" : "OFF",
             voice_active[1] ? "ON" : "OFF",
             voice_active[2] ? "ON" : "OFF");
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 65 + glitch_y, sid_buf, 0xFFFFFF00, 1);
 
    huc6280_psg.channels[0].freq = sid_chip.voices[0].freq / 2;
    huc6280_psg.channels[0].volume = (sid_chip.volume * 31) / 15;
    huc6280_psg.channels[0].pan = 0xDA;
    huc6280_psg.channels[1].freq = sid_chip.voices[1].freq / 2;
    huc6280_psg.channels[1].volume = (sid_chip.volume * 24) / 15;
    huc6280_psg.channels[1].pan = 0x24;
 
    for (int i = 0; i < 32; i++) {
        huc6280_psg.channels[0].waveform[i] = (uint8_t)(15.0f + 15.0f * sinf(i * 2.0f * M_PI / 32.0f));
        huc6280_psg.channels[1].waveform[i] = (uint8_t)(i < 16 ? i * 2 : (31 - i) * 2);
        huc6280_psg.channels[2].waveform[i] = (uint8_t)(i * 31 / 32);
        huc6280_psg.channels[3].waveform[i] = (uint8_t)(i < 16 ? 31 : 0);
        huc6280_psg.channels[4].waveform[i] = (uint8_t)((i + (int)(retro_time * 50.0f)) % 7 == 0 ? 25 : 5);
        huc6280_psg.channels[5].waveform[i] = (uint8_t)(i < 8 ? 31 : 0);
    }
    int osc_base_x = 120 + glitch_x;
    int osc_base_y = win_height - 80 + glitch_y;
    for (int c = 0; c < 6; c++) {
        char osc_label[8];
        snprintf(osc_label, sizeof(osc_label), "CH%d", c + 1);
        draw_string(pixels, win_width, win_height, osc_base_x + c * 80, osc_base_y - 15, osc_label, 0xFF888888, 1);
        
        for (int col = 0; col < 32; col++) {
            int wave_val = huc6280_psg.channels[c].waveform[(col + (int)(retro_time * 40.0f)) % 32];
            int py = osc_base_y - (wave_val * 12 / 32);
            if (py >= 0 && py < win_height && (osc_base_x + c * 80 + col) < win_width) {
                pixels[py * win_width + osc_base_x + c * 80 + col] = 0xFF00FF00;
            }
        }
    }
 
    active_ubo.rotation_angle = 0.0f;
    active_ubo.camera_y = 0.0f;
    active_ubo.active_model = (uint32_t)raymarch_mode;
    active_ubo.raster_intensity = 1.0f;
    active_ubo.material_variant = (uint32_t)material_variant;
 
    static const HydraRenderDelegate hd_embree = { .render_scene = hd_embree_render };
    hd_embree.render_scene(pixels, win_width, win_height, &active_ubo);
 
    char psg_buf[256];
    snprintf(psg_buf, sizeof(psg_buf), 
             "HUDSON HUC6280 PSG | CH1: FREQ=0x%04X VOL=%2d PAN=0x%02X | CH2: FREQ=0x%04X VOL=%2d PAN=0x%02X",
             huc6280_psg.channels[0].freq, huc6280_psg.channels[0].volume, huc6280_psg.channels[0].pan,
             huc6280_psg.channels[1].freq, huc6280_psg.channels[1].volume, huc6280_psg.channels[1].pan);
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 52 + glitch_y, psg_buf, 0xFF00FF00, 1);
 
    ted_chip.ff09 = (ted_chip.ff09 + 1) & 0xFF;
    ted_chip.ff0b = vic_d012;
    ted_chip.ff15 = (uint8_t)(sid_chip.voices[1].freq >> 8);
    ted_chip.ff19 = (uint8_t)(sid_chip.voices[0].freq >> 8);
 
    char ted_buf[256];
    snprintf(ted_buf, sizeof(ted_buf),
             "TED MOS 8360 | ff06=0x%02X ff07=0x%02X ff09=0x%02X ff0b=%3d | WATER INDEX: %02d/40",
             ted_chip.ff06, ted_chip.ff07, ted_chip.ff09, ted_chip.ff0b,
             current_water_idx);
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 39 + glitch_y, ted_buf, 0xFF00FFFF, 1);
 
    const char *initials = ((int)retro_time % 2 == 0) ? "TSN" : "TNS";
    
    struct timespec t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double latency_us = (t_end.tv_sec - t_start.tv_sec) * 1000000.0 + (t_end.tv_nsec - t_start.tv_nsec) / 1000.0;
    (void)latency_us;

    const char *m_names[3] = {"CAC", "LET", "UNI"};
    const char *v_names[3] = {"GLD", "CLY", "CLO"};
    char help_buf[256];
    snprintf(help_buf, sizeof(help_buf), 
             "VIC: d012=%d | RAY: %s (%s) | RAST: %3.0fus | 'B' VAR, 'S' SAVE | TR: %s", 
             vic_d012, m_names[raymarch_mode], v_names[material_variant], latency_us, initials);
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 26 + glitch_y, help_buf, 0xFFFFCC00, 1);
    
    wl_surface_attach(surface, wl_buffers[current_buffer_idx], 0, 0);
    wl_surface_damage(surface, 0, 0, win_width, win_height);
    wl_surface_commit(surface);
}
