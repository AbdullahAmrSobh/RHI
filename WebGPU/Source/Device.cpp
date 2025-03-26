#include "Device.hpp"

#include <RHI-WebGPU/Loader.hpp>
#include <webgpu/webgpu.h>

#include "CommandList.hpp"
#include "Common.hpp"
#include "Resources.hpp"
#include "RenderGraph.hpp"
#include "Swapchain.hpp"

#include <tracy/Tracy.hpp>

RHI::Device* RHI::CreateWebGPUDevice()
{
    auto device = TL::Allocator::Construct<RHI::WebGPU::IDevice>();
    auto result = device->Init();
    if (IsSuccess(result))
    {
        return device;
    }
    return nullptr;
}

void RHI::DestroyWebGPUDevice(RHI::Device* _device)
{
    auto device = (RHI::WebGPU::IDevice*)_device;
    device->Shutdown();
    TL::Allocator::Destruct(device);
}

namespace RHI::WebGPU
{
    IDevice::IDevice()  = default;
    IDevice::~IDevice() = default;

    ResultCode IDevice::Init()
    {
        WGPUInstanceDescriptor instanceDesc = {
            .nextInChain  = nullptr,
            .capabilities = {
                             .nextInChain          = nullptr,
                             .timedWaitAnyEnable   = false,
                             .timedWaitAnyMaxCount = 0,
                             },
        };
        m_instance = wgpuCreateInstance(&instanceDesc);

        WGPURequestAdapterOptions options{
            .nextInChain          = nullptr,
            .compatibleSurface    = {},
            .featureLevel         = WGPUFeatureLevel_Core,
            .powerPreference      = WGPUPowerPreference_HighPerformance,
            .backendType          = WGPUBackendType_D3D12,
            .forceFallbackAdapter = false,
        };
        WGPURequestAdapterCallbackInfo cb{
            .nextInChain = nullptr,
            .mode        = WGPUCallbackMode_WaitAnyOnly,
            .callback    = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, struct WGPUStringView message, void* userdata1, void* userdata2)
            {
                *(WGPUAdapter*)userdata1 = adapter;
            },
            .userdata1 = &m_adapter,
            .userdata2 = nullptr,
        };
        auto               future = wgpuInstanceRequestAdapter(m_instance, &options, cb);
        WGPUFutureWaitInfo waitInfo{
            .future    = future,
            .completed = true,
        };
        auto result = wgpuInstanceWaitAny(m_instance, 1, &waitInfo, 0);
        TL_ASSERT(result == WGPUWaitStatus_Success);

        WGPUDeviceDescriptor deviceDesc{
            .nextInChain                 = nullptr,
            .label                       = ConvertToStringView("RHI::WebGPU::Device"),
            .requiredFeatureCount        = 0,
            .requiredFeatures            = {},
            .requiredLimits              = {},
            .defaultQueue                = {},
            .deviceLostCallbackInfo      = {},
            .uncapturedErrorCallbackInfo = {},
        };
        m_device = wgpuAdapterCreateDevice(m_adapter, &deviceDesc);

        return ResultCode::Success;
    }

    void IDevice::Shutdown()
    {
        // m_queue
        // m_device
        // m_adapter
        wgpuDeviceRelease(m_device);
        wgpuAdapterRelease(m_adapter);
        wgpuInstanceRelease(m_instance);
    }

    void IDevice::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo)
    {
    }

    DeviceMemoryPtr IDevice::MapBuffer(Handle<Buffer> handle)
    {
        auto buffer = m_bufferOwner.Get(handle);

        DeviceMemoryPtr ptr;
        ptr = wgpuBufferGetMappedRange(buffer->buffer, 0, 0xffffffff);

        // vs
        // WGPUBufferMapCallbackInfo cb{
        //     .nextInChain = nullptr,
        //     .mode        = WGPUCallbackMode_WaitAnyOnly,
        //     .callback    = [](WGPUMapAsyncStatus status, struct WGPUStringView message, void* userdata1, void* userdata2) {

        //     },
        //     .userdata1   = &ptr,
        //     .userdata2   = {},
        // };
        // auto future = wgpuBufferMapAsync(m_bufferOwner, WGPUMapMode_Read | WGPUMapMode_Write, 0, 0xffffffff, cb);
        return ptr;
    }

    void IDevice::UnmapBuffer(Handle<Buffer> handle)
    {
        auto buffer = m_bufferOwner.Get(handle);
        wgpuBufferUnmap(buffer->buffer);
    }

    StagingBuffer IDevice::StagingAllocate(size_t size)
    {
        return {};
    }

    uint64_t IDevice::UploadImage(const ImageUploadInfo& uploadInfo)
    {
        return {};

        TL::Span<uint8_t> data;

        auto queue = m_queue[(uint32_t)QueueType::Transfer];

        WGPUTexelCopyTextureInfo  copyInfo{};
        WGPUTexelCopyBufferLayout bufferLayout{};
        WGPUExtent3D              writeSize{};
        wgpuQueueWriteTexture(queue, &copyInfo, data.data(), data.size(), &bufferLayout, &writeSize);
    }

    void IDevice::CollectResources()
    {
        // Should be no op
    }

    bool IDevice::WaitForQueueTimelineValue(QueueType queueType, uint64_t value, uint64_t waitDuration)
    {
        // Should be no op too?
        return true;
    }

    void IDevice::ExecuteCommandList(ICommandList* commandList)
    {
        auto queue = m_queue[(int)QueueType::Graphics];
        wgpuQueueSubmit(queue, 1, &commandList->m_cmdBuffer);
    }

    #define IMPLEMENT_DEVICE_CREATE_METHOD_UNIQUE_WITH_INFO(ResourceType)                       \
    ResourceType* IDevice::Create##ResourceType(const ResourceType##CreateInfo& createInfo) \
    {                                                                                       \
        ZoneScoped;                                                                         \
        auto handle = new I##ResourceType();                                                \
        auto result = handle->Init(this, createInfo);                                       \
        TL_ASSERT(IsSuccess(result));                                                       \
        return handle;                                                                      \
    }

