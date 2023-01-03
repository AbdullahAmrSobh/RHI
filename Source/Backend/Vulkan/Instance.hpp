#pragma once
#include "RHI/Instance.hpp"

#include "Backend/Vulkan/DeviceObject.hpp"

namespace RHI
{
namespace Vulkan
{

class Instance final : public IInstance
{
public:
    ~Instance();

    VkResult Init();

    inline VkInstance GetHandle() const
    {
        return m_instance;
    }

    virtual Expected<Unique<ISurface>> CreateSurface(const struct Win32SurfaceDesc& desc) override;

    virtual Expected<Unique<IDevice>> CreateDevice(const IPhysicalDevice& physicalDevice) override;

private:
    VkInstance               m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
};

}  // namespace Vulkan
}  // namespace RHI