#pragma once

#include <RHI/Device.hpp>
#include <RHI/Queue.hpp>

#include <TL/Containers.hpp>

#include <RHI-Vulkan/Loader.hpp>

#include "BindGroup.hpp"
#include "Buffer.hpp"
#include "Common.hpp"
#include "DeleteQueue.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "Queue.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"

namespace RHI::Vulkan
{
    class BindGroupAllocator;
    class CommandAllocator;
    class StagingBufferAllocator;

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

        IQueue& GetDeviceQueue(QueueType type);

        // Interface Implementation
        RenderGraph*             CreateRenderGraph() override;
        void                     DestroyRenderGraph(RenderGraph* renderGraph) override;
        Swapchain*               CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void                     DestroySwapchain(Swapchain* swapchain) override;
        TL::Ptr<ShaderModule>    CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        CommandList*             CreateCommandList(const CommandListCreateInfo& createInfo) override;
        Handle<BindGroupLayout>  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                     DestroyBindGroupLayout(Handle<BindGroupLayout> handle) override;
        Handle<BindGroup>        CreateBindGroup(Handle<BindGroupLayout> handle) override;
        void                     DestroyBindGroup(Handle<BindGroup> handle) override;
        void                     UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo) override;
        Handle<PipelineLayout>   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                     DestroyPipelineLayout(Handle<PipelineLayout> handle) override;
        Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                     DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle) override;
        Handle<ComputePipeline>  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                     DestroyComputePipeline(Handle<ComputePipeline> handle) override;
        Handle<Sampler>          CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                     DestroySampler(Handle<Sampler> handle) override;
        Result<Handle<Image>>    CreateImage(const ImageCreateInfo& createInfo) override;
        void                     DestroyImage(Handle<Image> handle) override;
        Result<Handle<Buffer>>   CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                     DestroyBuffer(Handle<Buffer> handle) override;
        DeviceMemoryPtr          MapBuffer(Handle<Buffer> handle) override;
        void                     UnmapBuffer(Handle<Buffer> handle) override;
        StagingBuffer            StagingAllocate(size_t size) override;
        uint64_t                 UploadImage(const ImageUploadInfo& uploadInfo) override;
        void                     CollectResources() override;

    public:
        /// @todo: everything here should be made private
        TL::Arena                                     m_tempAllocator = TL::Arena();
        // Vulkan instance and core objects
        VkInstance                                    m_instance;            ///< Vulkan instance handle.
        VkDebugUtilsMessengerEXT                      m_debugUtilsMessenger; ///< Debug messenger for Vulkan validation layers.
        VkPhysicalDevice                              m_physicalDevice;      ///< Physical device selected for use.
        VkDevice                                      m_device;              ///< Logical device handle.
        VmaAllocator                                  m_deviceAllocator;     ///< Vulkan memory allocator.
        VulkanFunctions                               m_pfn;                 ///< Function pointers for Vulkan extensions and commands.
        // Resource pools
        HandlePool<IImage>                            m_imageOwner;            ///< Pool for managing image resource handles.
        HandlePool<IBuffer>                           m_bufferOwner;           ///< Pool for managing buffer resource handles.
        HandlePool<IBindGroupLayout>                  m_bindGroupLayoutsOwner; ///< Pool for managing bind group layout handles.
        HandlePool<IBindGroup>                        m_bindGroupOwner;        ///< Pool for managing bind group handles.
        HandlePool<IPipelineLayout>                   m_pipelineLayoutOwner;   ///< Pool for managing pipeline layout handles.
        HandlePool<IGraphicsPipeline>                 m_graphicsPipelineOwner; ///< Pool for managing graphics pipeline handles.
        HandlePool<IComputePipeline>                  m_computePipelineOwner;  ///< Pool for managing compute pipeline handles.
        HandlePool<ISampler>                          m_samplerOwner;          ///< Pool for managing sampler handles.
        // Queue and allocator management
        std::array<TL::Ptr<IQueue>, AsyncQueuesCount> m_queue;              ///< Array of queues for different operations (e.g., graphics, compute).
        TL::Ptr<DeleteQueue>                          m_destroyQueue;       ///< Queue for handling deferred resource destruction.
        TL::Ptr<BindGroupAllocator>                   m_bindGroupAllocator; ///< Allocator for bind group resources.
        TL::Ptr<CommandAllocator>                     m_commandsAllocator;  ///< Allocator for Vulkan command buffers.
        TL::Ptr<StagingBufferAllocator>               m_stagingAllocator;   ///< Allocator for staging buffers.
        // Frame
        std::atomic_uint64_t                          m_frameIndex;
    };

    template<typename T>
    inline void IDevice::SetDebugName(T handle, const char* name) const
    {
        return SetDebugName(GetObjectType<T>(), reinterpret_cast<uint64_t>(handle), name);
    }

    inline IQueue& IDevice::GetDeviceQueue(QueueType type)
    {
        return *m_queue[(uint32_t)type].get();
    }

} // namespace RHI::Vulkan