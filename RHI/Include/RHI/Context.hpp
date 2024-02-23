#pragma once
#include "RHI/Resources.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/FrameScheduler.hpp"
#include "RHI/Export.hpp"

#include "RHI/Common/Ptr.h"
#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Result.hpp"
#include "RHI/Common/Debug.hpp"

namespace RHI
{
    struct StagingBuffer;

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

    // This class provides the core interface for interacting with the graphics rendering API.
    // It allows for creating and managing various resources like images, buffers, pipelines,
    // and shaders.
    class RHI_EXPORT Context
    {
    public:
        virtual ~Context() = default;

        /// Returns a reference to the frame scheduler object.
        ///
        /// @return Reference to the FrameScheduler object.
        /// @const If called through a const reference to the Context object,
        ///        returns a const reference to the FrameScheduler.
        inline FrameScheduler&            GetScheduler() { return *m_frameScheduler; }

        /// Creates an image object and copies the provided content into it.
        ///
        /// @param createInfo Information describing the desired image characteristics.
        /// @param content The raw data to be copied into the image.
        /// @return Result object indicating success or failure of the operation.
        ///         On success, it also contains a handle to the created image.
        Result<Handle<Image>>             CreateImageWithContent(const ImageCreateInfo& createInfo, TL::Span<uint8_t> content);

        /// Creates a buffer object and copies the provided content into it.
        ///
        /// @param createInfo Information describing the desired buffer characteristics.
        /// @param content The raw data to be copied into the buffer.
        /// @return Result object indicating success or failure of the operation.
        ///         On success, it also contains a handle to the created buffer.
        Result<Handle<Buffer>>            CreateBufferWithContent(const BufferCreateInfo& createInfo, TL::Span<uint8_t> content);

        /// Reads data from an image object into a provided memory location.
        ///
        /// @param handle The handle to the image object to read from.
        /// @param offset The starting offset within the image data to read from.
        /// @param size The size of the data to read in bytes.
        /// @param signalFence (Optional) A fence to be signaled once the read operation completes.
        /// @param outPtr The pointer to the memory location to write the read data to.
        /// @return ResultCode indicating success or failure of the operation.
        ResultCode                        ReadImageContent(Handle<Image> handle, ImageOffset offset, ImageSize3D size, Fence& signalFence, void* outPtr);

        /// Reads data from an image object into a provided memory location.
        ///
        /// @param handle The handle to the image object to read from.
        /// @param offset The starting offset within the image data to read from.
        /// @param size The size of the data to read in bytes.
        /// @param signalFence (Optional) A fence to be signaled once the read operation completes.
        /// @param outPtr The pointer to the memory location to write the read data to.
        /// @return ResultCode indicating success or failure of the operation.
        ResultCode                        ReadBufferContent(Handle<Buffer> handle, size_t offset, size_t range, Fence& signalFence, void* outPtr);

        /// Creates a new swapchain object with specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired charactersitics of the swapchain object.
        /// @return RAII scoped pointer containing a pointer to the created swapchain
        virtual Ptr<Swapchain>            CreateSwapchain(const SwapchainCreateInfo& createInfo)               = 0;

        /// Creates a new shader module object with specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired charactersitics of the shader module object.
        /// @return RAII scoped pointer containing a pointer to the created shader module
        virtual Ptr<ShaderModule>         CreateShaderModule(TL::Span<const uint8_t> shaderBlob)               = 0;

        /// Creates a new fence object with specified configuration.
        ///
        /// @return RAII scoped pointer containing a pointer to the created fence
        virtual Ptr<Fence>                CreateFence()                                                        = 0;

        /// Creates a new command list allocator object with specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired charactersitics of the command list allocator object.
        /// @return RAII scoped pointer containing a pointer to the created command list allocator
        virtual Ptr<CommandListAllocator> CreateCommandListAllocator(QueueType queueType)                      = 0;

        /// Creates a new resource pool object with specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired charactersitics of the resource pool object.
        /// @return RAII scoped pointer containing a pointer to the created resource pool
        virtual Ptr<ResourcePool>         CreateResourcePool(const ResourcePoolCreateInfo& createInfo)         = 0;

        /// Creates a new bind group layout object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the bind group layout object.
        /// @return A handle to the created bind group layout object.
        virtual Handle<BindGroupLayout>   CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)   = 0;

        /// Destroys the specified bind group layout object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the bind group layout object to destroy.
        virtual void                      DestroyBindGroupLayout(Handle<BindGroupLayout> handle)               = 0;

        /// Creates a new bind group object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the bind group object.
        /// @return A handle to the created bind group object.
        virtual Handle<BindGroup>         CreateBindGroup(Handle<BindGroupLayout> handle)                      = 0;

        /// Destroys the specified bind group object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the bind group object to destroy.
        virtual void                      DestroyBindGroup(Handle<BindGroup> handle)                           = 0;

