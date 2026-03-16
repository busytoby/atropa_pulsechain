#!/bin/bash
# TSFi Cockpit Throttle Verification Script

echo "=== TSFi Cockpit Throttle Verification ==="

# 1. Build the driver
gcc -Iinc -Iplugins -O3 -march=native -o tests/test_throttle_driver tests/test_throttle_driver.c \
    src/tsfi_logic.c src/tsfi_svdag.c src/lau_registry.c src/lau_memory.c src/lau_audit.c \
    src/tsfi_crash_handler.c src/tsfi_wire_firmware.c src/tsfi_io.c src/tsfi_fused_operator.c \
    src/tsfi_reaction.c src/tsfi_math.c src/tsfi_dys_math.c src/tsfi_c_math.c \
    src/tsfi_wave512_asm.c src/lau_vram.c src/tsfi_zhao.c src/tsfi_hotloader.c \
    src/tsfi_fourier.c src/tsfi_genetic.c src/tsfi_hilbert_avx512.c src/tsfi_vision.c \
    src/tsfi_spirv_kernels.c src/tsfi_vision_metrics.c src/tsfi_ottype_geom.c \
    src/tsfi_hilbert.c src/tsfi_font_shaping_parser.c src/tsfi_k0rn_compiler.c \
    src/tsfi_soft_encode.c src/tsfi_broadcaster_core.c src/tsfi_vision_vk.c \
    src/tsfi_ottype_shape.c src/tsfi_video_thunks.c src/tsfi_rtmp.c \
    src/tsfi_vision_thunks.c src/tsfi_broadcaster_bandwidth_throttle.c \
    src/tsfi_wire_pty.c src/tsfi_resonance.c src/tsfi_dl_thunks.c \
    src/tsfi_k0rn_codegen.c src/tsfi_font_rasterizer.c src/tsfi_phenotype_loader.c \
    src/tsfi_broadcaster_ui.c src/tsfi_font_ai.c src/tsfi_ottype_layout.c \
    src/tsfi_helmk0rn_factory.c src/tsfi_vulkan_video_encode.c src/tsfi_vision_io.c \
    src/tsfi_vec_math.c src/tsfi_variations.c src/tsfi_font_engine.c \
    src/tsfi_vision_analysis.c src/tsfi_config.c src/tsfi_cff.c src/tsfi_trace.c \
    src/tsfi_ottype.c src/tsfi_dysnomia.c src/tsfi_zmm_rpc.c src/tsfi_font_registry.c \
    src/tsfi_ipomoea_lifecycle.c src/tsfi_shader_dispatch.c src/tsfi_glob.c \
    src/tsfi_operator.c src/tsfi_yang.c src/tsfi_quan.c src/tsfi_vision_classify.c \
    src/tsfi_cli.c src/lau_thunk.c src/tsfi_wiring.c src/tsfi_vulkan_cells.c \
    src/tsfi_zmm_vm.c src/tsfi_merkle.c src/tsfi_pool.c src/tsfi_time.c \
    src/tsfi_safetensors_cache.c src/tsfi_vae_firmware.c src/tsfi_nand_trap_firmware.c \
    src/tsfi_vessel_anchor.c src/tsfi_vae_upsampler.c src/tsfi_fur_shader.c \
    src/tsfi_masterpiece_render.c src/tsfi_moon_chuuker.c src/tsfi_dream_streamer.c \
    src/tsfi_path_tracer.c src/tsfi_pbr.c src/tsfi_hair.c src/tsfi_merkle_fast.c \
    src/tsfi_scramble_fast.c src/tsfi_jpeg_encoder.c src/tsfi_yang_baseline.c \
    src/tsfi_yang_fast.c src/tsfi_comm.c src/tsfi_controlnet_shm.c \
    src/firmware/LauWireFirmware_rtl.c src/tsfi_elf_reflect.c src/tsfi_liang_barsky.c \
    src/tsfi_opt_zmm.c -lvulkan -lm -lrt -ljpeg -rdynamic -lpthread

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
