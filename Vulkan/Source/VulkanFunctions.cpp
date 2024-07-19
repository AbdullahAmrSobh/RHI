#include "VulkanFunctions.hpp"
#include "Context.hpp"

#define VULKAN_DEVICE_FUNC_LOAD(device, proc) reinterpret_cast<PFN_##proc>(vkGetDeviceProcAddr(device, #proc));
#define VULKAN_INSTANCE_FUNC_LOAD(instance, proc) reinterpret_cast<PFN_##proc>(vkGetInstanceProcAddr(instance, #proc));

namespace RHI::Vulkan
{
    void FunctionsTable::Init(IContext* context, bool debugEnabled)
    {
        auto device = context->m_device;
#if RHI_DEBUG
        if (debugEnabled)
        {
            m_vkCmdBeginDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdBeginDebugUtilsLabelEXT);
            m_vkCmdEndDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdEndDebugUtilsLabelEXT);
            m_vkCmdInsertDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdInsertDebugUtilsLabelEXT);
            m_vkCreateDebugUtilsMessengerEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCreateDebugUtilsMessengerEXT);
            m_vkDestroyDebugUtilsMessengerEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkDestroyDebugUtilsMessengerEXT);
            m_vkQueueBeginDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkQueueBeginDebugUtilsLabelEXT);
            m_vkQueueEndDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkQueueEndDebugUtilsLabelEXT);
            m_vkQueueInsertDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkQueueInsertDebugUtilsLabelEXT);
            m_vkSetDebugUtilsObjectNameEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkSetDebugUtilsObjectNameEXT);
            m_vkSetDebugUtilsObjectTagEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkSetDebugUtilsObjectTagEXT);
            m_vkSubmitDebugUtilsMessageEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkSubmitDebugUtilsMessageEXT);
        }
#endif

        m_cmdBeginConditionalRenderingEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdBeginConditionalRenderingEXT);
        m_cmdEndConditionalRenderingEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdEndConditionalRenderingEXT);
    }

} // namespace RHI::Vulkan