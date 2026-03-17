CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -Iinc -Iplugins -Iplugins/vulkan -Iplugins/window_inc -Iplugins/window_src -Iassets/sd_cpp/include \
        -O3 -g -march=native -flax-vector-conversions \
        -fstack-protector-strong -D_FORTIFY_SOURCE=3 -fPIE -pie -Wl,-z,relro,-z,now \
        -DTSFI_PROVENANCE_KEY=\"SIG_2026_USLM_AFFIRMED\"
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -O3 -g -march=native -Iinc -Iplugins -Iplugins/vulkan -Iassets/sd_cpp/include
LDFLAGS = -ljpeg -rdynamic -lubsan -lwayland-client -lvulkan -Wl,-z,noexecstack -lm -lgomp

OBJ_DIR = obj
BIN_DIR = bin
SRC_DIR = src

# Object files for core library
CORE_OBJS = $(OBJ_DIR)/tsfi_wire_firmware.o $(OBJ_DIR)/tsfi_logic.o $(OBJ_DIR)/tsfi_hotloader.o \
            $(OBJ_DIR)/tsfi_fourier.o $(OBJ_DIR)/tsfi_genetic.o $(OBJ_DIR)/tsfi_hilbert_avx512.o \
            $(OBJ_DIR)/tsfi_vision.o $(OBJ_DIR)/lau_audit.o $(OBJ_DIR)/tsfi_crash_handler.o \
            $(OBJ_DIR)/tsfi_spirv_kernels.o $(OBJ_DIR)/tsfi_fused_operator.o $(OBJ_DIR)/tsfi_vision_metrics.o \
            $(OBJ_DIR)/lau_registry.o $(OBJ_DIR)/tsfi_ottype_geom.o $(OBJ_DIR)/tsfi_hilbert.o \
            $(OBJ_DIR)/tsfi_font_shaping_parser.o $(OBJ_DIR)/tsfi_k0rn_compiler.o $(OBJ_DIR)/tsfi_soft_encode.o \
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
            $(OBJ_DIR)/lau_thunk.o $(OBJ_DIR)/tsfi_wiring.o $(OBJ_DIR)/tsfi_vulkan_cells.o \
            $(OBJ_DIR)/tsfi_zmm_vm.o $(OBJ_DIR)/tsfi_merkle.o \
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
            $(OBJ_DIR)/tsfi_merkle_fast.o $(OBJ_DIR)/tsfi_scramble_fast.o $(OBJ_DIR)/tsfi_jpeg_encoder.o $(OBJ_DIR)/tsfi_yang_baseline.o $(OBJ_DIR)/tsfi_yang_fast.o $(OBJ_DIR)/tsfi_comm.o \
            $(OBJ_DIR)/tsfi_controlnet_shm.o $(OBJ_DIR)/tsfi_puppetry.o \
            $(OBJ_DIR)/tsfi_pulsechain_rpc.o $(OBJ_DIR)/tsfi_tls.o


VULKAN_PLUGIN_OBJS = $(OBJ_DIR)/vulkan_init.o $(OBJ_DIR)/vulkan_main.o $(OBJ_DIR)/vulkan_render.o \
                     $(OBJ_DIR)/vulkan_input.o $(OBJ_DIR)/backend_display.o $(OBJ_DIR)/input_vty.o \
                     $(OBJ_DIR)/tsfi_input.o $(OBJ_DIR)/window_rendering.o \
                     $(OBJ_DIR)/vulkan_logic.o $(OBJ_DIR)/vulkan_xdg.o $(OBJ_DIR)/vulkan_dmabuf.o \
                     $(OBJ_DIR)/xdg-shell-protocol.o $(OBJ_DIR)/linux-dmabuf-v1-protocol.o \
                     $(OBJ_DIR)/wlr-layer-shell-unstable-v1-protocol.o \
                     $(OBJ_DIR)/tsfi_k0rn_rasterizer.o

BALLET_BINS = bin/ballet_ouverture bin/ballet_plier bin/ballet_etendre bin/ballet_relever \
              bin/ballet_glisser bin/ballet_sauter bin/ballet_elancer bin/ballet_tourner \
              bin/ballet_adagio bin/ballet_batterie bin/ballet_pointe \
              bin/ballet_pas_de_deux bin/ballet_variation bin/ballet_coda bin/ballet_reverence \
              bin/bench_rebar_lattice_mcp

