#pragma once

#include "RHI/Resources.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/FrameScheduler.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/Export.hpp"

#include "RHI/Common/Ptr.h"
#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Result.hpp"
#include "RHI/Common/Debug.hpp"
#include "RHI/Common/LeakDetector.hpp"

namespace RHI
{
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

    struct Version
    {
        uint16_t major;
        uint16_t minor;
        uint32_t patch;
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
        const char* name;
        DeviceType  type;
        Vendor      vendor;
    };

    struct Limits
    {
        size_t stagingMemoryLimit;
    };

    // This class provides the core interface for interacting with the graphics rendering API.
    // It allows for creating and managing various resources like images, buffers, pipelines,
    // and shaders.
    class RHI_EXPORT Context
    {
        friend class FrameScheduler;

    public:
        virtual ~Context() = default;

        inline Limits&                                  GetLimits() { return *m_limits; }

        /// Returns a reference to the frame scheduler object.
        ///
        /// @return Reference to the FrameScheduler object.
        inline FrameScheduler&                          GetScheduler() { return *m_frameScheduler; }

        /// Creates a new swapchain object with specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired charactersitics of the swapchain object.
        /// @return RAII scoped pointer containing a pointer to the created swapchain
        RHI_NODISCARD virtual Ptr<Swapchain>            CreateSwapchain(const SwapchainCreateInfo& createInfo)               = 0;

        /// Creates a new shader module object with specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired charactersitics of the shader module object.
        /// @return RAII scoped pointer containing a pointer to the created shader module
        RHI_NODISCARD virtual Ptr<ShaderModule>         CreateShaderModule(TL::Span<const uint8_t> shaderBlob)               = 0;

        /// Creates a new fence object with specified configuration.
        ///
        /// @return RAII scoped pointer containing a pointer to the created fence
        RHI_NODISCARD virtual Ptr<Fence>                CreateFence()                                                        = 0;

        /// Creates a new command list allocator object with specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired charactersitics of the command list allocator object.
        /// @return RAII scoped pointer containing a pointer to the created command list allocator
        RHI_NODISCARD virtual Ptr<CommandListAllocator> CreateCommandListAllocator()                                         = 0;

        /// Creates a new resource pool object with specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired charactersitics of the resource pool object.
        /// @return RAII scoped pointer containing a pointer to the created resource pool
        RHI_NODISCARD virtual Ptr<ResourcePool>         CreateResourcePool(const ResourcePoolCreateInfo& createInfo)         = 0;

        /// Creates a new bind group layout object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the bind group layout object.
        /// @return A handle to the created bind group layout object.
        RHI_NODISCARD virtual Handle<BindGroupLayout>   CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)   = 0;

        /// Destroys the specified bind group layout object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the bind group layout object to destroy.
        virtual void                                    DestroyBindGroupLayout(Handle<BindGroupLayout> handle)               = 0;

        /// Creates a new bind group object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the bind group object.
        /// @return A handle to the created bind group object.
        RHI_NODISCARD virtual Handle<BindGroup>         CreateBindGroup(Handle<BindGroupLayout> handle)                      = 0;

        /// Destroys the specified bind group object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the bind group object to destroy.
        virtual void                                    DestroyBindGroup(Handle<BindGroup> handle)                           = 0;

