CC       = gcc
CXX      = g++
CFLAGS   = -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -Iinc -Isrc -Iplugins -Iplugins/vulkan -Iplugins/window_inc -Iplugins/window_src -Iassets/sd_cpp/include -Isrc/firmware \
           -O3 -g -march=native -flax-vector-conversions \
           -fstack-protector-strong -D_FORTIFY_SOURCE=3 -fPIE -pie -Wl,-z,relro,-z,now \
           -DTSFI_PROVENANCE_KEY=\"SIG_2026_USLM_AFFIRMED\"
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -O3 -g -march=native -Iinc -Iplugins -Iplugins/vulkan -Iassets/sd_cpp/include
LDFLAGS  = -L/usr/local/lib -ljpeg -rdynamic -lwayland-client -lvulkan -Wl,-z,noexecstack -lm -lgomp -lrt -lpthread -ldl -lstdc++ -lasound

OBJ_DIR  = obj
BIN_DIR  = bin
SRC_DIR  = src

# --- Core Simulation Objects ---
CORE_OBJS = $(OBJ_DIR)/tsfi_wire_firmware.o $(OBJ_DIR)/tsfi_logic.o $(OBJ_DIR)/tsfi_hotloader.o \
            $(OBJ_DIR)/tsfi_fourier.o $(OBJ_DIR)/tsfi_genetic.o $(OBJ_DIR)/tsfi_hilbert_avx512.o \
            $(OBJ_DIR)/tsfi_vision.o $(OBJ_DIR)/lau_audit.o $(OBJ_DIR)/tsfi_crash_handler.o \
            $(OBJ_DIR)/tsfi_spirv_kernels.o $(OBJ_DIR)/tsfi_fused_operator.o $(OBJ_DIR)/tsfi_vision_metrics.o \
            $(OBJ_DIR)/lau_registry.o $(OBJ_DIR)/tsfi_ottype_geom.o $(OBJ_DIR)/tsfi_hilbert.o \
            $(OBJ_DIR)/tsfi_font_shaping_parser.o $(OBJ_DIR)/tsfi_k0rn_compiler.o $(OBJ_DIR)/tsfi_verified_encode.o \
            $(OBJ_DIR)/tsfi_broadcaster_core.o $(OBJ_DIR)/tsfi_vision_vk.o $(OBJ_DIR)/tsfi_ottype_shape.o \
            $(OBJ_DIR)/tsfi_video_thunks.o $(OBJ_DIR)/tsfi_io.o $(OBJ_DIR)/tsfi_rtmp.o \
            $(OBJ_DIR)/tsfi_vision_thunks.o $(OBJ_DIR)/tsfi_broadcaster_bandwidth_throttle.o \
            $(OBJ_DIR)/tsfi_wire_pty.o $(OBJ_DIR)/tsfi_zhao.o $(OBJ_DIR)/lau_vram.o \
            $(OBJ_DIR)/tsfi_resonance.o $(OBJ_DIR)/tsfi_dl_thunks.o $(OBJ_DIR)/tsfi_k0rn_codegen.o \
            $(OBJ_DIR)/tsfi_reaction.o $(OBJ_DIR)/tsfi_font_rasterizer.o $(OBJ_DIR)/tsfi_phenotype_loader.o \
            $(OBJ_DIR)/tsfi_broadcaster_ui.o $(OBJ_DIR)/tsfi_font_ai.o $(OBJ_DIR)/tsfi_ottype_layout.o \
            $(OBJ_DIR)/tsfi_helmk0rn_factory.o $(OBJ_DIR)/tsfi_vulkan_video_encode.o $(OBJ_DIR)/tsfi_vision_io.o \
            $(OBJ_DIR)/tsfi_vec_math.o $(OBJ_DIR)/tsfi_variations.o $(OBJ_DIR)/tsfi_font_engine.o \
            $(OBJ_DIR)/tsfi_vision_analysis.o $(OBJ_DIR)/tsfi_math.o $(OBJ_DIR)/tsfi_config.o \
            $(OBJ_DIR)/tsfi_dys_math.o $(OBJ_DIR)/tsfi_cff.o $(OBJ_DIR)/tsfi_wave512_asm.o \
            $(OBJ_DIR)/tsfi_trace.o $(OBJ_DIR)/tsfi_ottype.o $(OBJ_DIR)/tsfi_dysnomia.o \
            $(OBJ_DIR)/tsfi_zmm_vm.o $(OBJ_DIR)/tsfi_font_registry.o \
            $(OBJ_DIR)/tsfi_ipomoea_lifecycle.o \
            $(OBJ_DIR)/tsfi_shader_dispatch.o $(OBJ_DIR)/lau_memory.o $(OBJ_DIR)/tsfi_elf_reflect.o \
            $(OBJ_DIR)/tsfi_opt_zmm_unity.o $(OBJ_DIR)/tsfi_opt_zmm.o $(OBJ_DIR)/tsfi_glob.o \
            $(OBJ_DIR)/tsfi_operator.o $(OBJ_DIR)/tsfi_yang.o $(OBJ_DIR)/tsfi_quan.o \
            $(OBJ_DIR)/tsfi_c_math.o $(OBJ_DIR)/firmware/LauWireFirmware_rtl.o \
            $(OBJ_DIR)/tsfi_vision_classify.o $(OBJ_DIR)/tsfi_cli.o $(OBJ_DIR)/tsfi_svdag.o \
            $(OBJ_DIR)/lau_thunk.o $(OBJ_DIR)/lau_yul_thunk.o $(OBJ_DIR)/tsfi_wiring.o $(OBJ_DIR)/tsfi_vulkan_cells.o \
            $(OBJ_DIR)/tsfi_merkle.o \
            $(OBJ_DIR)/tsfi_pool.o $(OBJ_DIR)/tsfi_time.o $(OBJ_DIR)/tsfi_staging.o \
            $(OBJ_DIR)/tsfi_safetensors_cache.o \
            $(OBJ_DIR)/tsfi_taste_cache.o \
            $(OBJ_DIR)/tsfi_shading_manifold.o \
            $(OBJ_DIR)/tsfi_vae_firmware.o $(OBJ_DIR)/tsfi_liang_barsky.o \
            $(OBJ_DIR)/tsfi_nand_trap_firmware.o $(OBJ_DIR)/tsfi_vessel_anchor.o \
            $(OBJ_DIR)/tsfi_vae_upsampler.o \
            $(OBJ_DIR)/tsfi_fur_shader.o \
            $(OBJ_DIR)/tsfi_masterpiece_render.o \
            $(OBJ_DIR)/tsfi_moon_chuuker.o \
            $(OBJ_DIR)/tsfi_dream_streamer.o \
            $(OBJ_DIR)/tsfi_path_tracer.o \
            $(OBJ_DIR)/tsfi_pbr.o \
            $(OBJ_DIR)/tsfi_hair.o \
            $(OBJ_DIR)/tsfi_merkle_fast.o $(OBJ_DIR)/tsfi_scramble_fast.o $(OBJ_DIR)/tsfi_jpeg_encoder.o \
            $(OBJ_DIR)/tsfi_yang_baseline.o $(OBJ_DIR)/tsfi_yang_fast.o $(OBJ_DIR)/tsfi_comm.o \
            $(OBJ_DIR)/tsfi_controlnet_shm.o $(OBJ_DIR)/tsfi_puppetry.o \
            $(OBJ_DIR)/tsfi_pulsechain_rpc.o $(OBJ_DIR)/tsfi_tls.o \
            $(OBJ_DIR)/tsfi_journal.o $(OBJ_DIR)/tsfi_drm_lease_thunk.o $(OBJ_DIR)/tsfi_evolution_bridge.o $(OBJ_DIR)/tsfi_ui_kernels.o \
            $(OBJ_DIR)/tsfi_dl_thunks.o $(OBJ_DIR)/lau_vram.o \
            $(OBJ_DIR)/drm-lease-v1-protocol.o

