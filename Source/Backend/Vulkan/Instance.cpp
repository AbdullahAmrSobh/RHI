#include "RHI/Common.hpp"
#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Instance.hpp"
#include "Backend/Vulkan/Swapchain.hpp"

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

    static VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        auto callback = reinterpret_cast<IDebugCallbacks*>(pUserData);
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            callback->Log(pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            callback->Warn(pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            callback->Error(pCallbackData->pMessage);
        }
        else
        {
            callback->Fatel(pCallbackData->pMessage);
        }
        
        return VK_FALSE;
    }
    
    Instance::~Instance()
    {
        vkDestroyInstance(m_instance, nullptr);
    }

    VkResult Instance::Init()
    {
        VkApplicationInfo appInfo  = {};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext              = 0;
        appInfo.pApplicationName   = "RHI Application";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.pEngineName        = "Engine Application";
        appInfo.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
        appInfo.apiVersion         = VK_API_VERSION_1_3;

        std::vector<const char*> enabledLayers;
        std::vector<const char*> enabledExtensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_xlib_surface"};
        
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        if (m_debugCallbacks != nullptr)
        {
            enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
            enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            
            debugCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.pNext           = nullptr;
            debugCreateInfo.flags           = 0;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT;
            debugCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
            debugCreateInfo.pUserData       = m_debugCallbacks.get();
        }
        
        VkInstanceCreateInfo createInfo    = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext                   = m_debugCallbacks != nullptr ? nullptr : &debugCreateInfo;
        createInfo.flags                   = 0;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledLayerCount       = CountElements(enabledLayers);
        createInfo.ppEnabledLayerNames     = enabledLayers.data();
        createInfo.enabledExtensionCount   = CountElements(enabledExtensions);
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        
        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
        
        for (VkPhysicalDevice physicalDevice : GetPhysicalDevices(m_instance))
        {
            m_physicalDevices.push_back(std::move(CreateUnique<PhysicalDevice>(physicalDevice)));    
        }
        
        return result;
    }
    
    Expected<Unique<IDevice>> Instance::CreateDevice(const IPhysicalDevice& physicalDevice)
    {
        auto     device = CreateUnique<Device>();
        VkResult result = device->Init(*this, static_cast<const PhysicalDevice&>(physicalDevice));
        if (result != VK_SUCCESS)
        {
            return std::move(device);
        }
        else
        {
            return Unexpected(ConvertResult(result));
        }
    }

} // namespace Vulkan
} // namespace RHI