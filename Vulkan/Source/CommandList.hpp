#pragma once

#include <RHI/CommandList.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    class IPassSubmitData;

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IContext* context, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
        ~ICommandList() = default;

        void Begin() override;
        void Begin(RenderGraph& renderGraph, Handle<Pass> pass) override;
        void End() override;
        void DebugMarkerPush(const char* name, ColorValue<float> color) override;
        void DebugMarkerPop() override;
        void BeginConditionalCommands(Handle<Buffer> buffer, size_t offset, bool inverted) override;
        void EndConditionalCommands() override;
        void Execute(TL::Span<const CommandList*> commandLists) override;
        void SetViewport(const Viewport& viewport) override;
        void SetSicssor(const Scissor& sicssor) override;
        void Draw(const DrawInfo& drawInfo) override;
        void Dispatch(const DispatchInfo& dispatchInfo) override;
        void Copy(const BufferCopyInfo& copyInfo) override;
        void Copy(const ImageCopyInfo& copyInfo) override;
        void Copy(const BufferToImageCopyInfo& copyInfo) override;
        void Copy(const ImageToBufferCopyInfo& copyInfo) override;

        void PipelineBarrier(TL::Span<const VkMemoryBarrier2> memoryBarriers,
                             TL::Span<const VkBufferMemoryBarrier2> bufferBarriers,
                             TL::Span<const VkImageMemoryBarrier2> imageBarriers);

        void BindShaderBindGroups(VkPipelineBindPoint bindPoint,
                                  VkPipelineLayout pipelineLayout,
                                  TL::Span<const Handle<BindGroup>> bindGroups,
                                  TL::Span<const uint32_t> dynamicOffset);

        VkCommandBuffer m_commandBuffer;
        VkCommandPool m_commandPool;
        VkCommandBufferLevel m_level;

    private:
        IContext* m_context;
        IPassSubmitData* m_passSubmitData;
    };

} // namespace RHI::Vulkan