#pragma once

#include "RHI/Export.hpp"
#include "RHI/Common/Ptr.hpp"
#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Result.hpp"
#include "RHI/Common/Debug.hpp"
#include "RHI/Common/Span.hpp"

#include "RHI/Resources.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/CommandList.hpp"

namespace RHI
{
    class RHI_EXPORT Context
    {
    public:
        virtual ~Context();

        /// @brief Returns the limits of the RHI implementation
        ///
        /// @return Limits object containing the RHI implementation limits
        RHI_NODISCARD Limits GetLimits() const;

        /// @brief Creates a new render graph
        ///
        /// @return Pointer to the newly created RenderGraph
        RHI_NODISCARD Ptr<RenderGraph> CreateRenderGraph();

        /// @brief Compiles the given render graph
        ///
        /// @param renderGraph The render graph to compile
        void CompileRenderGraph(RenderGraph& renderGraph);

        /// @brief Executes the given render graph
        ///
        /// @param renderGraph The render graph to execute
        /// @param signalFence Optional fence to signal upon completion
        void ExecuteRenderGraph(RenderGraph& renderGraph, Fence* signalFence = nullptr);

        /// @brief Creates a new swapchain
        ///
        /// @param createInfo The creation information for the swapchain
        /// @return Pointer to the newly created Swapchain
        RHI_NODISCARD Ptr<Swapchain> CreateSwapchain(const SwapchainCreateInfo& createInfo);

        /// @brief Creates a new shader module from the provided shader binary
        ///
        /// @param shaderBlob The shader binary data
        /// @return Pointer to the newly created ShaderModule
        RHI_NODISCARD Ptr<ShaderModule> CreateShaderModule(TL::Span<const uint32_t> shaderBlob);

        /// @brief Creates a new fence
        ///
        /// @return Pointer to the newly created Fence
        RHI_NODISCARD Ptr<Fence> CreateFence();

        /// @brief Creates a new command encoder
        ///
        /// @return Pointer to the newly created CommandEncoder
        RHI_NODISCARD Ptr<CommandEncoder> CreateCommandEncoder();

        /// @brief Creates a new bind group layout
        ///
        /// @param createInfo The creation information for the bind group layout
        /// @return Handle to the newly created BindGroupLayout
        RHI_NODISCARD Handle<BindGroupLayout> CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo);

        /// @brief Destroys a bind group layout
        ///
        /// @param handle Handle to the BindGroupLayout to destroy
        void DestroyBindGroupLayout(Handle<BindGroupLayout> handle);

        /// @brief Creates a new bind group
        ///
        /// @param handle Handle to the BindGroupLayout to use
        /// @param bindlessElementsCount Number of bindless elements (default: UINT32_MAX)
        /// @return Handle to the newly created BindGroup
        RHI_NODISCARD Handle<BindGroup> CreateBindGroup(Handle<BindGroupLayout> handle, uint32_t bindlessElementsCount = UINT32_MAX);

        /// @brief Destroys a bind group
        ///
        /// @param handle Handle to the BindGroup to destroy
        void DestroyBindGroup(Handle<BindGroup> handle);

