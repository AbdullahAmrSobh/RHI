#pragma once

#include "RHI/Export.hpp"
#include "RHI/Common.hpp"
#include "RHI/Resources.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/Swapchain.hpp"

#include <TL/Span.hpp>
#include <TL/UniquePtr.hpp>
#include <TL/Library.hpp>

namespace RHI
{
    enum class BackendType
    {
        Vulkan1_3,
        DirectX12_2,
        WebGPU,
    };

    enum class NativeHandleType
    {
        None,
        Device,
        CommandList,
        Buffer,
        Image,
        ImageView,
        Sampler,
        ShaderModule,
        Pipeline,
        PipelineLayout,
        BindGroupLayout,
        BindGroup,
        Swapchain,
    };

    struct RenderGraphCreateInfo;

    struct Version
    {
        uint16_t major = 0;
        uint16_t minor = 0;
        uint32_t patch = 0;
    };

    struct ApplicationInfo
    {
        const char* applicationName    = nullptr; // The name of the users application.
        Version     applicationVersion = {};      // The version of the users application.
        const char* engineName         = nullptr; // The version of the users application.
        Version     engineVersion      = {};      // The version of the users application.
    };

    struct DeviceLimits
    {
        // TODO: remove defaults
        uint32_t minUniformBufferOffsetAlignment = 256;
    };

    struct QueueWaitInfo
    {
        QueueType     queueType;
        uint64_t      timelineValue;
        PipelineStage waitStage;
    };

    struct SwapchainImageAcquireInfo
    {
        Swapchain*    swapchain;
        PipelineStage waitStage;
    };

    struct QueueSubmitInfo
    {
        TL::Span<CommandList* const>  commandLists  = {};
        PipelineStage                 signalStage   = {};
        TL::Span<const QueueWaitInfo> waitInfos     = {};
        bool                          signalPresent = false;
    };

    struct BufferStreamInfo
    {
        Buffer*   buffer;
        size_t    offset;
        TL::Block block;
    };

    struct ImageStreamInfo
    {
        Image*        image;
        ImageOffset3D offset;
        ImageSize3D   size;
        uint32_t      mipLevel;
        uint32_t      arrayLayer;
        TL::Block     block;
    };

    /// @brief Provides an interface to the Renderdoc graphics debugger for frame capture and debugging.
    class RHI_EXPORT Renderdoc
    {
    public:
        RHI_INTERFACE_BOILERPLATE(Renderdoc);

        /// @brief Initializes the Renderdoc interface for the given device.
        /// @param device The device to associate with Renderdoc.
        /// @return ResultCode indicating success or failure.
        ResultCode Init(class Device* device);

        /// @brief Shuts down the Renderdoc interface and releases resources.
        void       Shutdown();

        /// @brief Triggers multi-frame capture for the specified number of frames.
        /// @param numFrames Number of frames to capture.
        void       FrameTriggerMultiCapture(uint32_t numFrames);

        /// @brief Checks if Renderdoc is currently capturing a frame.
        /// @return True if capturing, false otherwise.
        bool       FrameIsCapturing();

        /// @brief Starts a frame capture.
        void       FrameStartCapture();

        /// @brief Ends a frame capture.
        void       FrameEndCapture();

    private:
        Device*     m_device;       ///< Associated device.
        TL::Library m_library;      ///< Handle to the Renderdoc library.
        void*       m_renderdocAPi; ///< Pointer to the Renderdoc API interface.
    };

    /// @brief Represents a single frame in flight, providing per-frame resource management and command submission.
    class RHI_EXPORT Frame
    {
    public:
        RHI_INTERFACE_BOILERPLATE(Frame);

        /// @brief Returns the arena allocator valid for the duration of the current frame.
        virtual TL::IAllocator& GetAllocator() = 0;

        /// @brief Marks the next frame to be captured through Renderdoc.
        /// @todo Rename to RdocCaptureNextFrame().
        virtual void            CaptureNextFrame() = 0;

        /// @brief Begins the frame.
        virtual void            Begin(TL::Span<SwapchainImageAcquireInfo> swapchainToAcquire) = 0;

        /// @brief Ends the frame.
        virtual void            End() = 0;

        /// @brief Allocates a command list valid only for the duration of the current frame.
        virtual CommandList*    CreateCommandList(const CommandListCreateInfo& createInfo) = 0;

        /// @brief Submits commands to be executed on the specified queue.
        virtual uint64_t        QueueSubmit(QueueType queueType, const QueueSubmitInfo& submitInfo) = 0;

