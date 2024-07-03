#pragma once

#include <RHI/Common/Result.hpp>
#include <RHI/Common/Ptr.hpp>
#include <RHI/CommandList.hpp>
#include <RHI/Definitions.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    struct CommandBuffer : CommandList
    {
        VkCommandBufferLevel level;
        VkCommandBuffer handle;
    };

    class ICommandEncoder final : public CommandEncoder
    {
    public:
        ICommandEncoder(IContext* context);
        virtual ~ICommandEncoder();

        void BindShaderBindGroups(Handle<CommandList> commandList, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups);
        void BindVertexBuffers(Handle<CommandList> commandList, uint32_t firstBinding, TL::Span<const BufferBindingInfo> bindingInfos);
        void BindIndexBuffer(Handle<CommandList> commandList, const BufferBindingInfo& bindingInfo, VkIndexType indexType);
        void BeginPrimary(Handle<CommandList> commandList, RenderGraph& renderGraph, Handle<Pass> pass);
        void BeginSecondary(Handle<CommandList> commandList, const RenderTargetLayoutDesc& renderTargetLayoutDesc);

        void PipelineBarrier(Handle<CommandList> commandList, TL::Span<const VkMemoryBarrier2> memoryBarriers, TL::Span<const VkImageMemoryBarrier2> imageBarriers, TL::Span<const VkBufferMemoryBarrier2> bufferBarriers);

        void Allocate(Flags<CommandFlags> flags, RHI_OUT_PARM TL::Span<Handle<CommandList>> commandLists) override;
        void Release(TL::Span<Handle<CommandList>> commandList) override;
        void Reset(TL::Span<Handle<CommandList>> commandList) override;
        void Begin(Handle<CommandList> commandList) override;
        void Begin(Handle<CommandList> commandList, const CommandListBeginInfo& beginInfo) override;
        void End(Handle<CommandList> commandList) override;
        void DebugMarkerPush(Handle<CommandList> commandList, const char* name, ColorValue<float> color) override;
        void DebugMarkerPop(Handle<CommandList> commandList) override;
        void BeginConditionalCommands(Handle<CommandList> commandList, Handle<Buffer> buffer, size_t offset, CommandConditionMode inverted) override;
        void EndConditionalCommands(Handle<CommandList> commandList) override;
        void Execute(Handle<CommandList> commandList, TL::Span<const Handle<CommandList>> commandLists) override;
        void SetViewport(Handle<CommandList> commandList, const Viewport& viewport) override;
        void SetScissor(Handle<CommandList> commandList, const Scissor& scissor) override;
        void Draw(Handle<CommandList> commandList, const DrawInfo& drawInfo) override;
        void Dispatch(Handle<CommandList> commandList, const DispatchInfo& dispatchInfo) override;
        void CopyBuffer(Handle<CommandList> commandList, const BufferCopyInfo& copyInfo) override;
        void CopyImage(Handle<CommandList> commandList, const ImageCopyInfo& copyInfo) override;
        void CopyImageToBuffer(Handle<CommandList> commandList, const BufferImageCopyInfo& copyInfo) override;
        void CopyBufferToImage(Handle<CommandList> commandList, const BufferImageCopyInfo& copyInfo) override;

    private:
        IContext* m_context;

        VkCommandPool commandPool[4][3];

        HandlePool<CommandBuffer> m_commandBufferPool;

    };
} // namespace RHI::Vulkan