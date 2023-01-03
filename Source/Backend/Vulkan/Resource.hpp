#pragma once
#include "RHI/Resource.hpp"

#include "Backend/Vulkan/DeviceObject.hpp"

namespace RHI
{
namespace Vulkan
{

template<typename T>
class Resource : public DeviceObject<T>
{
public:
    Resource(Device& device, T handle = VK_NULL_HANDLE, VmaAllocation allocation = VK_NULL_HANDLE)
        : DeviceObject<T>(device, handle)
        , m_allocation(allocation)
    {
    }

    VmaAllocation GetAllocation() const
    {
        return m_allocation;
    }

    const VmaAllocationInfo& GetAllocationInfo() const
    {
        return m_allocationInfo;
    }

protected:
    VmaAllocation     m_allocation = VK_NULL_HANDLE;
    VmaAllocationInfo m_allocationInfo;
};

class ShaderModule final
    : public IShaderProgram
    , public DeviceObject<VkShaderModule>
{
public:
    ShaderModule(Device& device, std::string name)
        : IShaderProgram(name)
        , DeviceObject(device)
    {
    }
    ~ShaderModule();

    VkResult Init(const ShaderProgramDesc& desc);
};

class Fence final
    : public IFence
    , public DeviceObject<VkFence>
{
public:
    Fence(Device& device)
        : DeviceObject(device)
    {
    }
    ~Fence();

    VkResult Init();

    ResultCode Wait() const override;
    ResultCode Reset() const override;
    ResultCode GetStatus() const override;
};

class Sampler final
    : public ISampler
    , public DeviceObject<VkSampler>
{
public:
    Sampler(Device& device)
        : DeviceObject(device)
    {
    }
    ~Sampler();

    VkResult Init(const SamplerDesc& desc);
};

class Semaphore final : public DeviceObject<VkSemaphore>
{
public:
    Semaphore(Device& device, VkSemaphoreCreateFlags flags = 0);
    ~Semaphore();

    static Unique<Semaphore> Create(Device& device, VkSemaphoreCreateFlags flags = 0)
    {
        return CreateUnique<Semaphore>(device, flags);
    }
};

}  // namespace Vulkan
}  // namespace RHI