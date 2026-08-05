#!/bin/bash
# TSFi Cockpit Throttle Verification Script

echo "=== TSFi Cockpit Throttle Verification ==="

# 1. Build the driver
gcc -Iinc -Iplugins -O3 -march=native -o tests/test_throttle_driver tests/test_throttle_driver.c \
    tsfi2-deepseek/src/tsfi_logic.c tsfi2-deepseek/src/tsfi_svdag.c tsfi2-deepseek/src/lau_registry.c tsfi2-deepseek/src/lau_memory.c tsfi2-deepseek/src/lau_audit.c \
    tsfi2-deepseek/src/tsfi_crash_handler.c tsfi2-deepseek/src/tsfi_wire_firmware.c tsfi2-deepseek/src/tsfi_io.c tsfi2-deepseek/src/tsfi_fused_operator.c \
    tsfi2-deepseek/src/tsfi_reaction.c tsfi2-deepseek/src/tsfi_math.c tsfi2-deepseek/src/tsfi_dys_math.c tsfi2-deepseek/src/tsfi_c_math.c \
    tsfi2-deepseek/src/tsfi_wave512_asm.c tsfi2-deepseek/src/lau_vram.c tsfi2-deepseek/src/tsfi_zhao.c tsfi2-deepseek/src/tsfi_hotloader.c \
    tsfi2-deepseek/src/tsfi_fourier.c tsfi2-deepseek/src/tsfi_genetic.c tsfi2-deepseek/src/tsfi_hilbert_avx512.c tsfi2-deepseek/src/tsfi_vision.c \
    tsfi2-deepseek/src/tsfi_spirv_kernels.c tsfi2-deepseek/src/tsfi_vision_metrics.c tsfi2-deepseek/src/tsfi_ottype_geom.c \
    tsfi2-deepseek/src/tsfi_hilbert.c tsfi2-deepseek/src/tsfi_font_shaping_parser.c tsfi2-deepseek/src/tsfi_k0rn_compiler.c \
    tsfi2-deepseek/src/tsfi_soft_encode.c tsfi2-deepseek/src/tsfi_broadcaster_core.c tsfi2-deepseek/src/tsfi_vision_vk.c \
    tsfi2-deepseek/src/tsfi_ottype_shape.c tsfi2-deepseek/src/tsfi_video_thunks.c tsfi2-deepseek/src/tsfi_rtmp.c \
    tsfi2-deepseek/src/tsfi_vision_thunks.c tsfi2-deepseek/src/tsfi_broadcaster_bandwidth_throttle.c \
    tsfi2-deepseek/src/tsfi_wire_pty.c tsfi2-deepseek/src/tsfi_resonance.c tsfi2-deepseek/src/tsfi_dl_thunks.c \
    tsfi2-deepseek/src/tsfi_k0rn_codegen.c tsfi2-deepseek/src/tsfi_font_rasterizer.c tsfi2-deepseek/src/tsfi_phenotype_loader.c \
    tsfi2-deepseek/src/tsfi_broadcaster_ui.c tsfi2-deepseek/src/tsfi_font_ai.c tsfi2-deepseek/src/tsfi_ottype_layout.c \
    tsfi2-deepseek/src/tsfi_helmk0rn_factory.c tsfi2-deepseek/src/tsfi_vulkan_video_encode.c tsfi2-deepseek/src/tsfi_vision_io.c \
    tsfi2-deepseek/src/tsfi_vec_math.c tsfi2-deepseek/src/tsfi_variations.c tsfi2-deepseek/src/tsfi_font_engine.c \
    tsfi2-deepseek/src/tsfi_vision_analysis.c tsfi2-deepseek/src/tsfi_config.c tsfi2-deepseek/src/tsfi_cff.c tsfi2-deepseek/src/tsfi_trace.c \
    tsfi2-deepseek/src/tsfi_ottype.c tsfi2-deepseek/src/tsfi_dysnomia.c tsfi2-deepseek/src/tsfi_zmm_rpc.c tsfi2-deepseek/src/tsfi_font_registry.c \
    tsfi2-deepseek/src/tsfi_ipomoea_lifecycle.c tsfi2-deepseek/src/tsfi_shader_dispatch.c tsfi2-deepseek/src/tsfi_glob.c \
    tsfi2-deepseek/src/tsfi_operator.c tsfi2-deepseek/src/tsfi_yang.c tsfi2-deepseek/src/tsfi_quan.c tsfi2-deepseek/src/tsfi_vision_classify.c \
    tsfi2-deepseek/src/tsfi_cli.c tsfi2-deepseek/src/lau_thunk.c tsfi2-deepseek/src/tsfi_wiring.c tsfi2-deepseek/src/tsfi_vulkan_cells.c \
    tsfi2-deepseek/src/tsfi_zmm_vm.c tsfi2-deepseek/src/tsfi_merkle.c tsfi2-deepseek/src/tsfi_pool.c tsfi2-deepseek/src/tsfi_time.c \
    tsfi2-deepseek/src/tsfi_safetensors_cache.c tsfi2-deepseek/src/tsfi_vae_firmware.c tsfi2-deepseek/src/tsfi_nand_trap_firmware.c \
    tsfi2-deepseek/src/tsfi_vessel_anchor.c tsfi2-deepseek/src/tsfi_vae_upsampler.c tsfi2-deepseek/src/tsfi_fur_shader.c \
    tsfi2-deepseek/src/tsfi_masterpiece_render.c tsfi2-deepseek/src/tsfi_moon_chuuker.c tsfi2-deepseek/src/tsfi_dream_streamer.c \
    tsfi2-deepseek/src/tsfi_path_tracer.c tsfi2-deepseek/src/tsfi_pbr.c tsfi2-deepseek/src/tsfi_hair.c tsfi2-deepseek/src/tsfi_merkle_fast.c \
    tsfi2-deepseek/src/tsfi_scramble_fast.c tsfi2-deepseek/src/tsfi_jpeg_encoder.c tsfi2-deepseek/src/tsfi_yang_baseline.c \
    tsfi2-deepseek/src/tsfi_yang_fast.c tsfi2-deepseek/src/tsfi_comm.c tsfi2-deepseek/src/tsfi_controlnet_shm.c \
    tsfi2-deepseek/src/firmware/LauWireFirmware_rtl.c tsfi2-deepseek/src/tsfi_elf_reflect.c tsfi2-deepseek/src/tsfi_liang_barsky.c \
    tsfi2-deepseek/src/tsfi_opt_zmm.c -lvulkan -lm -lrt -ljpeg -rdynamic -lpthread

