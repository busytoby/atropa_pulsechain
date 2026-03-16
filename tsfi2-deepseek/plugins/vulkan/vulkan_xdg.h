#ifndef VULKAN_XDG_H
#define VULKAN_XDG_H

#include "vulkan_system.h"

extern const struct xdg_wm_base_listener xdg_wm_base_listener;
extern const struct xdg_surface_listener xdg_surface_listener;
extern const struct xdg_toplevel_listener xdg_toplevel_listener;

void vulkan_xdg_set_title(VulkanSystem *s, const char *title);
void vulkan_xdg_set_fullscreen(VulkanSystem *s, bool fullscreen);
void vulkan_xdg_set_maximized(VulkanSystem *s, bool maximized);

#endif
