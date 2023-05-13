#pragma once

#include "RHI/Backend/Vulkan/Vulkan.hpp"
#include "RHI/Context.hpp"

namespace RHI
{
struct ShaderResourceGroupLayout;
}

namespace Vulkan
{

class RHI_EXPORT Context final : public RHI::Context
{
public:
    Context()
        : m_surfaceCache(3)
        , m_descriptorSetLayoutCache(16)
        , m_pipelineLayoutCache(16)
    {
    }

    ~Context();

    RHI::ResultCode Init(const RHI::ApplicationInfo& appInfo) override;
    RHI::ResultCode SetDevice(uint32_t device_id) override;

    void SetImageContent(RHI::Image& image, size_t byteOffset, void* data, size_t byteSize) override;
    void SetBufferContent(RHI::Buffer& image, size_t byteOffset, void* data, size_t byteSize) override;

    vk::Device GetDevice()
    {
        return m_device;
    }

    vk::PhysicalDevice GetPhysicalDevice()
    {
        return m_physicalDevice;
    }

    VmaAllocator GetAllocator()
    {
        return m_allocator;
    }

    std::shared_ptr<vk::UniqueSurfaceKHR> CreateSurface(void* windowHandle);

public:
    vk::Queue m_graphicsQueue;
    vk::Queue m_computeQueue;
    vk::Queue m_transferQueue;

private:
    vk::Instance       m_instance;
    vk::PhysicalDevice m_physicalDevice;
    vk::Device         m_device;
    VmaAllocator       m_allocator;

    RHI::LRUCache<vk::UniqueSurfaceKHR>          m_surfaceCache;
    RHI::LRUCache<vk::UniqueDescriptorSetLayout> m_descriptorSetLayoutCache;
    RHI::LRUCache<vk::UniquePipelineLayout>      m_pipelineLayoutCache;
};

}  // namespace Vulkan