        /// Updates the contents of a bind group with the provided data.
        ///
        /// @param handle The handle to the bind group to update.
        /// @param content The data to update in the bind group, containing binding indices and corresponding data pointers.
        virtual void                                    UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data) = 0;

        /// Creates a new pipeline layout object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the pipeline layout object.
        /// @return A handle to the created pipeline layout object.
        RHI_NODISCARD virtual Handle<PipelineLayout>    CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)     = 0;

        /// Destroys the specified pipeline object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the pipeline layout object to destroy.
        virtual void                                    DestroyPipelineLayout(Handle<PipelineLayout> handle)                 = 0;

        /// Creates a new graphics pipeline object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the graphics pipeline object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created graphics pipeline object.
        RHI_NODISCARD virtual Handle<GraphicsPipeline>  CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;

        /// Destroys the specified graphics pipeline object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the graphics pipeline object to destroy.
        virtual void                                    DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)             = 0;

        /// Creates a new compute pipeline object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the compute pipeline object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created compute pipeline object.
        RHI_NODISCARD virtual Handle<ComputePipeline>   CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)   = 0;

        /// Destroys the specified compute pipeline object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the compute pipeline object to destroy.
        virtual void                                    DestroyComputePipeline(Handle<ComputePipeline> handle)               = 0;

        /// Creates a new sampler object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the sampler object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created sampler object.
        RHI_NODISCARD virtual Handle<Sampler>           CreateSampler(const SamplerCreateInfo& createInfo)                   = 0;

        /// Destroys the specified sampler object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the sampler object to destroy.
        virtual void                                    DestroySampler(Handle<Sampler> handle)                               = 0;

        /// Creates a new image object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the image object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created image object.
        RHI_NODISCARD virtual Result<Handle<Image>>     CreateImage(const ImageCreateInfo& createInfo)                       = 0;

        /// Destroys the specified image object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the image object to destroy.
        virtual void                                    DestroyImage(Handle<Image> handle)                                   = 0;

        /// Creates a new buffer object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the buffer object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created buffer object.
        RHI_NODISCARD virtual Result<Handle<Buffer>>    CreateBuffer(const BufferCreateInfo& createInfo)                     = 0;

        /// Destroys the specified buffer object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the buffer object to destroy.
        virtual void                                    DestroyBuffer(Handle<Buffer> handle)                                 = 0;

        /// Creates a new image view object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the image view object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created image view object.
        RHI_NODISCARD virtual Handle<ImageView>         CreateImageView(const ImageViewCreateInfo& createInfo)               = 0;

        /// Destroys the specified image view object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the image view object to destroy.
        virtual void                                    DestroyImageView(Handle<ImageView> handle)                           = 0;

        /// Creates a new buffer view object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the buffer view object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created buffer view object.
        RHI_NODISCARD virtual Handle<BufferView>        CreateBufferView(const BufferViewCreateInfo& createInfo)             = 0;

        /// Destroys the specified buffer view object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the buffer view object to destroy.
        virtual void                                    DestroyBufferView(Handle<BufferView> handle)                         = 0;

        /// Maps the contents of a buffer into host memory, allowing direct access for reading and writing.
        /// This method is typically used for smaller, frequently accessed buffers to avoid performance overhead
        /// associated with individual read/write operations.
        ///
        /// @param handle The handle to the buffer object to map.
        /// @return A pointer to the mapped memory region, or nullptr on failure.
        RHI_NODISCARD virtual DeviceMemoryPtr           MapBuffer(Handle<Buffer> handle)                                     = 0;

        /// Unmaps a previously mapped buffer, releasing the host memory access previously granted by `MapBuffer`.
        ///
        /// @param handle The handle to the buffer object to unmap.
        virtual void                                    UnmapBuffer(Handle<Buffer> handle)                                   = 0;

    protected:
        Context(Ptr<DebugCallbacks> debugCallbacks);

        virtual void DestroyResources() = 0;

        void         OnDestruct();

        void         DebugLogError(std::string_view message);
        void         DebugLogWarn(std::string_view message);
        void         DebugLogInfo(std::string_view message);

    protected:
        struct
        {
            ResourceLeakDetector<Image>            m_images;
            ResourceLeakDetector<Buffer>           m_buffers;
            ResourceLeakDetector<ImageView>        m_imageViews;
            ResourceLeakDetector<BufferView>       m_bufferViews;
            ResourceLeakDetector<BindGroupLayout>  m_bindGroupLayouts;
            ResourceLeakDetector<BindGroup>        m_bindGroups;
            ResourceLeakDetector<PipelineLayout>   m_pipelineLayouts;
            ResourceLeakDetector<GraphicsPipeline> m_graphicsPipelines;
            ResourceLeakDetector<ComputePipeline>  m_computePipelines;
            ResourceLeakDetector<Sampler>          m_samplers;
        } m_LeakDetector;

        Ptr<FrameScheduler> m_frameScheduler;
        Ptr<Limits>         m_limits;

    private:
        Ptr<DebugCallbacks> m_debugCallbacks;
    };

    inline Context::Context(Ptr<DebugCallbacks> debugCallbacks)
        : m_limits(CreatePtr<Limits>())
        , m_debugCallbacks(std::move(debugCallbacks))
    {
    }

    template<typename T>
    RHI_EXPORT Result<Handle<Image>> CreateImageWithData(Context& context, const ImageCreateInfo& createInfo, TL::Span<const T> content)
    {
        // clang-format off
        auto& scheduler = context.GetScheduler();

        auto [handle, result] = context.CreateImage(createInfo);

        if (result != ResultCode::Success)
            return result;

        RHI::BufferCreateInfo _createInfo {};
        _createInfo.byteSize = content.size_bytes();
        _createInfo.usageFlags = BufferUsage::CopySrc;
        auto tmpBuffer = context.CreateBuffer(_createInfo).GetValue();

        BufferToImageCopyInfo copyInfo{};
        copyInfo.srcBuffer = tmpBuffer;
        copyInfo.srcOffset = 0;
        copyInfo.dstImage  = handle;
        auto ptr = context.MapBuffer(tmpBuffer);
        memcpy(ptr, content.data(), content.size_bytes());
        context.UnmapBuffer(tmpBuffer);

        scheduler.WriteImageContent(handle, {}, createInfo.size, {}, content);


        return handle;
        // clang-format on
    }

    template<typename T>
    RHI_EXPORT Result<Handle<Buffer>> CreateBufferWithData(Context& context, Flags<BufferUsage> usageFlags, TL::Span<const T> content)
    {
        BufferCreateInfo createInfo {};
        createInfo.byteSize = content.size_bytes();
        createInfo.usageFlags = usageFlags;

        auto [handle, result] = context.CreateBuffer(createInfo);

        if (result != ResultCode::Success)
            return result;

        if (content.size_bytes() <= context.GetLimits().stagingMemoryLimit)
        {
            auto ptr = context.MapBuffer(handle);
            memcpy(ptr, content.data(), content.size_bytes());
            context.UnmapBuffer(handle);
        }
        else
        {
            RHI_UNREACHABLE();
        }

        return handle;
    }
} // namespace RHI