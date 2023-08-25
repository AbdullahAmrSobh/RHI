#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Result.hpp"
#include "RHI/Span.hpp"

namespace RHI
{

struct DeviceProperties;

typedef uint32_t Version;

typedef std::function<uint32_t(TL::Span<DeviceProperties>)> DeviceSelectionCallback;

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
    /// @brief The name of the users application.
    std::string applicationName;

    /// @brief The version of the users application.
    Version applicationVersion;
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

/// @brief An interface implemented by the user, which the API use to log.
class DebugCallbacks
{
public:
    virtual ~DebugCallbacks() = default;

    /// @brief Log an information.
    virtual void LogInfo(std::string_view message) = 0;

    /// @brief Log an warnning.
    virtual void LogWarnning(std::string_view message) = 0;

    /// @brief Log an error.
    virtual void LogError(std::string_view message) = 0;
};

/// @brief RHI Context, represent an instance of the API.
class RHI_EXPORT Context
{
public:
    Context()          = default;
    virtual ~Context() = default;

    /// @brief Creates a new pass producer.
    template<typename T, typename... Args>
    std::unique_ptr<class PassProducer> CreatePassProducer(Args... args);

    /// @brief Creates a new ShaderModule
    virtual std::unique_ptr<class ShaderModule> CreateShaderModule(TL::Span<uint32_t> code) = 0;

    /// @brief Creates a shader bind group allocator.
    virtual std::unique_ptr<class ShaderBindGroupAllocator> CreateShaderBindGroupAllocator() = 0;

    /// @brief Creates a new FrameScheduler.
    virtual std::unique_ptr<class FrameScheduler> CreateFrameScheduler() = 0;

    /// @brief Creates a new Swapchain.
    virtual std::unique_ptr<class Swapchain> CreateSwapchain(const struct SwapchainCreateInfo& createInfo) = 0;

    /// @brief Creates a new Pool for all resources.
    virtual std::unique_ptr<class ResourcePool> CreateResourcePool(const struct ResourcePoolCreateInfo& createInfo) = 0;

    /// @brief Creates a new graphics pipeline state object for graphics.
    virtual Handle<class GraphicsPipelineState> CreateGraphicsPipelineState(const struct GraphicsPipelineStateCreateInfo& createInfo) = 0;

    /// @brief Creates a new compute pipeline state object for graphics.
    virtual Handle<class ComputePipelineState> CreateComputePipelineState(const struct ComputePipelineStateCreateInfo& createInfo) = 0;

    /// @brief Creates a new Sampler state.
    virtual Handle<class SamplerState> CreateSampler(const struct SamplerStateCreateInfo& createInfo) = 0;

    /// @brief Frees the given graphics pipeline state object.
    virtual void Free(Handle<GraphicsPipelineState> pso) = 0;

    /// @brief Frees the given compute pipeline state object.
    virtual void Free(Handle<ComputePipelineState> pso) = 0;

    /// @brief Frees the given sampler state object.
    virtual void Free(Handle<SamplerState> pso) = 0;

protected:
    /// @brief Creates a new Pass.
    virtual std::unique_ptr<class Pass> CreatePass(const struct PassCreateInfo& createInfo) = 0;

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

template<typename T, typename... Args>
std::unique_ptr<PassProducer> Context::CreatePassProducer(Args... args)
{
    // PassCreateInfo createInfo {};
    // auto pass = CreatePass(createInfo);
    return std::make_unique<T>(std::forward(args)...);
}

}  // namespace RHI