        /// Updates the contents of a bind group with the provided data.
        ///
        /// @todo: Should this method allows modifying individual bindings within a bind group without creating a new one?
        /// It's efficient for updating frequently changing data like uniforms or textures within a bind group.
        ///
        /// @param handle The handle to the bind group to update.
        /// @param content The data to update in the bind group, containing binding indices and corresponding data pointers.
        virtual void                      UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data) = 0;

        /// Creates a new pipeline layout object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the pipeline layout object.
        /// @return A handle to the created pipeline layout object.
        virtual Handle<PipelineLayout>    CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)     = 0;

        /// Destroys the specified pipeline object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the pipeline layout object to destroy.
        virtual void                      DestroyPipelineLayout(Handle<PipelineLayout> handle)                 = 0;

        /// Creates a new graphics pipeline object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the graphics pipeline object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created graphics pipeline object.
        virtual Handle<GraphicsPipeline>  CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;

        /// Destroys the specified graphics pipeline object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the graphics pipeline object to destroy.
        virtual void                      DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)             = 0;

        /// Creates a new compute pipeline object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the compute pipeline object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created compute pipeline object.
        virtual Handle<ComputePipeline>   CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)   = 0;

        /// Destroys the specified compute pipeline object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the compute pipeline object to destroy.
        virtual void                      DestroyComputePipeline(Handle<ComputePipeline> handle)               = 0;

        /// Creates a new sampler object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the sampler object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created sampler object.
        virtual Handle<Sampler>           CreateSampler(const SamplerCreateInfo& createInfo)                   = 0;

        /// Destroys the specified sampler object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the sampler object to destroy.
        virtual void                      DestroySampler(Handle<Sampler> handle)                               = 0;

        /// Creates a new image object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the image object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created image object.
        virtual Result<Handle<Image>>     CreateImage(const ImageCreateInfo& createInfo)                       = 0;

        /// Destroys the specified image object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the image object to destroy.
        virtual void                      DestroyImage(Handle<Image> handle)                                   = 0;

        /// Creates a new buffer object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the buffer object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created buffer object.
        virtual Result<Handle<Buffer>>    CreateBuffer(const BufferCreateInfo& createInfo)                     = 0;

        /// Destroys the specified buffer object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the buffer object to destroy.
        virtual void                      DestroyBuffer(Handle<Buffer> handle)                                 = 0;

        /// Creates a new image view object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the image view object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created image view object.
        virtual Handle<ImageView>         CreateImageView(const ImageViewCreateInfo& createInfo)               = 0;

        /// Destroys the specified image view object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the image view object to destroy.
        virtual void                      DestroyImageView(Handle<ImageView> handle)                           = 0;

        /// Creates a new buffer view object with the specified configuration.
        ///
        /// @param createInfo [in] Information describing the desired characteristics of the buffer view object.
        /// @return A Result object indicating success or failure.
        ///         On success, it also contains a handle to the created buffer view object.
        virtual Handle<BufferView>        CreateBufferView(const BufferViewCreateInfo& createInfo)             = 0;

        /// Destroys the specified buffer view object and releases its associated resources.
        ///
        /// @param handle [in] The handle to the buffer view object to destroy.
        virtual void                      DestroyBufferView(Handle<BufferView> handle)                         = 0;

        /// Maps the contents of a buffer into host memory, allowing direct access for reading and writing.
        /// This method is typically used for smaller, frequently accessed buffers to avoid performance overhead
        /// associated with individual read/write operations.
        ///
        /// @param handle The handle to the buffer object to map.
        /// @return A pointer to the mapped memory region, or nullptr on failure.
        virtual DeviceMemoryPtr           MapBuffer(Handle<Buffer> handle)                                     = 0;

        /// Unmaps a previously mapped buffer, releasing the host memory access previously granted by `MapBuffer`.
        ///
        /// @param handle The handle to the buffer object to unmap.
        virtual void                      UnmapBuffer(Handle<Buffer> handle)                                   = 0;

    protected:
        void   DebugLogError(std::string_view message);
        void   DebugLogWarn(std::string_view message);
        void   DebugLogInfo(std::string_view message);

        size_t CalculateRequiredSize(const ImageCreateInfo& createInfo);
        size_t CalculateRequiredSize(const BufferCreateInfo& createInfo);

    protected:
        Context(Ptr<DebugCallbacks> debugCallbacks);

        Ptr<FrameScheduler>       m_frameScheduler;
        Ptr<StagingBuffer>        m_stagingBuffer;
        Ptr<CommandListAllocator> m_transferCommandsAllocator;

    private:
        Ptr<DebugCallbacks> m_debugCallbacks;
    };

    inline Context::Context(Ptr<DebugCallbacks> debugCallbacks)
        : m_debugCallbacks(std::move(debugCallbacks))
    {
    }

} // namespace RHI