#include "DeleteQueue.hpp"
#include "Device.hpp"

#include <TL/Assert.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{

    DeleteQueue::DeleteQueue(IDevice* device)
        : m_device(device)
        , m_semaphores()
        , m_commandPool()
        , m_commandBuffers()
        , m_fences()
        , m_allocations()
        , m_buffers()
        , m_images()
        , m_events()
        , m_queryPools()
        , m_bufferViews()
        , m_imageViews()
        , m_pipelineLayouts()
        , m_pipelines()
        , m_descriptorSetLayouts()
        , m_samplers()
        , m_descriptorPools()
        , m_descriptorSets()
        , m_commandPools()
        , m_surfaces()
        , m_swapchains()
        , m_accelerationStructures()
    {
    }

    DeleteQueue::~DeleteQueue() {}

    void DeleteQueue::DestroyObject(VkSemaphore semaphore)
    {
        m_semaphores.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = semaphore});
    }

    void DeleteQueue::DestroyObject(VkCommandPool commandPool)
    {
        m_commandPools.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = commandPool});
    }

    void DeleteQueue::DestroyObject(VkCommandPool commandPool, TL::Span<const VkCommandBuffer> commandBuffer)
    {
        m_commandBuffers.push_back(
            {.timelineValue = m_device->GetTimelineValue(), .pool = commandPool, .resource = {commandBuffer.begin(), commandBuffer.end()}});
    }

    void DeleteQueue::DestroyObject(VkFence fence)
    {
        m_fences.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = fence});
    }

    void DeleteQueue::DestroyObject(VmaAllocation allocation)
    {
        m_allocations.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = allocation});
    }

    void DeleteQueue::DestroyObject(VkBuffer buffer)
    {
        m_buffers.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = buffer});
    }

    void DeleteQueue::DestroyObject(VkImage image)
    {
        m_images.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = image});
    }

    void DeleteQueue::DestroyObject(VkEvent event)
    {
        m_events.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = event});
    }

    void DeleteQueue::DestroyObject(VkQueryPool queryPool)
    {
        m_queryPools.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = queryPool});
    }

    void DeleteQueue::DestroyObject(VkBufferView bufferView)
    {
        m_bufferViews.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = bufferView});
    }

    void DeleteQueue::DestroyObject(VkImageView imageView)
    {
        m_imageViews.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = imageView});
    }

    void DeleteQueue::DestroyObject(VkPipelineLayout pipelineLayout)
    {
        m_pipelineLayouts.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = pipelineLayout});
    }

    void DeleteQueue::DestroyObject(VkPipeline pipeline)
    {
        m_pipelines.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = pipeline});
    }

    void DeleteQueue::DestroyObject(VkDescriptorSetLayout descriptorSetLayout)
    {
        m_descriptorSetLayouts.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = descriptorSetLayout});
    }

    void DeleteQueue::DestroyObject(VkSampler sampler)
    {
        m_samplers.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = sampler});
    }

    void DeleteQueue::DestroyObject(VkDescriptorPool descriptorPool)
    {
        m_descriptorPools.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = descriptorPool});
    }

    void DeleteQueue::DestroyObject(VkDescriptorPool descriptorPool, TL::Span<const VkDescriptorSet> descriptorSet)
    {
        m_descriptorSets.push_back(
            {.timelineValue = m_device->GetTimelineValue(),
             .pool          = descriptorPool,
             .resource      = {descriptorSet.begin(), descriptorSet.end()}});
    }

    void DeleteQueue::DestroyObject(VkSurfaceKHR surface)
    {
        m_surfaces.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = surface});
    }

    void DeleteQueue::DestroyObject(VkSwapchainKHR swapchain)
    {
        m_swapchains.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = swapchain});
    }

    void DeleteQueue::DestroyObject(VkAccelerationStructureKHR accelerationStructure)
    {
        m_accelerationStructures.push_back({.timelineValue = m_device->GetTimelineValue(), .resource = accelerationStructure});
    }

    void DeleteQueue::DestroyQueued()
    {
        ZoneScoped;

        uint64_t timelineGpuValue = 0;
        vkGetSemaphoreCounterValue(m_device->m_device, m_device->GetTimelineSemaphore(), &timelineGpuValue);

        // Destroy semaphores
        for (auto semaphore : m_semaphores)
        {
            if (semaphore.timelineValue < timelineGpuValue)
            {
                vkDestroySemaphore(m_device->m_device, semaphore.resource, nullptr);
            }
        }

        // Destroy command pools
        for (auto commandPool : m_commandPools)
        {
            if (commandPool.timelineValue < timelineGpuValue)
            {
                vkDestroyCommandPool(m_device->m_device, commandPool.resource, nullptr);
            }
        }

        // Destroy command buffers
        for (auto commandBuffers : m_commandBuffers)
        {
            if (commandBuffers.timelineValue < timelineGpuValue)
            {
                vkFreeCommandBuffers(
                    m_device->m_device, commandBuffers.pool, (uint32_t)commandBuffers.resource.size(), commandBuffers.resource.data());
            }
        }

        // Destroy fences
        for (auto fence : m_fences)
        {
            if (fence.timelineValue < timelineGpuValue)
            {
                vkDestroyFence(m_device->m_device, fence.resource, nullptr);
            }
        }

        // Free VMA allocations
        for (auto allocation : m_allocations)
        {
            if (allocation.timelineValue < timelineGpuValue)
            {
                vmaFreeMemory(m_device->m_allocator, allocation.resource);
            }
        }

        // Destroy buffers
        for (auto buffer : m_buffers)
        {
            if (buffer.timelineValue < timelineGpuValue)
            {
                vkDestroyBuffer(m_device->m_device, buffer.resource, nullptr);
            }
        }

        // Destroy images
        for (auto image : m_images)
        {
            if (image.timelineValue < timelineGpuValue)
            {
                vkDestroyImage(m_device->m_device, image.resource, nullptr);
            }
        }

        // Destroy events
        for (auto event : m_events)
        {
            if (event.timelineValue < timelineGpuValue)
            {
                vkDestroyEvent(m_device->m_device, event.resource, nullptr);
            }
        }

        // Destroy query pools
        for (auto queryPool : m_queryPools)
        {
            if (queryPool.timelineValue < timelineGpuValue)
            {
                vkDestroyQueryPool(m_device->m_device, queryPool.resource, nullptr);
            }
        }

        // Destroy buffer views
        for (auto bufferView : m_bufferViews)
        {
            if (bufferView.timelineValue < timelineGpuValue)
            {
                vkDestroyBufferView(m_device->m_device, bufferView.resource, nullptr);
            }
        }

        // Destroy image views
        for (auto imageView : m_imageViews)
        {
            if (imageView.timelineValue < timelineGpuValue)
            {
                vkDestroyImageView(m_device->m_device, imageView.resource, nullptr);
            }
        }

        // Destroy pipeline layouts
        for (auto pipelineLayout : m_pipelineLayouts)
        {
            if (pipelineLayout.timelineValue < timelineGpuValue)
            {
                vkDestroyPipelineLayout(m_device->m_device, pipelineLayout.resource, nullptr);
            }
        }

        // Destroy pipelines
        for (auto pipeline : m_pipelines)
        {
            if (pipeline.timelineValue < timelineGpuValue)
            {
                vkDestroyPipeline(m_device->m_device, pipeline.resource, nullptr);
            }
        }

        // Destroy descriptor set layouts
        for (auto descriptorSetLayout : m_descriptorSetLayouts)
        {
            if (descriptorSetLayout.timelineValue < timelineGpuValue)
            {
                vkDestroyDescriptorSetLayout(m_device->m_device, descriptorSetLayout.resource, nullptr);
            }
        }

        // Destroy samplers
        for (auto sampler : m_samplers)
        {
            if (sampler.timelineValue < timelineGpuValue)
            {
                vkDestroySampler(m_device->m_device, sampler.resource, nullptr);
            }
        }

        // Destroy descriptor pools
        for (auto descriptorPool : m_descriptorPools)
        {
            if (descriptorPool.timelineValue < timelineGpuValue)
            {
                vkDestroyDescriptorPool(m_device->m_device, descriptorPool.resource, nullptr);
            }
        }

        // Destroy descriptor sets
        for (auto descriptorSets : m_descriptorSets)
        {
            if (descriptorSets.timelineValue < timelineGpuValue)
            {
                vkFreeDescriptorSets(
                    m_device->m_device, descriptorSets.pool, (uint32_t)descriptorSets.resource.size(), descriptorSets.resource.data());
            }
        }

        // Destroy surfaces
        for (auto surface : m_surfaces)
        {
            if (surface.timelineValue < timelineGpuValue)
            {
                vkDestroySurfaceKHR(m_device->m_instance, surface.resource, nullptr);
            }
        }

        // Destroy swapchains
        for (auto swapchain : m_swapchains)
        {
            if (swapchain.timelineValue < timelineGpuValue)
            {
                vkDestroySwapchainKHR(m_device->m_device, swapchain.resource, nullptr);
            }
        }

        // Destroy acceleration structures (if using Vulkan ray tracing extensions)
        // for (auto accelerationStructure : m_accelerationStructures)
        // {
        //     if (accelerationStructure.timelineValue < m_device->TimelineGetCurrentGPUValue())
        //     {
        //         vkDestroyAccelerationStructureKHR(m_device->m_device, accelerationStructure.resource, nullptr);
        //     }
        // }
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