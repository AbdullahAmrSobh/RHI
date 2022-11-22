#pragma once
#include "RHI/Instance.hpp"
#include "Backend/Vulkan/Common.hpp"

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

#ifdef RHI_LINUX
        virtual Expected<Unique<ISurface>> CreateSurface(const struct X11SurfaceDesc& desc) override;
#elif defined(RHI_WINDOWS)
        virtual Expected<Unique<ISurface>> CreateSurface(const struct Win32SurfaceDesc& desc) override;
#endif
        virtual Expected<Unique<IDevice>> CreateDevice(const IPhysicalDevice& physicalDevice) override;
    
    private:
        VkInstance               m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
    };

} // namespace Vulkan
} // namespace RHI