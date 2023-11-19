#pragma once

#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace RHI
{
    enum class MemoryAllocationFlags;

    // Forward decelerations
    struct ShaderModuleCreateInfo;
    struct SwapchainCreateInfo;
    struct ResourcePoolCreateInfo;
    struct ImageCreateInfo;
    struct BufferCreateInfo;
    struct GraphicsPipelineCreateInfo;
    struct ComputePipelineCreateInfo;
    struct SamplerCreateInfo;
    struct PassCreateInfo;
    struct DeviceProperties;
    struct ImageAttachmentUseInfo;
    struct BufferAttachmentUseInfo;

    struct Image;
    struct Buffer;
    struct ImageView;
    struct BufferView;
    struct GraphicsPipeline;
    struct ComputePipeline;
    struct Sampler;

    class ShaderModule;
    class Swapchain;
    class ResourcePool;
    class Pass;
    class FrameScheduler;
    class ShaderBindGroupAllocator;

    typedef uint32_t Version;
    typedef void*    DeviceMemoryPtr;

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
        CPU,
        Integerated,
        Dedicated,
        Virtual
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
        uint32_t    id;
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
        virtual void LogInfo(std::string_view message, ...) = 0;

        /// @brief Log an warnning.
        virtual void LogWarnning(std::string_view message, ...) = 0;

        /// @brief Log an error.
        virtual void LogError(std::string_view message, ...) = 0;
    };

    /// @brief RHI Context, represent an instance of the API.
    class RHI_EXPORT Context
    {
    public:
        Context()          = default;
        virtual ~Context() = default;

        DebugCallbacks& GetDebugMessenger() const;

        /// @brief Creates a new ShaderModule
        virtual std::unique_ptr<ShaderModule> CreateShaderModule(const ShaderModuleCreateInfo& createInfo) = 0;

        /// @brief Creates a new Swapchain.
        virtual std::unique_ptr<Swapchain> CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;

        /// @brief Creates a new Pool for all resources.
        virtual std::unique_ptr<ResourcePool> CreateResourcePool(const ResourcePoolCreateInfo& createInfo) = 0;

        /// @brief Creates a new graphics pipeline state for graphics.
        virtual Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;

        /// @brief Frees the given graphics pipeline object.
        virtual void Free(Handle<GraphicsPipeline> pso) = 0;

        /// @brief Creates a new compute pipeline state for graphics.
        virtual Handle<ComputePipeline> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;

        /// @brief Creates a new Sampler state.
        virtual Handle<Sampler> CreateSampler(const SamplerCreateInfo& createInfo) = 0;

        /// @brief Creates a new ImageView.
        virtual Handle<ImageView> CreateImageView(Handle<Image> handle, const ImageAttachmentUseInfo& useInfo) = 0;

        /// @brief Creates a new BufferView.
        virtual Handle<BufferView> CreateBufferView(Handle<Buffer> handle, const BufferAttachmentUseInfo& useInfo) = 0;

        /// @brief Creates a new FrameScheduler object.
        virtual std::unique_ptr<FrameScheduler> CreateFrameScheduler() = 0;

        /// @brief Creates a ShaderBindGroupAllocator object.
        virtual std::unique_ptr<ShaderBindGroupAllocator> CreateShaderBindGroupAllocator() = 0;

        /// @brief Maps the image resource for read or write operations.
        /// @return returns a pointer to GPU memory, or a nullptr in case of failure
        virtual DeviceMemoryPtr MapResource(Handle<Image> image) = 0;

        /// @brief Maps the buffer resource for read or write operations.
        /// @return returns a pointer to GPU memory, or a nullptr in case of failure
        virtual DeviceMemoryPtr MapResource(Handle<Buffer> buffer) = 0;

        /// @brief Unamps the image resource.
        virtual void Unmap(Handle<Image> image) = 0;

        /// @brief Unmaps the buffer resource.
        virtual void Unmap(Handle<Buffer> buffer) = 0;

        /// @brief Frees the given compute pipeline object.
        virtual void Free(Handle<ComputePipeline> pso) = 0;

        /// @brief Frees the given sampler object.
        virtual void Free(Handle<Sampler> sampler) = 0;

        /// @brief Frees the given compute pipeline object.
        virtual void Free(Handle<ImageView> view) = 0;

        /// @brief Frees the given sampler object.
        virtual void Free(Handle<BufferView> view) = 0;
    };

} // namespace RHI