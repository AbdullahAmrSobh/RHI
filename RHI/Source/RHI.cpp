#include "RHI/RHI.h"

#include <TL/Assert.hpp>

// #if RHI_COMPILE_BACKEND_VULKAN
//     #include "RHI/Backend/Vulkan.h"
//     #define IMPL ::RHI::Vulkan::
// #elif RHI_COMPILE_BACKEND_D3D12
//     #include "RHI/Backend/D3D12.h"
//     #define IMPL ::RHI::D3D12::
// #elif RHI_COMPILE_BACKEND_WGPU
//     #include "RHI/Backend/WGPU.h"
//     #define IMPL ::RHI::WGPU::
// #elif RHI_COMPILE_BACKEND_DYNAMIC
//     #include "RHI/Backend/Dynamic.h"
//     #define IMPL ::RHI::Dynamic
// #elif RHI_COMPILE_BACKEND_EMPTY
//     #include "RHI/Backend/EMPTY.h"
//     #define IMPL ::RHI::Empty
// #else
//     #error "Invalid Backend"
// #endif

namespace RHI
{
    // clang-format off
    // Copied from nvrhi
    // Format mapping table. The rows must be in the exactly same order as Format enum members are defined.
    static const FormatInfo k_FormatInfoLUT[] = {
        // format                    name              bytes blk         type               red   green   blue  alpha  depth  stencl signed  srgb
        {  "Unknown",          Format::Unknown,           0,  0, FormatType::Integer,      false, false, false, false, false, false, false, false },
        {  "R8_UINT",          Format::R8_UINT,           1,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        {  "R8_SINT",          Format::R8_SINT,           1,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        {  "R8_UNORM",         Format::R8_UNORM,          1,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "R8_SNORM",         Format::R8_SNORM,          1,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "RG8_UINT",         Format::RG8_UINT,          2,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        {  "RG8_SINT",         Format::RG8_SINT,          2,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        {  "RG8_UNORM",        Format::RG8_UNORM,         2,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "RG8_SNORM",        Format::RG8_SNORM,         2,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "R16_UINT",         Format::R16_UINT,          2,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        {  "R16_SINT",         Format::R16_SINT,          2,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        {  "R16_UNORM",        Format::R16_UNORM,         2,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "R16_SNORM",        Format::R16_SNORM,         2,  1, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "R16_FLOAT",        Format::R16_FLOAT,         2,  1, FormatType::Float,        true,  false, false, false, false, false, true,  false },
        {  "BGRA4_UNORM",      Format::BGRA4_UNORM,       2,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "B5G6R5_UNORM",     Format::B5G6R5_UNORM,      2,  1, FormatType::Normalized,   true,  true,  true,  false, false, false, false, false },
        {  "B5G5R5A1_UNORM",   Format::B5G5R5A1_UNORM,    2,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "RGBA8_UINT",       Format::RGBA8_UINT,        4,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        {  "RGBA8_SINT",       Format::RGBA8_SINT,        4,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        {  "RGBA8_UNORM",      Format::RGBA8_UNORM,       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "RGBA8_SNORM",      Format::RGBA8_SNORM,       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BGRA8_UNORM",      Format::BGRA8_UNORM,       4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "SRGBA8_UNORM",     Format::SRGBA8_UNORM,      4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        {  "SBGRA8_UNORM",     Format::SBGRA8_UNORM,      4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "R10G10B10A2_UNORM",Format::R10G10B10A2_UNORM, 4,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "R11G11B10_FLOAT",  Format::R11G11B10_FLOAT,   4,  1, FormatType::Float,        true,  true,  true,  false, false, false, false, false },
        {  "RG16_UINT",        Format::RG16_UINT,         4,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        {  "RG16_SINT",        Format::RG16_SINT,         4,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        {  "RG16_UNORM",       Format::RG16_UNORM,        4,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "RG16_SNORM",       Format::RG16_SNORM,        4,  1, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "RG16_FLOAT",       Format::RG16_FLOAT,        4,  1, FormatType::Float,        true,  true,  false, false, false, false, true,  false },
        {  "R32_UINT",         Format::R32_UINT,          4,  1, FormatType::Integer,      true,  false, false, false, false, false, false, false },
        {  "R32_SINT",         Format::R32_SINT,          4,  1, FormatType::Integer,      true,  false, false, false, false, false, true,  false },
        {  "R32_FLOAT",        Format::R32_FLOAT,         4,  1, FormatType::Float,        true,  false, false, false, false, false, true,  false },
        {  "RGBA16_UINT",      Format::RGBA16_UINT,       8,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        {  "RGBA16_SINT",      Format::RGBA16_SINT,       8,  1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        {  "RGBA16_FLOAT",     Format::RGBA16_FLOAT,      8,  1, FormatType::Float,        true,  true,  true,  true,  false, false, true,  false },
        {  "RGBA16_UNORM",     Format::RGBA16_UNORM,      8,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "RGBA16_SNORM",     Format::RGBA16_SNORM,      8,  1, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "RG32_UINT",        Format::RG32_UINT,         8,  1, FormatType::Integer,      true,  true,  false, false, false, false, false, false },
        {  "RG32_SINT",        Format::RG32_SINT,         8,  1, FormatType::Integer,      true,  true,  false, false, false, false, true,  false },
        {  "RG32_FLOAT",       Format::RG32_FLOAT,        8,  1, FormatType::Float,        true,  true,  false, false, false, false, true,  false },
        {  "RGB32_UINT",       Format::RGB32_UINT,        12, 1, FormatType::Integer,      true,  true,  true,  false, false, false, false, false },
        {  "RGB32_SINT",       Format::RGB32_SINT,        12, 1, FormatType::Integer,      true,  true,  true,  false, false, false, true,  false },
        {  "RGB32_FLOAT",      Format::RGB32_FLOAT,       12, 1, FormatType::Float,        true,  true,  true,  false, false, false, true,  false },
        {  "RGBA32_UINT",      Format::RGBA32_UINT,       16, 1, FormatType::Integer,      true,  true,  true,  true,  false, false, false, false },
        {  "RGBA32_SINT",      Format::RGBA32_SINT,       16, 1, FormatType::Integer,      true,  true,  true,  true,  false, false, true,  false },
        {  "RGBA32_FLOAT",     Format::RGBA32_FLOAT,      16, 1, FormatType::Float,        true,  true,  true,  true,  false, false, true,  false },
        {  "D16",              Format::D16,               2,  1, FormatType::DepthStencil, false, false, false, false, true,  false, false, false },
        {  "D24S8",            Format::D24S8,             4,  1, FormatType::DepthStencil, false, false, false, false, true,  true,  false, false },
        {  "X24G8_UINT",       Format::X24G8_UINT,        4,  1, FormatType::Integer,      false, false, false, false, false, true,  false, false },
        {  "D32",              Format::D32,               4,  1, FormatType::DepthStencil, false, false, false, false, true,  false, false, false },
        {  "D32S8",            Format::D32S8,             8,  1, FormatType::DepthStencil, false, false, false, false, true,  true,  false, false },
        {  "X32G8_UINT",       Format::X32G8_UINT,        8,  1, FormatType::Integer,      false, false, false, false, false, true,  false, false },
        {  "BC1_UNORM",        Format::BC1_UNORM,         8,  4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BC1_UNORM_SRGB",   Format::BC1_UNORM_SRGB,    8,  4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        {  "BC2_UNORM",        Format::BC2_UNORM,         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BC2_UNORM_SRGB",   Format::BC2_UNORM_SRGB,    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        {  "BC3_UNORM",        Format::BC3_UNORM,         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BC3_UNORM_SRGB",   Format::BC3_UNORM_SRGB,    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        {  "BC4_UNORM",        Format::BC4_UNORM,         8,  4, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "BC4_SNORM",        Format::BC4_SNORM,         8,  4, FormatType::Normalized,   true,  false, false, false, false, false, false, false },
        {  "BC5_UNORM",        Format::BC5_UNORM,         16, 4, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "BC5_SNORM",        Format::BC5_SNORM,         16, 4, FormatType::Normalized,   true,  true,  false, false, false, false, false, false },
        {  "BC6H_UFLOAT",      Format::BC6H_UFLOAT,       16, 4, FormatType::Float,        true,  true,  true,  false, false, false, false, false },
        {  "BC6H_SFLOAT",      Format::BC6H_SFLOAT,       16, 4, FormatType::Float,        true,  true,  true,  false, false, false, true,  false },
        {  "BC7_UNORM",        Format::BC7_UNORM,         16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, false },
        {  "BC7_UNORM_SRGB",   Format::BC7_UNORM_SRGB,    16, 4, FormatType::Normalized,   true,  true,  true,  true,  false, false, false, true  },
    };

    // clang-format on

    const FormatInfo& GetFormatInfo(Format format)
    {
        static_assert(
            sizeof(k_FormatInfoLUT) / sizeof(FormatInfo) == size_t(Format::COUNT),
            "The format info table doesn't have the right number of elements");

        if (uint32_t(format) >= uint32_t(Format::COUNT)) return k_FormatInfoLUT[0]; // UNKNOWN

        const FormatInfo& info = k_FormatInfoLUT[uint32_t(format)];
        TL_ASSERT(info.format == format);
        return info;
    }

    uint32_t GetFormatByteSize(Format format)
    {
        return GetFormatInfo(format).bytesPerBlock;
    }

    // IMPL

    // clang-format off
#if 0
struct Instance
{
    Impl                            m_impl;
    // QUEUE
    void                            (*BeginAnnotation)(IDevice& self, const char* name, uint32_t bgra) = nullptr;
    void                            (*EndAnnotation)(IDevice& self, ) = nullptr;
    void                            (*InsertAnnotation)(IDevice& self, const char* name, uint32_t bgra) = nullptr;
    void                            (*Submit)(IDevice& self, const QueueSubmitInfo& submitInfo) = nullptr;
    void                            (*WaitIdle)(IDevice& self, ) = nullptr;
    void                            (*WaitFence)(IDevice& self, Fence* fence, uint64_t value) = nullptr;
    // DEVICE
    uint64_t                        (*GarbageCollect)(uint64_t graphicsTimeline) = nullptr;
    uint64_t                        (*GetNativeHandle)(NativeHandleType type, uint64_t handle) = nullptr;
    Queue*                          (*GetQueue)(QueueType queueType) = nullptr;
    ShaderModule*                   (*CreateShaderModule)(const ShaderModuleCreateInfo& createInfo) = nullptr;
    void                            (*DestroyShaderModule)(ShaderModule* shaderModule) = nullptr;
    BindGroupLayout*                (*CreateBindGroupLayout)(const BindGroupLayoutCreateInfo& createInfo) = nullptr;
    void                            (*DestroyBindGroupLayout)(BindGroupLayout* handle) = nullptr;
    BindGroup*                      (*CreateBindGroup)(const BindGroupCreateInfo& createInfo) = nullptr;
    void                            (*DestroyBindGroup)(BindGroup* handle) = nullptr;
    void                            (*UpdateBindGroup)(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) = nullptr;
    PipelineLayout*                 (*CreatePipelineLayout)(const PipelineLayoutCreateInfo& createInfo) = nullptr;
    void                            (*DestroyPipelineLayout)(PipelineLayout* handle) = nullptr;
    GraphicsPipeline*               (*CreateGraphicsPipeline)(const GraphicsPipelineCreateInfo& createInfo) = nullptr;
    void                            (*DestroyGraphicsPipeline)(GraphicsPipeline* handle) = nullptr;
    ComputePipeline*                (*CreateComputePipeline)(const ComputePipelineCreateInfo& createInfo) = nullptr;
    void                            (*DestroyComputePipeline)(ComputePipeline* handle) = nullptr;
    RayTracingPipeline*             (*CreateRayTracingPipeline)(const RayTracingPipelineCreateInfo& createInfo) = nullptr;
    void                            (*DestroyRayTracingPipeline)(RayTracingPipeline* handle) = nullptr;
    void                            (*GetShaderBindingTableEntry)(RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle) = nullptr;
    Buffer*                         (*CreateBuffer)(const BufferCreateInfo& createInfo) = nullptr;
    void                            (*DestroyBuffer)(Buffer* handle) = nullptr;
    uint64_t                        (*GetBufferDeviceAddress)(Buffer* buffer) = nullptr;
    DeviceMemoryPtr                 (*MapBuffer)(Buffer* buffer, uint64_t offset, uint64_t sizeBytes) = nullptr;
    void                            (*UnmapBuffer)(Buffer* buffer) = nullptr;
    Image*                          (*CreateImage)(const ImageCreateInfo& createInfo) = nullptr;
    Image*                          (*CreateImageView)(const ImageViewCreateInfo& createInfo) = nullptr;
    void                            (*DestroyImage)(Image* handle) = nullptr;
    Sampler*                        (*CreateSampler)(const SamplerCreateInfo& createInfo) = nullptr;
    void                            (*DestroySampler)(Sampler* handle) = nullptr;
    AccelerationStructure*          (*CreateAccelerationStructure)(const AccelerationStructureCreateInfo& createInfo) = nullptr;
    void                            (*DestroyAccelerationStructure)(AccelerationStructure* handle) = nullptr;
    uint64_t                        (*GetAccelerationStructureDeviceAddress)(AccelerationStructure* handle) = nullptr;
    AccelerationStructureSizesInfo  (*GetAccelerationStructureSizesInfo)(AccelerationStructure* as) = nullptr;
    Micromap*                       (*CreateMicromap)(const MicromapCreateInfo& createInfo) = nullptr;
    void                            (*DestroyMicromap)(Micromap* handle) = nullptr;
    CommandPool*                    (*CreateCommandPool)(const CommandPoolCreateInfo& createInfo) = nullptr;
    void                            (*DestroyCommandPool)(CommandPool* handle) = nullptr;
    Fence*                          (*CreateFence)(const FenceCreateInfo& createInfo) = nullptr;
    void                            (*DestroyFence)(Fence* handle) = nullptr;
    uint64_t                        (*GetFenceValue)(Fence* handle) = nullptr;
    QueryPool*                      (*CreateQueryPool)(const QueryPoolCreateInfo& createInfo) = nullptr;
    void                            (*DestroyQueryPool)(QueryPool* handle) = nullptr;
    Swapchain*                      (*CreateSwapchain)(const SwapchainCreateInfo& createInfo) = nullptr;
    void                            (*DestroySwapchain)(Swapchain* swapchain) = nullptr;
    uint32_t                        (*GetSwapchainImagesCount)(Swapchain* swapchain) = nullptr;
    SwapchainAcquireResult          (*AcquireSwapchainImage)(Swapchain* swapchain) = nullptr;
    SurfaceCapabilities             (*GetSwapchainSurfaceCapabilities)(Swapchain* swapchain) = nullptr;
    ResultCode                      (*ResizeSwapchain)(Swapchain* swapchain, const ImageSize2D& size) = nullptr;
    ResultCode                      (*ConfigureSwapchain)(Swapchain* swapchain, const SwapchainConfigureInfo& configInfo) = nullptr;
    // COMMAND POOL
    void                            (*Reset)() = nullptr;
    CommandList*                    (*Allocate)() = nullptr;
    // COMMAND LIST
    void                            (*Begin)() = nullptr;
    void                            (*End)() = nullptr;
    void                            (*PushDebugMarker)(const char* name, uint32_t bgra) = nullptr;
    void                            (*PopDebugMarker)() = nullptr;
    void                            (*InsertDebugMarker)(const char* name, uint32_t bgra) = nullptr;
    void                            (*AddPipelineBarrier)(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers) = nullptr;
    void                            (*BeginRenderPass)(const RenderPassBeginInfo& beginInfo) = nullptr;
    void                            (*EndRenderPass)() = nullptr;
    void                            (*BeginComputePass)(const ComputePassBeginInfo& beginInfo) = nullptr;
    void                            (*EndComputePass)() = nullptr;
    void                            (*BeginConditionalCommands)(const BufferBindingInfo& conditionBuffer, bool inverted) = nullptr;
    void                            (*EndConditionalCommands)() = nullptr;
    void                            (*Execute)(TL::Span<const CommandList*> commandLists) = nullptr;
    void                            (*BindPipelineLayout)(BindPoint bindPoint, const PipelineLayout* pipelineLayout) = nullptr;
    void                            (*SetPushConstants)(BindPoint bindPoint, uint32_t offset, TL::Block content) = nullptr;
    void                            (*PushBindGroup)(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos) = nullptr;
    void                            (*SetBindGroups)(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups) = nullptr;
    void                            (*BindGraphicsPipeline)(const GraphicsPipeline* pipelineState) = nullptr;
    void                            (*BindComputePipeline)(const ComputePipeline* pipelineState) = nullptr;
    void                            (*BindRayTracingPipeline)(const RayTracingPipeline* pipelineState) = nullptr;
    void                            (*SetViewport)(float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth) = nullptr;
    void                            (*SetScissor)(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) = nullptr;
    void                            (*BindVertexBuffers)(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers) = nullptr;
    void                            (*BindIndexBuffer)(const BufferBindingInfo& indexBuffer, IndexType indexType) = nullptr;
    void                            (*Draw)(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = nullptr;
    void                            (*DrawIndexed)(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = nullptr;
    void                            (*DrawMeshTasks)(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1) = nullptr;
    void                            (*DrawIndirect)(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) = nullptr;
    void                            (*DrawIndexedIndirect)(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride) = nullptr;
    void                            (*DrawMeshTasksIndirect)(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride) = nullptr;
    void                            (*Dispatch)(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1) = nullptr;
    void                            (*DispatchIndirect)(const BufferBindingInfo& argumentBuffer) = nullptr;
    void                            (*DispatchRays)(const DispatchRaysInfo& dispatchRaysDesc) = nullptr;
    void                            (*DispatchRaysIndirect)(const BufferBindingInfo& argumentBuffer) = nullptr;
    void                            (*CopyBuffer)(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size) = nullptr;
    void                            (*CopyImage)(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size) = nullptr;
    void                            (*CopyImageToBuffer)(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer) = nullptr;
    void                            (*CopyBufferToImage)(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout) = nullptr;
    void                            (*CopyAccelerationStructure)(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode) = nullptr;
    void                            (*CopyMicromap)(Micromap* dst, const Micromap* src, CopyMode copyMode) = nullptr;
    void                            (*BuildTlas)(TL::Span<const TlasBuildInfo> buildInfos) = nullptr;
    void                            (*BuildBlas)(TL::Span<const BlasBuildInfo> buildInfos) = nullptr;
    void                            (*BuildMicromaps)(TL::Span<const MicromapBuildInfo> buildInfos) = nullptr;
    void                            (*WriteAccelerationStructuresSizes)(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset) = nullptr;
    void                            (*WriteMicromapsSizes)(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset) = nullptr;
};
#endif
} // namespace RHI