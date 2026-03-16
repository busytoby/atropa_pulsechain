#ifndef VULKAN_MAIN_H
#define VULKAN_MAIN_H

#include "vulkan_system.h"

// Lifecycle
VulkanSystem* create_vulkan_system(void);
VulkanSystem* create_vulkan_display_system(void); // KMS/VTY
void destroy_vulkan_system(VulkanSystem *s);

// Forward declarations for master logic functions
void master_logic_epoch(int *ver);
bool master_logic_state(void *obj);
void master_logic_directive(int *cnt, char *dir);
void master_logic_scramble(void *ws_ptr);
void master_logic_provenance(void *ws_ptr);

#endif