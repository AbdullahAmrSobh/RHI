#pragma once
#include "Backend/Vulkan/Vma/vk_mem_alloc.hpp"

namespace RHI
{
namespace Vulkan
{
class Device;

template<typename HandleType>
class DeviceObject
{
public:
    DeviceObject(Device& device, HandleType handle = VK_NULL_HANDLE)
        : m_device(&device)
        , m_handle(handle)
    {
    }

    HandleType GetHandle() const
    {
        return m_handle;
    }

protected:
    Device*    m_device;
    HandleType m_handle;
};

}  // namespace Vulkan
}  // namespace RHI