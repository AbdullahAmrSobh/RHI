#pragma once

#include <RHI/Common/Span.hpp>
#include <RHI/Definitions.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    class IFence;
    class ICommandList;

    struct SubmitInfo
    {
        TL::Span<const VkSemaphoreSubmitInfoKHR> waitSemaphores;
        TL::Span<const VkSemaphoreSubmitInfoKHR> signalSemaphores;
        TL::Span<ICommandList* const> commandLists;
    };

    class Queue
    {
    public:
        Queue() = default;

        Queue(VkDevice device, uint32_t familyIndex);

        VkQueue GetHandle() const { return m_queue; }

        uint32_t GetFamilyIndex() const { return m_familyIndex; }

        void BeginLabel(IContext* context, const char* name, ColorValue<float> color);

        void EndLabel(IContext* context);

        void Submit(IContext* context, SubmitInfo submitGroups, IFence* fence);

        void Present(IContext* context, VkSemaphore semaphore, class ISwapchain& swapchain);

    private:
        VkQueue m_queue;
        uint32_t m_familyIndex;
    };
} // namespace RHI::Vulkan