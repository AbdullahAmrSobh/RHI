#pragma once

#include "RHI/Debug.hpp"

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT RHI_ASSERT

#include "RHI/Backend/Vulkan/vma/vk_mem_alloc.h"

#include "vulkan/vulkan.hpp"

namespace Vulkan
{

template<typename Handle>
class DeviceObject
{
public:
    DeviceObject() = default;

    DeviceObject(Handle handle)
        : m_handle(handle)
    {
    }

    virtual ~DeviceObject() = default;

    const Handle GetHandle() const
    {
        return m_handle;
    }

    Handle GetHandle()
    {
        return m_handle;
    }

protected:
    Handle m_handle;
};

}  // namespace Vulkan