        /// @brief Updates the bindings of a bind group
        ///
        /// @param handle Handle to the BindGroup to update
        /// @param bindings Span of ResourceBinding objects to update with
        void UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const ResourceBinding> bindings);

        /// @brief Creates a new pipeline layout
        ///
        /// @param createInfo The creation information for the pipeline layout
        /// @return Handle to the newly created PipelineLayout
        RHI_NODISCARD Handle<PipelineLayout> CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo);

        /// @brief Destroys a pipeline layout
        ///
        /// @param handle Handle to the PipelineLayout to destroy
        void DestroyPipelineLayout(Handle<PipelineLayout> handle);

        /// @brief Creates a new graphics pipeline
        ///
        /// @param createInfo The creation information for the graphics pipeline
        /// @return Handle to the newly created GraphicsPipeline
        RHI_NODISCARD Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);

        /// @brief Destroys a graphics pipeline
        ///
        /// @param handle Handle to the GraphicsPipeline to destroy
        void DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle);

        /// @brief Creates a new compute pipeline
        ///
        /// @param createInfo The creation information for the compute pipeline
        /// @return Handle to the newly created ComputePipeline
        RHI_NODISCARD Handle<ComputePipeline> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo);

        /// @brief Destroys a compute pipeline
        ///
        /// @param handle Handle to the ComputePipeline to destroy
        void DestroyComputePipeline(Handle<ComputePipeline> handle);

        /// @brief Creates a new sampler
        ///
        /// @param createInfo The creation information for the sampler
        /// @return Handle to the newly created Sampler
        RHI_NODISCARD Handle<Sampler> CreateSampler(const SamplerCreateInfo& createInfo);

        /// @brief Destroys a sampler
        ///
        /// @param handle Handle to the Sampler to destroy
        void DestroySampler(Handle<Sampler> handle);

        /// @brief Creates a new image
        ///
        /// @param createInfo The creation information for the image
        /// @return Result containing a handle to the newly created Image
        RHI_NODISCARD Result<Handle<Image>> CreateImage(const ImageCreateInfo& createInfo);

        /// @brief Destroys an image
        ///
        /// @param handle Handle to the Image to destroy
        void DestroyImage(Handle<Image> handle);

        /// @brief Creates a new buffer
        ///
        /// @param createInfo The creation information for the buffer
        /// @return Result containing a handle to the newly created Buffer
        RHI_NODISCARD Result<Handle<Buffer>> CreateBuffer(const BufferCreateInfo& createInfo);

        /// @brief Destroys a buffer
        ///
        /// @param handle Handle to the Buffer to destroy
        void DestroyBuffer(Handle<Buffer> handle);

        /// @brief Creates a new image view
        ///
        /// @param createInfo The creation information for the image view
        /// @return Handle to the newly created ImageView
        RHI_NODISCARD Handle<ImageView> CreateImageView(const ImageViewCreateInfo& createInfo);

        /// @brief Destroys an image view
        ///
        /// @param handle Handle to the ImageView to destroy
        void DestroyImageView(Handle<ImageView> handle);

        /// @brief Creates a new buffer view
        ///
        /// @param createInfo The creation information for the buffer view
        /// @return Handle to the newly created BufferView
        RHI_NODISCARD Handle<BufferView> CreateBufferView(const BufferViewCreateInfo& createInfo);

        /// @brief Destroys a buffer view
        ///
        /// @param handle Handle to the BufferView to destroy
        void DestroyBufferView(Handle<BufferView> handle);

        /// @brief Maps a buffer and returns a pointer to the mapped memory
        ///
        /// @param handle Handle to the Buffer to map
        /// @return Pointer to the mapped device memory
        RHI_NODISCARD DeviceMemoryPtr MapBuffer(Handle<Buffer> handle);

        /// @brief Unmaps a previously mapped buffer
        ///
        /// @param handle Handle to the Buffer to unmap
        void UnmapBuffer(Handle<Buffer> handle);

        /// @brief Dispatches a render graph
        ///
        /// @param renderGraph The render graph to dispatch
        void DispatchGraph(RenderGraph& renderGraph);

        /// @brief Allocates a temporary buffer
        ///
        /// @param size The size of the buffer to allocate
        /// @return A StagingBuffer object representing the allocated buffer
        RHI_NODISCARD StagingBuffer AllocateTempBuffer(size_t size);

        /// @brief Stages a write operation to an image
        ///
        /// @param image Handle to the Image to write to
        /// @param subresources The subresources of the image to write to
        /// @param stagingBuffer The staging buffer containing the data to write
        void StageImageWrite(Handle<Image> image, ImageSubresourceLayers subresources, StagingBuffer stagingBuffer);

        /// @brief Stages a write operation to a buffer
        ///
        /// @param buffer Handle to the Buffer to write to
        /// @param subregion The subregion of the buffer to write to
        /// @param stagingBuffer The staging buffer containing the data to write
        void StageBufferWrite(Handle<Buffer> buffer, BufferSubregion subregion, StagingBuffer stagingBuffer);

        /// @brief Stages a read operation from an image
        ///
        /// @param image Handle to the Image to read from
        /// @param subresources The subresources of the image to read from
        /// @param stagingBuffer The staging buffer to store the read data
        /// @param fence fence to signal when the read operation is complete
        void StageImageRead(Handle<Image> image, ImageSubresourceLayers subresources, StagingBuffer stagingBuffer, Fence* fence);

        /// @brief Stages a read operation from a buffer
        ///
        /// @param buffer Handle to the Buffer to read from
        /// @param subregion The subregion of the buffer to read from
        /// @param stagingBuffer The staging buffer to store the read data
        /// @param fence fence to signal when the read operation is complete
        void StageBufferRead(Handle<Buffer> buffer, BufferSubregion subregion, StagingBuffer stagingBuffer, Fence* fence);

    protected:
        Context(Ptr<DebugCallbacks> debugCallbacks);

        void Shutdown();

        void DebugLogError(std::string_view message);
        void DebugLogWarn(std::string_view message);
        void DebugLogInfo(std::string_view message);

        // clang-format off
        virtual Ptr<Swapchain>           Internal_CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;
        virtual Ptr<ShaderModule>        Internal_CreateShaderModule(TL::Span<const uint32_t> shaderBlob) = 0;
        virtual Ptr<Fence>               Internal_CreateFence() = 0;
        virtual Ptr<CommandEncoder>      Internal_CreateCommandEncoder() = 0;
        virtual Handle<BindGroupLayout>  Internal_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyBindGroupLayout(Handle<BindGroupLayout> handle) = 0;
        virtual Handle<BindGroup>        Internal_CreateBindGroup(Handle<BindGroupLayout> handle, uint32_t bindlessElementsCount) = 0;
        virtual void                     Internal_DestroyBindGroup(Handle<BindGroup> handle) = 0;
        virtual void                     Internal_UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const ResourceBinding> bindings) = 0;
        virtual Handle<PipelineLayout>   Internal_CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyPipelineLayout(Handle<PipelineLayout> handle) = 0;
        virtual Handle<GraphicsPipeline> Internal_CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle) = 0;
        virtual Handle<ComputePipeline>  Internal_CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyComputePipeline(Handle<ComputePipeline> handle) = 0;
        virtual Handle<Sampler>          Internal_CreateSampler(const SamplerCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroySampler(Handle<Sampler> handle) = 0;
        virtual Result<Handle<Image>>    Internal_CreateImage(const ImageCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyImage(Handle<Image> handle) = 0;
        virtual Result<Handle<Buffer>>   Internal_CreateBuffer(const BufferCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyBuffer(Handle<Buffer> handle) = 0;
        virtual Handle<ImageView>        Internal_CreateImageView(const ImageViewCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyImageView(Handle<ImageView> handle) = 0;
        virtual Handle<BufferView>       Internal_CreateBufferView(const BufferViewCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyBufferView(Handle<BufferView> handle) = 0;
        virtual DeviceMemoryPtr          Internal_MapBuffer(Handle<Buffer> handle) = 0;
        virtual void                     Internal_DispatchGraph(RenderGraph& renderGraph, Fence* signalFence) = 0;
        virtual void                     Internal_UnmapBuffer(Handle<Buffer> handle) = 0;
        virtual void                     Intenral_StageImageWrite(Handle<Image> image, ImageSubresourceLayers subresources, StagingBuffer stagingBuffererOffset) = 0;
        virtual void                     Intenral_StageBufferWrite(Handle<Buffer> buffer, BufferSubregion subregion, StagingBuffer stagingBufferset) = 0;
        virtual void                     Intenral_StageImageRead(Handle<Image> image, ImageSubresourceLayers subresources, StagingBuffer stagingBufferrOffset, Fence* fence) = 0;
        virtual void                     Intenral_StageBufferRead(Handle<Buffer> buffer, BufferSubregion subregion, StagingBuffer stagingBufferet, Fence* fence) = 0;
        // clang-format on

    protected:
        Ptr<Limits> m_limits;

    private:
        Ptr<DebugCallbacks> m_debugCallbacks;

        TL::Vector<Handle<Buffer>> m_stagingBuffers;
    };
} // namespace RHI