        /// @brief Schedules a buffer update operation.
        virtual void            BufferWrite(Buffer* buffer, size_t offset, TL::Block block) = 0;

        /// @brief Schedules an image update operation.
        virtual void            ImageWrite(Image* image, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block) = 0;
    };

    /// @brief Represents a logical rendering device and provides resource creation and management.
    class RHI_EXPORT Device
    {
    public:
        RHI_INTERFACE_BOILERPLATE(Device);

        /// @brief Returns the backend type.
        BackendType               GetBackend() const { return m_backend; }

        /// @brief Returns the Renderdoc debug interface, if available.
        Renderdoc*                GetDebugRenderdoc() const { return m_renderdoc.get(); }

        /// @brief Returns device limits.
        DeviceLimits              GetLimits() const;

        /// @brief Creates a render graph.
        RenderGraph*              CreateRenderGraph(const RenderGraphCreateInfo& createInfo);

        /// @brief Destroys a render graph.
        void                      DestroyRenderGraph(RenderGraph* renderGraph);

        /// @brief Retrieves a native handle for the specified type and object.
        virtual uint64_t          GetNativeHandle(NativeHandleType type, uint64_t handle) = 0;

        /// @brief Creates a swapchain.
        virtual Swapchain*        CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;

        /// @brief Destroys a swapchain.
        virtual void              DestroySwapchain(Swapchain* swapchain) = 0;

        /// @brief Creates a shader module.
        virtual ShaderModule*     CreateShaderModule(const ShaderModuleCreateInfo& createInfo) = 0;

        /// @brief Destroys a shader module.
        virtual void              DestroyShaderModule(ShaderModule* shaderModule) = 0;

        /// @brief Creates a bind group layout.
        virtual BindGroupLayout*  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) = 0;

        /// @brief Destroys a bind group layout.
        virtual void              DestroyBindGroupLayout(BindGroupLayout* handle) = 0;

        /// @brief Creates a bind group.
        virtual BindGroup*        CreateBindGroup(const BindGroupCreateInfo& createInfo) = 0;

        /// @brief Destroys a bind group.
        virtual void              DestroyBindGroup(BindGroup* handle) = 0;

        /// @brief Updates a bind group.
        virtual void              UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) = 0;

        /// @brief Creates a buffer.
        virtual Buffer*           CreateBuffer(const BufferCreateInfo& createInfo) = 0;

        /// @brief Destroys a buffer.
        virtual void              DestroyBuffer(Buffer* handle) = 0;

        /// @brief Creates an image.
        virtual Image*            CreateImage(const ImageCreateInfo& createInfo) = 0;

        /// @brief Creates an image view.
        virtual Image*            CreateImageView(const ImageViewCreateInfo& createInfo) = 0;

        /// @brief Destroys an image or image view.
        virtual void              DestroyImage(Image* handle) = 0;

        /// @brief Creates a sampler.
        virtual Sampler*          CreateSampler(const SamplerCreateInfo& createInfo) = 0;

        /// @brief Destroys a sampler.
        virtual void              DestroySampler(Sampler* handle) = 0;

        /// @brief Creates a pipeline layout.
        virtual PipelineLayout*   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) = 0;

        /// @brief Destroys a pipeline layout.
        virtual void              DestroyPipelineLayout(PipelineLayout* handle) = 0;

        /// @brief Creates a graphics pipeline.
        virtual GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;

        /// @brief Destroys a graphics pipeline.
        virtual void              DestroyGraphicsPipeline(GraphicsPipeline* handle) = 0;

        /// @brief Creates a compute pipeline.
        virtual ComputePipeline*  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;

        /// @brief Destroys a compute pipeline.
        virtual void              DestroyComputePipeline(ComputePipeline* handle) = 0;

        /// @brief Sets the number of frames in flight.
        virtual ResultCode        SetFramesInFlightCount(uint32_t count) = 0;

        /// @brief Returns the current frame.
        virtual Frame*            GetCurrentFrame() = 0;

    protected:
        BackendType           m_backend;   ///< Backend type used by this device.
        TL::Ptr<DeviceLimits> m_limits;    ///< Device limits.
        TL::Ptr<Renderdoc>    m_renderdoc; ///< Optional Renderdoc interface.
    };

    RHI_EXPORT Image* CreateImageWithContent(Device& device, const ImageCreateInfo& createInfo, TL::Block content);
} // namespace RHI