#pragma once
#include "RHI/Commands.hpp"

#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Fence.hpp"
#include "RHI/Backend/Vulkan/Texture.hpp"

namespace RHI
{
namespace Vulkan
{
    class CommandContext final
        : public ICommandContext
        , public DeviceObject<VkCommandBuffer>
    {
    public:
        ~CommandContext();

        virtual void Begin() override;
        virtual void End() override;

        virtual void SetViewports(ArrayView<const Viewport>) override;
        virtual void SetScissors(ArrayView<const Rect>) override;
		
        virtual void Submit(const DrawCommand&) override;
        virtual void Submit(const CopyCommand&) override;
        virtual void Submit(const DispatchCommand&) override;

    private:
        friend class Queue;
        std::vector<VkSemaphore>          m_waitSemaphores;
        std::vector<VkPipelineStageFlags> m_waitStages;
        std::vector<VkSemaphore>          m_signalSemaphores;
    };
} // namespace Vulkan
} // namespace RHI
