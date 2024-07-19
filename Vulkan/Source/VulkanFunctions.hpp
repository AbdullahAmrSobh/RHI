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

#ifdef RHI_DEBUG
        // VK_EXT_debug_utils
        PFN_vkCmdBeginDebugUtilsLabelEXT m_vkCmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT m_vkCmdEndDebugUtilsLabelEXT;
        PFN_vkCmdInsertDebugUtilsLabelEXT m_vkCmdInsertDebugUtilsLabelEXT;
        PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT;
        PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT;
        PFN_vkQueueBeginDebugUtilsLabelEXT m_vkQueueBeginDebugUtilsLabelEXT;
        PFN_vkQueueEndDebugUtilsLabelEXT m_vkQueueEndDebugUtilsLabelEXT;
        PFN_vkQueueInsertDebugUtilsLabelEXT m_vkQueueInsertDebugUtilsLabelEXT;
        PFN_vkSetDebugUtilsObjectNameEXT m_vkSetDebugUtilsObjectNameEXT;
        PFN_vkSetDebugUtilsObjectTagEXT m_vkSetDebugUtilsObjectTagEXT;
        PFN_vkSubmitDebugUtilsMessageEXT m_vkSubmitDebugUtilsMessageEXT;
#endif
        PFN_vkCmdBeginConditionalRenderingEXT m_cmdBeginConditionalRenderingEXT;
        PFN_vkCmdEndConditionalRenderingEXT m_cmdEndConditionalRenderingEXT;
    };
} // namespace RHI::Vulkan