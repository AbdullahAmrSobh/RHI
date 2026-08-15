#include "RHI/RHI.h"

#if RHI_COMPILE_BACKEND_VULKAN
    #include "backends/vulkan/device.h"
namespace Impl = RHI::Vulkan;
#elif RHI_COMPILE_BACKEND_D3D12
    #include <backends/D3D12/device.h>
namespace Impl = RHI::D3D12;
#elif RHI_COMPILE_BACKEND_WEBGPU
    #include "backends/webgpu/device.h"
namespace Impl = RHI::WebGPU;
#else
    #error "No RHI backend selected"
#endif

#include <cstring>

#include <tracy/Tracy.hpp>

namespace RHI
{
    inline static constexpr uint32_t TracyZoneColor = 0xFF1493;

    // clang-format off
    // Copied from nvrhi
    // Format mapping table. The rows must be in the exactly same order as Format enum members are defined.
    static const FormatInfo k_FormatInfoLUT[] =
    {
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
        ZoneScopedC(TracyZoneColor);
        return GetFormatInfo(format).bytesPerBlock;
    }

    void Queue::BeginAnnotation(const char* name, uint32_t bgra)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::queueBeginAnnotation((::Impl::IQueue*)this, name, bgra);
    }

    void Queue::EndAnnotation()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::queueEndAnnotation((::Impl::IQueue*)this);
    }

    void Queue::InsertAnnotation(const char* name, uint32_t bgra)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::queueInsertAnnotation((::Impl::IQueue*)this, name, bgra);
    }

    void Queue::Submit(const QueueSubmitInfo& submitInfo)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::queueSubmit((::Impl::IQueue*)this, submitInfo);
    }

    void Queue::WaitIdle()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::queueWaitIdle((::Impl::IQueue*)this);
    }

    void Queue::WaitFence(Fence* fence, uint64_t value)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::queueWaitFence((::Impl::IQueue*)this, fence, value);
    }

    Device* CreateDevice(BackendType /*backend*/, const ApplicationInfo& appInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createDevice(appInfo);
    }

    void DestroyDevice(Device* device)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyDevice(device);
    }

    BackendType Device::GetBackend() const
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::deviceGetBackend((::Impl::IDevice*)this);
    }

    DeviceFeatures Device::GetFeatures() const
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::deviceGetFeatures((::Impl::IDevice*)this);
    }

    DeviceLimits Device::GetLimits() const
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::deviceGetLimits((::Impl::IDevice*)this);
    }

    uint64_t Device::GarbageCollect()
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::deviceGarbageCollect((::Impl::IDevice*)this, 0);
    }

    uint64_t Device::GarbageCollect(uint64_t graphicsTimeline)
    {
        (void)graphicsTimeline;
        return GarbageCollect();
    }

    uint64_t Device::GetNativeHandle(NativeHandleType type, uint64_t handle)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::deviceGetNativeHandle((::Impl::IDevice*)this, type, handle);
    }

    void Device::WaitIdle()
    {
        ZoneScopedC(TracyZoneColor);
        ((::Impl::IDevice*)this)->WaitIdle();
    }

    Queue* Device::GetQueue(QueueType queueType)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::deviceGetQueue((::Impl::IDevice*)this, queueType);
    }

    ShaderModule* Device::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createShaderModule((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyShaderModule(ShaderModule* shaderModule)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyShaderModule((::Impl::IDevice*)this, shaderModule);
    }

    BindGroupLayout* Device::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createBindGroupLayout((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyBindGroupLayout(BindGroupLayout* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyBindGroupLayout((::Impl::IDevice*)this, handle);
    }

    BindGroup* Device::CreateBindGroup(const BindGroupCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createBindGroup((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyBindGroup(BindGroup* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyBindGroup((::Impl::IDevice*)this, handle);
    }

    void Device::UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::bindGroupUpdate((::Impl::IDevice*)this, handle, updateInfo);
    }

    PipelineLayout* Device::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createPipelineLayout((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyPipelineLayout(PipelineLayout* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyPipelineLayout((::Impl::IDevice*)this, handle);
    }

    GraphicsPipeline* Device::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createGraphicsPipeline((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyGraphicsPipeline(GraphicsPipeline* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyGraphicsPipeline((::Impl::IDevice*)this, handle);
    }

    ComputePipeline* Device::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createComputePipeline((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyComputePipeline(ComputePipeline* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyComputePipeline((::Impl::IDevice*)this, handle);
    }

    RayTracingPipeline* Device::CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createRayTracingPipeline((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyRayTracingPipeline(RayTracingPipeline* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyRayTracingPipeline((::Impl::IDevice*)this, handle);
    }

    void Device::GetShaderBindingTableEntry(RayTracingPipeline* handle, uint32_t group, size_t size, void* dstHandle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::rayTracingPipelineGetShaderBindingTableEntry((::Impl::IDevice*)this, handle, group, size, dstHandle);
    }

    Buffer* Device::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createBuffer((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyBuffer(Buffer* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyBuffer((::Impl::IDevice*)this, handle);
    }

    uint64_t Device::GetBufferDeviceAddress(Buffer* buffer)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::bufferGetDeviceAddress((::Impl::IDevice*)this, buffer);
    }

    DeviceMemoryPtr Device::MapBuffer(Buffer* buffer, uint64_t offset, uint64_t sizeBytes)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::bufferMap((::Impl::IDevice*)this, buffer, offset, sizeBytes);
    }

    void Device::UnmapBuffer(Buffer* buffer)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::bufferUnmap((::Impl::IDevice*)this, buffer);
    }

    void Device::mappedBufferReadMappedRanges(DeviceMemoryPtr* dst, void* src, size_t size)
    {
        ZoneScopedC(TracyZoneColor);
        if (dst && *dst && src && size)
            std::memcpy(*dst, src, size);
    }

    void Device::mappedBufferWriteMappedRanges(void* dst, const DeviceMemoryPtr* src, size_t size)
    {
        ZoneScopedC(TracyZoneColor);
        if (dst && src && *src && size)
            std::memcpy(dst, *src, size);
    }

    Image* Device::CreateImage(const ImageCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createImage((::Impl::IDevice*)this, createInfo);
    }

    Image* Device::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createImageView((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyImage(Image* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyImage((::Impl::IDevice*)this, handle);
    }

    Sampler* Device::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createSampler((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroySampler(Sampler* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroySampler((::Impl::IDevice*)this, handle);
    }

    AccelerationStructure* Device::CreateAccelerationStructure(const AccelerationStructureCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createAccelerationStructure((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyAccelerationStructure(AccelerationStructure* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyAccelerationStructure((::Impl::IDevice*)this, handle);
    }

    uint64_t Device::GetAccelerationStructureDeviceAddress(AccelerationStructure* handle)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::accelerationStructureGetDeviceAddress((::Impl::IDevice*)this, handle);
    }

    AccelerationStructureSizesInfo Device::GetAccelerationStructureSizesInfo(AccelerationStructure* as)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::accelerationStructureGetSizesInfo((::Impl::IDevice*)this, as);
    }

    Micromap* Device::CreateMicromap(const MicromapCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createMicromap((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyMicromap(Micromap* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyMicromap((::Impl::IDevice*)this, handle);
    }

    CommandPool* Device::CreateCommandPool(const CommandPoolCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createCommandPool((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyCommandPool(CommandPool* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyCommandPool((::Impl::IDevice*)this, handle);
    }

    Fence* Device::CreateFence(const FenceCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createFence((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyFence(Fence* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyFence((::Impl::IDevice*)this, handle);
    }

    uint64_t Device::GetFenceValue(Fence* handle)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::fenceGetValue((::Impl::IDevice*)this, handle);
    }

    QueryPool* Device::CreateQueryPool(const QueryPoolCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createQueryPool((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroyQueryPool(QueryPool* handle)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroyQueryPool((::Impl::IDevice*)this, handle);
    }

    Swapchain* Device::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::createSwapchain((::Impl::IDevice*)this, createInfo);
    }

    void Device::DestroySwapchain(Swapchain* swapchain)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::destroySwapchain((::Impl::IDevice*)this, swapchain);
    }

    uint32_t Device::GetSwapchainImagesCount(Swapchain* swapchain)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::swapchainGetImagesCount((::Impl::IDevice*)this, swapchain);
    }

    SwapchainAcquireResult Device::AcquireSwapchainImage(Swapchain* swapchain)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::swapchainAcquireImage((::Impl::IDevice*)this, swapchain);
    }

    SurfaceCapabilities Device::GetSwapchainSurfaceCapabilities(Swapchain* swapchain)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::swapchainGetSurfaceCapabilities((::Impl::IDevice*)this, swapchain);
    }

    ResultCode Device::ResizeSwapchain(Swapchain* swapchain, const ImageSize2D& size)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::swapchainResize((::Impl::IDevice*)this, swapchain, size);
    }

    ResultCode Device::ConfigureSwapchain(Swapchain* swapchain, const SwapchainConfigureInfo& configInfo)
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::swapchainConfigure((::Impl::IDevice*)this, swapchain, configInfo);
    }

    void CommandPool::Reset()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::commandPoolReset((::Impl::ICommandPool*)this);
    }

    CommandList* CommandPool::Allocate()
    {
        ZoneScopedC(TracyZoneColor);
        return ::Impl::commandPoolAllocate((::Impl::ICommandPool*)this);
    }

    void CommandList::Begin()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBegin((::Impl::ICommandList*)this);
    }

    void CommandList::End()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdEnd((::Impl::ICommandList*)this);
    }

    void CommandList::PushDebugMarker(const char* name, uint32_t bgra)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdPushDebugMarker((::Impl::ICommandList*)this, name, bgra);
    }

    void CommandList::PopDebugMarker()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdPopDebugMarker((::Impl::ICommandList*)this);
    }

    void CommandList::InsertDebugMarker(const char* name, uint32_t bgra)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdInsertDebugMarker((::Impl::ICommandList*)this, name, bgra);
    }

    void CommandList::AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdAddPipelineBarrier((::Impl::ICommandList*)this, barriers, imageBarriers, bufferBarriers);
    }

    void CommandList::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBeginRenderPass((::Impl::ICommandList*)this, beginInfo);
    }

    void CommandList::EndRenderPass()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdEndRenderPass((::Impl::ICommandList*)this);
    }

    void CommandList::BeginComputePass(const ComputePassBeginInfo& beginInfo)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBeginComputePass((::Impl::ICommandList*)this, beginInfo);
    }

    void CommandList::EndComputePass()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdEndComputePass((::Impl::ICommandList*)this);
    }

    void CommandList::BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBeginConditionalCommands((::Impl::ICommandList*)this, conditionBuffer, inverted);
    }

    void CommandList::EndConditionalCommands()
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdEndConditionalCommands((::Impl::ICommandList*)this);
    }

    void CommandList::Execute(TL::Span<const CommandList*> commandLists)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdExecute((::Impl::ICommandList*)this, commandLists);
    }

    void CommandList::BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBindPipelineLayout((::Impl::ICommandList*)this, bindPoint, pipelineLayout);
    }

    void CommandList::SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdSetPushConstants((::Impl::ICommandList*)this, bindPoint, offset, content);
    }

    void CommandList::PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdPushBindGroup((::Impl::ICommandList*)this, bindPoint, firstGroup, updateInfos);
    }

    void CommandList::SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdSetBindGroups((::Impl::ICommandList*)this, bindPoint, bindGroups);
    }

    void CommandList::BindGraphicsPipeline(const GraphicsPipeline* pipelineState)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBindGraphicsPipeline((::Impl::ICommandList*)this, pipelineState);
    }

    void CommandList::BindComputePipeline(const ComputePipeline* pipelineState)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBindComputePipeline((::Impl::ICommandList*)this, pipelineState);
    }

    void CommandList::BindRayTracingPipeline(const RayTracingPipeline* pipelineState)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBindRayTracingPipeline((::Impl::ICommandList*)this, pipelineState);
    }

    void CommandList::SetViewport(float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdSetViewport((::Impl::ICommandList*)this, offsetX, offsetY, width, height, minDepth, maxDepth);
    }

    void CommandList::SetScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdSetScissor((::Impl::ICommandList*)this, offsetX, offsetY, width, height);
    }

    void CommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBindVertexBuffers((::Impl::ICommandList*)this, firstBinding, vertexBuffers);
    }

    void CommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBindIndexBuffer((::Impl::ICommandList*)this, indexBuffer, indexType);
    }

    void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDraw((::Impl::ICommandList*)this, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDrawIndexed((::Impl::ICommandList*)this, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void CommandList::DrawMeshTasks(uint32_t x, uint32_t y, uint32_t z)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDrawMeshTasks((::Impl::ICommandList*)this, x, y, z);
    }

    void CommandList::DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDrawIndirect((::Impl::ICommandList*)this, argumentBuffer, countBuffer, maxDrawCount, stride);
    }

    void CommandList::DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDrawIndexedIndirect((::Impl::ICommandList*)this, argumentBuffer, countBuffer, maxDrawCount, stride);
    }

    void CommandList::DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDrawMeshTasksIndirect((::Impl::ICommandList*)this, argumentBuffer, countBuffer, drawNum, stride);
    }

    void CommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDispatch((::Impl::ICommandList*)this, x, y, z);
    }

    void CommandList::DispatchIndirect(const BufferBindingInfo& argumentBuffer)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDispatchIndirect((::Impl::ICommandList*)this, argumentBuffer);
    }

    void CommandList::DispatchRays(const DispatchRaysInfo& dispatchRaysDesc)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDispatchRays((::Impl::ICommandList*)this, dispatchRaysDesc);
    }

    void CommandList::DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdDispatchRaysIndirect((::Impl::ICommandList*)this, argumentBuffer);
    }

    void CommandList::CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdCopyBuffer((::Impl::ICommandList*)this, srcBuffer, srcOffset, dstBuffer, dstOffset, size);
    }

    void CommandList::CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdCopyImage((::Impl::ICommandList*)this, srcImage, dstImage, size);
    }

    void CommandList::CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdCopyImageToBuffer((::Impl::ICommandList*)this, srcImage, layout, dstBuffer);
    }

    void CommandList::CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdCopyBufferToImage((::Impl::ICommandList*)this, srcBuffer, dstImage, layout);
    }

    void CommandList::CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdCopyAccelerationStructure((::Impl::ICommandList*)this, dst, src, copyMode);
    }

    void CommandList::CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdCopyMicromap((::Impl::ICommandList*)this, dst, src, copyMode);
    }

    void CommandList::ClearBuffer(Buffer* dst, size_t offset, size_t size)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdClearBuffer((::Impl::ICommandList*)this, dst, offset, size);
    }

    void CommandList::BuildTlas(TL::Span<const TlasBuildInfo> buildInfos)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBuildTlas((::Impl::ICommandList*)this, buildInfos);
    }

    void CommandList::BuildBlas(TL::Span<const BlasBuildInfo> buildInfos)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBuildBlas((::Impl::ICommandList*)this, buildInfos);
    }

    void CommandList::BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdBuildMicromaps((::Impl::ICommandList*)this, buildInfos);
    }

    void CommandList::WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdWriteAccelerationStructuresSizes((::Impl::ICommandList*)this, accelerationStructures, queryPool, queryPoolOffset);
    }

    void CommandList::WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
        ZoneScopedC(TracyZoneColor);
        ::Impl::cmdWriteMicromapsSizes((::Impl::ICommandList*)this, micromaps, queryPool, queryPoolOffset);
    }

} // namespace RHI
