#pragma once

#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/Swapchain.hpp"

#include <TL/Span.hpp>
#include <TL/Ptr.hpp>
#include <TL/Library.hpp>

namespace RHI
{
    enum class BackendType
    {
        Vulkan1_3,
        DirectX12_2,
        WebGPU,
    };

    enum class NativeHandleType
    {
        None,
        Device,
        CommandList,
        Buffer,
        Image,
        ImageView,
        Sampler,
        ShaderModule,
        Pipeline,
        PipelineLayout,
        BindGroupLayout,
        BindGroup,
        Swapchain,
    };

    struct RenderGraphCreateInfo;

    struct DeviceLimits
    {
        uint32_t minUniformBufferOffsetAlignment;
        uint32_t minStorageBufferOffsetAlignment;
    };

    struct SwapchainSignalInfo
    {
        Swapchain*    swapchain = nullptr;
        PipelineStage stage     = PipelineStage::None;
    };

    struct QueueSubmitInfo
    {
        TL::Span<FenceSubmitInfo>    waitFences        = {};
        TL::Span<CommandList* const> commandLists      = {};
        TL::Span<FenceSubmitInfo>    signalFences      = {};
        TL::Span<Swapchain*>         presentSwapchains = {};
    };

    class RenderGraph;

    class RHI_EXPORT Queue
    {
    public:
        Queue()          = default;
        virtual ~Queue() = default;

        /// @brief
        virtual void BeginAnnotation(const char* name, uint32_t bgra) = 0;

        /// @brief ...
        virtual void EndAnnotation() = 0;

        /// @brief ...
        virtual void InsertAnnotation(const char* name, uint32_t bgra) = 0;

        /// @brief ...
        virtual void Submit(const QueueSubmitInfo& submitInfo) = 0;

        /// @brief ...
        virtual void WaitIdle() = 0;

        /// @brief ...
        virtual void WaitFence(Fence* fence, uint64_t value) = 0;
    };

    /// @brief Represents a logical rendering device and provides resource creation and management.
    class RHI_EXPORT Device
    {
    public:
        Device()          = default;
        virtual ~Device() = default;

        /// @brief Returns the backend type.
        BackendType                 GetBackend() const { return m_backend; }

        /// @brief Returns device limits.
        DeviceLimits                GetLimits() const { return m_limits; }

        virtual uint64_t            GarbageCollect(uint64_t graphicsTimeline) = 0;

        /// @brief Retrieves a native handle for the specified type and object.
        virtual uint64_t            GetNativeHandle(NativeHandleType type, uint64_t handle) = 0;

        /// @brief Returns Device Queue with queueType.
        virtual Queue*              GetQueue(QueueType queueType) = 0;

        /// @brief Creates a fence.
        virtual CommandPool*        CreateCommandPool(const CommandPoolCreateInfo& createInfo) = 0;

        /// @brief Destroy a CommandPool.
        virtual void                DestroyCommandPool(CommandPool* handle) = 0;

        /// @brief Creates a fence.
        virtual Fence*              CreateFence(const FenceCreateInfo& createInfo) = 0;

        /// @brief Destroy a fence.
        virtual void                DestroyFence(Fence* handle) = 0;

        /// @brief Query fence value.
        virtual uint64_t            GetFenceValue(Fence* handle) = 0;

        /// @brief Creates a swapchain.
        virtual Swapchain*          CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;

        /// @brief Destroys a swapchain.
        virtual void                DestroySwapchain(Swapchain* swapchain) = 0;

        /// @brief Creates a shader module.
        virtual ShaderModule*       CreateShaderModule(const ShaderModuleCreateInfo& createInfo) = 0;

        /// @brief Destroys a shader module.
        virtual void                DestroyShaderModule(ShaderModule* shaderModule) = 0;

        /// @brief Creates a bind group layout.
        virtual BindGroupLayout*    CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) = 0;

        /// @brief Destroys a bind group layout.
        virtual void                DestroyBindGroupLayout(BindGroupLayout* handle) = 0;

        /// @brief Creates a bind group.
        virtual BindGroup*          CreateBindGroup(const BindGroupCreateInfo& createInfo) = 0;

        /// @brief Destroys a bind group.
        virtual void                DestroyBindGroup(BindGroup* handle) = 0;

        /// @brief Updates a bind group.
        virtual void                UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) = 0;

        /// @brief Creates a query pool.
        virtual QueryPool*          CreateQueryPool(const QueryPoolCreateInfo& createInfo) = 0;

        /// @brief Destroys a query pool.
        virtual void                DestroyQueryPool(QueryPool* handle) = 0;

        /// @brief Creates a buffer.
        virtual Buffer*             CreateBuffer(const BufferCreateInfo& createInfo) = 0;

        /// @brief Destroys a buffer.
        virtual void                DestroyBuffer(Buffer* handle) = 0;

        virtual uint64_t            GetBufferDeviceAddress(Buffer* buffer) = 0;

        /// @brief Maps buffer
        virtual DeviceMemoryPtr     MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes) = 0;

        /// @brief Unmaps buffer
        virtual void                UnmapBuffer(Buffer* buffer) = 0;

        /// @brief Creates an image.
        virtual Image*              CreateImage(const ImageCreateInfo& createInfo) = 0;

        /// @brief Creates an image view.
        virtual Image*              CreateImageView(const ImageViewCreateInfo& createInfo) = 0;

        /// @brief Destroys an image or image view.
        virtual void                DestroyImage(Image* handle) = 0;

        // virtual AccelerationStructure* CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo) = 0;
        // virtual void DestroyAccelerationStructure(AccelerationStructure* handle) = 0;

        // virtual QueryPool* CreateQueryPool(const QueryPoolCreateInfo& createInfo) = 0;
        // virtual void DestroyQueryPool(QueryPool* handle) = 0;

        // virtual Micromap* CreateMicromap(const MicromapCreateInfo& createInfo) = 0;
        // virtual void DestroyMicromap(Micromap* handle) = 0;

        /// @brief Creates a sampler.
        virtual Sampler*            CreateSampler(const SamplerCreateInfo& createInfo) = 0;

        /// @brief Destroys a sampler.
        virtual void                DestroySampler(Sampler* handle) = 0;

        /// @brief Creates a pipeline layout.
        virtual PipelineLayout*     CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) = 0;

        /// @brief Destroys a pipeline layout.
        virtual void                DestroyPipelineLayout(PipelineLayout* handle) = 0;

        /// @brief Creates a graphics pipeline.
        virtual GraphicsPipeline*   CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;

        /// @brief Destroys a graphics pipeline.
        virtual void                DestroyGraphicsPipeline(GraphicsPipeline* handle) = 0;

        /// @brief Creates a ray tracing pipeline.
        virtual RayTracingPipeline* CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) = 0;

        /// @brief Destroys a ray tracing pipeline.
        virtual void                DestroyRayTracingPipeline(RayTracingPipeline* handle) = 0;

        /// @brief Creates a compute pipeline.
        virtual ComputePipeline*    CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;

        /// @brief Destroys a compute pipeline.
        virtual void                DestroyComputePipeline(ComputePipeline* handle) = 0;

    protected:
        BackendType  m_backend;
        DeviceLimits m_limits;
    };
} // namespace RHI