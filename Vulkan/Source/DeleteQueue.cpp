#include "DeleteQueue.hpp"
#include "Context.hpp"

#include <TL/Assert.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{

    DeleteQueue::DeleteQueue(IContext* context)
        : m_context(context)
        , m_currentFrameIndex(0)
        , m_maxFramesCount(Swapchain::MaxImageCount)
        , m_deleteQueues()
    {
    }

    void DeleteQueue::Shutdown()
    {
        for (auto& deleteQueue : m_deleteQueues)
            deleteQueue.ExecuteDeletions(m_context);
    }

    void DeleteQueue::DestroyObject(VkSemaphore semaphore)
    {
        GetCurrentQueue().m_semaphores.push_back(semaphore);
    }

    void DeleteQueue::DestroyObject(VkCommandPool commandPool)
    {
        GetCurrentQueue().m_commandPools.push_back(commandPool);
    }

    void DeleteQueue::DestroyObject(VkCommandPool commandPool, VkCommandBuffer commandBuffer)
    {
        GetCurrentQueue().m_commandBuffers[commandPool].push_back(commandBuffer);
    }

    void DeleteQueue::DestroyObject(VkFence fence)
    {
        GetCurrentQueue().m_fences.push_back(fence);
    }

    void DeleteQueue::DestroyObject(VmaAllocation allocation)
    {
        GetCurrentQueue().m_allocations.push_back(allocation);
    }

    void DeleteQueue::DestroyObject(VkBuffer buffer)
    {
        GetCurrentQueue().m_buffers.push_back(buffer);
    }

    void DeleteQueue::DestroyObject(VkImage image)
    {
        GetCurrentQueue().m_images.push_back(image);
    }

    void DeleteQueue::DestroyObject(VkEvent event)
    {
        GetCurrentQueue().m_events.push_back(event);
    }

    void DeleteQueue::DestroyObject(VkQueryPool queryPool)
    {
        GetCurrentQueue().m_queryPools.push_back(queryPool);
    }

    void DeleteQueue::DestroyObject(VkBufferView bufferView)
    {
        GetCurrentQueue().m_bufferViews.push_back(bufferView);
    }

    void DeleteQueue::DestroyObject(VkImageView imageView)
    {
        GetCurrentQueue().m_imageViews.push_back(imageView);
    }

    void DeleteQueue::DestroyObject(VkPipelineLayout pipelineLayout)
    {
        GetCurrentQueue().m_pipelineLayouts.push_back(pipelineLayout);
    }

    void DeleteQueue::DestroyObject(VkPipeline pipeline)
    {
        GetCurrentQueue().m_pipelines.push_back(pipeline);
    }

    void DeleteQueue::DestroyObject(VkDescriptorSetLayout descriptorSetLayout)
    {
        GetCurrentQueue().m_descriptorSetLayouts.push_back(descriptorSetLayout);
    }

    void DeleteQueue::DestroyObject(VkSampler sampler)
    {
        GetCurrentQueue().m_samplers.push_back(sampler);
    }

    void DeleteQueue::DestroyObject(VkDescriptorPool descriptorPool)
    {
        GetCurrentQueue().m_descriptorPools.push_back(descriptorPool);
    }

    void DeleteQueue::DestroyObject(VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet)
    {
        GetCurrentQueue().m_descriptorSets[descriptorPool].push_back(descriptorSet);
    }

    void DeleteQueue::DestroyObject(VkSurfaceKHR surface)
    {
        GetCurrentQueue().m_surfaces.push_back(surface);
    }

    void DeleteQueue::DestroyObject(VkSwapchainKHR swapchain)
    {
        GetCurrentQueue().m_swapchains.push_back(swapchain);
    }

    void DeleteQueue::DestroyObject(VkAccelerationStructureKHR accelerationStructure)
    {
        GetCurrentQueue().m_accelerationStructures.push_back(accelerationStructure);
    }

    void DeleteQueue::ExecuteDeletions()
    {
        ZoneScoped;
        GetCurrentQueue().ExecuteDeletions(m_context);
        m_currentFrameIndex = (m_currentFrameIndex + 1) % m_maxFramesCount;
    }

    void DeleteQueue::PerFrameInflightQueue::ExecuteDeletions(IContext* context)
    {
        // Destroy semaphores
        for (auto semaphore : m_semaphores)
            vkDestroySemaphore(context->m_device, semaphore, nullptr);

        // Destroy command pools
        for (auto commandPool : m_commandPools)
            vkDestroyCommandPool(context->m_device, commandPool, nullptr);

        // Destroy command buffers
        for (auto [commandPool, commandBuffers] : m_commandBuffers)
            vkFreeCommandBuffers(context->m_device, commandPool, (uint32_t)commandBuffers.size(), commandBuffers.data());

        // Destroy fences
        for (auto fence : m_fences)
            vkDestroyFence(context->m_device, fence, nullptr);

        // Free VMA allocations
        for (auto allocation : m_allocations)
            vmaFreeMemory(context->m_allocator, allocation);

        // Destroy buffers
        for (auto buffer : m_buffers)
            vkDestroyBuffer(context->m_device, buffer, nullptr);

        // Destroy images
        for (auto image : m_images)
            vkDestroyImage(context->m_device, image, nullptr);

        // Destroy events
        for (auto event : m_events)
            vkDestroyEvent(context->m_device, event, nullptr);

        // Destroy query pools
        for (auto queryPool : m_queryPools)
            vkDestroyQueryPool(context->m_device, queryPool, nullptr);

        // Destroy buffer views
        for (auto bufferView : m_bufferViews)
            vkDestroyBufferView(context->m_device, bufferView, nullptr);

        // Destroy image views
        for (auto imageView : m_imageViews)
            vkDestroyImageView(context->m_device, imageView, nullptr);

        // Destroy pipeline layouts
        for (auto pipelineLayout : m_pipelineLayouts)
            vkDestroyPipelineLayout(context->m_device, pipelineLayout, nullptr);

        // Destroy pipelines
        for (auto pipeline : m_pipelines)
            vkDestroyPipeline(context->m_device, pipeline, nullptr);

        // Destroy descriptor set layouts
        for (auto descriptorSetLayout : m_descriptorSetLayouts)
            vkDestroyDescriptorSetLayout(context->m_device, descriptorSetLayout, nullptr);

        // Destroy samplers
        for (auto sampler : m_samplers)
            vkDestroySampler(context->m_device, sampler, nullptr);

        // Destroy descriptor pools
        for (auto descriptorPool : m_descriptorPools)
            vkDestroyDescriptorPool(context->m_device, descriptorPool, nullptr);

        // Destroy descriptor sets
        for (auto [descriptorPool, descriptorSets] : m_descriptorSets)
            vkFreeDescriptorSets(context->m_device, descriptorPool, (uint32_t)descriptorSets.size(), descriptorSets.data());

        // Destroy surfaces
        for (auto surface : m_surfaces)
            vkDestroySurfaceKHR(context->m_instance, surface, nullptr);

        // Destroy swapchains
        for (auto swapchain : m_swapchains)
            vkDestroySwapchainKHR(context->m_device, swapchain, nullptr);

        // Destroy acceleration structures (if using Vulkan ray tracing extensions)
        // for (auto accelerationStructure : m_accelerationStructures)
        //     vkDestroyAccelerationStructureKHR(context->m_device, accelerationStructure, nullptr);
        TL_ASSERT(m_accelerationStructures.empty());

        m_semaphores.clear();
        m_commandPool.clear();
        m_commandBuffers.clear();
        m_fences.clear();
        m_allocations.clear();
        m_buffers.clear();
        m_images.clear();
        m_events.clear();
        m_queryPools.clear();
        m_bufferViews.clear();
        m_imageViews.clear();
        m_pipelineLayouts.clear();
        m_pipelines.clear();
        m_descriptorSetLayouts.clear();
        m_samplers.clear();
        m_descriptorPools.clear();
        m_descriptorSets.clear();
        m_commandPools.clear();
        m_surfaces.clear();
        m_swapchains.clear();
        m_accelerationStructures.clear();
    }
} // namespace RHI::Vulkan