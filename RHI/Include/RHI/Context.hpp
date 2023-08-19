#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "RHI/Assert.hpp"
#include "RHI/Common.hpp"
#include "RHI/Debug.hpp"
#include "RHI/Export.hpp"
#include "RHI/Result.hpp"

namespace RHI
{

// Forward declarations
struct PassCreateInfo;
struct SwapchainCreateInfo;
struct ResourcePoolCreateInfo;
struct BufferPoolCreateInfo;
struct ImagePoolCreateInfo;
struct QueryPoolCreateInfo;
struct ShaderBindGroupAllocatorCreateInfo;
struct SamplerCreateInfo;
struct PipelineStateCacheCreateInfo;
struct GraphicsPipelineCreateInfo;
struct ComputePipelineCreateInfo;
struct RayTracingPipelineCreateInfo;

class PassProducer;
class Pass;
class FrameScheduler;
class Swapchain;
class ResourcePool;
class BufferPool;
class ImagePool;
class QueryPool;
class Fence;
class ShaderBindGroupAllocator;
class Sampler;
class PipelineStateCache;
class PipelineState;

typedef uint32_t Version;
typedef uint32_t(std::span<DeviceProperties> properties) DeviceSelectionCallback;

/// @brief Type of backend Graphics API
enum class Backend
{
    Validate,
    Vulkan13,
    DirectX12,
};

/// @brief The type of the Physical GPU
enum class DeviceType
{
    Integerated,
    Dedicated
};

// Identify the manufactuerer for the reported device
enum class Vendor
{
    Intel,
    Nvida,
    AMD,
    Other,
};

/// @brief Describes information needed to initalize the RHI context
struct ApplicationInfo
{
    Backend     graphicsBackend;
    const char* applicationName;
    Version     applicationVersion;
    const char* engineName;
    Version     engineVersion;
};

/// @brief Properties about a Physical GPU
struct DeviceProperties
{
    std::string name;
    DeviceType  type;
    Vendor      vendor;
};

/// @brief Creates a Version from major.minor.patch
RHI_FORCE_INLINE constexpr Version MakeVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
    return (major << 16) | (minor << 8) | patch;
}

/// @brief RHI Context, represent an instance of the API.
class RHI_EXPORT Context
{
public:
    Context()          = default;
    virtual ~Context() = default;

    /// @brief Creates a new pass producer.
    template<typename... Args>
    std::unique_ptr<PassProducer> CreatePassProducer(Args... args);

    /// @brief Creates a new Pass.
    virtual std::unique_ptr<Pass> CreatePass(const PassCreateInfo& createInfo) = 0;

    /// @brief Creates a new FrameScheduler.
    virtual std::unique_ptr<FrameScheduler> CreateFrameScheduler() = 0;

    /// @brief Creates a new Swapchain.
    virtual std::unique_ptr<Swapchain> CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;

    /// @brief Creates a new Pool for all resources.
    virtual std::unique_ptr<ResourcePool> CreateResourcePool(const ResourcePoolCreateInfo& createInfo) = 0;

    /// @brief Creates a new PipelineStateCache.
    virtual std::unique_ptr<PipelineStateCache> CreatePipelineStateCache(const PipelineStateCacheCreateInfo& createInfo) = 0;

    /// @brief Creates a new PipelineState object for graphics.
    virtual std::unique_ptr<PipelineState> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;

    /// @brief Creates a new PipelineState object for graphics.
    virtual std::unique_ptr<PipelineState> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;

    /// @brief Creates a new PipelineState object for compute.
    virtual std::unique_ptr<PipelineState> CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) = 0;

    /// @brief Creates a new Sampler state.
    virtual std::unique_ptr<Sampler> CreateSampler(const SamplerCreateInfo& createInfo) = 0;

protected:
    /// @brief Pointer to the user provided debug callbacks.
    std::unique_ptr<DebugCallbacks> m_debugCallbacks;w
};

}  // namespace RHI