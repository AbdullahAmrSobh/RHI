#pragma once
#include <algorithm>
#include <string_view>
#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Device.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace RHI
{

class IPhysicalDevice;
class ISurface;
class IDevice;

enum class EDebugMessageSeverity
{
    Log,
    Warn,
    Error,
    Fatel,
};

class IDebugCallbacks
{
public:
    virtual ~IDebugCallbacks() = default;

    virtual void Log(std::string_view message)   = 0;
    virtual void Warn(std::string_view message)  = 0;
    virtual void Error(std::string_view message) = 0;
    virtual void Fatel(std::string_view message) = 0;
};

enum class EBackend
{
    Vulkan,
    Null,
};

class IInstance
{
public:
    static Expected<Unique<IInstance>> Create(EBackend backend, Unique<IDebugCallbacks> callbacks = nullptr);

    virtual ~IInstance() = default;

    inline uint32_t GetPhysicalDeviceCount() const
    {
        return m_physicalDeviceCount;
    }

    std::vector<IPhysicalDevice*> GetPhysicalDevice() const;

#ifdef RHI_LINUX
    virtual Expected<Unique<ISurface>> CreateSurface(const struct X11SurfaceDesc& desc) = 0;
#elif defined(RHI_WINDOWS)
    virtual Expected<Unique<ISurface>> CreateSurface(const struct Win32SurfaceDesc& desc) = 0;
#endif

    virtual Expected<Unique<IDevice>> CreateDevice(const IPhysicalDevice& physicalDevice) = 0;

protected:
    uint32_t                             m_physicalDeviceCount;
    std::vector<Unique<IPhysicalDevice>> m_physicalDevices;
    Unique<IDebugCallbacks>              m_debugCallbacks;
};

} // namespace RHI
