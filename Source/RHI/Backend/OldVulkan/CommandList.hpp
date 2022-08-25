#pragma once
#include "RHI/Commands.hpp"

#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{
    class CommandList final
        : public RHI::ICommandList
        , public DeviceObject<VkCommandBuffer>
    {
    public:
        CommandList(Device& device)
            : DeviceObject(device)
        {
        }
        ~CommandList();

        virtual void Reset() override;

        virtual void Begin() override;
        virtual void End() override;

        virtual void SetViewports(Span<const Viewport>) override;
        virtual void SetScissors(Span<const Rect>) override;

        virtual void Submit(const DrawCommand&) override;
        virtual void Submit(const CopyCommand&) override;
        virtual void Submit(const DispatchCommand&) override;

        virtual void BeginConditionalRendering(ConditionBuffer condition) override;
        virtual void EndConditionalRendering() override;

    private:
        std::vector<VkSemaphore>          m_waitSemaphores;
        std::vector<VkPipelineStageFlags> m_waitStages;
        std::vector<VkSemaphore>          m_signalSemaphores;
    };

    struct CommandPoolDesc
    {
        CommandPoolDesc() = default;
    };

    class CommandPool final : public DeviceObject<VkCommandPool>
    {
    public:
        enum class Level
        {
            Primary   = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            MaxEnum   = VK_COMMAND_BUFFER_LEVEL_MAX_ENUM
        };

        CommandPool(Device& device)
            : DeviceObject(device)
        {
        }
        ~CommandPool();

        VkResult Init(uint32_t queueFamilyIndex);

        void                        ResetPool(VkCommandPoolResetFlags flags);
        std::vector<CommandListPtr> Allocate(Level level, uint32_t count);
        inline CommandListPtr       Allocate(Level level)
        {
            return std::move(Allocate(level, 1).front());
        }
    };

} // namespace Vulkan
} // namespace RHI