TARGET = tsfi2

.PHONY: all clean debug release

debug: $(TARGET) bin/tsfi_time bin/tsfi_cockpit bin/tsfi_poll bin/tsfi_teacher bin/tsfi_font_editor bin/tsfi_teddy_editor bin/tsfi_discovery bin/tsfi_fetch bin/tsfi_rpc $(BALLET_BINS) bin/test_trilateral_rigidity bin/tsfi_trilateral_observatory

release: CFLAGS += -O3 -DNDEBUG -march=native
release: LDFLAGS += -s
release: clean all

bin/tsfi_dna_crossover: src/tsfi_dna_crossover.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/tsfi_puppeteer: src/tsfi_puppeteer.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt
LDFLAGS  = -L/usr/local/lib -ljpeg -lrt -ldl -lpthread -lstdc++ -lwayland-client -lvulkan -lubsan -lgomp -lm

all: $(TARGET) $(BIN_DIR)/tsfi_time $(BIN_DIR)/tsfi_cockpit $(BIN_DIR)/tsfi_poll $(BIN_DIR)/tsfi_teacher bin/tsfi_discovery bin/tsfi_fetch bin/tsfi_rpc bin/tsfi_dna_crossover bin/tsfi_puppeteer tests/test_harmonic_pillars $(BALLET_BINS) bin/test_trilateral_rigidity bin/tsfi_trilateral_observatory bin/test_graph_cards bin/test_taste_atoms bin/tsfi_spider_crow_taste bin/tsfi_spider_teddy_taste
bin/tsfi_teddy_skeleton: src/tsfi_teddy_skeleton.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/tsfi_cockpit_vae_bridge: src/tsfi_cockpit_vae_bridge.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lrt

bin/tsfi_guidance_tuner: src/tsfi_guidance_tuner.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/test_graph_cards: tests/test_graph_cards.c obj/tsfi_graph_cards.o $(CORE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lrt

obj/tsfi_graph_cards.o: src/tsfi_graph_cards.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bin/tsfi_icpc_taste_tournament: src/tsfi_icpc_taste_tournament.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/tsfi_spider_teddy_taste: src/tsfi_spider_teddy_taste.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/tsfi_spider_crow_taste: src/tsfi_spider_crow_taste.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/test_taste_atoms: tests/test_taste_atoms.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/test_taste_cache: tests/test_taste_cache.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/test_fur_synthesis: tests/test_fur_synthesis.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/test_vae_culling: tests/test_vae_culling.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lrt

bin/test_trilateral_rigidity: tests/test_trilateral_rigidity.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_trilateral_observatory: tests/tsfi_trilateral_observatory.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET): $(OBJ_DIR)/main.o $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_v2c: tools/tsfi_v2c.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

src/firmware/LauWireFirmware_rtl.c src/firmware/LauWireFirmware_rtl.h: src/firmware/LauWireFirmware.v bin/tsfi_v2c
	./bin/tsfi_v2c $<

$(OBJ_DIR)/firmware/%.o: src/firmware/%.c | $(OBJ_DIR)/firmware
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/firmware:
	mkdir -p $(OBJ_DIR)/firmware

bin/tsfi_font_editor: $(OBJ_DIR)/tsfi_font_editor.o $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lvulkan -lwayland-client -lm -lm

bin/tsfi_time: $(OBJ_DIR)/tsfi_time_cli.o $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_interactive_pty: tools/tsfi_interactive_pty.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lwayland-client

bin/tsfi_inspect_mouse: tools/tsfi_inspect_mouse.c
	$(CC) $(CFLAGS) -o $@ $^

bin/tsfi_mcp_server: $(OBJ_DIR)/tsfi_mcp_server.o $(CORE_OBJS) $(OBJ_DIR)/tsfi_zmm_vm.o $(OBJ_DIR)/tsfi_zmm_rpc.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/tsfi_cockpit: src/tsfi_cockpit.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm -lpthread

bin/tsfi_teacher: src/tsfi_teacher.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/tsfi_poll: src/tsfi_poll.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/tsfi_discovery: src/tsfi_discovery.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/tsfi_fetch: src/tsfi_fetch.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/tsfi_rpc: tools/tsfi_rpc.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/ballet_ouverture: tests/ballet_ouverture.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/ballet_plier: tests/ballet_plier.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -mavx512ifma -o $@ $^ $(LDFLAGS) -lm

