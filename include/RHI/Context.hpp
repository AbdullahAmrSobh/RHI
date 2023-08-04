#pragma once

#include <memory>
#include <span>
#include <string_view>

#include "RHI/Debug.hpp"
#include "RHI/Export.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/LRUCache.hpp"
#include "RHI/Pipeline.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Result.hpp"
#include "RHI/ShaderResourceGroup.hpp"

namespace RHI
{

using Version = uint32_t;

// Type of backend Graphics API
enum class Backend
{
    // A backend used for testing, and validation of the application.
    Validate,
    // Backend using Vulkan 1.3.x API
    Vulkan,
};

// The type of the Physical GPU
enum class DeviceType
{
    // An integerated GPU device type
    Integerated,
    // A dedicated GPU device type
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

// Describes information needed to initalize the RHI context
struct ApplicationInfo
{
    Backend     graphicsBackend;
    const char* applicationName;
    Version     applicationVersion;
    const char* engineName;
    Version     engineVersion;
};

// Properties about a Physical GPU
struct DeviceProperties
{
    std::string name;
    DeviceType  type;
    Vendor      vendor;
};

// Creates a Version from major.minor.patch
inline constexpr Version MakeVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
    return (major << 16) | (minor << 8) | patch;
}

// The RHI Context class, used to create all graphics objects, and manage them
class RHI_EXPORT Context
{
public:
    static std::unique_ptr<Context> Create(const ApplicationInfo& appInfo, std::unique_ptr<DebugCallbacks> debugCallbacks);

    Context()          = default;
    virtual ~Context() = default;

    // Returns a list of the available physical devices and their properties
    inline std::span<const DeviceProperties> GetPhysicalDevicesProperties() const
    {
        return m_physcialDevicesProperties;
    }

    // Initalizes the context
    virtual ResultCode Init(const ApplicationInfo& appInfo) = 0;

    // Set the Change the physical device being
    // NOTE: it must be called before any graphics objects are created
    virtual ResultCode SetDevice(uint32_t deviceId) = 0;

    // Sets the content of the given Image resource
    virtual void SetImageContent(Image& image, size_t byteOffset, void* data, size_t byteSize) = 0;

    // Sets the content of the given Buffer resource
    virtual void SetBufferContent(Buffer& image, size_t byteOffset, void* data, size_t byteSize) = 0;

    // Create a ShaderResourceGroupAllocator
    std::unique_ptr<ShaderResourceGroupAllocator> CreateShaderResourceGroupAllocator();

    // Creates a new Image resource
    std::unique_ptr<Image> CreateImage(const ResourceAllocationInfo& allocationInfo, const ImageCreateInfo& createInfo);

    // Creates a new Buffer resource
    std::unique_ptr<Buffer> CreateBuffer(const ResourceAllocationInfo& allocationInfo, const BufferCreateInfo& createInfo);

    // Creates a new Swapchain
    std::unique_ptr<Swapchain> CreateSwapchain(const SwapchainCreateInfo& createInfo);

    // Creates a new Fence
    std::unique_ptr<Fence> CreateFence();

    // Creates a new Sampler state
    std::unique_ptr<Sampler> CreateSampler(const SamplerCreateInfo& createInfo);

    // Creates a new FrameScheduler
    std::unique_ptr<FrameScheduler> CreateFrameScheduler();

    // Creates a new PipelineState object for graphics
    std::unique_ptr<PipelineState> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);

protected:
    std::vector<DeviceProperties> m_physcialDevicesProperties;
};

}  // namespace RHI