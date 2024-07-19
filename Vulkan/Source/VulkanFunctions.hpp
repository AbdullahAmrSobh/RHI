#pragma once

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    class FunctionsTable
    {
    public:
        FunctionsTable() = default;

        void Init(IContext* context, bool debugEnabled);

        PFN_vkQueueBeginDebugUtilsLabelEXT m_queueBeginDebugUtilsLabelEXT;
        PFN_vkQueueEndDebugUtilsLabelEXT m_queueEndDebugUtilsLabelEXT;
        PFN_vkCmdDebugMarkerBeginEXT m_cmdDebugMarkerBeginEXT;
        PFN_vkCmdDebugMarkerInsertEXT m_cmdDebugMarkerInsertEXT;
        PFN_vkCmdDebugMarkerEndEXT m_cmdDebugMarkerEndEXT;
        PFN_vkCmdBeginConditionalRenderingEXT m_cmdBeginConditionalRenderingEXT;
        PFN_vkCmdEndConditionalRenderingEXT m_cmdEndConditionalRenderingEXT;
        PFN_vkDebugMarkerSetObjectNameEXT m_debugMarkerSetObjectNameEXT;
    };
} // namespace RHI::Vulkan