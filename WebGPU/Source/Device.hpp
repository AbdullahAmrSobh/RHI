#pragma once

#include <RHI/Device.hpp>

#include <RHI-WebGPU/Loader.hpp>
#include <webgpu/webgpu.h>

#include "CommandList.hpp"
#include "Common.hpp"
#include "Resources.hpp"

namespace RHI::WebGPU
{
    class IQueue final : public Queue
    {
    public:
        ResultCode Init(IDevice* device, QueueType queueType);
        void       Shutdown();

        void BeginAnnotation(const char* name, uint32_t bgra) override;
        void EndAnnotation() override;
        void InsertAnnotation(const char* name, uint32_t bgra) override;
        void Submit(const QueueSubmitInfo& submitInfo) override;
        void WaitIdle() override;
        void WaitFence(Fence* fence, uint64_t value) override;

        IDevice*  m_device    = nullptr;
        WGPUQueue m_queue     = nullptr;
        QueueType m_queueType = QueueType::Graphics;
    };

    class IDevice final : public RHI::Device
    {
    public:
        friend Device* RHI::CreateWebGPUDevice();
        friend void    RHI::DestroyWebGPUDevice(Device* device);

        IDevice();
        ~IDevice();

        ResultCode Init();
        void       Shutdown();

        // Interface implementation
        uint64_t               GarbageCollect(uint64_t graphicsTimeline) override;
        uint64_t               GetNativeHandle(NativeHandleType type, uint64_t handle) override;
        Queue*                 GetQueue(QueueType queueType) override;
        ShaderModule*          CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void                   DestroyShaderModule(ShaderModule* shaderModule) override;
        BindGroupLayout*       CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                   DestroyBindGroupLayout(BindGroupLayout* handle) override;
        BindGroup*             CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void                   DestroyBindGroup(BindGroup* handle) override;
        void                   UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) override;
        PipelineLayout*        CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                   DestroyPipelineLayout(PipelineLayout* handle) override;
        GraphicsPipeline*      CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                   DestroyGraphicsPipeline(GraphicsPipeline* handle) override;
        ComputePipeline*       CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                   DestroyComputePipeline(ComputePipeline* handle) override;
        RayTracingPipeline*    CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) override;
        void                   DestroyRayTracingPipeline(RayTracingPipeline* handle) override;
        void                   GetShaderBindingTableEntry(RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle) override;
        Buffer*                CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                   DestroyBuffer(Buffer* handle) override;
        uint64_t               GetBufferDeviceAddress(Buffer* buffer) override;
        DeviceMemoryPtr        MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes) override;
        void                   UnmapBuffer(Buffer* buffer) override;
        Image*                 CreateImage(const ImageCreateInfo& createInfo) override;
        Image*                 CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void                   DestroyImage(Image* handle) override;
        Sampler*               CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                   DestroySampler(Sampler* handle) override;
        AccelerationStructure* CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo) override;
        void                   DestroyAccelerationStructure(AccelerationStructure* handle) override;
        uint64_t               GetAccelerationStructureDeviceAddress(AccelerationStructure* handle) override;
        AccelerationStructureSizesInfo GetAccelerationStructureSizesInfo(AccelerationStructure* handle) override;
        Micromap*              CreateMicromap(const MicromapCreateInfo& createInfo) override;
        void                   DestroyMicromap(Micromap* handle) override;
        CommandPool*           CreateCommandPool(const CommandPoolCreateInfo& createInfo) override;
        void                   DestroyCommandPool(CommandPool* handle) override;
        Fence*                 CreateFence(const FenceCreateInfo& createInfo) override;
        void                   DestroyFence(Fence* handle) override;
        uint64_t               GetFenceValue(Fence* handle) override;
        QueryPool*             CreateQueryPool(const QueryPoolCreateInfo& createInfo) override;
        void                   DestroyQueryPool(QueryPool* handle) override;
        Swapchain*             CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void                   DestroySwapchain(Swapchain* swapchain) override;

    public:
        // WebGPU instance and core objects
        WGPUInstance m_instance = nullptr; ///< WGPU instance handle.
        WGPUAdapter  m_adapter  = nullptr; ///< Physical device selected for use.
        WGPUDevice   m_device   = nullptr; ///< Logical device handle.

        IQueue m_queue[(uint32_t)QueueType::Count] = {};
    };
} // namespace RHI::WebGPU