#define IMPLEMENT_DEVICE_DESTROY_METHOD_UNIQUE(ResourceType)   \
    void IDevice::Destroy##ResourceType(ResourceType* _handle) \
    {                                                          \
        ZoneScoped;                                            \
        auto handle = (I##ResourceType*)_handle;               \
        handle->Shutdown();                                    \
        delete handle;                                         \
    }

#define IMPLEMENT_DEVICE_CREATE_METHOD(HandleType, OwnerField)                               \
    Handle<HandleType> IDevice::Create##HandleType(const HandleType##CreateInfo& createInfo) \
    {                                                                                        \
        ZoneScoped;                                                                          \
        auto [handle, result] = OwnerField.Create(this, createInfo);                         \
        TL_ASSERT(IsSuccess(result));                                                        \
        return handle;                                                                       \
    }

#define IMPLEMENT_DEVICE_CREATE_METHOD_WITH_RESULT(HandleType, OwnerField)                           \
    Result<Handle<HandleType>> IDevice::Create##HandleType(const HandleType##CreateInfo& createInfo) \
    {                                                                                                \
        ZoneScoped;                                                                                  \
        auto [handle, result] = OwnerField.Create(this, createInfo);                                 \
        if (IsSuccess(result)) return (Handle<HandleType>)handle;                                    \
        return result;                                                                               \
    }

#define IMPLEMENT_DEVICE_DESTROY_METHOD(HandleType, OwnerField)                \
    void IDevice::Destroy##HandleType(Handle<HandleType> handle)               \
    {                                                                          \
        ZoneScoped;                                                            \
        TL_ASSERT(handle != NullHandle, "Cannot call destroy on null handle"); \
        OwnerField.Destroy(handle, this);                            \
    }

#define IMPLEMENT_DEVICE_RESOURCE_METHODS(ResourceType) \
    IMPLEMENT_DEVICE_CREATE_METHOD_UNIQUE(ResourceType) \
    IMPLEMENT_DEVICE_DESTROY_METHOD_UNIQUE(ResourceType)

#define IMPLEMENT_DISPATCHABLE_TYPES_FUNCTIONS(ResourceType)      \
    IMPLEMENT_DEVICE_CREATE_METHOD_UNIQUE_WITH_INFO(ResourceType) \
    IMPLEMENT_DEVICE_DESTROY_METHOD_UNIQUE(ResourceType)

#define IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS(HandleType, OwnerField) \
    IMPLEMENT_DEVICE_CREATE_METHOD(HandleType, OwnerField)                \
    IMPLEMENT_DEVICE_DESTROY_METHOD(HandleType, OwnerField)

#define IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS_WITH_RESULTS(HandleType, OwnerField) \
    IMPLEMENT_DEVICE_CREATE_METHOD_WITH_RESULT(HandleType, OwnerField)                 \
    IMPLEMENT_DEVICE_DESTROY_METHOD(HandleType, OwnerField)

    IMPLEMENT_DISPATCHABLE_TYPES_FUNCTIONS(RenderGraph);
    IMPLEMENT_DISPATCHABLE_TYPES_FUNCTIONS(Swapchain);
    IMPLEMENT_DISPATCHABLE_TYPES_FUNCTIONS(ShaderModule);
    IMPLEMENT_DISPATCHABLE_TYPES_FUNCTIONS(CommandList);
    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS(BindGroupLayout, m_bindGroupLayoutsOwner);
    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS(BindGroup, m_bindGroupOwner);
    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS(PipelineLayout, m_pipelineLayoutOwner);
    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS(GraphicsPipeline, m_graphicsPipelineOwner);
    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS(ComputePipeline, m_computePipelineOwner);
    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS(Sampler, m_samplerOwner);
    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS_WITH_RESULTS(Image, m_imageOwner);
    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS_WITH_RESULTS(Buffer, m_bufferOwner);
} // namespace RHI::WebGPU
