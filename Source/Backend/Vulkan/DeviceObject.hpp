#pragma once
#include "Backend/Vulkan/Vma/vk_mem_alloc.hpp"

namespace RHI
{
namespace Vulkan
{
class Device;

template<typename T>
class DeviceObject
{
public:
    DeviceObject(Device& device, T handle = VK_NULL_HANDLE)
        : m_device(&device)
        , m_handle(handle)
    {
    }

    T GetHandle() const
    {
        return m_handle;
    }

protected:
    Device* m_device;
    T       m_handle;
};

}  // namespace Vulkan
}  // namespace RHI