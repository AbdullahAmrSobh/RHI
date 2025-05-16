#pragma once

#include <RHI/Device.hpp>

#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>

#include <RHI-Vulkan/Loader.hpp>

#include "Common.hpp"
#include "Queue.hpp"
#include "Resources.hpp"

namespace RHI::Vulkan
{
    class BindGroupAllocator;
    class CommandPool;
    class StagingBuffer;

    struct VulkanFunctions
    {
#ifdef RHI_DEBUG
        // VK_EXT_debug_utils
        PFN_vkCmdBeginDebugUtilsLabelEXT    m_vkCmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT      m_vkCmdEndDebugUtilsLabelEXT;
        PFN_vkCmdInsertDebugUtilsLabelEXT   m_vkCmdInsertDebugUtilsLabelEXT;
        PFN_vkCreateDebugUtilsMessengerEXT  m_vkCreateDebugUtilsMessengerEXT;
        PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT;
        PFN_vkQueueBeginDebugUtilsLabelEXT  m_vkQueueBeginDebugUtilsLabelEXT;
        PFN_vkQueueEndDebugUtilsLabelEXT    m_vkQueueEndDebugUtilsLabelEXT;
        PFN_vkQueueInsertDebugUtilsLabelEXT m_vkQueueInsertDebugUtilsLabelEXT;
        PFN_vkSetDebugUtilsObjectNameEXT    m_vkSetDebugUtilsObjectNameEXT;
        PFN_vkSetDebugUtilsObjectTagEXT     m_vkSetDebugUtilsObjectTagEXT;
        PFN_vkSubmitDebugUtilsMessageEXT    m_vkSubmitDebugUtilsMessageEXT;
#endif
        PFN_vkCmdBeginConditionalRenderingEXT m_vkCmdBeginConditionalRenderingEXT;
        PFN_vkCmdEndConditionalRenderingEXT   m_vkCmdEndConditionalRenderingEXT;
    };

    class IDevice final : public Device
    {
    private:
        friend Device* RHI::CreateVulkanDevice(const ApplicationInfo& appInfo);
        friend void    RHI::DestroyVulkanDevice(Device* device);

        IDevice();
        ~IDevice();

    public:
        /// @brief Initializes the device with application-specific information.
        ResultCode Init(const ApplicationInfo& appInfo);

        /// @brief Shuts down the device and releases all associated resources.
        void Shutdown();

        /// @brief Assigns a debug name to a Vulkan object.
        void SetDebugName(VkObjectType type, uint64_t handle, const char* name) const;

        /// @brief Assigns a debug name to a Vulkan object.
        template<typename T>
        void SetDebugName(T handle, const char* name) const;

        template<typename T, typename... FMT_ARGS>
        void SetDebugName(T handle, const char* fmt, FMT_ARGS... args) const;

        IQueue* GetDeviceQueue(QueueType type);

        void WaitIdle() { vkDeviceWaitIdle(m_device); }

        uint64_t GetFrameIndex() const { return m_frameIndex; }

