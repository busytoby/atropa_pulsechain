#ifndef VULKAN_LOGIC_H
#define VULKAN_LOGIC_H

#include "vulkan_system.h"

__attribute__((visibility("hidden"))) void vulkan_logic_epoch(int *ver);
__attribute__((visibility("hidden"))) bool vulkan_logic_state(void *obj);
__attribute__((visibility("hidden"))) void vulkan_logic_directive(int *cnt, char *dir);
__attribute__((visibility("hidden"))) void vulkan_logic_scramble(void *ws_ptr);
__attribute__((visibility("hidden"))) void vulkan_logic_provenance(void *ws_ptr);

__attribute__((visibility("hidden"))) void set_vulkan_system(VulkanSystem *s);
__attribute__((visibility("hidden"))) VulkanSystem* get_vulkan_system(void);

// Moved lifecycle here? No, they are in main now.
// But logic needs destroy_vulkan_system.
void destroy_vulkan_system(VulkanSystem *s);

#endif