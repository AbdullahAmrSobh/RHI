#pragma once

#include <RHI/CommandList.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

#include <array>
#include <mutex>
#include <thread>

namespace RHI::Vulkan
{
    class IDevice;

    VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource, Format format);

    VkResolveModeFlagBits ConvertResolveMode(ResolveMode resolveMode);

    struct PipelineBarriers
    {
        TL::Span<const VkMemoryBarrier2>       memoryBarriers = {};
        TL::Span<const VkImageMemoryBarrier2>  imageBarriers  = {};
        TL::Span<const VkBufferMemoryBarrier2> bufferBarriers = {};
    };

    class CommandPool
    {
    public:
        CommandPool();
        ~CommandPool();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        /// @brief Allocates a command list for the specified queue type.
        /// @param queueType The type of queue (Graphics, Compute, Transfer) for the command list.
        /// @return A pointer to the allocated CommandList.
        VkCommandBuffer AllocateCommandBuffer(QueueType queueType);

        void ReleaseCommandBuffers(TL::Span<const VkCommandBuffer> commandBuffers);

        /// @brief Resets all command lists used within the current frame.
        void Reset();

    private:
        using CommandPoolPerQueue = std::array<VkCommandPool, AsyncQueuesCount>;

        IDevice* m_device = nullptr;

        std::mutex                                    m_poolMutex;
        TL::Map<std::thread::id, CommandPoolPerQueue> m_pools;

        // Helper functions to create and destroy command pools
        VkCommandPool CreateCommandPool(QueueType queueType);
        void          DestroyCommandPools();
    };

    class ICommandList final : public CommandList
    {
    public:
        ICommandList();
        ~ICommandList();

        ResultCode Init(IDevice* device, const CommandListCreateInfo& createInfo);
        void       Shutdown();

        // [[deprecated]]
        void AddPipelineBarriers(const PipelineBarriers& barriers);

        // Interface implementation
        void Begin() override;
        void End() override;
        void AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers) override;
        void BeginRenderPass(const RenderPassBeginInfo& beginInfo) override;
        void EndRenderPass() override;
        void BeginComputePass(const ComputePassBeginInfo& beginInfo) override;
        void EndComputePass() override;
        void PushDebugMarker(const char* name, ClearValue color) override;
        void PopDebugMarker() override;
        void BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted) override;
        void EndConditionalCommands() override;
        void Execute(TL::Span<const CommandList*> commandLists) override;
        void BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups) override;
        void SetViewport(const Viewport& viewport) override;
        void SetScissor(const Scissor& sicssor) override;
        void BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) override;
        void BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType) override;
        void Draw(const DrawParameters& parameters) override;
        void DrawIndexed(const DrawIndexedParameters& parameters) override;
        void DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) override;
        void DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) override;
        void Dispatch(const DispatchParameters& parameters) override;
        void DispatchIndirect(const BufferBindingInfo& argumentBuffer) override;
        void CopyBuffer(const BufferCopyInfo& copyInfo) override;
        void CopyImage(const ImageCopyInfo& copyInfo) override;
        void CopyImageToBuffer(const BufferImageCopyInfo& copyInfo) override;
        void CopyBufferToImage(const BufferImageCopyInfo& copyInfo) override;

        VkCommandBuffer GetHandle() const { return m_commandBuffer; }

    private:
        void BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups);

    private:
        IDevice*        m_device                      = nullptr;
        VkCommandBuffer m_commandBuffer               = VK_NULL_HANDLE;
        bool            m_hasVertexBuffer         : 1 = false;
        bool            m_hasIndexBuffer          : 1 = false;
        bool            m_isGraphicsPipelineBound : 1 = false;
        bool            m_isComputePipelineBound  : 1 = false;
        bool            m_hasViewportSet          : 1 = false;
        bool            m_hasScissorSet           : 1 = false;
    };
} // namespace RHI::Vulkan