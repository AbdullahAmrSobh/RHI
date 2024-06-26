#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Common/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    class IPassSubmitData;

    class ICommandPool final : public CommandPool
    {
    public:
        ICommandPool(IContext* context);
        ~ICommandPool();

        ResultCode Init(CommandPoolFlags flags);

        void Reset() override;
        TL::Vector<CommandList*> Allocate(QueueType queueType, CommandListLevel level, uint32_t count) override;
        void Release(TL::Span<const CommandList* const> commandLists) override;

    private:
        TL::Vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level);

    private:
        IContext* m_context;
        VkCommandPool m_commandPools[uint32_t(QueueType::Count)];
    };

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IContext* context, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
        ~ICommandList() = default;

        void BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups);

        void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> bindingInfos);

        void BindIndexBuffer(const BufferBindingInfo& bindingInfo, VkIndexType indexType);

        void PipelineBarrier(TL::Span<const VkMemoryBarrier2> memoryBarriers, TL::Span<const VkBufferMemoryBarrier2> bufferBarriers, TL::Span<const VkImageMemoryBarrier2> imageBarriers);

        void Begin() override;
        void Begin(const CommandListBeginInfo& beginInfo) override;
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
        void CopyBuffer(const BufferCopyInfo& copyInfo) override;
        void CopyImage(const ImageCopyInfo& copyInfo) override;
        void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) override;
        void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) override;

        VkCommandBuffer m_commandBuffer;
        VkCommandPool m_commandPool;
        VkCommandBufferLevel m_level;

    private:
        IContext* m_context;
        IPassSubmitData* m_passSubmitData;
    };

} // namespace RHI::Vulkan