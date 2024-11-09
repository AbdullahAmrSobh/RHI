#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>

#include "Common.hpp"

namespace RHI::Vulkan
{
    class IDevice;

    class DeleteQueue
    {
    public:
        DeleteQueue(IDevice* device);
        ~DeleteQueue();

        void DestroyObject(VkSemaphore semaphore);
        void DestroyObject(VkCommandPool commandPool);
        void DestroyObject(VkCommandPool commandPool, TL::Span<const VkCommandBuffer> commandBuffer);
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
        void DestroyObject(VkDescriptorPool descriptorPool, TL::Span<const VkDescriptorSet> descriptorSet);
        void DestroyObject(VkSurfaceKHR surface);
        void DestroyObject(VkSwapchainKHR swapchain);
        void DestroyObject(VkAccelerationStructureKHR accelerationStructure);

        void DestroyQueued();

    private:
        template<typename HandleType>
        struct DestroyResource
        {
            uint64_t   timelineValue;
            HandleType resource;
        };

        template<typename T, typename U>
        struct DestroyPooledResource
        {
            uint64_t      timelineValue;
            T             pool;
            TL::Vector<U> resource;
        };

        IDevice* m_device;

        TL::Vector<DestroyResource<VkSemaphore>>                             m_semaphores;
        TL::Vector<DestroyResource<VkCommandPool>>                           m_commandPool;
        TL::Vector<DestroyPooledResource<VkCommandPool, VkCommandBuffer>>    m_commandBuffers;
        TL::Vector<DestroyResource<VkFence>>                                 m_fences;
        TL::Vector<DestroyResource<VmaAllocation>>                           m_allocations;
        TL::Vector<DestroyResource<VkBuffer>>                                m_buffers;
        TL::Vector<DestroyResource<VkImage>>                                 m_images;
        TL::Vector<DestroyResource<VkEvent>>                                 m_events;
        TL::Vector<DestroyResource<VkQueryPool>>                             m_queryPools;
        TL::Vector<DestroyResource<VkBufferView>>                            m_bufferViews;
        TL::Vector<DestroyResource<VkImageView>>                             m_imageViews;
        TL::Vector<DestroyResource<VkPipelineLayout>>                        m_pipelineLayouts;
        TL::Vector<DestroyResource<VkPipeline>>                              m_pipelines;
        TL::Vector<DestroyResource<VkDescriptorSetLayout>>                   m_descriptorSetLayouts;
        TL::Vector<DestroyResource<VkSampler>>                               m_samplers;
        TL::Vector<DestroyResource<VkDescriptorPool>>                        m_descriptorPools;
        TL::Vector<DestroyPooledResource<VkDescriptorPool, VkDescriptorSet>> m_descriptorSets;
        TL::Vector<DestroyResource<VkCommandPool>>                           m_commandPools;
        TL::Vector<DestroyResource<VkSurfaceKHR>>                            m_surfaces;
        TL::Vector<DestroyResource<VkSwapchainKHR>>                          m_swapchains;
        TL::Vector<DestroyResource<VkAccelerationStructureKHR>>              m_accelerationStructures;
    };

} // namespace RHI::Vulkan