if [ $? -ne 0 ]; then
    echo "[FRACTURE] Driver compilation failed."
    exit 1
fi

# 2. Run via Cockpit
echo "[RUN] Executing via Cockpit..."
bin/tsfi_cockpit ./tests/test_throttle_driver > .logs/throttle_integration.log 2>&1

# 3. Verify Telemetry in Logs
echo "[VERIFY] Checking Cockpit logs for THROTTLE events..."
if grep -q "THROTTLE:" .logs/throttle_integration.log; then
    THROTTLE_COUNT=$(grep "THROTTLE:" .logs/throttle_integration.log | tail -n 1 | awk -F'|' '{print $1}' | awk '{print $3}')
    STALL_MS=$(grep "STALL:" .logs/throttle_integration.log | tail -n 1 | awk -F'|' '{print $2}' | awk '{print $2}')
    
    echo "[PASS] Cockpit detected $THROTTLE_COUNT throttles and $STALL_MS ms of stall."
    if [ "$THROTTLE_COUNT" -gt 0 ]; then
        echo "[SUCCESS] Alligator Throttling verified via Cockpit."
        exit 0
    else
        echo "[FAIL] Throttle count is 0."
        exit 1
    fi
else
    echo "[FAIL] No THROTTLE telemetry found in Cockpit output."
    exit 1
fi
