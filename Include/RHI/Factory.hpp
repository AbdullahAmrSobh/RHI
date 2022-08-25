#pragma once
#include "RHI/Common.hpp"

namespace RHI
{

enum class EDebugMessageSeverity
{
    Log,
    Warn,
    Error,
    Fatel,
};

class IDebugMessenger
{
public:
    virtual ~IDebugMessenger() = default;
    
    virtual void Log(std::string_view message)  = 0;
    virtual void Warn(std::string_view message)  = 0;
    virtual void Error(std::string_view message) = 0;
    virtual void Fatel(std::string_view message) = 0;
};

enum class EBackend
{
    Vulkan,
    Null,
};

class IFactory
{
public:
    static Expected<Unique<Instance>> Create(EBackend backend);
    
    virtual ~IFactory() = default;

    virtual EResultCode EnableDebugCallbacks(DebugCallbackInterface* callback) = 0;

    virtual Expected<Unique<Surface>> CreateSurface(const Win32SurfaceDesc& desc) = 0;
    virtual uint32_t                GetPhysicalDeviceCount() const              = 0;
    virtual const PhysicalDevice&   GetPhysicalDevice(uint32_t index = 0) const = 0;
    virtual Expected<Unique<Device>>  CreateDevice()
};

} // namespace RHI