# --- Plugin Objects ---
VULKAN_PLUGIN_OBJS = $(OBJ_DIR)/vulkan_init.o $(OBJ_DIR)/vulkan_main.o $(OBJ_DIR)/vulkan_render.o \
                     $(OBJ_DIR)/vulkan_input.o $(OBJ_DIR)/backend_display.o $(OBJ_DIR)/input_vty.o \
                     $(OBJ_DIR)/tsfi_input.o $(OBJ_DIR)/window_rendering.o \
                     $(OBJ_DIR)/vulkan_logic.o $(OBJ_DIR)/vulkan_xdg.o $(OBJ_DIR)/vulkan_dmabuf.o \
                     $(OBJ_DIR)/xdg-shell-protocol.o $(OBJ_DIR)/linux-dmabuf-v1-protocol.o \
                     $(OBJ_DIR)/wlr-layer-shell-unstable-v1-protocol.o

.PHONY: all clean

all: tsfi2 bin/test_genesis_standalone tests/test_ahoy_issue1_resonance bin/test_wayland_terminal_shell bin/tsfi_font_editor bin/test_spider_registry_reap bin/test_spider_120f_reap tests/test_wayland_hex_rpg tests/test_wayland_lunar_lander tests/test_wayland_gauntlet tests/test_wayland_yars_revenge tests/test_yars_revenge_features