        // Interface Implementation
        Swapchain*               CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void                     DestroySwapchain(Swapchain* swapchain) override;
        ShaderModule*            CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void                     DestroyShaderModule(ShaderModule* shaderModule) override;
        CommandList*             CreateCommandList(const CommandListCreateInfo& createInfo) override;
        void                     DestroyCommandList(CommandList*);
        Handle<BindGroupLayout>  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                     DestroyBindGroupLayout(Handle<BindGroupLayout> handle) override;
        Handle<BindGroup>        CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void                     DestroyBindGroup(Handle<BindGroup> handle) override;
        void                     UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo) override;
        Handle<Buffer>           CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                     DestroyBuffer(Handle<Buffer> handle) override;
        Handle<Image>            CreateImage(const ImageCreateInfo& createInfo) override;
        Handle<Image>            CreateImageView(const ImageViewCreateInfo& createInfo) override;
        Handle<Sampler>          CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                     DestroySampler(Handle<Sampler> handle) override;
        Handle<PipelineLayout>   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                     DestroyPipelineLayout(Handle<PipelineLayout> handle) override;
        Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                     DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle) override;
        Handle<ComputePipeline>  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                     DestroyComputePipeline(Handle<ComputePipeline> handle) override;
        void                     DestroyImage(Handle<Image> handle) override;
        void                     BeginResourceUpdate(RenderGraph* renderGraph) override;
        void                     EndResourceUpdate() override;
        void                     BufferWrite(Handle<Buffer> buffer, size_t offset, TL::Block block) override;
        void                     ImageWrite(Handle<Image> image, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block) override;
        uint64_t                 QueueSubmit(const QueueSubmitInfo& submitInfo) override;

        // clang-format off
        IImage*                  Get(Handle<IImage> handle)                { return m_imageOwner.Get(handle); }
        void                     Release(Handle<IImage> handle)            { return m_imageOwner.Release(handle); }
        IBuffer*                 Get(Handle<IBuffer> handle)               { return m_bufferOwner.Get(handle); }
        void                     Release(Handle<IBuffer> handle)           { return m_bufferOwner.Release(handle); }
        IBindGroupLayout*        Get(Handle<IBindGroupLayout> handle)      { return m_bindGroupLayoutsOwner.Get(handle); }
        void                     Release(Handle<IBindGroupLayout> handle)  { return m_bindGroupLayoutsOwner.Release(handle); }
        IBindGroup*              Get(Handle<IBindGroup> handle)            { return m_bindGroupOwner.Get(handle); }
        void                     Release(Handle<IBindGroup> handle)        { return m_bindGroupOwner.Release(handle); }
        IPipelineLayout*         Get(Handle<IPipelineLayout> handle)       { return m_pipelineLayoutOwner.Get(handle); }
        void                     Release(Handle<IPipelineLayout> handle)   { return m_pipelineLayoutOwner.Release(handle); }
        IGraphicsPipeline*       Get(Handle<IGraphicsPipeline> handle)     { return m_graphicsPipelineOwner.Get(handle); }
        void                     Release(Handle<IGraphicsPipeline> handle) { return m_graphicsPipelineOwner.Release(handle); }
        IComputePipeline*        Get(Handle<IComputePipeline> handle)      { return m_computePipelineOwner.Get(handle); }
        void                     Release(Handle<IComputePipeline> handle)  { return m_computePipelineOwner.Release(handle); }
        ISampler*                Get(Handle<ISampler> handle)              { return m_samplerOwner.Get(handle); }
        void                     Release(Handle<ISampler> handle)          { return m_samplerOwner.Release(handle); }
        // clang-format on

    public:
        // Vulkan instance and core objects
        VkInstance                        m_instance;            ///< Vulkan instance handle.
        VkDebugUtilsMessengerEXT          m_debugUtilsMessenger; ///< Debug messenger for Vulkan validation layers.
        VkPhysicalDevice                  m_physicalDevice;      ///< Physical device selected for use.
        VkDevice                          m_device;              ///< Logical device handle.
        VmaAllocator                      m_deviceAllocator;     ///< Vulkan memory allocator.
        VulkanFunctions                   m_pfn;                 ///< Function pointers for Vulkan extensions and commands.
        // Resource pools
        HandlePool<IImage>                m_imageOwner;
        HandlePool<IBuffer>               m_bufferOwner;
        HandlePool<IBindGroupLayout>      m_bindGroupLayoutsOwner;
        HandlePool<IBindGroup>            m_bindGroupOwner;
        HandlePool<IPipelineLayout>       m_pipelineLayoutOwner;
        HandlePool<IGraphicsPipeline>     m_graphicsPipelineOwner;
        HandlePool<IComputePipeline>      m_computePipelineOwner;
        HandlePool<ISampler>              m_samplerOwner;
        // Queue and allocator management
        IQueue                            m_queue[AsyncQueuesCount]; ///< Array of queues for different operations (e.g., graphics, compute).
        TL::Ptr<class DeleteQueue>        m_destroyQueue;            ///< Queue for handling deferred resource destruction.
        TL::Ptr<class BindGroupAllocator> m_bindGroupAllocator;      ///< Allocator for bind group resources.
        TL::Ptr<class CommandPool>        m_commandsAllocator;       ///< Allocator for Vulkan command buffers.
        TL::Ptr<class StagingBuffer>      m_stagingBuffer;           ///< Allocator for staging buffers.
        // Frame
        std::atomic_uint64_t              m_frameIndex;

        //
        TL::Set<Handle<Buffer>> m_buffersToUnmap;
    };

    template<typename T>
    inline void IDevice::SetDebugName(T handle, const char* name) const
    {
        return SetDebugName(GetObjectType<T>(), reinterpret_cast<uint64_t>(handle), name);
    }

    template<typename T, typename... FMT_ARGS>
    void IDevice::SetDebugName(T handle, const char* fmt, FMT_ARGS... args) const
    {
        auto name = std::vformat(fmt, std::make_format_args(args...));
        SetDebugName(handle, name.c_str());
    }

    inline IQueue* IDevice::GetDeviceQueue(QueueType type)
    {
        auto& queue = m_queue[(uint32_t)type];
        if (queue.GetHandle() != VK_NULL_HANDLE)
        {
            return &m_queue[(int)QueueType::Graphics];
        }
        return nullptr;
    }

} // namespace RHI::Vulkan