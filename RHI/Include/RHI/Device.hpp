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
        uint32_t maxMeshWorkGroupInvocations;
        uint32_t maxMeshWorkGroupSize[3];
    };

    struct QueueSubmitInfo
    {
        TL::Span<const FenceSubmitInfo> waitFences        = {};
        TL::Span<CommandList* const>    commandLists      = {};
        TL::Span<const FenceSubmitInfo> signalFences      = {};
        TL::Span<Swapchain*>            presentSwapchains = {};
    };

    struct AccelerationStructureSizesInfo
    {
        uint64_t size              = 0;
        uint64_t buildScratchSize  = 0;
        uint64_t updateScratchSize = 0;
    };

    class RenderGraph;

    class RHI_EXPORT Queue
    {
    public:
        Queue()          = default;
        virtual ~Queue() = default;

        virtual void BeginAnnotation(const char* name, uint32_t bgra)  = 0;
        virtual void EndAnnotation()                                   = 0;
        virtual void InsertAnnotation(const char* name, uint32_t bgra) = 0;

        virtual void Submit(const QueueSubmitInfo& submitInfo) = 0;

        virtual void WaitIdle()                              = 0;
        virtual void WaitFence(Fence* fence, uint64_t value) = 0;
    };

    class RHI_EXPORT Device
    {
    public:
        Device()          = default;
        virtual ~Device() = default;

        BackendType                    GetBackend() const { return m_backend; }

        DeviceLimits                   GetLimits() const { return m_limits; }

        virtual uint64_t               GarbageCollect(uint64_t graphicsTimeline)               = 0;
        virtual uint64_t               GetNativeHandle(NativeHandleType type, uint64_t handle) = 0;

        virtual Queue*                 GetQueue(QueueType queueType)                           = 0;

        // ShaderModule
        virtual ShaderModule*          CreateShaderModule(const ShaderModuleCreateInfo& createInfo) = 0;
        virtual void                   DestroyShaderModule(ShaderModule* shaderModule)              = 0;

        // BindGroupLayout
        virtual BindGroupLayout*       CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) = 0;
        virtual void                   DestroyBindGroupLayout(BindGroupLayout* handle)                    = 0;

        // BindGroup
        virtual BindGroup*             CreateBindGroup(const BindGroupCreateInfo& createInfo)                    = 0;
        virtual void                   DestroyBindGroup(BindGroup* handle)                                       = 0;
        virtual void                   UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) = 0;

        // PipelineLayout
        virtual PipelineLayout*        CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) = 0;
        virtual void                   DestroyPipelineLayout(PipelineLayout* handle)                    = 0;

        // Pipelines
        virtual GraphicsPipeline*      CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)     = 0;
        virtual void                   DestroyGraphicsPipeline(GraphicsPipeline* handle)                        = 0;
        virtual ComputePipeline*       CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)       = 0;
        virtual void                   DestroyComputePipeline(ComputePipeline* handle)                          = 0;
        virtual RayTracingPipeline*    CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) = 0;
        virtual void                   DestroyRayTracingPipeline(RayTracingPipeline* handle)                    = 0;

        // Buffer
        virtual Buffer*                CreateBuffer(const BufferCreateInfo& createInfo)               = 0;
        virtual void                   DestroyBuffer(Buffer* handle)                                  = 0;
        virtual uint64_t               GetBufferDeviceAddress(Buffer* buffer)                         = 0;
        virtual DeviceMemoryPtr        MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes) = 0;
        virtual void                   UnmapBuffer(Buffer* buffer)                                    = 0;

        // Image
        virtual Image*                 CreateImage(const ImageCreateInfo& createInfo)         = 0;
        virtual Image*                 CreateImageView(const ImageViewCreateInfo& createInfo) = 0;
        virtual void                   DestroyImage(Image* handle)                            = 0;

        // Sampler
        virtual Sampler*               CreateSampler(const SamplerCreateInfo& createInfo) = 0;
        virtual void                   DestroySampler(Sampler* handle)                    = 0;

        // Acceleration structures
        virtual AccelerationStructure* CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo) = 0;
        virtual void                   DestroyAccelerationStructure(AccelerationStructure* handle)                    = 0;
        virtual uint64_t               GetAccelerationStructureDeviceAddress(AccelerationStructure* handle)           = 0;
        virtual Micromap*              CreateMicromap(const MicromapCreateInfo& createInfo)                           = 0;
        virtual void                   DestroyMicromap(Micromap* handle)                                              = 0;

        // CommandPool
        virtual CommandPool*           CreateCommandPool(const CommandPoolCreateInfo& createInfo) = 0;
        virtual void                   DestroyCommandPool(CommandPool* handle)                    = 0;

        // Fence
        virtual Fence*                 CreateFence(const FenceCreateInfo& createInfo) = 0;
        virtual void                   DestroyFence(Fence* handle)                    = 0;
        virtual uint64_t               GetFenceValue(Fence* handle)                   = 0;

        // QueryPool
        virtual QueryPool*             CreateQueryPool(const QueryPoolCreateInfo& createInfo) = 0;
        virtual void                   DestroyQueryPool(QueryPool* handle)                    = 0;

        // Swapchain
        virtual Swapchain*             CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;
        virtual void                   DestroySwapchain(Swapchain* swapchain)                 = 0;

    protected:
        BackendType  m_backend;
        DeviceLimits m_limits;
    };
} // namespace RHI