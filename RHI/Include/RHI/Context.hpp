#pragma once

#include "RHI/Common/Handle.hpp"
#include "RHI/Resources.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/FrameScheduler.hpp"
#include "RHI/Export.hpp"
#include "RHI/Common/Allocator.hpp"
#include "RHI/Common/Ptr.h"

#include <string>
#include <string_view>

namespace RHI
{
    using Version = uint32_t;

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
        const char* applicationName;    // The name of the users application.
        Version     applicationVersion; // The version of the users application.
        const char* engineName;         // The version of the users application.
        Version     engineVersion;      // The version of the users application.
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
        virtual ~DebugCallbacks()                               = default;

        /// @brief Log an information.
        virtual void LogInfo(std::string_view message, ...)     = 0;

        /// @brief Log an warnning.
        virtual void LogWarnning(std::string_view message, ...) = 0;

        /// @brief Log an error.
        virtual void LogError(std::string_view message, ...)    = 0;
    };

    /// @brief RHI Context, represent an instance of the API.
    class RHI_EXPORT Context
    {
    public:
        virtual ~Context()                                                                                              = default;

        /// @brief Creates a new Swapchain.
        virtual Ptr<Swapchain>            CreateSwapchain(const SwapchainCreateInfo& createInfo)                        = 0;

        /// @brief Creates a new ShaderModule
        virtual Ptr<ShaderModule>         CreateShaderModule(const ShaderModuleCreateInfo& createInfo)                  = 0;

        /// @brief Creates a fence object.
        virtual Ptr<Fence>                CreateFence()                                                                 = 0;

        /// @brief Creates a Frame Graph Pass object.
        virtual Ptr<Pass>                 CreatePass(const char* name, QueueType type)                                  = 0;

        /// @brief Creates a new FrameScheduler object.
        virtual Ptr<FrameScheduler>       CreateFrameScheduler()                                                        = 0;

        /// @brief Creates a new command list allocator object.
        virtual Ptr<CommandListAllocator> CreateCommandListAllocator(QueueType queueType, uint32_t bufferedFramesCount) = 0;

        /// @brief Creates a shader bind group layout object.
        virtual Handle<BindGroupLayout>   CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)            = 0;

        /// @brief Frees the given shader bind group layout
        virtual void                      DestroyBindGroupLayout(Handle<BindGroupLayout> layout)                        = 0;

        /// @brief Creates a shader bind group layout object.
        virtual Handle<PipelineLayout>    CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)              = 0;

        /// @brief Frees the given shader bind group layout
        virtual void                      DestroyPipelineLayout(Handle<PipelineLayout> layout)                          = 0;

        /// @brief Creates a BindGroupAllocator object.
        virtual Ptr<BindGroupAllocator>   CreateBindGroupAllocator()                                                    = 0;

        /// @brief Creates a new Pool for all buffer resources.
        virtual Ptr<BufferPool>           CreateBufferPool(const PoolCreateInfo& createInfo)                            = 0;

        /// @brief Creates a new Pool for all image resources.
        virtual Ptr<ImagePool>            CreateImagePool(const PoolCreateInfo& createInfo)                             = 0;

        /// @brief Creates a new graphics pipeline state for graphics.
        virtual Handle<GraphicsPipeline>  CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)          = 0;

        /// @brief Frees the given graphics pipeline object.
        virtual void                      DestroyGraphicsPipeline(Handle<GraphicsPipeline> pso)                         = 0;

        /// @brief Creates a new compute pipeline state for graphics.
        virtual Handle<ComputePipeline>   CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)            = 0;

        /// @brief Frees the given compute pipeline object.
        virtual void                      DestroyComputePipeline(Handle<ComputePipeline> pso)                           = 0;

        /// @brief Creates a new Sampler state.
        virtual Handle<Sampler>           CreateSampler(const SamplerCreateInfo& createInfo)                            = 0;

        /// @brief Frees the given sampler object.
        virtual void                      DestroySampler(Handle<Sampler> sampler)                                       = 0;

        /// @brief Creates a new ImageView.
        virtual Handle<ImageView>         CreateImageView(const ImageViewCreateInfo& useInfo)                           = 0;

        /// @brief Frees the given compute pipeline object.
        virtual void                      DestroyImageView(Handle<ImageView> view)                                      = 0;

        /// @brief Creates a new BufferView.
        virtual Handle<BufferView>        CreateBufferView(const BufferViewCreateInfo& useInfo)                         = 0;

        /// @brief Frees the given sampler object.
        virtual void                      DestroyBufferView(Handle<BufferView> view)                                    = 0;

    protected:
        Context(Ptr<DebugCallbacks> debugMessengerCallbacks)
            : m_debugMessenger(std::move(debugMessengerCallbacks))
        {
        }

        Ptr<DebugCallbacks> m_debugMessenger;
        Ptr<Allocator>      m_allocator;
    };

} // namespace RHI