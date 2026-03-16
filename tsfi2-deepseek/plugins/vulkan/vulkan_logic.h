#ifndef VULKAN_LOGIC_H
#define VULKAN_LOGIC_H

#include "vulkan_system.h"

void vulkan_logic_epoch(int *ver);
bool vulkan_logic_state(void *obj);
void vulkan_logic_directive(int *cnt, char *dir);
void vulkan_logic_scramble(void *ws_ptr);
void vulkan_logic_provenance(void *ws_ptr);

void set_vulkan_system(VulkanSystem *s);
VulkanSystem* get_vulkan_system(void);

// Moved lifecycle here? No, they are in main now.
// But logic needs destroy_vulkan_system.
void destroy_vulkan_system(VulkanSystem *s);

#endif