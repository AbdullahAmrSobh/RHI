#pragma once

#include <RHI/Queue.hpp>

#include <TL/Span.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    class IFence;
    class ICommandList;

    class IQueue final : public Queue
    {
    public:
        IQueue() = default;
        IQueue(IContext* context, uint32_t familyIndex);

        inline VkQueue GetHandle() const { return m_queue; }

        inline uint32_t GetFamilyIndex() const { return m_familyIndex; }

        void BeginLabel(const char* name, ColorValue<float> color) override;
        void EndLabel() override;

        void Submit(TL::Span<CommandList* const> commandLists, Fence* fence) override
        {
            TL::Span<ICommandList*> list{ (ICommandList**)commandLists.data(), commandLists.size() };
            Submit(list, (IFence*)fence);
        }

        void Submit(TL::Span<ICommandList* const> commandLists, IFence* fence);
        void Present(VkSemaphore semaphore, class ISwapchain& swapchain);

    private:
        IContext* m_context;

        VkQueue m_queue;
        uint32_t m_familyIndex;
    };
} // namespace RHI::Vulkan