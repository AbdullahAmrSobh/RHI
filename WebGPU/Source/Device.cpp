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
                .nextInChain                  = nullptr,
                .timedwaitTimelineAnyEnable   = false,
                .timedwaitTimelineAnyMaxCount = 0,
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
                TL::LogInfo("RHI::WebGPU: {}", message.data);
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
            .mode        = WGPUCallbackMode_waitTimelineAnyOnly,
            .callback    = requestAdapterCallback,
            .userdata1   = &m_adapter,
            .userdata2   = nullptr,
        };
        auto                       future = wgpuInstanceRequestAdapter(m_instance, &options, cb);
        WGPUFuturewaitTimelineInfo waitTimelineInfo{
            .future    = future,
            .completed = true,
        };
        auto result = wgpuInstancewaitTimelineAny(m_instance, 1, &waitTimelineInfo, 0);
        TL_ASSERT(result == WGPUwaitTimelineStatus_Success);

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
            TL::LogError("RHI::WebGPU Device lost. Reported message:  {}", message.data);
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
            TL::LogError("RHI::WebGPU Uncaptured error. Reported message: {}", message.data);
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
            case WGPULoggingType_Verbose: TL::LogInfo("RHI::WebGPU (Verbose): {}", message.data); break;
            case WGPULoggingType_Info:    TL::LogInfo("RHI::WebGPU (Info): {}", message.data); break;
            case WGPULoggingType_Warning: TL::LogWarn("RHI::WebGPU (Warning): {}", message.data); break;
            case WGPULoggingType_Error:   TL::LogError("RHI::WebGPU (Error): {}", message.data); break;
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
        waitTimelineIdle();

        for (auto& frame : m_framesInFlight)
        {
            frame->Shutdown();
        }

        // Report live object stack traces
        {
            auto liveSwapchains = m_liveSwapchains;
            for (auto [ptr, stacktrace] : liveSwapchains)
            {
                TL::LogWarn("Leaked: RHI::Swapchain at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroySwapchain(ptr);
            }

            auto liveShaderModules = m_liveShaderModules;
            for (auto [ptr, stacktrace] : liveShaderModules)
            {
                TL::LogWarn("Leaked: RHI::ShaderModule at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyShaderModule(ptr);
            }

            auto liveImages = m_liveImages;
            for (auto [ptr, stacktrace] : liveImages)
            {
                TL::LogWarn("Leaked: RHI::Image at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyImage(ptr);
            }

            auto liveBuffers = m_liveBuffers;
            for (auto [ptr, stacktrace] : liveBuffers)
            {
                TL::LogWarn("Leaked: RHI::Buffer at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBuffer(ptr);
            }

            auto liveBindGroupLayouts = m_liveBindGroupLayouts;
            for (auto [ptr, stacktrace] : liveBindGroupLayouts)
            {
                TL::LogWarn("Leaked: RHI::BindGroupLayout at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBindGroupLayout(ptr);
            }

            auto liveBindGroups = m_liveBindGroups;
            for (auto [ptr, stacktrace] : liveBindGroups)
            {
                TL::LogWarn("Leaked: RHI::BindGroup at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyBindGroup(ptr);
            }

            auto livePipelineLayouts = m_livePipelineLayouts;
            for (auto [ptr, stacktrace] : livePipelineLayouts)
            {
                TL::LogWarn("Leaked: RHI::PipelineLayout at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyPipelineLayout(ptr);
            }

            auto liveGraphicsPipelines = m_liveGraphicsPipelines;
            for (auto [ptr, stacktrace] : liveGraphicsPipelines)
            {
                TL::LogWarn("Leaked: RHI::GraphicsPipeline at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyGraphicsPipeline(ptr);
            }

            auto liveComputePipelines = m_liveComputePipelines;
            for (auto [ptr, stacktrace] : liveComputePipelines)
            {
                TL::LogWarn("Leaked: RHI::ComputePipeline at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroyComputePipeline(ptr);
            }

            auto liveSamplers = m_liveSamplers;
            for (auto [ptr, stacktrace] : liveSamplers)
            {
                TL::LogWarn("Leaked: RHI::Sampler at:\n{}", TL::ReportStacktrace(stacktrace));
                DestroySampler(ptr);
            }
        }

        wgpuDeviceRelease(m_device);
        wgpuAdapterRelease(m_adapter);
        wgpuInstanceRelease(m_instance);
    }

    void IDevice::waitTimelineIdle()
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

    uint64_t IDevice::GetNativeHandle(NativeHandleType type, uint64_t _resource)
    {
        switch (type)
        {
        case NativeHandleType::None: return 0;
        case NativeHandleType::Device:
            {
                auto resource = (IDevice*)_resource;
                return (uint64_t)resource->m_device;
            }
        case NativeHandleType::CommandList:
            {
                auto resource = (ICommandList*)_resource;
                return (uint64_t)resource->m_commandBuffer;
            }
        case NativeHandleType::Buffer:
            {
                auto resource = (IBuffer*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::Image:
            {
                auto resource = (IImage*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::ImageView:
            {
                auto resource = (IImage*)_resource;
                return (uint64_t)resource->viewHandle;
            }
        case NativeHandleType::Sampler:
            {
                auto resource = (ISampler*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::ShaderModule:
            {
                auto resource = (IShaderModule*)_resource;
                return (uint64_t)resource->m_shaderModule;
            }
        case NativeHandleType::Pipeline:
            {
                auto resource = (IGraphicsPipeline*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::PipelineLayout:
            {
                auto resource = (IPipelineLayout*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::BindGroupLayout:
            {
                auto resource = (IBindGroupLayout*)_resource;
                return (uint64_t)resource->handle;
            }
        case NativeHandleType::BindGroup:
            {
                auto resource = (IBindGroup*)_resource;
                return (uint64_t)resource->descriptorSet;
            }
        case NativeHandleType::Swapchain:
            {
                auto resource = (ISwapchain*)_resource;
                return (uint64_t)resource->GetHandle();
            }
        default:
            TL_UNREACHABLE_MSG("Unknown NativeHandleType");
        }
        return 0;
    }

    // interface implementation

    template<typename Resource, typename... Args>
    inline Resource* createImpl(IDevice* device, TL::Map<Resource*, TL::Stacktrace>& liveResources, Args... args)
    {
        Resource* resource = TL ::construct<Resource>();
        resource->Init(device, args...);
        liveResources.emplace(resource, TL::CaptureStacktrace());
        return resource;
    }

    template<typename Resource>
    inline void destroyImpl(IDevice* device, Resource* resource, TL::Map<Resource*, TL::Stacktrace>& liveResources)
    {
        auto erased = liveResources.erase(resource);
        TL_ASSERT(erased);
        resource->Shutdown(device);
    }

    // clang-format off
    CommandPool*        IDevice::CreateCommandPool(const CommandPoolCreateInfo& createInfo)                { return createImpl<ICommandPool>(this, this->m_liveCommandPools, createInfo);                             }
    void                IDevice::DestroyCommandPool(CommandPool* resource)                                 { destroyImpl<ICommandPool>(this, (ICommandPool*)resource, this->m_liveCommandPools);                      }
    Fence*              IDevice::CreateFence(const FenceCreateInfo& createInfo)                            { return createImpl<IFence>(this, this->m_liveFences, createInfo);                                         }
    void                IDevice::DestroyFence(Fence* resource)                                             { destroyImpl<IFence>(this, (IFence*)resource, this->m_liveFences);                                        }
    Swapchain*          IDevice::CreateSwapchain(const SwapchainCreateInfo& createInfo)                    { return createImpl<ISwapchain>(this, this->m_liveSwapchains, createInfo);                                 }
    void                IDevice::DestroySwapchain(Swapchain* resource)                                     { destroyImpl<ISwapchain>(this, (ISwapchain*)resource, this->m_liveSwapchains);                            }
    ShaderModule*       IDevice::CreateShaderModule(const ShaderModuleCreateInfo& createInfo)              { return createImpl<IShaderModule>(this, this->m_liveShaderModules, createInfo);                           }
    void                IDevice::DestroyShaderModule(ShaderModule* resource)                               { destroyImpl<IShaderModule>(this, (IShaderModule*)resource, this->m_liveShaderModules);                   }
    BindGroupLayout*    IDevice::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)        { return createImpl<IBindGroupLayout>(this, this->m_liveBindGroupLayouts, createInfo);                     }
    void                IDevice::DestroyBindGroupLayout(BindGroupLayout* resource)                         { destroyImpl<IBindGroupLayout>(this, (IBindGroupLayout*)resource, this->m_liveBindGroupLayouts);          }
    BindGroup*          IDevice::CreateBindGroup(const BindGroupCreateInfo& createInfo)                    { return createImpl<IBindGroup>(this, this->m_liveBindGroups, createInfo);                                 }
    void                IDevice::DestroyBindGroup(BindGroup* resource)                                     { destroyImpl<IBindGroup>(this, (IBindGroup*)resource, this->m_liveBindGroups);                            }
    QueryPool*          IDevice::CreateQueryPool(const QueryPoolCreateInfo& createInfo)                    { return createImpl<IQueryPool>(this, this->m_liveQueryPools, createInfo);                                 }
    void                IDevice::DestroyQueryPool(QueryPool* resource)                                     { destroyImpl<IQueryPool>(this, (IQueryPool*)resource, this->m_liveQueryPools);                            }
    Buffer*             IDevice::CreateBuffer(const BufferCreateInfo& createInfo)                          { return createImpl<IBuffer>(this, this->m_liveBuffers, createInfo);                                       }
    void                IDevice::DestroyBuffer(Buffer* resource)                                           { destroyImpl<IBuffer>(this, (IBuffer*)resource, this->m_liveBuffers);                                     }
    Image*              IDevice::CreateImage(const ImageCreateInfo& createInfo)                            { return createImpl<IImage>(this, this->m_liveImages, createInfo);                                         }
    Image*              IDevice::CreateImageView(const ImageViewCreateInfo& createInfo)                    { return createImpl<IImage>(this, this->m_liveImages, createInfo);                                         }
    void                IDevice::DestroyImage(Image* resource)                                             { destroyImpl<IImage>(this, (IImage*)resource, this->m_liveImages);                                        }
    Sampler*            IDevice::CreateSampler(const SamplerCreateInfo& createInfo)                        { return createImpl<ISampler>(this, this->m_liveSamplers, createInfo);                                     }
    void                IDevice::DestroySampler(Sampler* resource)                                         { destroyImpl<ISampler>(this, (ISampler*)resource, this->m_liveSamplers);                                  }
    PipelineLayout*     IDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)          { return createImpl<IPipelineLayout>(this, this->m_livePipelineLayouts, createInfo);                       }
    void                IDevice::DestroyPipelineLayout(PipelineLayout* resource)                           { destroyImpl<IPipelineLayout>(this, (IPipelineLayout*)resource, this->m_livePipelineLayouts);             }
    GraphicsPipeline*   IDevice::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)      { return createImpl<IGraphicsPipeline>(this, this->m_liveGraphicsPipelines, createInfo);                   }
    void                IDevice::DestroyGraphicsPipeline(GraphicsPipeline* resource)                       { destroyImpl<IGraphicsPipeline>(this, (IGraphicsPipeline*)resource, this->m_liveGraphicsPipelines);       }
    RayTracingPipeline* IDevice::CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo)  { return createImpl<IRayTracingPipeline>(this, this->m_liveRayTracingPipelines, createInfo);               }
    void                IDevice::DestroyRayTracingPipeline(RayTracingPipeline* resource)                   { destroyImpl<IRayTracingPipeline>(this, (IRayTracingPipeline*)resource, this->m_liveRayTracingPipelines); }
    ComputePipeline*    IDevice::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)        { return createImpl<IComputePipeline>(this, this->m_liveComputePipelines, createInfo);                     }
    void                IDevice::DestroyComputePipeline(ComputePipeline* resource)                         { destroyImpl<IComputePipeline>(this, (IComputePipeline*)resource, this->m_liveComputePipelines);          }

    // clang-format on

} // namespace RHI::WebGPU
