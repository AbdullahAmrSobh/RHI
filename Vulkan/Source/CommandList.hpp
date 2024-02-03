#pragma once

#include <RHI/FrameScheduler.hpp>
#include <RHI/CommandList.hpp>

#include <memory>
#include <array>

#include <vulkan/vulkan.h>

namespace Vulkan
{
    namespace TL = ::RHI::TL;

    class Pass;
    class Context;
    class CommandList;
    class Fence;

    enum class BarrierType
    {
        PrePass,
        PostPass,
        Transition,
    };

    class CommandPool
    {
    public:
        VkResult Init(Context* context, uint32_t queueFamilyIndex);

        void Shutdown(Context* context);

        void Reset(Context* context);

        CommandList* Allocate(Context* context);

        void Release(CommandList* commandList);

    private:
        VkCommandPool m_commandPool;
        std::vector<std::unique_ptr<CommandList>> m_commandLists;
        std::vector<CommandList*> m_availableCommandLists;
    };

    class CommandListAllocator final : public RHI::CommandListAllocator
    {
    public:
        CommandListAllocator(Context* context, uint32_t maxFrameBufferingCount)
            : m_context(context)
            , m_maxFrameBufferingCount(maxFrameBufferingCount)
        {
        }

        ~CommandListAllocator();

        VkResult Init(uint32_t queueFamilyIndex);

        void Flush(uint32_t newFrameIndex) override;
        RHI::CommandList* Allocate() override;

    private:
        Context* m_context;
        uint32_t m_maxFrameBufferingCount;
        uint32_t m_currentFrameIndex;
        std::array<CommandPool, 3> m_commandPools;
    };

    class CommandList final : public RHI::CommandList
    {
    public:
        CommandList(Context* context, VkCommandBuffer commandBuffer)
            : m_context(context)
            , m_pass(nullptr)
            , m_commandBuffer(commandBuffer)
        {
        }

        void Begin() override;

        void Begin(RHI::Pass& pass) override;

        void End() override;

        void SetViewport(const RHI::Viewport& viewport) override;

        void SetSicssor(const RHI::Scissor& sicssor) override;

        void Submit(const RHI::CommandDraw& command) override;

        void Submit(const RHI::CommandCompute& command) override;

        void Submit(const RHI::CopyBufferDescriptor& command) override;

        void Submit(const RHI::CopyImageDescriptor& command) override;

        void Submit(const RHI::CopyBufferToImageDescriptor& command) override;

        void Submit(const RHI::CopyImageToBufferDescriptor& command) override;

        VkRenderingAttachmentInfo GetAttachmentInfo(const RHI::ImagePassAttachment& passAttachment) const;

        void RenderingBegin(Pass& pass);

        void RenderingEnd(Pass& pass);

        void PushDebugMarker(const char* name);

        void PopDebugMarker();

        void BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<RHI::Handle<RHI::BindGroup>> bindGroups);

        void TransitionPassAttachments(BarrierType barrierType, TL::Span<RHI::ImagePassAttachment*> passAttachments) const;

        void TransitionPassAttachments(BarrierType barrierType, TL::Span<RHI::BufferPassAttachment*> passAttachments) const;

        Context* m_context;

        Pass* m_pass;

        CommandPool* m_parentPool;

        VkCommandBuffer m_commandBuffer;
    };

    void QueueSubmit(VkQueue queue, TL::Span<CommandList> commandlists, Fence* signalFence);

} // namespace Vulkan