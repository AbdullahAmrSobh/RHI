#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    class IContext;

    class DeleteQueue
    {
    public:
        DeleteQueue(IContext* context);
        void Shutdown();

        void DestroyObject(VkSemaphore semaphore);
        void DestroyObject(VkCommandPool commandPool);
        void DestroyObject(VkCommandPool commandPool, VkCommandBuffer commandBuffer);
        void DestroyObject(VkFence fence);
        void DestroyObject(VmaAllocation allocation);
        void DestroyObject(VkBuffer buffer);
        void DestroyObject(VkImage image);
        void DestroyObject(VkEvent event);
        void DestroyObject(VkQueryPool queryPool);
        void DestroyObject(VkBufferView bufferView);
        void DestroyObject(VkImageView imageView);
        void DestroyObject(VkPipelineLayout pipelineLayout);
        void DestroyObject(VkPipeline pipeline);
        void DestroyObject(VkDescriptorSetLayout descriptorSetLayout);
        void DestroyObject(VkSampler sampler);
        void DestroyObject(VkDescriptorPool descriptorPool);
        void DestroyObject(VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet);
        void DestroyObject(VkSurfaceKHR surface);
        void DestroyObject(VkSwapchainKHR swapchain);
        void DestroyObject(VkAccelerationStructureKHR accelerationStructure);

        void ExecuteDeletions();

    private:
        struct PerFrameInflightQueue
        {
            PerFrameInflightQueue() = default;
            void ExecuteDeletions(IContext* context);

            TL::Vector<VkSemaphore> m_semaphores;
            TL::Vector<VkCommandPool> m_commandPool;
            TL::UnorderedMap<VkCommandPool, TL::Vector<VkCommandBuffer>> m_commandBuffers;
            TL::Vector<VkFence> m_fences;
            TL::Vector<VmaAllocation> m_allocations;
            TL::Vector<VkBuffer> m_buffers;
            TL::Vector<VkImage> m_images;
            TL::Vector<VkEvent> m_events;
            TL::Vector<VkQueryPool> m_queryPools;
            TL::Vector<VkBufferView> m_bufferViews;
            TL::Vector<VkImageView> m_imageViews;
            TL::Vector<VkPipelineLayout> m_pipelineLayouts;
            TL::Vector<VkPipeline> m_pipelines;
            TL::Vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
            TL::Vector<VkSampler> m_samplers;
            TL::Vector<VkDescriptorPool> m_descriptorPools;
            TL::UnorderedMap<VkDescriptorPool, TL::Vector<VkDescriptorSet>> m_descriptorSets;
            TL::Vector<VkCommandPool> m_commandPools;
            TL::Vector<VkSurfaceKHR> m_surfaces;
            TL::Vector<VkSwapchainKHR> m_swapchains;
            TL::Vector<VkAccelerationStructureKHR> m_accelerationStructures;
        };

        IContext* m_context;
        uint32_t m_currentFrameIndex;
        uint32_t m_maxFramesCount;
        PerFrameInflightQueue m_deleteQueues[Swapchain::MaxImageCount];

        PerFrameInflightQueue& GetCurrentQueue()
        {
            return m_deleteQueues[m_currentFrameIndex];
        }
    };

} // namespace RHI::Vulkan