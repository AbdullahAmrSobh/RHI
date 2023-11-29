#pragma once

#include <RHI/CommandList.hpp>

#include <memory>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    namespace TL = ::RHI::TL;

    class Pass;
    class Context;
    class CommandList;

    enum class BarrierType
    {
        PrePass,
        PostPass,
        Transition,
    };

    // per frame
    class CommandPool
    {
    public:
        VkResult Init(Context* context, uint32_t queueFamilyIndex);

        void Shutdown(Context* context);

        void Reset(Context* context);

        CommandList* Allocate(Context* context);

        void Release(Context* context, CommandList* commandList);

    private:
        VkCommandPool m_commandPool;
        std::vector<std::unique_ptr<CommandList>> m_commandLists;
        std::vector<CommandList*> m_availableCommandLists;
    };

    class CommandListAllocator final
    {
    public:
        CommandListAllocator(Context* context)
            : m_context(context)
        {
        }

        ~CommandListAllocator();

        VkResult Init(uint32_t queueFamilyIndex, uint32_t frameCount);

        void SetFrameIndex(uint32_t frameIndex);

        CommandList* Allocate();

        void Release(CommandList* commandList);

    private:
        Context* m_context;
        uint32_t m_frameIndex;
        std::vector<CommandPool> m_commandPools;
    };

    class CommandList final : public RHI::CommandList
    {
    public:
        CommandList() = default;

        CommandList(Context* context, VkCommandBuffer commandBuffer)
            : m_context(context)
            , m_commandBuffer(commandBuffer)
        {
        }

        void Reset();

        void Begin();

        void End();

        void RenderingBegin(Pass& pass);

        void RenderingEnd(Pass& pass);

        void PushDebugMarker(const char* name);

        void PopDebugMarker();

        void SetViewport(const RHI::Viewport& viewport) override;

        void SetSicssor(const RHI::Scissor& sicssor) override;

        void Submit(const RHI::CommandDraw& command) override;

        void Submit(const RHI::CommandCompute& command) override;

        void Submit(const RHI::CopyBufferDescriptor& command) override;

        void Submit(const RHI::CopyImageDescriptor& command) override;

        void Submit(const RHI::CopyBufferToImageDescriptor& command) override;

        void Submit(const RHI::CopyImageToBufferDescriptor& command) override;

        void BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<RHI::Handle<RHI::BindGroup>> bindGroups);

        VkRenderingAttachmentInfo GetAttachmentInfo(const RHI::ImagePassAttachment& passAttachment) const;

        void TransitionPassAttachments(BarrierType barrierType, TL::Span<RHI::ImagePassAttachment*> passAttachments) const;

        void TransitionPassAttachments(BarrierType barrierType, TL::Span<RHI::BufferPassAttachment*> passAttachments) const;

        Context* m_context = nullptr;

        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    };

} // namespace Vulkan