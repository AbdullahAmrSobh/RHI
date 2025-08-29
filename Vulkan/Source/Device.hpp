#pragma once

#include "Common.hpp"
#include "Queue.hpp"
#include "Resources.hpp"

#include <RHI/Device.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>

namespace RHI::Vulkan
{
    class BindGroupAllocator;

    struct VulkanAPI
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
        ResultCode Init(const ApplicationInfo& appInfo);
        void       Shutdown();

        /// @brief Assigns a debug name to a Vulkan object.
        void SetDebugName(VkObjectType type, uint64_t handle, const char* name) const;
        template<typename T>
        void SetDebugName(T handle, const char* name) const;
        template<typename T, typename... FMT_ARGS>
        void SetDebugName(T handle, const char* fmt, FMT_ARGS... args) const;

        IQueue* GetDeviceQueue(QueueType type);

        void WaitIdle();

        // Interface Implementation
        uint64_t          GetNativeHandle(NativeHandleType type, uint64_t handle) override;
        Swapchain*        CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void              DestroySwapchain(Swapchain* swapchain) override;
        ShaderModule*     CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void              DestroyShaderModule(ShaderModule* shaderModule) override;
        BindGroupLayout*  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void              DestroyBindGroupLayout(BindGroupLayout* handle) override;
        BindGroup*        CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void              DestroyBindGroup(BindGroup* handle) override;
        void              UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) override;
        Buffer*           CreateBuffer(const BufferCreateInfo& createInfo) override;
        void              DestroyBuffer(Buffer* handle) override;
        Image*            CreateImage(const ImageCreateInfo& createInfo) override;
        Image*            CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void              DestroyImage(Image* handle) override;
        Sampler*          CreateSampler(const SamplerCreateInfo& createInfo) override;
        void              DestroySampler(Sampler* handle) override;
        PipelineLayout*   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void              DestroyPipelineLayout(PipelineLayout* handle) override;
        GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void              DestroyGraphicsPipeline(GraphicsPipeline* handle) override;
        ComputePipeline*  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void              DestroyComputePipeline(ComputePipeline* handle) override;
        ResultCode        SetFramesInFlightCount(uint32_t count) override;
        Frame*            GetCurrentFrame() override;

        /// Frame
        TL::IAllocator& GetTempAllocator();

        ///

    public:
        // Vulkan instance and core objects
        VkInstance               m_instance;
        VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
        VkPhysicalDevice         m_physicalDevice;
        VkDevice                 m_device;
        VmaAllocator             m_deviceAllocator;
        VulkanAPI                m_pfn;
        TL::Ptr<Renderdoc>       m_renderdoc;

        // Queue and allocator management
        IQueue                            m_queue[AsyncQueuesCount];
        TL::Ptr<class DeleteQueue>        m_destroyQueue;
        TL::Ptr<class BindGroupAllocator> m_bindGroupAllocator;

        // Frames in flight
        uint32_t                          m_currentFrameIndex;
        TL::Vector<TL::Ptr<class IFrame>> m_framesInFlight;
        IFrame*                           m_activeFrame;
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