bin/ballet_etendre: tests/ballet_etendre.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/ballet_relever: tests/ballet_relever.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/ballet_glisser: tests/ballet_glisser.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/ballet_sauter: tests/ballet_sauter.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/ballet_elancer: tests/ballet_elancer.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/vtube_deepseek_live: tests/vtube_deepseek_live.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/test_vulkan_font_validation: tests/test_vulkan_font_validation.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/ballet_tourner: tests/ballet_tourner.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/ballet_adagio: tests/ballet_adagio.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/ballet_batterie: tests/ballet_batterie.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/ballet_pointe: tests/ballet_pointe.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/ballet_acoustic_proof: tests/ballet_acoustic_proof.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/ballet_pas_de_deux: tests/ballet_pas_de_deux.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/ballet_variation: tests/ballet_variation.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/ballet_coda: tests/ballet_coda.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/ballet_reverence: tests/ballet_reverence.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lm

bin/bench_rebar_lattice_mcp: tests/bench_rebar_lattice_mcp.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS) -lrt

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/vulkan_%.o: plugins/vulkan/vulkan_%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/window_src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/vulkan/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

tests/%: tests/%.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_kernel: tests/test_zmm_kernel.c $(filter-out $(OBJ_DIR)/tsfi_merkle.o, $(CORE_OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_signature_rigidity: tests/test_signature_rigidity.c $(filter-out $(OBJ_DIR)/tsfi_merkle.o, $(CORE_OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tsfi_merkle_cli: src/tsfi_merkle_cli.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tsfi_cli: src/main.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

all: tsfi_cli tsfi_merkle_cli

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(TARGET) tests/bench_pty_* tests/test_pty_* tests/test_layer0_* tests/test_mariner_* tests/test_vulkan_thunks tests/test_hypervisor tests/test_firmware_zhong tests/test_flow_swapchain tests/test_flow_network tests/test_neural_gather tests/verify_rendering

bin/seed_level12_genesis: tests/seed_level12_genesis.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lvulkan -lwayland-client -lm

bin/tech_01_zmm_add: tests/tech_01_zmm_add.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tech_02_yang_heartbeat: tests/tech_02_yang_heartbeat.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tech_03_svdag_traverse: tests/tech_03_svdag_traverse.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tech_03_svdag_rigid: tests/tech_03_svdag_rigid.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tech_04_lau_scramble: tests/tech_04_lau_scramble.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tech_05_merkle_reduce: tests/tech_05_merkle_reduce.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/bench_merkle_fast: tests/bench_merkle_fast.c obj/tsfi_merkle_fast.o $(OBJ_DIR)/tsfi_scramble_fast.o $(OBJ_DIR)/tsfi_jpeg_encoder.o $(OBJ_DIR)/tsfi_yang_baseline.o $(OBJ_DIR)/tsfi_yang_fast.o $(OBJ_DIR)/tsfi_comm.o $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lvulkan -lwayland-client -lm

bin/bench_yang_baseline: tests/bench_yang_baseline.c obj/tsfi_yang_baseline.o $(OBJ_DIR)/tsfi_yang_fast.o $(OBJ_DIR)/tsfi_comm.o $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lvulkan -lwayland-client -lm

bin/bench_yang_fast: tests/bench_yang_fast.c obj/tsfi_yang_fast.o $(OBJ_DIR)/tsfi_comm.o $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lvulkan -lwayland-client -lm

bin/bench_scramble_baseline: tests/bench_scramble_baseline.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lvulkan -lwayland-client -lm

bin/bench_scramble_fast: tests/bench_scramble_fast.c obj/tsfi_scramble_fast.o $(OBJ_DIR)/tsfi_scramble_fast.o $(OBJ_DIR)/tsfi_jpeg_encoder.o $(OBJ_DIR)/tsfi_yang_baseline.o $(OBJ_DIR)/tsfi_yang_fast.o $(OBJ_DIR)/tsfi_comm.o $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lvulkan -lwayland-client -lm

bin/tsfi_teddy_editor: $(OBJ_DIR)/tsfi_teddy_editor.o $(filter-out $(OBJ_DIR)/tsfi_font_editor.o, $(CORE_OBJS)) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -rdynamic -lubsan -lwayland-client -Wl,-z,noexecstack -ljpeg -lvulkan -lpthread -lm
