#include "tsfi_shader_dispatch.h"
#include "tsfi_zhao.h"
#include "lau_memory.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    VulkanContext* vk;
    const uint32_t* spv_code;
    size_t spv_size;
    VkShaderEXT* pShaderOut;
    volatile bool* pReady;
} ShaderPrepareJob;

static void shader_prepare_worker(void* arg) {
    ShaderPrepareJob* job = (ShaderPrepareJob*)arg;
    if (!job || !job->vk || !job->vk->vkCreateShadersEXT) return;

    VkShaderCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .nextStage = 0,
        .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
        .codeSize = job->spv_size,
        .pCode = job->spv_code,
        .pName = "main"
    };

    VkResult res = job->vk->vkCreateShadersEXT(job->vk->device, 1, &createInfo, NULL, job->pShaderOut);
    
    if (res == VK_SUCCESS) {
        *job->pReady = true;
    } else {
        printf("[TSFI_SHADER] Async Shader Preparation Failed (Res: %d)\n", res);
    }

    lau_free(job);
}

void tsfi_shader_object_prepare_async(VulkanContext* vk, const uint32_t* spv_code, size_t spv_size, VkShaderEXT* pShaderOut, volatile bool* pReady) {
    if (!vk || !vk->vkCreateShadersEXT || !pReady) return;

    *pReady = false;
    ShaderPrepareJob* job = (ShaderPrepareJob*)lau_malloc(sizeof(ShaderPrepareJob));
    job->vk = vk;
    job->spv_code = spv_code;
    job->spv_size = spv_size;
    job->pShaderOut = pShaderOut;
    job->pReady = pReady;

    tsfi_zhao_submit(shader_prepare_worker, job);
}

void tsfi_shader_object_bind_and_dispatch(VkCommandBuffer cmd, VulkanContext* vk, VkShaderEXT shader, uint32_t x, uint32_t y, uint32_t z) {
    if (!vk || !vk->vkCmdBindShadersEXT || !shader) return;

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_COMPUTE_BIT;
    vk->vkCmdBindShadersEXT(cmd, 1, &stage, &shader);
    vk->vkCmdDispatch(cmd, x, y, z);
}

typedef struct {
    VulkanContext* vk;
    const uint32_t* v_spv; size_t v_size;
    const uint32_t* f_spv; size_t f_size;
    VkShaderEXT* v_out; VkShaderEXT* f_out;
    volatile bool* pReady;
} GraphicsPrepareJob;

static void graphics_prepare_worker(void* arg) {
    GraphicsPrepareJob* job = (GraphicsPrepareJob*)arg;
    if (!job || !job->vk || !job->vk->vkCreateShadersEXT) return;

    VkShaderCreateInfoEXT infos[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .nextStage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
            .codeSize = job->v_size,
            .pCode = job->v_spv,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .nextStage = 0,
            .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
            .codeSize = job->f_size,
            .pCode = job->f_spv,
            .pName = "main"
        }
    };

    VkShaderEXT shaders[2];
    VkResult res = job->vk->vkCreateShadersEXT(job->vk->device, 2, infos, NULL, shaders);
    
    if (res == VK_SUCCESS) {
        *job->v_out = shaders[0];
        *job->f_out = shaders[1];
        *job->pReady = true;
    } else {
        printf("[TSFI_SHADER] Graphics Shader Preparation Failed (Res: %d)\n", res);
    }

    lau_free(job);
}

void tsfi_graphics_shader_object_prepare_async(VulkanContext* vk, 
                                              const uint32_t* v_spv, size_t v_size,
                                              const uint32_t* f_spv, size_t f_size,
                                              VkShaderEXT* v_out, VkShaderEXT* f_out,
                                              volatile bool* pReady) {
    if (!vk || !vk->vkCreateShadersEXT || !pReady) return;

    *pReady = false;
    GraphicsPrepareJob* job = (GraphicsPrepareJob*)lau_malloc(sizeof(GraphicsPrepareJob));
    job->vk = vk;
    job->v_spv = v_spv; job->v_size = v_size;
    job->f_spv = f_spv; job->f_size = f_size;
    job->v_out = v_out; job->f_out = f_out;
    job->pReady = pReady;

    tsfi_zhao_submit(graphics_prepare_worker, job);
}

void tsfi_ui_render_thunk(VkCommandBuffer cmd, VulkanContext* vk, 
                         VkShaderEXT v_shader, VkShaderEXT f_shader,
                         void* ui_data, size_t ui_size) {
    if (!vk || !vk->vkCmdBindShadersEXT || !v_shader || !f_shader) return;

    VkShaderStageFlagBits stages[2] = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
    VkShaderEXT shaders[2] = { v_shader, f_shader };
    vk->vkCmdBindShadersEXT(cmd, 2, stages, shaders);

    // Push UI Constants (e.g. coordinates, colors)
    if (ui_data && ui_size > 0 && vk->vkCmdPushConstants) {
        // We assume the compute layout is compatible for minimal push constants, 
        // OR we use a common pipeline layout for UI.
        vk->vkCmdPushConstants(cmd, vk->compute_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, (uint32_t)ui_size, ui_data);
    }

    // Draw full-screen quad (3 vertices for triangle strip generation)
    vk->vkCmdDraw(cmd, 3, 1, 0, 0); 
}
