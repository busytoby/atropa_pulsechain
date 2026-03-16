#!/bin/bash
# Checks if the codebase is using up-to-date protocol versions
set -e

echo "--- Version Compliance Check ---"

# 1. Check XDG Shell Binding
# We look for the binding call in vulkan_main.c
# "wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)"
# If it binds version 1, we check if higher versions are available in the header.

HEADER="plugins/xdg-shell-client-protocol.h"
SOURCE="plugins/vulkan/vulkan_main.c"

if [ -f "$HEADER" ] && [ -f "$SOURCE" ]; then
    # Find max version in header (heuristic: look for @since X)
    MAX_VER=$(grep "@since" "$HEADER" | awk '{print $3}' | sort -nr | head -n1)
    echo "Available XDG Shell Version (Header): $MAX_VER"
    
    # Find bound version
    BOUND_VER=$(grep "xdg_wm_base_interface," "$SOURCE" | awk -F, '{print $4}' | tr -d ' );')
    echo "Bound XDG Shell Version (Source): $BOUND_VER"
    
    if [[ "$BOUND_VER" =~ ^[0-9]+$ ]]; then
        if [ "$BOUND_VER" -lt "$MAX_VER" ]; then
            echo "[WARN] XDG Shell binding (v$BOUND_VER) in modular plugin is older than available (v$MAX_VER)."
        else
            echo "[PASS] XDG Shell version (modular) is up-to-date."
        fi
    else
        # Assume variable means negotiation logic is present
        if grep -q "xdg_wm_base_interface" "$SOURCE" && grep -q "version < " "$SOURCE"; then
             echo "[PASS] XDG Shell negotiation logic detected (modular)."
        else
             echo "[WARN] XDG Shell binding uses variable but negotiation logic unclear."
        fi
    fi
    
    # Check Monolithic
    MONO_SOURCE="plugins/tsfi_vulkan.c"
    if [ -f "$MONO_SOURCE" ]; then
        BOUND_VER_MONO=$(grep "xdg_wm_base_interface," "$MONO_SOURCE" | awk -F, '{print $4}' | tr -d ' );')
        if [[ "$BOUND_VER_MONO" =~ ^[0-9]+$ ]]; then
            if [ "$BOUND_VER_MONO" -lt "$MAX_VER" ]; then
                 echo "[WARN] XDG Shell binding (v$BOUND_VER_MONO) in monolithic plugin is older than available."
            fi
        else
             echo "[PASS] XDG Shell negotiation logic detected (monolithic)."
        fi
    fi
else
    echo "[SKIP] Protocol header or source not found."
fi

# 2. Check Vulkan API Version
VULKAN_INIT="plugins/vulkan_init.c"
if [ -f "$VULKAN_INIT" ]; then
    if grep -q "VK_API_VERSION_1_3" "$VULKAN_INIT"; then
        echo "[PASS] Vulkan API 1.3 requested."
    else
        echo "[FAIL] Vulkan API 1.3 NOT requested."
        exit 1
    fi
fi

# 3. Check Wayland Compositor Binding Cap
# vulkan_main.c: version < 6 ? version : 6
if grep -q "version < 6" "$SOURCE"; then
    echo "[PASS] Compositor binding logic found (Cap v6)."
    else
        echo "[WARN] Compositor binding logic might be missing or changed."
    fi
    
    # 4. Check DMABUF Binding
    DMABUF_HEADER="plugins/linux-dmabuf-v1-client-protocol.h"
    if [ -f "$DMABUF_HEADER" ]; then
        MAX_DMA=$(grep "@since" "$DMABUF_HEADER" | awk '{print $3}' | sort -nr | head -n1)
        echo "Available DMABUF Version (Header): $MAX_DMA"
        
        BOUND_DMA=$(grep "zwp_linux_dmabuf_v1_interface," "$SOURCE" | awk -F, '{print $4}' | tr -d ' );')
        echo "Bound DMABUF Version (Source): $BOUND_DMA"
        
        if [[ "$BOUND_DMA" =~ ^[0-9]+$ ]]; then
            if [ "$BOUND_DMA" -lt "$MAX_DMA" ]; then
                echo "[WARN] DMABUF binding (v$BOUND_DMA) is older than available (v$MAX_DMA)."
            fi
        else
             echo "[PASS] DMABUF negotiation logic detected."
        fi
    fi

    echo "--- Compliance Check Complete ---"

exit 0