# --- Primary Targets ---
tsfi2: $(OBJ_DIR)/main.o $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_genesis_standalone: tests/test_genesis_standard_cell.c $(CORE_OBJS) $(OBJ_DIR)/Genesis_rtl.o $(OBJ_DIR)/Dai_rtl.o $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -Isrc/firmware -o $@ $^ $(LDFLAGS)

bin/test_spider_registry_reap: tests/test_spider_registry_reap.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_spider_120f_reap: tests/test_spider_120f_reap.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_ahoy_issue1_resonance: tests/test_ahoy_issue1_resonance.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_fighter: tests/test_zmm_fighter.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_graphics: tests/test_zmm_graphics.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_parallel_cpu: tests/test_zmm_parallel_cpu.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_yars_revenge_features: tests/test_yars_revenge_features.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_hardball: tests/test_wayland_hardball.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_smurfs: tests/test_wayland_smurfs.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_hex_rpg: tests/test_wayland_hex_rpg.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_lunar_lander: tests/test_wayland_lunar_lander.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_yars_revenge: tests/test_wayland_yars_revenge.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_gauntlet: tests/test_wayland_gauntlet.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_wayland_terminal_shell: tests/test_wayland_terminal_shell.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_font_editor: src/tsfi_font_editor.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# --- Tool Realization ---
bin/tsfi_v2c: tools/tsfi_v2c.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

bin/tsfi_rdl2c: tools/tsfi_rdl2c.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# --- Firmware Realization ---
src/firmware/Genesis_rtl.c src/firmware/Genesis_rtl.h: src/firmware/Genesis.v bin/tsfi_v2c
	./bin/tsfi_v2c $<

src/firmware/Dai_rtl.c src/firmware/Dai_rtl.h: src/firmware/00_Dai.rdl bin/tsfi_rdl2c
	./bin/tsfi_rdl2c $<

src/firmware/LauWireFirmware_rtl.c src/firmware/LauWireFirmware_rtl.h: src/firmware/LauWireFirmware.v bin/tsfi_v2c
	./bin/tsfi_v2c $<

# --- Object Rules ---
$(OBJ_DIR)/tsfi_wire_firmware.o: src/tsfi_wire_firmware.c src/firmware/LauWireFirmware_rtl.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: src/firmware/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/firmware/%.o: src/firmware/%.c | $(OBJ_DIR)/firmware
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/vulkan/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/window_src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR)/firmware:
	mkdir -p $(OBJ_DIR)/firmware

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) tsfi2 src/firmware/*_rtl.*
