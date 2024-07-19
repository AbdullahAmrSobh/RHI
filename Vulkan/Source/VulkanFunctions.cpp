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
            m_queueBeginDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkQueueBeginDebugUtilsLabelEXT);
            m_queueEndDebugUtilsLabelEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkQueueEndDebugUtilsLabelEXT);
            m_cmdDebugMarkerBeginEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdDebugMarkerBeginEXT);
            m_cmdDebugMarkerInsertEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdDebugMarkerInsertEXT);
            m_cmdDebugMarkerEndEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdDebugMarkerEndEXT);
            m_debugMarkerSetObjectNameEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkDebugMarkerSetObjectNameEXT);

            RHI_ASSERT(m_queueBeginDebugUtilsLabelEXT);
            RHI_ASSERT(m_queueEndDebugUtilsLabelEXT);
            RHI_ASSERT(m_cmdDebugMarkerBeginEXT);
            RHI_ASSERT(m_cmdDebugMarkerInsertEXT);
            RHI_ASSERT(m_cmdDebugMarkerEndEXT);
            RHI_ASSERT(m_debugMarkerSetObjectNameEXT);
        }
#endif

        m_cmdBeginConditionalRenderingEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdBeginConditionalRenderingEXT);
        m_cmdEndConditionalRenderingEXT = VULKAN_DEVICE_FUNC_LOAD(device, vkCmdEndConditionalRenderingEXT);
    }

} // namespace RHI::Vulkan