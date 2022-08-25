#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/Common.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

#include "RHI/Backend/Vulkan/RenderPass.hpp"
#include "RHI/Backend/Vulkan/SwapChain.hpp"

namespace RHI
{
namespace Vulkan
{
    static std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        return devices;
    }

    VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (pUserData)
        {
            IDebugMessenger* pCallback = reinterpret_cast<IDebugMessenger*>(pUserData);
            switch (messageSeverity)
            {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: pCallback->Info(pCallbackData->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: pCallback->Warn(pCallbackData->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: pCallback->Error(pCallbackData->pMessage); break;
            default: pCallback->Error(pCallbackData->pMessage); break;
            }
        }
        return VK_FALSE;
    }

    Factory::~Factory()
    {
        auto pfnDestroyDebugUtilsMessenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));

        pfnDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);

        vkDestroyInstance(m_instance, nullptr);
    }

    EResultCode Factory::Init(VkApplicationInfo _appInfo, IDebugMessenger& debugMessengerCallback)
    {

        // Initalize a vulkan instance.
        {
            _appInfo.apiVersion = VK_API_VERSION_1_2;

            std::vector<const char*> layers     = {"VK_LAYER_KHRONOS_validation"};
            std::vector<const char*> extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"};

            VkInstanceCreateInfo createInfo    = {};
            createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pNext                   = nullptr;
            createInfo.flags                   = 0;
            createInfo.pApplicationInfo        = &_appInfo;
            createInfo.enabledLayerCount       = static_cast<uint32_t>(layers.size());
            createInfo.ppEnabledLayerNames     = layers.data();
            createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
            if (result != VK_SUCCESS)
                return ToResultCode(result);
        }

        // Initalize a vulkan debug messenger callback.
        {
            m_pDebugMessengerCallback = &debugMessengerCallback;

            auto pfnCreateDebugUtilsMessenger =
                reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));

            VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
            createInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.pNext                              = nullptr;
            createInfo.flags                              = 0;
            createInfo.messageSeverity                    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            createInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = DebugCallback;
            createInfo.pUserData       = &m_pDebugMessengerCallback;

            VkResult result = pfnCreateDebugUtilsMessenger(m_instance, &createInfo, nullptr, &m_debugMessenger);
            if (result != VK_SUCCESS)
                return ToResultCode(result);
        }

        // Create a Device.
        m_device        = CreateUnique<Device>();
        VkResult result = m_device->Init(m_instance, GetPhysicalDevices(m_instance).front());
        return ToResultCode(result);
    }

} // namespace Vulkan
} // namespace RHI
