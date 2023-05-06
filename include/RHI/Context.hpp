#pragma once

#include <memory>
#include <span>
#include <string_view>

#include "RHI/Debug.hpp"
#include "RHI/Export.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/Pipeline.hpp"
#include "RHI/Resources.hpp"
#include "RHI/ShaderResourceGroup.hpp"

namespace RHI
{

enum class Backend
{
    Validation,
    Vulkan,
    D3D12,
    Metal2,
    WebGPU,
};

enum class DeviceType
{
    Integerated,
    Dedicated
};

struct ApplicationInfo
{
    std::string_view applicationName;
    uint32_t         applicationVersion;
    std::string_view engineName;
    uint32_t         engineVersion;
};

struct DeviceInfo
{
    uint32_t   deviceIndex;
    DeviceType deviceType;
};

inline constexpr uint32_t MakeVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
    return (major << 16) | (minor << 8) | patch;
}

class RHI_EXPORT Context
{
public:
    static std::unique_ptr<Context> Create(const ApplicationInfo& appInfo, std::unique_ptr<DebugCallbacks> debugCallbacks, Backend backend);

    Context() = default;
    virtual ~Context() = default;

    virtual ResultCode Init(const ApplicationInfo& appInfo) = 0;
    
    virtual ResultCode SetDevice(uint32_t deviceId) = 0;

    const std::span<const DeviceInfo> QueryDevicesInfos() const
    {
        return m_devices;
    }

    std::unique_ptr<ShaderResourceGroupAllocator> CreateShaderResourceGroupAllocator();

    std::unique_ptr<Image> CreateImage(const ResourceAllocationInfo& allocationInfo, const ImageCreateInfo& createInfo);

    std::unique_ptr<Buffer> CreateBuffer(const ResourceAllocationInfo& allocationInfo, const BufferCreateInfo& createInfo);

    std::unique_ptr<Swapchain> CreateSwapchain(const SwapchainCreateInfo& createInfo);

    std::unique_ptr<Fence> CreateFence();

    std::unique_ptr<Sampler> CreateSampler(const SamplerCreateInfo& createInfo);

    std::unique_ptr<FrameScheduler> CreateFrameScheduler();

    std::unique_ptr<PipelineState> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);

    std::unique_ptr<PipelineState> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo);

    std::unique_ptr<PipelineState> CreateRaytraceingPipeline(const RayTracingPipelineCreateInfo& createInfo);

private:
    RHI_SUPPRESS_C4251
    std::vector<DeviceInfo> m_devices;
    Backend                 m_backend;
};

}  // namespace RHI