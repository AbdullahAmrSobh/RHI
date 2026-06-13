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

    uint32_t GetFormatComponentByteSize(Format format)
    {
        return GetFormatInfo(format).bytesPerBlock;
    }

    FormatType GetFormatType(Format format)
    {
        return GetFormatInfo(format).type;
    }

    TL::Flags<ImageAspect> GetFormatAspects(Format format)
    {
        auto                   formatInfo = GetFormatInfo(format);
        TL::Flags<ImageAspect> flags      = ImageAspect::None;
        if (formatInfo.hasDepth) flags |= ImageAspect::Depth;
        if (formatInfo.hasStencil) flags |= ImageAspect::Stencil;
        if (formatInfo.hasRed || formatInfo.hasGreen || formatInfo.hasBlue || formatInfo.hasAlpha) flags |= ImageAspect::Color;
        return flags;
    }

    // IMPL

    // void Device::BeginAnnotation(const char* name, uint32_t bgra)
    // {
    //     return IMPL::BeginAnnotation(...);
    // }

    // void Device::EndAnnotation()
    // {
    //     return IMPL::EndAnnotation(...);
    // }

    // void Device::InsertAnnotation(const char* name, uint32_t bgra)
    // {
    //     return IMPL::InsertAnnotation(...);
    // }

    // void Device::Submit(const QueueSubmitInfo& submitInfo)
    // {
    //     return IMPL::Submit(...);
    // }

    // void Device::WaitIdle()
    // {
    //     return IMPL::WaitIdle(...);
    // }

    // void Device::WaitFence(Fence* fence, uint64_t value)
    // {
    //     return IMPL::WaitFence(...);
    // }

    // uint64_t Device::GarbageCollect(uint64_t graphicsTimeline)
    // {
    //     return IMPL::GarbageCollect(...);
    // }

    // uint64_t Device::GetNativeHandle(NativeHandleType type, uint64_t handle)
    // {
    //     return IMPL::GetNativeHandle(...);
    // }

    // Queue* Device::GetQueue(QueueType queueType)
    // {
    //     return IMPL:: GetQueue(...);
    // }

    // ShaderModule* Device::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateShaderModule(...);
    // }

    // void Device::DestroyShaderModule(ShaderModule* shaderModule)
    // {
    //     return IMPL::DestroyShaderModule(...);
    // }

    // BindGroupLayout* Device::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateBindGroupLayout(...);
    // }

    // void Device::DestroyBindGroupLayout(BindGroupLayout* handle)
    // {
    //     return IMPL::DestroyBindGroupLayout(...);
    // }

    // BindGroup* Device::CreateBindGroup(const BindGroupCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateBindGroup(...);
    // }

    // void Device::DestroyBindGroup(BindGroup* handle)
    // {
    //     return IMPL::DestroyBindGroup(...);
    // }

    // void Device::UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    // {
    //     return IMPL::UpdateBindGroup(...);
    // }

    // PipelineLayout* Device::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    // {
    //     return IMPL:: CreatePipelineLayout(...);
    // }

    // void Device::DestroyPipelineLayout(PipelineLayout* handle)
    // {
    //     return IMPL::DestroyPipelineLayout(...);
    // }

    // GraphicsPipeline* Device::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateGraphicsPipeline(...);
    // }

    // void Device::DestroyGraphicsPipeline(GraphicsPipeline* handle)
    // {
    //     return IMPL::DestroyGraphicsPipeline(...);
    // }

    // ComputePipeline* Device::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateComputePipeline(...);
    // }

    // void Device::DestroyComputePipeline(ComputePipeline* handle)
    // {
    //     return IMPL::DestroyComputePipeline(...);
    // }

    // RayTracingPipeline* Device::CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateRayTracingPipeline(...);
    // }

    // void Device::DestroyRayTracingPipeline(RayTracingPipeline* handle)
    // {
    //     return IMPL::DestroyRayTracingPipeline(...);
    // }

    // void Device::GetShaderBindingTableEntry(RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle)
    // {
    //     return IMPL::GetShaderBindingTableEntry(...);
    // }

    // Buffer* Device::CreateBuffer(const BufferCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateBuffer(...);
    // }

    // void Device::DestroyBuffer(Buffer* handle)
    // {
    //     return IMPL::DestroyBuffer(...);
    // }

    // uint64_t Device::GetBufferDeviceAddress(Buffer* buffer)
    // {
    //     return IMPL::GetBufferDeviceAddress(...);
    // }

    // DeviceMemoryPtr Device::MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    // {
    //     return IMPL::MapBuffer(...);
    // }

    // void Device::UnmapBuffer(Buffer* buffer)
    // {
    //     return IMPL::UnmapBuffer(...);
    // }

    // Image* Device::CreateImage(const ImageCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateImage(...);
    // }

    // Image* Device::CreateImageView(const ImageViewCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateImageView(...);
    // }

    // void Device::DestroyImage(Image* handle)
    // {
    //     return IMPL::DestroyImage(...);
    // }

    // Sampler* Device::CreateSampler(const SamplerCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateSampler(...);
    // }

    // void Device::DestroySampler(Sampler* handle)
    // {
    //     return IMPL::DestroySampler(...);
    // }

    // AccelerationStructure* Device::CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateAccelerationStructure(...);
    // }

    // void Device::DestroyAccelerationStructure(AccelerationStructure* handle)
    // {
    //     return IMPL::DestroyAccelerationStructure(...);
    // }

    // uint64_t Device::GetAccelerationStructureDeviceAddress(AccelerationStructure* handle)
    // {
    //     return IMPL::GetAccelerationStructureDeviceAddress(...);
    // }

    // AccelerationStructureSizesInfo Device::GetAccelerationStructureSizesInfo(AccelerationStructure* as)
    // {
    //     return IMPL::GetAccelerationStructureSizesInfo(...);
    // }

    // Micromap* Device::CreateMicromap(const MicromapCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateMicromap(...);
    // }

    // void Device::DestroyMicromap(Micromap* handle)
    // {
    //     return IMPL::DestroyMicromap(...);
    // }

    // CommandPool* Device::CreateCommandPool(const CommandPoolCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateCommandPool(...);
    // }

    // void Device::DestroyCommandPool(CommandPool* handle)
    // {
    //     return IMPL::DestroyCommandPool(...);
    // }

    // Fence* Device::CreateFence(const FenceCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateFence(...);
    // }

    // void Device::DestroyFence(Fence* handle)
    // {
    //     return IMPL::DestroyFence(...);
    // }

    // uint64_t Device::GetFenceValue(Fence* handle)
    // {
    //     return IMPL::GetFenceValue(...);
    // }

    // QueryPool* Device::CreateQueryPool(const QueryPoolCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateQueryPool(...);
    // }

    // void Device::DestroyQueryPool(QueryPool* handle)
    // {
    //     return IMPL::DestroyQueryPool(...);
    // }

    // Swapchain* Device::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    // {
    //     return IMPL:: CreateSwapchain(...);
    // }

    // void Device::DestroySwapchain(Swapchain* swapchain)
    // {
    //     return IMPL::DestroySwapchain(...);
    // }

    // uint32_t Device::GetSwapchainImagesCount(Swapchain* swapchain)
    // {
    //     return IMPL::GetSwapchainImagesCount(...);
    // }

    // SwapchainAcquireResult Device::AcquireSwapchainImage(Swapchain* swapchain)
    // {
    //     return IMPL::AcquireSwapchainImage(...);
    // }

    // SurfaceCapabilities Device::GetSwapchainSurfaceCapabilities(Swapchain* swapchain)
    // {
    //     return IMPL::GetSwapchainSurfaceCapabilities(...);
    // }

    // ResultCode Device::ResizeSwapchain(Swapchain* swapchain, const ImageSize2D& size)
    // {
    //     return IMPL::ResizeSwapchain(...);
    // }

    // ResultCode Device::ConfigureSwapchain(Swapchain* swapchain, const SwapchainConfigureInfo& configInfo)
    // {
    //     return IMPL::ConfigureSwapchain(...);
    // }

    // void Device::Reset()
    // {
    //     return IMPL::Reset(...);
    // }

    // CommandList* Device::Allocate()
    // {
    //     return IMPL:: Allocate(...);
    // }

    // void CommandList::Begin()
    // {
    //     return IMPL::Begin(...);
    // }

    // void CommandList::End()
    // {
    //     return IMPL::End(...);
    // }

    // void CommandList::PushDebugMarker(const char* name, uint32_t bgra)
    // {
    //     return IMPL::PushDebugMarker(...);
    // }

    // void CommandList::PopDebugMarker()
    // {
    //     return IMPL::PopDebugMarker(...);
    // }

    // void CommandList::InsertDebugMarker(const char* name, uint32_t bgra)
    // {
    //     return IMPL::InsertDebugMarker(...);
    // }

    // void CommandList::AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    // {
    //     return IMPL::AddPipelineBarrier(...);
    // }

    // void CommandList::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
    // {
    //     return IMPL::BeginRenderPass(...);
    // }

    // void CommandList::EndRenderPass()
    // {
    //     return IMPL::EndRenderPass(...);
    // }

    // void CommandList::BeginComputePass(const ComputePassBeginInfo& beginInfo)
    // {
    //     return IMPL::BeginComputePass(...);
    // }

    // void CommandList::EndComputePass()
    // {
    //     return IMPL::EndComputePass(...);
    // }

    // void CommandList::BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted)
    // {
    //     return IMPL::BeginConditionalCommands(...);
    // }

    // void CommandList::EndConditionalCommands()
    // {
    //     return IMPL::EndConditionalCommands(...);
    // }

    // void CommandList::Execute(TL::Span<const CommandList*> commandLists)
    // {
    //     return IMPL::Execute(...);
    // }

    // void CommandList::BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    // {
    //     return IMPL::BindPipelineLayout(...);
    // }

    // void CommandList::SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content)
    // {
    //     return IMPL::SetPushConstants(...);
    // }

    // void CommandList::PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    // {
    //     return IMPL::PushBindGroup(...);
    // }

    // void CommandList::SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    // {
    //     return IMPL::SetBindGroups(...);
    // }

    // void CommandList::BindGraphicsPipeline(const GraphicsPipeline* pipelineState)
    // {
    //     return IMPL::BindGraphicsPipeline(...);
    // }

    // void CommandList::BindComputePipeline(const ComputePipeline* pipelineState)
    // {
    //     return IMPL::BindComputePipeline(...);
    // }

    // void CommandList::BindRayTracingPipeline(const RayTracingPipeline* pipelineState)
    // {
    //     return IMPL::BindRayTracingPipeline(...);
    // }

    // void CommandList::SetViewport(float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth)
    // {
    //     return IMPL::SetViewport(...);
    // }

    // void CommandList::SetScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height)
    // {
    //     return IMPL::SetScissor(...);
    // }

    // void CommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    // {
    //     return IMPL::BindVertexBuffers(...);
    // }

    // void CommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    // {
    //     return IMPL::BindIndexBuffer(...);
    // }

    // void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0)
    // {
    //     return IMPL::Draw(...);
    // }

    // void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0)
    // {
    //     return IMPL::DrawIndexed(...);
    // }

    // void CommandList::DrawMeshTasks(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1)
    // {
    //     return IMPL::DrawMeshTasks(...);
    // }

    // void CommandList::DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    // {
    //     return IMPL::DrawIndirect(...);
    // }

    // void CommandList::DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    // {
    //     return IMPL::DrawIndexedIndirect(...);
    // }

    // void CommandList::DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    // {
    //     return IMPL::DrawMeshTasksIndirect(...);
    // }

    // void CommandList::Dispatch(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1)
    // {
    //     return IMPL::Dispatch(...);
    // }

    // void CommandList::DispatchIndirect(const BufferBindingInfo& argumentBuffer)
    // {
    //     return IMPL::DispatchIndirect(...);
    // }

    // void CommandList::DispatchRays(const DispatchRaysInfo& dispatchRaysDesc)
    // {
    //     return IMPL::DispatchRays(...);
    // }

    // void CommandList::DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer)
    // {
    //     return IMPL::DispatchRaysIndirect(...);
    // }

    // void CommandList::CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size)
    // {
    //     return IMPL::CopyBuffer(...);
    // }

    // void CommandList::CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)
    // {
    //     return IMPL::CopyImage(...);
    // }

    // void CommandList::CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)
    // {
    //     return IMPL::CopyImageToBuffer(...);
    // }

    // void CommandList::CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)
    // {
    //     return IMPL::CopyBufferToImage(...);
    // }

    // void CommandList::CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    // {
    //     return IMPL::CopyAccelerationStructure(...);
    // }

    // void CommandList::CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode)
    // {
    //     return IMPL::CopyMicromap(...);
    // }

    // void CommandList::BuildTlas(TL::Span<const TlasBuildInfo> buildInfos)
    // {
    //     return IMPL::BuildTlas(...);
    // }

    // void CommandList::BuildBlas(TL::Span<const BlasBuildInfo> buildInfos)
    // {
    //     return IMPL::BuildBlas(...);
    // }

    // void CommandList::BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos)
    // {
    //     return IMPL::BuildMicromaps(...);
    // }

    // void CommandList::WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset)
    // {
    //     return IMPL::WriteAccelerationStructuresSizes(...);
    // }

    // void CommandList::WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)
    // {
    //     return IMPL::WriteMicromapsSizes(...);
    // }
} // namespace RHI