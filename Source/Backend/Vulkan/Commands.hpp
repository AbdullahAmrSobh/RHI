#pragma once

#include <vector>
#include "RHI/Commands.hpp"
#include "RHI/Common.hpp"

#include "Backend/Vulkan/Resource.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace RHI
{
namespace Vulkan
{   
    class Semaphore;

    class CommandBuffer final
        : public ICommandBuffer
        , public Resource<VkCommandBuffer>
    {
    public:

        CommandBuffer();
        ~CommandBuffer();

        virtual void Begin() override;
        virtual void End() override;

        virtual void SetViewports(const std::vector<Viewport>& viewports) override;
        virtual void SetScissors(const std::vector<Rect>& scissors) override;

        virtual void Submit(const DrawCommand& drawCommand) override;
        virtual void Submit(const CopyCommand& copyCommand) override;
        virtual void Submit(const DispatchCommand& dispatchCommand) override;
    };

    class CommandAllocator final
    {
    public:
        VkResult Reset();

        Unique<CommandBuffer>              AllocateCommandBuffer();
        std::vector<Unique<CommandBuffer>> AllocateCommandBuffers(uint32_t count);

    };

} // namespace Vulkan
} // namespace RHI