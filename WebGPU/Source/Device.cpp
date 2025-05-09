#include "Device.hpp"

#include "RHI/Resources.hpp"

#include <RHI-WebGPU/Loader.hpp>
#include <tracy/Tracy.hpp>
#include <webgpu/webgpu.h>

#include "CommandList.hpp"
#include "Common.hpp"
#include "RenderGraph.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"

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
        m_backend = BackendType::WebGPU;

        WGPUInstanceDescriptor instanceDesc = {
            .nextInChain  = nullptr,
            .capabilities = {
                             .nextInChain          = nullptr,
                             .timedWaitAnyEnable   = false,
                             .timedWaitAnyMaxCount = 0,
                             },
        };
        m_instance = wgpuCreateInstance(&instanceDesc);

        auto requestAdapterCallback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, struct WGPUStringView message, void* userdata1, void* userdata2)
        {
            switch (status)
            {
            case WGPURequestAdapterStatus_Success:
                *(WGPUAdapter*)userdata1 = adapter;
                break;
            case WGPURequestAdapterStatus_InstanceDropped:
            case WGPURequestAdapterStatus_Unavailable:
            case WGPURequestAdapterStatus_Error:
                TL_LOG_INFO("RHI::WebGPU: {}", message.data);
            case WGPURequestAdapterStatus_Force32:
                break;
            }
        };
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
            .callback    = requestAdapterCallback,
            .userdata1   = &m_adapter,
            .userdata2   = nullptr,
        };
        auto               future = wgpuInstanceRequestAdapter(m_instance, &options, cb);
        WGPUFutureWaitInfo waitInfo{
            .future    = future,
            .completed = true,
        };
        auto result = wgpuInstanceWaitAny(m_instance, 1, &waitInfo, 0);
        TL_ASSERT(result == WGPUWaitStatus_Success);

        // auto buffermapcallback = [](WGPUMapAsyncStatus status, struct WGPUStringView message, void* userdata1, void* userdata2)
        // {
        // };

        // auto compilationInfoCallback = [](WGPUCompilationInfoRequestStatus status, struct WGPUCompilationInfo const* compilationInfo, void* userdata1, void* userdata2)
        // {
        // };

        // auto createComputePipelineAsyncCallback = [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline pipeline, struct WGPUStringView message, void* userdata1, void* userdata2)
        // {
        // };

        // auto createRenderPipelineAsyncCallback = [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline pipeline, struct WGPUStringView message, void* userdata1, void* userdata2)
        // {
        // };

        auto deviceLostCallback = [](WGPUDevice const* device, WGPUDeviceLostReason reason, struct WGPUStringView message, void* userdata1, void* userdata2)
        {
            TL_LOG_ERROR("RHI::WebGPU Device lost. Reported message:  {}", message.data);
            TL_DEBUG_BREAK();
        };

        // auto popErrorScopeCallback = [](WGPUPopErrorScopeStatus status, WGPUErrorType type, struct WGPUStringView message, void* userdata1, void* userdata2)
        // {
        // };

        // auto queueWorkDoneCallback = [](WGPUQueueWorkDoneStatus status, void* userdata1, void* userdata2)
        // {
        // };

        // auto requestDeviceCallback = [](WGPURequestDeviceStatus status, WGPUDevice device, struct WGPUStringView message, void* userdata1, void* userdata2)
        // {
        // };

        auto uncapturedErrorCallback = [](WGPUDevice const* device, WGPUErrorType type, struct WGPUStringView message, void* userdata1, void* userdata2)
        {
            TL_LOG_ERROR("RHI::WebGPU Uncaptured error. Reported message: {}", message.data);
        };

        TL::Vector<WGPUFeatureName> enabledFeatures{};

        WGPULimits requiredLimits{WGPU_LIMITS_INIT};
        requiredLimits.maxColorAttachmentBytesPerSample = 128ul;

        WGPUDeviceDescriptor deviceDesc{
            .nextInChain          = nullptr,
            .label                = ConvertToStringView("RHI::WebGPU::Device"),
            .requiredFeatureCount = enabledFeatures.size(),
            .requiredFeatures     = enabledFeatures.data(),
            .requiredLimits       = &requiredLimits,
            .defaultQueue         = {
                                     .nextInChain = nullptr,
                                     .label       = {},
                                     },
            .deviceLostCallbackInfo = {
                                     .nextInChain = nullptr,
                                     .mode        = WGPUCallbackMode_AllowProcessEvents,
                                     .callback    = deviceLostCallback,
                                     .userdata1   = this,
                                     .userdata2   = nullptr,
                                     },
            .uncapturedErrorCallbackInfo = {
                                     .nextInChain = nullptr,
                                     .callback    = uncapturedErrorCallback,
                                     .userdata1   = this,
                                     .userdata2   = nullptr,
                                     },
        };
        m_device = wgpuAdapterCreateDevice(m_adapter, &deviceDesc);

        auto loggingCallback = [](WGPULoggingType type, struct WGPUStringView message, void* userdata1, void* userdata2)
        {
            switch (type)
            {
            case WGPULoggingType_Verbose: TL_LOG_INFO("RHI::WebGPU (Verbose): {}", message.data); break;
            case WGPULoggingType_Info:    TL_LOG_INFO("RHI::WebGPU (Info): {}", message.data); break;
            case WGPULoggingType_Warning: TL_LOG_WARNNING("RHI::WebGPU (Warning): {}", message.data); break;
            case WGPULoggingType_Error:   TL_LOG_ERROR("RHI::WebGPU (Error): {}", message.data); break;
            case WGPULoggingType_Force32: TL_UNREACHABLE();
            }
        };
        WGPULoggingCallbackInfo loggingCallbackInfo{
            .nextInChain = nullptr,
            .callback    = loggingCallback,
            .userdata1   = this,
            .userdata2   = nullptr,
        };
        wgpuDeviceSetLoggingCallback(m_device, loggingCallbackInfo);

        m_queue = wgpuDeviceGetQueue(m_device);

        return ResultCode::Success;
    }

    void IDevice::Shutdown()
    {
        wgpuDeviceRelease(m_device);
        wgpuAdapterRelease(m_adapter);
        wgpuInstanceRelease(m_instance);
    }

    void IDevice::UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo)
    {
        auto bindGroup = m_bindGroupOwner.Get(handle);
        bindGroup->Update(this, updateInfo);
    }

    void IDevice::BeginResourceUpdate([[maybe_unused]] RenderGraph* renderGraph)
    {
        // No op
    }

    void IDevice::EndResourceUpdate()
    {
        for (auto buffer : m_buffersToUnmap)
        {
            wgpuBufferUnmap(buffer);
        }
    }

    void IDevice::BufferWrite(Handle<Buffer> bufferHandle, size_t offset, TL::Block block)
    {
        auto buffer = m_bufferOwner.Get(bufferHandle);
        // if (wgpuBufferGetMapState(buffer->buffer) == WGPUBufferMapState_Mapped)
        // {
        //     auto result = wgpuBufferWriteMappedRange(buffer->buffer, offset, block.ptr, block.size);
        //     TL_ASSERT(result == WGPUStatus_Success);
        //     return;
        // }
        wgpuQueueWriteBuffer(m_queue, buffer->buffer, offset, block.ptr, block.size);
    }

    void IDevice::ImageWrite(Handle<Image> imageHandle, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block)
    {
        TL_ASSERT(arrayLayer == 0, "Can't write to array layers");

        auto image = m_imageOwner.Get(imageHandle);

        auto formatInfo = GetFormatInfo(image->format);

        WGPUTexelCopyTextureInfo destination{
            .texture  = image->texture,
            .mipLevel = mipLevel,
            .origin   = ConvertToOffset3D(offset),
            .aspect   = WGPUTextureAspect_All,
        };
        WGPUTexelCopyBufferLayout dataLayout{
            .offset       = 0,
            .bytesPerRow  = size.width * formatInfo.bytesPerBlock,
            .rowsPerImage = size.height,
        };
        auto writeSize = ConvertToExtent3D(size);

        // auto mipSize     = CalcaulteImageMipSize(image->size, mipLevel);
        // auto widthValid  = (offset.x + size.width) >= mipSize.width;
        // auto heightValid = (offset.y + size.height) >= mipSize.height;
        // auto depthValid  = (offset.z + size.depth) >= mipSize.depth;
        // TL_ASSERT(
        //     widthValid && heightValid && depthValid,
        //     "Invalid Offset ({}, {}, {}) + Size ({}, {}, {}), Exceed image mip level size ({}, {}, {})",
        //     (int32_t)offset.x,
        //     (int32_t)offset.y,
        //     (int32_t)offset.z,
        //     (int32_t)size.width,
        //     (int32_t)size.height,
        //     (int32_t)size.depth,
        //     (int32_t)mipSize.width,
        //     (int32_t)mipSize.height,
        //     (int32_t)mipSize.depth);
        wgpuQueueWriteTexture(m_queue, &destination, block.ptr, block.size, &dataLayout, &writeSize);
    }

    void IDevice::CollectResources()
    {
        wgpuDeviceTick(m_device);
    }

    void IDevice::ExecuteCommandList(ICommandList* commandList)
    {
        wgpuQueueSubmit(m_queue, 1, &commandList->m_cmdBuffer);
        wgpuCommandBufferRelease(commandList->m_cmdBuffer);
        delete commandList;
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
        OwnerField.Destroy(handle, this);                                      \
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

    Handle<Image> IDevice::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        TL_UNREACHABLE_MSG("TODO! Implement image views for WebGPU backend!");
        return {};
    }

    IMPLEMENT_NONDISPATCHABLE_TYPES_FUNCTIONS_WITH_RESULTS(Buffer, m_bufferOwner);
} // namespace RHI::WebGPU
