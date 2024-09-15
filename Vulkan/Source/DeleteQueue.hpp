#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <functional>

namespace RHI::Vulkan
{
    class IContext;

    class DeleteQueue
    {
    public:
        DeleteQueue();
        ~DeleteQueue();

        void DeferCommand(std::function<void()> callback);

        void Flush();

    private:
        TL::Vector<std::function<void()>> m_callbacks;

        // TL::Vector<VkSemaphore> m_semaphores;
        // TL::Vector<VkCommandBuffer> m_commandBuffers;
        // TL::Vector<VkFence> m_fences;
        // TL::Vector<VmaAllocation> m_allocations;
        // TL::Vector<VkBuffer> m_buffers;
        // TL::Vector<VkImage> m_images;
        // TL::Vector<VkEvent> m_events;
        // TL::Vector<VkQueryPool> m_queryPools;
        // TL::Vector<VkBufferView> m_bufferViews;
        // TL::Vector<VkImageView> m_imageViews;
        // TL::Vector<VkPipelineLayout> m_pipelineLayouts;
        // TL::Vector<VkPipeline> m_pipelines;
        // TL::Vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        // TL::Vector<VkSampler> m_samplers;
        // TL::Vector<VkDescriptorPool> m_descriptorPools;
        // TL::Vector<VkDescriptorSet> m_descriptorSets;
        // TL::Vector<VkCommandPool> m_commandPools;
        // TL::Vector<VkSurfaceKHR> m_surfaces;
        // TL::Vector<VkSwapchainKHR> m_swapchains;
        // TL::Vector<VkAccelerationStructureKHR> m_accelerationStructures;
    };

    class FrameExecuteContext
    {
    public:
        FrameExecuteContext(IContext* context)
            : m_context(context)
        {
        }

        void AdvanceFrame();

        uint32_t GetFrameIndex() const;

        uint32_t GetNextFrameIndex() const;

        void DeferCommand(std::function<void()> callback);
        void DeferNextFrame(std::function<void()> callback);

    // private:
        IContext* m_context;

        uint32_t m_frameIndex;

        struct Data
        {
            DeleteQueue m_deleteQueue;
        } m_frame[2];

        const auto& CurrentFrame() const { return m_frame[m_frameIndex]; }

        auto& CurrentFrame() { return m_frame[m_frameIndex]; }
    };

} // namespace RHI::Vulkan