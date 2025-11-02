#include "Device.hpp"

#include "RHI/Resources.hpp"

#include <RHI-WebGPU/Loader.hpp>
#include <tracy/Tracy.hpp>
#include <webgpu/webgpu.h>

#include "CommandList.hpp"
#include "Common.hpp"
#include "Frame.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"

#include <TL/Log.hpp>
#include <TL/Assert.hpp>
#include <TL/Stacktrace.hpp>

RHI::Device* RHI::CreateWebGPUDevice()
{
    ZoneScoped;
    auto device = TL::construct<RHI::WebGPU::IDevice>();
    auto result = device->Init();
    TL_ASSERT(IsSuccess(result));
    return device;
}

void RHI::DestroyWebGPUDevice(RHI::Device* _device)
{
    auto device = (RHI::WebGPU::IDevice*)_device;
    device->Shutdown();
    TL::destruct(device);
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

        WGPUAdapterProperties adapterProps{};
        wgpuAdapterGetProperties(m_adapter, &adapterProps);
        m_limits                                  = TL::CreatePtr<DeviceLimits>();
        m_limits->minUniformBufferOffsetAlignment = 256; // WebGPU minimum
        m_limits->minStorageBufferOffsetAlignment = 256; // WebGPU minimum

        m_framesInFlight.resize(2);
        for (auto& frame : m_framesInFlight)
        {
            frame           = TL::CreatePtr<IFrame>();
            auto resultCode = frame->Init(this);
            if (IsError(resultCode))
            {
                Shutdown();
                return resultCode;
            }
        }

        return ResultCode::Success;
    }

    void IDevice::Shutdown()
    {
        WaitIdle();

        for (auto& frame : m_framesInFlight)
        {
            frame->Shutdown();
        }

        // Report live object stack traces
        {
            auto liveSwapchains = m_liveSwapchains;
            for (auto [ptr, stacktrace] : liveSwapchains)
            {
                TL_LOG_WARNNING("Leaked: RHI::Swapchain at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroySwapchain(ptr);
            }

            auto liveShaderModules = m_liveShaderModules;
            for (auto [ptr, stacktrace] : liveShaderModules)
            {
                TL_LOG_WARNNING("Leaked: RHI::ShaderModule at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyShaderModule(ptr);
            }

            auto liveImages = m_liveImages;
            for (auto [ptr, stacktrace] : liveImages)
            {
                TL_LOG_WARNNING("Leaked: RHI::Image at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyImage(ptr);
            }

            auto liveBuffers = m_liveBuffers;
            for (auto [ptr, stacktrace] : liveBuffers)
            {
                TL_LOG_WARNNING("Leaked: RHI::Buffer at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBuffer(ptr);
            }

            auto liveBindGroupLayouts = m_liveBindGroupLayouts;
            for (auto [ptr, stacktrace] : liveBindGroupLayouts)
            {
                TL_LOG_WARNNING("Leaked: RHI::BindGroupLayout at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBindGroupLayout(ptr);
            }

            auto liveBindGroups = m_liveBindGroups;
            for (auto [ptr, stacktrace] : liveBindGroups)
            {
                TL_LOG_WARNNING("Leaked: RHI::BindGroup at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBindGroup(ptr);
            }

            auto livePipelineLayouts = m_livePipelineLayouts;
            for (auto [ptr, stacktrace] : livePipelineLayouts)
            {
                TL_LOG_WARNNING("Leaked: RHI::PipelineLayout at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyPipelineLayout(ptr);
            }

            auto liveGraphicsPipelines = m_liveGraphicsPipelines;
            for (auto [ptr, stacktrace] : liveGraphicsPipelines)
            {
                TL_LOG_WARNNING("Leaked: RHI::GraphicsPipeline at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyGraphicsPipeline(ptr);
            }

            auto liveComputePipelines = m_liveComputePipelines;
            for (auto [ptr, stacktrace] : liveComputePipelines)
            {
                TL_LOG_WARNNING("Leaked: RHI::ComputePipeline at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyComputePipeline(ptr);
            }

            auto liveSamplers = m_liveSamplers;
            for (auto [ptr, stacktrace] : liveSamplers)
            {
                TL_LOG_WARNNING("Leaked: RHI::Sampler at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroySampler(ptr);
            }
        }

        wgpuDeviceRelease(m_device);
        wgpuAdapterRelease(m_adapter);
        wgpuInstanceRelease(m_instance);
    }

    void IDevice::WaitIdle()
    {
        ZoneScoped;
        wgpuDeviceTick(m_device);
    }

    void IDevice::UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo)
    {
        ZoneScoped;
        auto bindGroup = (IBindGroup*)(handle);
        bindGroup->Update(this, updateInfo);
    }

    TL::IAllocator& IDevice::GetTempAllocator()
    {
        return m_framesInFlight[m_currentFrameIndex]->GetAllocator();
    }

    ResultCode IDevice::SetFramesInFlightCount(uint32_t count)
    {
        auto previousSize = m_framesInFlight.size();
        m_framesInFlight.resize(count);
        for (size_t i = previousSize; i < m_framesInFlight.size(); i++)
        {
            auto result = m_framesInFlight[i]->Init(this);
            if (IsError(result))
                return result;
        }
        return ResultCode::Success;
    }

    Frame* IDevice::GetCurrentFrame()
    {
        return m_framesInFlight[m_currentFrameIndex].get();
    }

    uint64_t IDevice::GetNativeHandle(NativeHandleType type, uint64_t _handle)
    {
        switch (type)
        {
        case NativeHandleType::None: return 0;
        case NativeHandleType::Device:
            {
                auto handle = reinterpret_cast<IDevice*>(_handle);
                return (uint64_t)handle->m_device;
            }
        case NativeHandleType::CommandList:
            {
                auto handle = reinterpret_cast<ICommandList*>(_handle);
                return (uint64_t)handle->m_cmdBuffer;
            }
        case NativeHandleType::Buffer:
        case NativeHandleType::Image:
        case NativeHandleType::ImageView:
        case NativeHandleType::Sampler:
        case NativeHandleType::ShaderModule:
        case NativeHandleType::Pipeline:
        case NativeHandleType::PipelineLayout:
        case NativeHandleType::BindGroupLayout:
        case NativeHandleType::BindGroup:
        case NativeHandleType::Swapchain:
        default:
            TL_UNREACHABLE_MSG("TODO! implement");
        }
        return 0;
    }


    Swapchain* IDevice::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<ISwapchain>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveSwapchains.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroySwapchain(Swapchain* _handle)
    {
        ZoneScoped;
        auto erased = m_liveSwapchains.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (ISwapchain*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }

    ShaderModule* IDevice::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<IShaderModule>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveShaderModules.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyShaderModule(ShaderModule* _handle)
    {
        ZoneScoped;
        auto erased = m_liveShaderModules.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IShaderModule*)_handle;
        handle->Shutdown();
        TL::destruct(_handle);
    }

    BindGroupLayout* IDevice::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<IBindGroupLayout>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveBindGroupLayouts.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyBindGroupLayout(BindGroupLayout* _handle)
    {
        ZoneScoped;
        auto erased = m_liveBindGroupLayouts.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IBindGroupLayout*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }

    BindGroup* IDevice::CreateBindGroup(const BindGroupCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<IBindGroup>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveBindGroups.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyBindGroup(BindGroup* _handle)
    {
        ZoneScoped;
        auto erased = m_liveBindGroups.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IBindGroup*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }

    PipelineLayout* IDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<IPipelineLayout>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_livePipelineLayouts.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyPipelineLayout(PipelineLayout* _handle)
    {
        ZoneScoped;
        auto erased = m_livePipelineLayouts.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IPipelineLayout*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }

    GraphicsPipeline* IDevice::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<IGraphicsPipeline>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveGraphicsPipelines.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyGraphicsPipeline(GraphicsPipeline* _handle)
    {
        ZoneScoped;
        auto erased = m_liveGraphicsPipelines.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IGraphicsPipeline*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }

    ComputePipeline* IDevice::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<IComputePipeline>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveComputePipelines.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyComputePipeline(ComputePipeline* _handle)
    {
        ZoneScoped;
        auto erased = m_liveComputePipelines.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IComputePipeline*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }

    Sampler* IDevice::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<ISampler>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveSamplers.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroySampler(Sampler* _handle)
    {
        ZoneScoped;
        auto erased = m_liveSamplers.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (ISampler*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }

    Image* IDevice::CreateImage(const ImageCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<IImage>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveImages.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    Image* IDevice::CreateImageView(TL_MAYBE_UNUSED const ImageViewCreateInfo& createInfo)
    {
        TL_UNREACHABLE_MSG("TODO! Implement image views for WebGPU backend!");
        return {};
    }

    void IDevice::DestroyImage(Image* _handle)
    {
        ZoneScoped;
        auto erased = m_liveImages.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IImage*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }

    Buffer* IDevice::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;
        auto handle = TL::construct<IBuffer>();
        auto result = handle->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        m_liveBuffers.emplace(handle, TL::CaptureStacktrace());
        return handle;
    }

    void IDevice::DestroyBuffer(Buffer* _handle)
    {
        ZoneScoped;
        auto erased = m_liveBuffers.erase(_handle);
        TL_ASSERT(erased);
        auto handle = (IBuffer*)_handle;
        handle->Shutdown(this);
        TL::destruct(_handle);
    }
} // namespace RHI::WebGPU
