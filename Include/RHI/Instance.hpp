#pragma once
#include "RHI/Debug.hpp"
#include "RHI/Device.hpp"

namespace RHI
{

class IPhysicalDevice;
class ISurface;
class IDevice;

enum class BackendType
{
    Vulkan,
    Null,
};

class IInstance
{
public:
    static Expected<std::unique_ptr<IInstance>> Create(BackendType backend, std::unique_ptr<IDebugCallbacks> callbacks = nullptr);

    virtual ~IInstance() = default;

    uint32_t GetPhysicalDeviceCount() const
    {
        return m_physicalDeviceCount;
    }

    std::vector<const IPhysicalDevice*> GetPhysicalDevices() const;

    virtual Expected<std::unique_ptr<ISurface>> CreateSurface(const struct Win32SurfaceDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IDevice>> CreateDevice(const IPhysicalDevice& physicalDevice) = 0;

protected:
    uint32_t                             m_physicalDeviceCount = 0;
    std::vector<std::unique_ptr<IPhysicalDevice>> m_physicalDevices     = {};
};

}  // namespace RHI
