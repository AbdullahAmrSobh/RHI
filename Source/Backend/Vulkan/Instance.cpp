#include "RHI/Pch.hpp"
#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Instance.hpp"

#include "Backend/Vulkan/Device.hpp"

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

static VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                       void*                                       pUserData)
{
    (void)messageTypes;
    (void)pUserData;

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        RHI_LOG(pCallbackData->pMessage);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        RHI_WARN(pCallbackData->pMessage);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        RHI_ERROR(pCallbackData->pMessage);
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
    appInfo.apiVersion         = VK_API_VERSION_1_1;

    std::vector<const char*> enabledLayers;
    std::vector<const char*> enabledExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};

#ifdef RHI_WINDOWS
    enabledExtensions.push_back("VK_KHR_win32_surface");
#elif defined(RHI_LINUX)
    enabledExtensions.push_back("VK_KHR_xlib_surface");
#endif

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    // Enable debug
    {
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        debugCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.pNext           = nullptr;
        debugCreateInfo.flags           = 0;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT;
        debugCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
        debugCreateInfo.pUserData       = nullptr;
    }

    VkInstanceCreateInfo createInfo    = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext                   = &debugCreateInfo;
    createInfo.flags                   = 0;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = CountElements(enabledLayers);
    createInfo.ppEnabledLayerNames     = enabledLayers.data();
    createInfo.enabledExtensionCount   = CountElements(enabledExtensions);
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);

    VK_RETURN_ON_ERROR(result);

    for (VkPhysicalDevice physicalDevice : ::RHI::Vulkan::GetPhysicalDevices(m_instance))
    {
        m_physicalDevices.push_back(std::move(std::make_unique<PhysicalDevice>(physicalDevice)));
    }

    return result;
}

Expected<std::unique_ptr<IDevice>> Instance::CreateDevice(const IPhysicalDevice& physicalDevice)
{
    auto     device = std::make_unique<Device>(*this, static_cast<const PhysicalDevice&>(physicalDevice));
    VkResult result = device->Init(static_cast<const PhysicalDevice&>(physicalDevice));
    if (result != VK_SUCCESS)
    {
        return Unexpected(ConvertResult(result));
    }
    return std::move(device);
}

}  // namespace Vulkan
}  // namespace RHI