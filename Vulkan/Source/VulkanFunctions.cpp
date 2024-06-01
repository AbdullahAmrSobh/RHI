#include "VulkanFunctions.hpp"
#include "Context.hpp"

#define VULKAN_LOAD_PROC(device, proc) reinterpret_cast<PFN_##proc>(vkGetDeviceProcAddr(device, #proc));
#define VULKAN_LOAD_PROC_INSTANCE(instance, proc) reinterpret_cast<PFN_##proc>(vkGetInstanceProcAddr(instance, #proc));

namespace RHI::Vulkan
{
    void FunctionsTable::Init(IContext* context, bool debugEnabled)
    {
        auto device = context->m_device;
#if RHI_DEBUG
        if (debugEnabled)
        {
            m_cmdDebugMarkerBeginEXT = VULKAN_LOAD_PROC(device, vkCmdDebugMarkerBeginEXT);
            m_cmdDebugMarkerInsertEXT = VULKAN_LOAD_PROC(device, vkCmdDebugMarkerInsertEXT);
            m_cmdDebugMarkerEndEXT = VULKAN_LOAD_PROC(device, vkCmdDebugMarkerEndEXT);
            m_debugMarkerSetObjectNameEXT = VULKAN_LOAD_PROC(device, vkDebugMarkerSetObjectNameEXT);

            RHI_ASSERT(m_cmdDebugMarkerBeginEXT);
            RHI_ASSERT(m_cmdDebugMarkerInsertEXT);
            RHI_ASSERT(m_cmdDebugMarkerEndEXT);
            RHI_ASSERT(m_debugMarkerSetObjectNameEXT);
        }
#endif

        m_cmdBeginConditionalRenderingEXT = VULKAN_LOAD_PROC(device, vkCmdBeginConditionalRenderingEXT);
        m_cmdEndConditionalRenderingEXT = VULKAN_LOAD_PROC(device, vkCmdEndConditionalRenderingEXT);
    }

} // namespace RHI::Vulkan