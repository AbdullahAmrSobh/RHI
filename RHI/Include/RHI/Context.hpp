#pragma once

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Export.hpp"
#include "RHI/Result.hpp"
#include "RHI/Handle.hpp"

namespace RHI
{

/// Forward decelerations
struct DeviceProperties;
struct PassCreateInfo;
struct SwapchainCreateInfo;
struct ResourcePoolCreateInfo;
struct PipelineStateCacheCreateInfo;
struct GraphicsPipelineCreateInfo;
struct ComputePipelineCreateInfo;
struct RayTracingPipelineCreateInfo;
struct SamplerCreateInfo;

class PassProducer;
class Pass;
class FrameScheduler;
class Swapchain;
class ResourcePool;
class PipelineStateCache;
class PipelineState;
class Sampler;

typedef uint32_t Version;

typedef std::function<uint32_t(std::span<DeviceProperties>)> DeviceSelectionCallback;

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
constexpr Version MakeVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
    return (major << 16) | (minor << 8) | patch;
}

class DebugCallbacks
{
public:
    virtual ~DebugCallbacks() = default;

    virtual void Info(std::string_view message) = 0;

    virtual void Warn(std::string_view message) = 0;

    virtual void Error(std::string_view message) = 0;
};

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
    virtual Handle<Pass> CreatePass(const PassCreateInfo& createInfo) = 0;

    /// @brief Creates a new FrameScheduler.
    virtual std::unique_ptr<FrameScheduler> CreateFrameScheduler() = 0;

    /// @brief Creates a new Swapchain.
    virtual std::unique_ptr<Swapchain> CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;

    /// @brief Creates a new Pool for all resources.
    virtual std::unique_ptr<ResourcePool> CreateResourcePool(const ResourcePoolCreateInfo& createInfo) = 0;

    /// @brief Creates a new PipelineStateCache.
    virtual std::unique_ptr<PipelineStateCache> CreatePipelineStateCache(const PipelineStateCacheCreateInfo& createInfo) = 0;

    /// @brief Creates a new PipelineState object for graphics.
    virtual Handle<PipelineState> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;

    /// @brief Creates a new PipelineState object for graphics.
    virtual Handle<PipelineState> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;

    /// @brief Creates a new PipelineState object for compute.
    virtual Handle<PipelineState> CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) = 0;

    /// @brief Creates a new Sampler state.
    virtual Handle<Sampler> CreateSampler(const SamplerCreateInfo& createInfo) = 0;

    /// @brief Frees the given pass resource.
    virtual void Free(Handle<Pass> pass) = 0;

    /// @brief frees the given pipeline state object resource.
    virtual void Free(Handle<PipelineState> pso) = 0;

    /// @brief frees the given sampler resource.
    virtual void Free(Handle<Sampler> sampler) = 0;

private:
    friend class Object;

    RHI_FORCE_INLINE DebugCallbacks& GetDebugCallbacks() const
    {
        return *m_debugCallbacks;
    }

protected:
    /// @brief Pointer to the user provided debug callbacks.
    std::unique_ptr<DebugCallbacks> m_debugCallbacks;
};

}  // namespace RHI