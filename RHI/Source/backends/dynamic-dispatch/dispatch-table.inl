#pragma once

#include <RHI/RHI.h>

#include "../../backends/vulkan/device.h"

namespace RHI::DynamicDispatch
{
    struct Table
    {
        void (*pfnQueueBeginAnnotation)(Queue* self, const char* name, uint32_t bgra) = nullptr;
        void (*pfnQueueEndAnnotation)(Queue* self) = nullptr;
        void (*pfnQueueInsertAnnotation)(Queue* self, const char* name, uint32_t bgra) = nullptr;
        void (*pfnQueueSubmit)(Queue* self, const QueueSubmitInfo& submitInfo) = nullptr;
        void (*pfnQueueWaitIdle)(Queue* self) = nullptr;
        void (*pfnQueueWaitFence)(Queue* self, Fence* fence, uint64_t value) = nullptr;
        BackendType (*pfnDeviceGetBackend)(Device* self) = nullptr;
        DeviceFeatures (*pfnDeviceGetFeatures)(Device* self) = nullptr;
        DeviceLimits (*pfnDeviceGetLimits)(Device* self) = nullptr;
        uint64_t (*pfnDeviceGarbageCollect)(Device* self, uint64_t graphicsTimeline) = nullptr;
        uint64_t (*pfnDeviceGetNativeHandle)(Device* self, NativeHandleType type, uint64_t handle) = nullptr;
        Queue* (*pfnDeviceGetQueue)(Device* self, QueueType queueType) = nullptr;
        ShaderModule* (*pfnDeviceCreateShaderModule)(Device* self, const ShaderModuleCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyShaderModule)(Device* self, ShaderModule* shaderModule) = nullptr;
        BindGroupLayout* (*pfnDeviceCreateBindGroupLayout)(Device* self, const BindGroupLayoutCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyBindGroupLayout)(Device* self, BindGroupLayout* handle) = nullptr;
        BindGroup* (*pfnDeviceCreateBindGroup)(Device* self, const BindGroupCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyBindGroup)(Device* self, BindGroup* handle) = nullptr;
        void (*pfnDeviceUpdateBindGroup)(Device* self, BindGroup* handle, const BindGroupUpdateInfo& updateInfo) = nullptr;
        PipelineLayout* (*pfnDeviceCreatePipelineLayout)(Device* self, const PipelineLayoutCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyPipelineLayout)(Device* self, PipelineLayout* handle) = nullptr;
        GraphicsPipeline* (*pfnDeviceCreateGraphicsPipeline)(Device* self, const GraphicsPipelineCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyGraphicsPipeline)(Device* self, GraphicsPipeline* handle) = nullptr;
        ComputePipeline* (*pfnDeviceCreateComputePipeline)(Device* self, const ComputePipelineCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyComputePipeline)(Device* self, ComputePipeline* handle) = nullptr;
        RayTracingPipeline* (*pfnDeviceCreateRayTracingPipeline)(Device* self, const RayTracingPipelineCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyRayTracingPipeline)(Device* self, RayTracingPipeline* handle) = nullptr;
        void (*pfnDeviceGetShaderBindingTableEntry)(Device* self, RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle) = nullptr;
        Buffer* (*pfnDeviceCreateBuffer)(Device* self, const BufferCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyBuffer)(Device* self, Buffer* handle) = nullptr;
        uint64_t (*pfnDeviceGetBufferDeviceAddress)(Device* self, Buffer* buffer) = nullptr;
        DeviceMemoryPtr (*pfnDeviceMapBuffer)(Device* self, Buffer* buffer, uint64_t offset, uint64_t sizeBytes) = nullptr;
        void (*pfnDeviceUnmapBuffer)(Device* self, Buffer* buffer) = nullptr;
        Image* (*pfnDeviceCreateImage)(Device* self, const ImageCreateInfo& createInfo) = nullptr;
        Image* (*pfnDeviceCreateImageView)(Device* self, const ImageViewCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyImage)(Device* self, Image* handle) = nullptr;
        Sampler* (*pfnDeviceCreateSampler)(Device* self, const SamplerCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroySampler)(Device* self, Sampler* handle) = nullptr;
        AccelerationStructure* (*pfnDeviceCreateAccelerationStructure)(Device* self, const AccelerationStructureCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyAccelerationStructure)(Device* self, AccelerationStructure* handle) = nullptr;
        uint64_t (*pfnDeviceGetAccelerationStructureDeviceAddress)(Device* self, AccelerationStructure* handle) = nullptr;
        AccelerationStructureSizesInfo (*pfnDeviceGetAccelerationStructureSizesInfo)(Device* self, AccelerationStructure* as) = nullptr;
        Micromap* (*pfnDeviceCreateMicromap)(Device* self, const MicromapCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyMicromap)(Device* self, Micromap* handle) = nullptr;
        CommandPool* (*pfnDeviceCreateCommandPool)(Device* self, const CommandPoolCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyCommandPool)(Device* self, CommandPool* handle) = nullptr;
        Fence* (*pfnDeviceCreateFence)(Device* self, const FenceCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyFence)(Device* self, Fence* handle) = nullptr;
        uint64_t (*pfnDeviceGetFenceValue)(Device* self, Fence* handle) = nullptr;
        QueryPool* (*pfnDeviceCreateQueryPool)(Device* self, const QueryPoolCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroyQueryPool)(Device* self, QueryPool* handle) = nullptr;
        Swapchain* (*pfnDeviceCreateSwapchain)(Device* self, const SwapchainCreateInfo& createInfo) = nullptr;
        void (*pfnDeviceDestroySwapchain)(Device* self, Swapchain* swapchain) = nullptr;
        uint32_t (*pfnDeviceGetSwapchainImagesCount)(Device* self, Swapchain* swapchain) = nullptr;
        SwapchainAcquireResult (*pfnDeviceAcquireSwapchainImage)(Device* self, Swapchain* swapchain) = nullptr;
        SurfaceCapabilities (*pfnDeviceGetSwapchainSurfaceCapabilities)(Device* self, Swapchain* swapchain) = nullptr;
        ResultCode (*pfnDeviceResizeSwapchain)(Device* self, Swapchain* swapchain, const ImageSize2D& size) = nullptr;
        ResultCode (*pfnDeviceConfigureSwapchain)(Device* self, Swapchain* swapchain, const SwapchainConfigureInfo& configInfo) = nullptr;
        void (*pfnCommandPoolReset)(CommandPool* self) = nullptr;
        CommandList* (*pfnCommandPoolAllocate)(CommandPool* self) = nullptr;
        void (*pfnCommandListBegin)(CommandList* self) = nullptr;
        void (*pfnCommandListEnd)(CommandList* self) = nullptr;
        void (*pfnCommandListPushDebugMarker)(CommandList* self, const char* name, uint32_t bgra) = nullptr;
        void (*pfnCommandListPopDebugMarker)(CommandList* self) = nullptr;
        void (*pfnCommandListInsertDebugMarker)(CommandList* self, const char* name, uint32_t bgra) = nullptr;
        void (*pfnCommandListAddPipelineBarrier)(CommandList* self, TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers) = nullptr;
        void (*pfnCommandListBeginRenderPass)(CommandList* self, const RenderPassBeginInfo& beginInfo) = nullptr;
        void (*pfnCommandListEndRenderPass)(CommandList* self) = nullptr;
        void (*pfnCommandListBeginComputePass)(CommandList* self, const ComputePassBeginInfo& beginInfo) = nullptr;
        void (*pfnCommandListEndComputePass)(CommandList* self) = nullptr;
        void (*pfnCommandListBeginConditionalCommands)(CommandList* self, const BufferBindingInfo& conditionBuffer, bool inverted) = nullptr;
        void (*pfnCommandListEndConditionalCommands)(CommandList* self) = nullptr;
        void (*pfnCommandListExecute)(CommandList* self, TL::Span<const CommandList*> commandLists) = nullptr;
        void (*pfnCommandListBindPipelineLayout)(CommandList* self, BindPoint bindPoint, const PipelineLayout* pipelineLayout) = nullptr;
        void (*pfnCommandListSetPushConstants)(CommandList* self, BindPoint bindPoint, uint32_t offset, TL::Block content) = nullptr;
        void (*pfnCommandListPushBindGroup)(CommandList* self, BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos) = nullptr;
        void (*pfnCommandListSetBindGroups)(CommandList* self, BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups) = nullptr;
        void (*pfnCommandListBindGraphicsPipeline)(CommandList* self, const GraphicsPipeline* pipelineState) = nullptr;
        void (*pfnCommandListBindComputePipeline)(CommandList* self, const ComputePipeline* pipelineState) = nullptr;
        void (*pfnCommandListBindRayTracingPipeline)(CommandList* self, const RayTracingPipeline* pipelineState) = nullptr;
        void (*pfnCommandListSetViewport)(CommandList* self, float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth) = nullptr;
        void (*pfnCommandListSetScissor)(CommandList* self, int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) = nullptr;
        void (*pfnCommandListBindVertexBuffers)(CommandList* self, uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) = nullptr;
        void (*pfnCommandListBindIndexBuffer)(CommandList* self, const BufferBindingInfo& indexBuffer, IndexType indexType) = nullptr;
        void (*pfnCommandListDraw)(CommandList* self, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = nullptr;
        void (*pfnCommandListDrawIndexed)(CommandList* self, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = nullptr;
        void (*pfnCommandListDrawMeshTasks)(CommandList* self, uint32_t x, uint32_t y, uint32_t z) = nullptr;
        void (*pfnCommandListDrawIndirect)(CommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) = nullptr;
        void (*pfnCommandListDrawIndexedIndirect)(CommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) = nullptr;
        void (*pfnCommandListDrawMeshTasksIndirect)(CommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride) = nullptr;
        void (*pfnCommandListDispatch)(CommandList* self, uint32_t x, uint32_t y, uint32_t z) = nullptr;
        void (*pfnCommandListDispatchIndirect)(CommandList* self, const BufferBindingInfo& argumentBuffer) = nullptr;
        void (*pfnCommandListDispatchRays)(CommandList* self, const DispatchRaysInfo& dispatchRaysDesc) = nullptr;
        void (*pfnCommandListDispatchRaysIndirect)(CommandList* self, const BufferBindingInfo& argumentBuffer) = nullptr;
        void (*pfnCommandListCopyBuffer)(CommandList* self, const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size) = nullptr;
        void (*pfnCommandListCopyImage)(CommandList* self, const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size) = nullptr;
        void (*pfnCommandListCopyImageToBuffer)(CommandList* self, const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer) = nullptr;
        void (*pfnCommandListCopyBufferToImage)(CommandList* self, const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout) = nullptr;
        void (*pfnCommandListCopyAccelerationStructure)(CommandList* self, AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode) = nullptr;
        void (*pfnCommandListCopyMicromap)(CommandList* self, Micromap* dst, const Micromap* src, CopyMode copyMode) = nullptr;
        void (*pfnCommandListBuildTlas)(CommandList* self, TL::Span<const TlasBuildInfo> buildInfos) = nullptr;
        void (*pfnCommandListBuildBlas)(CommandList* self, TL::Span<const BlasBuildInfo> buildInfos) = nullptr;
        void (*pfnCommandListBuildMicromaps)(CommandList* self, TL::Span<const MicromapBuildInfo> buildInfos) = nullptr;
        void (*pfnCommandListWriteAccelerationStructuresSizes)(CommandList* self, TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset) = nullptr;
        void (*pfnCommandListWriteMicromapsSizes)(CommandList* self, TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset) = nullptr;
    };

    Table loadVulkan()
    {
        return {
            .pfnQueueBeginAnnotation = Vulkan::queueBeginAnnotation,
            .pfnQueueEndAnnotation = Vulkan::queueEndAnnotation,
            .pfnQueueInsertAnnotation = Vulkan::queueInsertAnnotation,
            .pfnQueueSubmit = Vulkan::queueSubmit,
            .pfnQueueWaitIdle = Vulkan::queueWaitIdle,
            .pfnQueueWaitFence = Vulkan::queueWaitFence,
            .pfnDeviceGetBackend = Vulkan::deviceGetBackend,
            .pfnDeviceGetFeatures = Vulkan::deviceGetFeatures,
            .pfnDeviceGetLimits = Vulkan::deviceGetLimits,
            .pfnDeviceGarbageCollect = Vulkan::deviceGarbageCollect,
            .pfnDeviceGetNativeHandle = Vulkan::deviceGetNativeHandle,
            .pfnDeviceGetQueue = Vulkan::deviceGetQueue,
            .pfnDeviceCreateShaderModule = Vulkan::deviceCreateShaderModule,
            .pfnDeviceDestroyShaderModule = Vulkan::deviceDestroyShaderModule,
            .pfnDeviceCreateBindGroupLayout = Vulkan::deviceCreateBindGroupLayout,
            .pfnDeviceDestroyBindGroupLayout = Vulkan::deviceDestroyBindGroupLayout,
            .pfnDeviceCreateBindGroup = Vulkan::deviceCreateBindGroup,
            .pfnDeviceDestroyBindGroup = Vulkan::deviceDestroyBindGroup,
            .pfnDeviceUpdateBindGroup = Vulkan::deviceUpdateBindGroup,
            .pfnDeviceCreatePipelineLayout = Vulkan::deviceCreatePipelineLayout,
            .pfnDeviceDestroyPipelineLayout = Vulkan::deviceDestroyPipelineLayout,
            .pfnDeviceCreateGraphicsPipeline = Vulkan::deviceCreateGraphicsPipeline,
            .pfnDeviceDestroyGraphicsPipeline = Vulkan::deviceDestroyGraphicsPipeline,
            .pfnDeviceCreateComputePipeline = Vulkan::deviceCreateComputePipeline,
            .pfnDeviceDestroyComputePipeline = Vulkan::deviceDestroyComputePipeline,
            .pfnDeviceCreateRayTracingPipeline = Vulkan::deviceCreateRayTracingPipeline,
            .pfnDeviceDestroyRayTracingPipeline = Vulkan::deviceDestroyRayTracingPipeline,
            .pfnDeviceGetShaderBindingTableEntry = Vulkan::deviceGetShaderBindingTableEntry,
            .pfnDeviceCreateBuffer = Vulkan::deviceCreateBuffer,
            .pfnDeviceDestroyBuffer = Vulkan::deviceDestroyBuffer,
            .pfnDeviceGetBufferDeviceAddress = Vulkan::deviceGetBufferDeviceAddress,
            .pfnDeviceMapBuffer = Vulkan::deviceMapBuffer,
            .pfnDeviceUnmapBuffer = Vulkan::deviceUnmapBuffer,
            .pfnDeviceCreateImage = Vulkan::deviceCreateImage,
            .pfnDeviceCreateImageView = Vulkan::deviceCreateImageView,
            .pfnDeviceDestroyImage = Vulkan::deviceDestroyImage,
            .pfnDeviceCreateSampler = Vulkan::deviceCreateSampler,
            .pfnDeviceDestroySampler = Vulkan::deviceDestroySampler,
            .pfnDeviceCreateAccelerationStructure = Vulkan::deviceCreateAccelerationStructure,
            .pfnDeviceDestroyAccelerationStructure = Vulkan::deviceDestroyAccelerationStructure,
            .pfnDeviceGetAccelerationStructureDeviceAddress = Vulkan::deviceGetAccelerationStructureDeviceAddress,
            .pfnDeviceGetAccelerationStructureSizesInfo = Vulkan::deviceGetAccelerationStructureSizesInfo,
            .pfnDeviceCreateMicromap = Vulkan::deviceCreateMicromap,
            .pfnDeviceDestroyMicromap = Vulkan::deviceDestroyMicromap,
            .pfnDeviceCreateCommandPool = Vulkan::deviceCreateCommandPool,
            .pfnDeviceDestroyCommandPool = Vulkan::deviceDestroyCommandPool,
            .pfnDeviceCreateFence = Vulkan::deviceCreateFence,
            .pfnDeviceDestroyFence = Vulkan::deviceDestroyFence,
            .pfnDeviceGetFenceValue = Vulkan::deviceGetFenceValue,
            .pfnDeviceCreateQueryPool = Vulkan::deviceCreateQueryPool,
            .pfnDeviceDestroyQueryPool = Vulkan::deviceDestroyQueryPool,
            .pfnDeviceCreateSwapchain = Vulkan::deviceCreateSwapchain,
            .pfnDeviceDestroySwapchain = Vulkan::deviceDestroySwapchain,
            .pfnDeviceGetSwapchainImagesCount = Vulkan::deviceGetSwapchainImagesCount,
            .pfnDeviceAcquireSwapchainImage = Vulkan::deviceAcquireSwapchainImage,
            .pfnDeviceGetSwapchainSurfaceCapabilities = Vulkan::deviceGetSwapchainSurfaceCapabilities,
            .pfnDeviceResizeSwapchain = Vulkan::deviceResizeSwapchain,
            .pfnDeviceConfigureSwapchain = Vulkan::deviceConfigureSwapchain,
            .pfnCommandPoolReset = Vulkan::commandPoolReset,
            .pfnCommandPoolAllocate = Vulkan::commandPoolAllocate,
            .pfnCommandListBegin = Vulkan::cmdBegin,
            .pfnCommandListEnd = Vulkan::cmdEnd,
            .pfnCommandListPushDebugMarker = Vulkan::cmdPushDebugMarker,
            .pfnCommandListPopDebugMarker = Vulkan::cmdPopDebugMarker,
            .pfnCommandListInsertDebugMarker = Vulkan::cmdInsertDebugMarker,
            .pfnCommandListAddPipelineBarrier = Vulkan::cmdAddPipelineBarrier,
            .pfnCommandListBeginRenderPass = Vulkan::cmdBeginRenderPass,
            .pfnCommandListEndRenderPass = Vulkan::cmdEndRenderPass,
            .pfnCommandListBeginComputePass = Vulkan::cmdBeginComputePass,
            .pfnCommandListEndComputePass = Vulkan::cmdEndComputePass,
            .pfnCommandListBeginConditionalCommands = Vulkan::cmdBeginConditionalCommands,
            .pfnCommandListEndConditionalCommands = Vulkan::cmdEndConditionalCommands,
            .pfnCommandListExecute = Vulkan::cmdExecute,
            .pfnCommandListBindPipelineLayout = Vulkan::cmdBindPipelineLayout,
            .pfnCommandListSetPushConstants = Vulkan::cmdSetPushConstants,
            .pfnCommandListPushBindGroup = Vulkan::cmdPushBindGroup,
            .pfnCommandListSetBindGroups = Vulkan::cmdSetBindGroups,
            .pfnCommandListBindGraphicsPipeline = Vulkan::cmdBindGraphicsPipeline,
            .pfnCommandListBindComputePipeline = Vulkan::cmdBindComputePipeline,
            .pfnCommandListBindRayTracingPipeline = Vulkan::cmdBindRayTracingPipeline,
            .pfnCommandListSetViewport = Vulkan::cmdSetViewport,
            .pfnCommandListSetScissor = Vulkan::cmdSetScissor,
            .pfnCommandListBindVertexBuffers = Vulkan::cmdBindVertexBuffers,
            .pfnCommandListBindIndexBuffer = Vulkan::cmdBindIndexBuffer,
            .pfnCommandListDraw = Vulkan::cmdDraw,
            .pfnCommandListDrawIndexed = Vulkan::cmdDrawIndexed,
            .pfnCommandListDrawMeshTasks = Vulkan::cmdDrawMeshTasks,
            .pfnCommandListDrawIndirect = Vulkan::cmdDrawIndirect,
            .pfnCommandListDrawIndexedIndirect = Vulkan::cmdDrawIndexedIndirect,
            .pfnCommandListDrawMeshTasksIndirect = Vulkan::cmdDrawMeshTasksIndirect,
            .pfnCommandListDispatch = Vulkan::cmdDispatch,
            .pfnCommandListDispatchIndirect = Vulkan::cmdDispatchIndirect,
            .pfnCommandListDispatchRays = Vulkan::cmdDispatchRays,
            .pfnCommandListDispatchRaysIndirect = Vulkan::cmdDispatchRaysIndirect,
            .pfnCommandListCopyBuffer = Vulkan::cmdCopyBuffer,
            .pfnCommandListCopyImage = Vulkan::cmdCopyImage,
            .pfnCommandListCopyImageToBuffer = Vulkan::cmdCopyImageToBuffer,
            .pfnCommandListCopyBufferToImage = Vulkan::cmdCopyBufferToImage,
            .pfnCommandListCopyAccelerationStructure = Vulkan::cmdCopyAccelerationStructure,
            .pfnCommandListCopyMicromap = Vulkan::cmdCopyMicromap,
            .pfnCommandListBuildTlas = Vulkan::cmdBuildTlas,
            .pfnCommandListBuildBlas = Vulkan::cmdBuildBlas,
            .pfnCommandListBuildMicromaps = Vulkan::cmdBuildMicromaps,
            .pfnCommandListWriteAccelerationStructuresSizes = Vulkan::cmdWriteAccelerationStructuresSizes,
            .pfnCommandListWriteMicromapsSizes = Vulkan::cmdWriteMicromapsSizes,
        };
    }

    Table loadWebGPU();
    Table loadD3D12();

    // Backend state the RHI facades' m_impl point at in dynamic mode. Each derives from its RHI
    // facade (so `self` upcasts to the facade the table PFNs expect) and adds the dispatch table.
    struct IQueue : RHI::Queue
    {
        const Table* table;
    };

    struct IDevice : RHI::Device
    {
        const Table* table;
    };

    struct ICommandPool : RHI::CommandPool
    {
        const Table* table;
    };

    struct ICommandList : RHI::CommandList
    {
        const Table* table;
    };

    // Forwarders matching the Vulkan backend interface (camelCase, state self), in the same order,
    // so RHI.cpp can select this backend via `namespace Impl = RHI::DynamicDispatch`.

    static void queueBeginAnnotation(IQueue* self, const char* name, uint32_t bgra)
    {
        self->table->pfnQueueBeginAnnotation(self, name, bgra);
    }

    static void queueEndAnnotation(IQueue* self)
    {
        self->table->pfnQueueEndAnnotation(self);
    }

    static void queueInsertAnnotation(IQueue* self, const char* name, uint32_t bgra)
    {
        self->table->pfnQueueInsertAnnotation(self, name, bgra);
    }

    static void queueSubmit(IQueue* self, const QueueSubmitInfo& submitInfo)
    {
        self->table->pfnQueueSubmit(self, submitInfo);
    }

    static void queueWaitIdle(IQueue* self)
    {
        self->table->pfnQueueWaitIdle(self);
    }

    static void queueWaitFence(IQueue* self, Fence* fence, uint64_t value)
    {
        self->table->pfnQueueWaitFence(self, fence, value);
    }

    static BackendType deviceGetBackend(IDevice* self)
    {
        return self->table->pfnDeviceGetBackend(self);
    }

    static DeviceFeatures deviceGetFeatures(IDevice* self)
    {
        return self->table->pfnDeviceGetFeatures(self);
    }

    static DeviceLimits deviceGetLimits(IDevice* self)
    {
        return self->table->pfnDeviceGetLimits(self);
    }

    static uint64_t deviceGarbageCollect(IDevice* self, uint64_t graphicsTimeline)
    {
        return self->table->pfnDeviceGarbageCollect(self, graphicsTimeline);
    }

    static uint64_t deviceGetNativeHandle(IDevice* self, NativeHandleType type, uint64_t handle)
    {
        return self->table->pfnDeviceGetNativeHandle(self, type, handle);
    }

    static Queue* deviceGetQueue(IDevice* self, QueueType queueType)
    {
        return self->table->pfnDeviceGetQueue(self, queueType);
    }

    static ShaderModule* createShaderModule(IDevice* self, const ShaderModuleCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateShaderModule(self, createInfo);
    }

    static void destroyShaderModule(IDevice* self, ShaderModule* shaderModule)
    {
        self->table->pfnDeviceDestroyShaderModule(self, shaderModule);
    }

    static BindGroupLayout* createBindGroupLayout(IDevice* self, const BindGroupLayoutCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateBindGroupLayout(self, createInfo);
    }

    static void destroyBindGroupLayout(IDevice* self, BindGroupLayout* handle)
    {
        self->table->pfnDeviceDestroyBindGroupLayout(self, handle);
    }

    static BindGroup* createBindGroup(IDevice* self, const BindGroupCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateBindGroup(self, createInfo);
    }

    static void destroyBindGroup(IDevice* self, BindGroup* handle)
    {
        self->table->pfnDeviceDestroyBindGroup(self, handle);
    }

    static void bindGroupUpdate(IDevice* self, BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        self->table->pfnDeviceUpdateBindGroup(self, handle, updateInfo);
    }

    static PipelineLayout* createPipelineLayout(IDevice* self, const PipelineLayoutCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreatePipelineLayout(self, createInfo);
    }

    static void destroyPipelineLayout(IDevice* self, PipelineLayout* handle)
    {
        self->table->pfnDeviceDestroyPipelineLayout(self, handle);
    }

    static GraphicsPipeline* createGraphicsPipeline(IDevice* self, const GraphicsPipelineCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateGraphicsPipeline(self, createInfo);
    }

    static void destroyGraphicsPipeline(IDevice* self, GraphicsPipeline* handle)
    {
        self->table->pfnDeviceDestroyGraphicsPipeline(self, handle);
    }

    static ComputePipeline* createComputePipeline(IDevice* self, const ComputePipelineCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateComputePipeline(self, createInfo);
    }

    static void destroyComputePipeline(IDevice* self, ComputePipeline* handle)
    {
        self->table->pfnDeviceDestroyComputePipeline(self, handle);
    }

    static RayTracingPipeline* createRayTracingPipeline(IDevice* self, const RayTracingPipelineCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateRayTracingPipeline(self, createInfo);
    }

    static void destroyRayTracingPipeline(IDevice* self, RayTracingPipeline* handle)
    {
        self->table->pfnDeviceDestroyRayTracingPipeline(self, handle);
    }

    static void rayTracingPipelineGetShaderBindingTableEntry(IDevice* self, RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle)
    {
        self->table->pfnDeviceGetShaderBindingTableEntry(self, handle, group, size, dstHandle);
    }

    static Buffer* createBuffer(IDevice* self, const BufferCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateBuffer(self, createInfo);
    }

    static void destroyBuffer(IDevice* self, Buffer* handle)
    {
        self->table->pfnDeviceDestroyBuffer(self, handle);
    }

    static uint64_t bufferGetDeviceAddress(IDevice* self, Buffer* buffer)
    {
        return self->table->pfnDeviceGetBufferDeviceAddress(self, buffer);
    }

    static DeviceMemoryPtr bufferMap(IDevice* self, Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        return self->table->pfnDeviceMapBuffer(self, buffer, offset, sizeBytes);
    }

    static void bufferUnmap(IDevice* self, Buffer* buffer)
    {
        self->table->pfnDeviceUnmapBuffer(self, buffer);
    }

    static Image* createImage(IDevice* self, const ImageCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateImage(self, createInfo);
    }

    static Image* createImageView(IDevice* self, const ImageViewCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateImageView(self, createInfo);
    }

    static void destroyImage(IDevice* self, Image* handle)
    {
        self->table->pfnDeviceDestroyImage(self, handle);
    }

    static Sampler* createSampler(IDevice* self, const SamplerCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateSampler(self, createInfo);
    }

    static void destroySampler(IDevice* self, Sampler* handle)
    {
        self->table->pfnDeviceDestroySampler(self, handle);
    }

    static AccelerationStructure* createAccelerationStructure(IDevice* self, const AccelerationStructureCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateAccelerationStructure(self, createInfo);
    }

    static void destroyAccelerationStructure(IDevice* self, AccelerationStructure* handle)
    {
        self->table->pfnDeviceDestroyAccelerationStructure(self, handle);
    }

    static uint64_t accelerationStructureGetDeviceAddress(IDevice* self, AccelerationStructure* handle)
    {
        return self->table->pfnDeviceGetAccelerationStructureDeviceAddress(self, handle);
    }

    static AccelerationStructureSizesInfo accelerationStructureGetSizesInfo(IDevice* self, AccelerationStructure* as)
    {
        return self->table->pfnDeviceGetAccelerationStructureSizesInfo(self, as);
    }

    static Micromap* createMicromap(IDevice* self, const MicromapCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateMicromap(self, createInfo);
    }

    static void destroyMicromap(IDevice* self, Micromap* handle)
    {
        self->table->pfnDeviceDestroyMicromap(self, handle);
    }

    static CommandPool* createCommandPool(IDevice* self, const CommandPoolCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateCommandPool(self, createInfo);
    }

    static void destroyCommandPool(IDevice* self, CommandPool* handle)
    {
        self->table->pfnDeviceDestroyCommandPool(self, handle);
    }

    static Fence* createFence(IDevice* self, const FenceCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateFence(self, createInfo);
    }

    static void destroyFence(IDevice* self, Fence* handle)
    {
        self->table->pfnDeviceDestroyFence(self, handle);
    }

    static uint64_t fenceGetValue(IDevice* self, Fence* handle)
    {
        return self->table->pfnDeviceGetFenceValue(self, handle);
    }

    static QueryPool* createQueryPool(IDevice* self, const QueryPoolCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateQueryPool(self, createInfo);
    }

    static void destroyQueryPool(IDevice* self, QueryPool* handle)
    {
        self->table->pfnDeviceDestroyQueryPool(self, handle);
    }

    static Swapchain* createSwapchain(IDevice* self, const SwapchainCreateInfo& createInfo)
    {
        return self->table->pfnDeviceCreateSwapchain(self, createInfo);
    }

    static void destroySwapchain(IDevice* self, Swapchain* swapchain)
    {
        self->table->pfnDeviceDestroySwapchain(self, swapchain);
    }

    static uint32_t swapchainGetImagesCount(IDevice* self, Swapchain* swapchain)
    {
        return self->table->pfnDeviceGetSwapchainImagesCount(self, swapchain);
    }

    static SwapchainAcquireResult swapchainAcquireImage(IDevice* self, Swapchain* swapchain)
    {
        return self->table->pfnDeviceAcquireSwapchainImage(self, swapchain);
    }

    static SurfaceCapabilities swapchainGetSurfaceCapabilities(IDevice* self, Swapchain* swapchain)
    {
        return self->table->pfnDeviceGetSwapchainSurfaceCapabilities(self, swapchain);
    }

    static ResultCode swapchainResize(IDevice* self, Swapchain* swapchain, const ImageSize2D& size)
    {
        return self->table->pfnDeviceResizeSwapchain(self, swapchain, size);
    }

    static ResultCode swapchainConfigure(IDevice* self, Swapchain* swapchain, const SwapchainConfigureInfo& configInfo)
    {
        return self->table->pfnDeviceConfigureSwapchain(self, swapchain, configInfo);
    }

    static void commandPoolReset(ICommandPool* self)
    {
        self->table->pfnCommandPoolReset(self);
    }

    static CommandList* commandPoolAllocate(ICommandPool* self)
    {
        return self->table->pfnCommandPoolAllocate(self);
    }

    static void cmdBegin(ICommandList* self)
    {
        self->table->pfnCommandListBegin(self);
    }

    static void cmdEnd(ICommandList* self)
    {
        self->table->pfnCommandListEnd(self);
    }

    static void cmdPushDebugMarker(ICommandList* self, const char* name, uint32_t bgra)
    {
        self->table->pfnCommandListPushDebugMarker(self, name, bgra);
    }

    static void cmdPopDebugMarker(ICommandList* self)
    {
        self->table->pfnCommandListPopDebugMarker(self);
    }

    static void cmdInsertDebugMarker(ICommandList* self, const char* name, uint32_t bgra)
    {
        self->table->pfnCommandListInsertDebugMarker(self, name, bgra);
    }

    static void cmdAddPipelineBarrier(ICommandList* self, TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
        self->table->pfnCommandListAddPipelineBarrier(self, barriers, imageBarriers, bufferBarriers);
    }

    static void cmdBeginRenderPass(ICommandList* self, const RenderPassBeginInfo& beginInfo)
    {
        self->table->pfnCommandListBeginRenderPass(self, beginInfo);
    }

    static void cmdEndRenderPass(ICommandList* self)
    {
        self->table->pfnCommandListEndRenderPass(self);
    }

    static void cmdBeginComputePass(ICommandList* self, const ComputePassBeginInfo& beginInfo)
    {
        self->table->pfnCommandListBeginComputePass(self, beginInfo);
    }

    static void cmdEndComputePass(ICommandList* self)
    {
        self->table->pfnCommandListEndComputePass(self);
    }

    static void cmdBeginConditionalCommands(ICommandList* self, const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        self->table->pfnCommandListBeginConditionalCommands(self, conditionBuffer, inverted);
    }

    static void cmdEndConditionalCommands(ICommandList* self)
    {
        self->table->pfnCommandListEndConditionalCommands(self);
    }

    static void cmdExecute(ICommandList* self, TL::Span<const CommandList*> commandLists)
    {
        self->table->pfnCommandListExecute(self, commandLists);
    }

    static void cmdBindPipelineLayout(ICommandList* self, BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    {
        self->table->pfnCommandListBindPipelineLayout(self, bindPoint, pipelineLayout);
    }

    static void cmdSetPushConstants(ICommandList* self, BindPoint bindPoint, uint32_t offset, TL::Block content)
    {
        self->table->pfnCommandListSetPushConstants(self, bindPoint, offset, content);
    }

    static void cmdPushBindGroup(ICommandList* self, BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {
        self->table->pfnCommandListPushBindGroup(self, bindPoint, firstGroup, updateInfos);
    }

    static void cmdSetBindGroups(ICommandList* self, BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        self->table->pfnCommandListSetBindGroups(self, bindPoint, bindGroups);
    }

    static void cmdBindGraphicsPipeline(ICommandList* self, const GraphicsPipeline* pipelineState)
    {
        self->table->pfnCommandListBindGraphicsPipeline(self, pipelineState);
    }

    static void cmdBindComputePipeline(ICommandList* self, const ComputePipeline* pipelineState)
    {
        self->table->pfnCommandListBindComputePipeline(self, pipelineState);
    }

    static void cmdBindRayTracingPipeline(ICommandList* self, const RayTracingPipeline* pipelineState)
    {
        self->table->pfnCommandListBindRayTracingPipeline(self, pipelineState);
    }

    static void cmdSetViewport(ICommandList* self, float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth)
    {
        self->table->pfnCommandListSetViewport(self, offsetX, offsetY, width, height, minDepth, maxDepth);
    }

    static void cmdSetScissor(ICommandList* self, int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height)
    {
        self->table->pfnCommandListSetScissor(self, offsetX, offsetY, width, height);
    }

    static void cmdBindVertexBuffers(ICommandList* self, uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        self->table->pfnCommandListBindVertexBuffers(self, firstBinding, vertexBuffers);
    }

    static void cmdBindIndexBuffer(ICommandList* self, const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        self->table->pfnCommandListBindIndexBuffer(self, indexBuffer, indexType);
    }

    static void cmdDraw(ICommandList* self, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        self->table->pfnCommandListDraw(self, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    static void cmdDrawIndexed(ICommandList* self, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        self->table->pfnCommandListDrawIndexed(self, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    static void cmdDrawMeshTasks(ICommandList* self, uint32_t x, uint32_t y, uint32_t z)
    {
        self->table->pfnCommandListDrawMeshTasks(self, x, y, z);
    }

    static void cmdDrawIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        self->table->pfnCommandListDrawIndirect(self, argumentBuffer, countBuffer, maxDrawCount, stride);
    }

    static void cmdDrawIndexedIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        self->table->pfnCommandListDrawIndexedIndirect(self, argumentBuffer, countBuffer, maxDrawCount, stride);
    }

    static void cmdDrawMeshTasksIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        self->table->pfnCommandListDrawMeshTasksIndirect(self, argumentBuffer, countBuffer, drawNum, stride);
    }

    static void cmdDispatch(ICommandList* self, uint32_t x, uint32_t y, uint32_t z)
    {
        self->table->pfnCommandListDispatch(self, x, y, z);
    }

    static void cmdDispatchIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer)
    {
        self->table->pfnCommandListDispatchIndirect(self, argumentBuffer);
    }

    static void cmdDispatchRays(ICommandList* self, const DispatchRaysInfo& dispatchRaysDesc)
    {
        self->table->pfnCommandListDispatchRays(self, dispatchRaysDesc);
    }

    static void cmdDispatchRaysIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer)
    {
        self->table->pfnCommandListDispatchRaysIndirect(self, argumentBuffer);
    }

    static void cmdCopyBuffer(ICommandList* self, const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size)
    {
        self->table->pfnCommandListCopyBuffer(self, srcBuffer, srcOffset, dstBuffer, dstOffset, size);
    }

    static void cmdCopyImage(ICommandList* self, const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)
    {
        self->table->pfnCommandListCopyImage(self, srcImage, dstImage, size);
    }

    static void cmdCopyImageToBuffer(ICommandList* self, const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)
    {
        self->table->pfnCommandListCopyImageToBuffer(self, srcImage, layout, dstBuffer);
    }

    static void cmdCopyBufferToImage(ICommandList* self, const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)
    {
        self->table->pfnCommandListCopyBufferToImage(self, srcBuffer, dstImage, layout);
    }

    static void cmdCopyAccelerationStructure(ICommandList* self, AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    {
        self->table->pfnCommandListCopyAccelerationStructure(self, dst, src, copyMode);
    }

    static void cmdCopyMicromap(ICommandList* self, Micromap* dst, const Micromap* src, CopyMode copyMode)
    {
        self->table->pfnCommandListCopyMicromap(self, dst, src, copyMode);
    }

    static void cmdBuildTlas(ICommandList* self, TL::Span<const TlasBuildInfo> buildInfos)
    {
        self->table->pfnCommandListBuildTlas(self, buildInfos);
    }

    static void cmdBuildBlas(ICommandList* self, TL::Span<const BlasBuildInfo> buildInfos)
    {
        self->table->pfnCommandListBuildBlas(self, buildInfos);
    }

    static void cmdBuildMicromaps(ICommandList* self, TL::Span<const MicromapBuildInfo> buildInfos)
    {
        self->table->pfnCommandListBuildMicromaps(self, buildInfos);
    }

    static void cmdWriteAccelerationStructuresSizes(ICommandList* self, TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        self->table->pfnCommandListWriteAccelerationStructuresSizes(self, accelerationStructures, queryPool, queryPoolOffset);
    }

    static void cmdWriteMicromapsSizes(ICommandList* self, TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        self->table->pfnCommandListWriteMicromapsSizes(self, micromaps, queryPool, queryPoolOffset);
    }
} // namespace RHI::DynamicDispatch