#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{

Expected<FactoryPtr> IFactory::Create(EBackendType _type, IDebugMessenger& debugMessengerCallback)
{
    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "VulkanRHI";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "VulkanRHI";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_2;

    auto factory = CreateUnique<Vulkan::Factory>();
    auto result  = factory->Init(appInfo, debugMessengerCallback);

    if (result != EResultCode::Success)
        return tl::unexpected(result);
    
    return factory;
}

} // namespace RHI
