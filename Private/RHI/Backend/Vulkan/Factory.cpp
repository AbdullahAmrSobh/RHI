#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/Common.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

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
        for (auto& [key, surface] : m_surfaceMap)
            vkDestroySurfaceKHR(m_instance, surface, nullptr);

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

        // Create a context.
        m_context = CreateUnique<Context>(*m_device);

        return ToResultCode(result);
    }

    VkResult Factory::CreateSurfaceIfNotExist(NativeWindowHandle _nativeWindowHandle, VkSurfaceKHR* _outSurface)
    {
        // obtain the surface for the window, and create it if it doesn't exist.
        if (m_surfaceMap.find(_nativeWindowHandle) != m_surfaceMap.end())
        {
            *_outSurface = m_surfaceMap[_nativeWindowHandle];
            return VK_SUCCESS;
        }

        VkWin32SurfaceCreateInfoKHR createInfo           = {};
        createInfo.sType                                 = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.flags                                 = 0;
        createInfo.pNext                                 = 0;
        createInfo.hinstance                             = GetModuleHandle(nullptr);
        createInfo.hwnd                                  = static_cast<HWND>(_nativeWindowHandle);
        static PFN_vkCreateWin32SurfaceKHR createSurface = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkCreateWin32SurfaceKHR");
        VkResult                           result        = createSurface(m_instance, &createInfo, nullptr, _outSurface);

        // Query presentation support on that surface.
        VkBool32 surfaceSupport = VK_FALSE;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(m_device->GetPhysicalDevice().GetHandle(), m_device->m_queueSettings.presentQueueIndex, *_outSurface,
                                                      &surfaceSupport);
        if (result != VK_SUCCESS)
            return result;

        if (surfaceSupport == VK_FALSE)
            return VK_ERROR_INITIALIZATION_FAILED;

        return result;
    }
    
	//------------------------------------------------------------------------------------------------------
	// this method for creating renderpass is wrong. 
	//------------------------------------------------------------------------------------------------------
    VkResult Factory::CreateRenderPassIfNotExist(const RenderTargetDesc& _desc, VkRenderPass* _outRenderPass)
    {
        static std::hash<size_t> hasher;
        size_t                   hash = 0;

        for (const auto& format : _desc.colorAttachments)
            hash = Utils::hashCombine(hash, size_t(format));

        if (m_renderPassMap.find(hash) != m_renderPassMap.end())
        {
            *_outRenderPass = m_renderPassMap[hash];
            return VK_SUCCESS;
        }

        std::vector<VkAttachmentDescription> attachments;

		// Preserve the first element for depth stencil attachment, if it exists.
		if(_desc.pDepthStencilAttachment != nullptr)
			attachments.push_back({});
        
        std::vector<VkAttachmentReference>   colorAttachmentReference;
        VkAttachmentReference                depthAttachmentReference = {};
        
        for (auto& colorAttachment : _desc.colorAttachments)
        {
            auto& texture = colorAttachment->GetUnderlyingTexture();
            
            // For each color attachment.
            VkAttachmentDescription attachment = {};
            attachment.format                  = Utils::ToVkFormat(colorAttachment->GetUnderlyingTexture().GetPixelFormat());
            attachment.samples                 = static_cast<VkSampleCountFlagBits>(texture.GetSampleCount());
            attachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
            
            if (texture.GetUsage() & RHI::ETextureUsageFlagBits::Present)
                attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            else
                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            
            if (texture.GetUsage() & ETextureUsageFlagBits::ColorAttachment)
            {
                attachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachments.push_back(attachment);
            }
            else if (texture.GetUsage() & ETextureUsageFlagBits::DepthStencilAttachment)
            {
                attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				// the first element of the vector is preserved for depth stenci attachment.
				attachments[0] = attachment;
            }
            else
            {
                m_pDebugMessengerCallback->Error("Invalid texture usage for an attachment.");
                return VK_ERROR_INITIALIZATION_FAILED;
            }
            
        }
        
        for (uint32_t i = 0; i < _desc.colorAttachments.size() + 1; ++i)
        {
            
            colorAttachmentReference.push_back({});
            colorAttachmentReference[i].attachment = i;
            colorAttachmentReference[i].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        
        VkSubpassDescription subpass = {};
        subpass.flags                = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments    = nullptr;
        subpass.colorAttachmentCount = static_cast<uint32_t>(_desc.colorAttachments.size());
        subpass.pColorAttachments    = colorAttachmentReference.data();
        subpass.pResolveAttachments  = nullptr;
        
        if (_desc.pDepthStencilAttachment != nullptr)
        {
            subpass.pDepthStencilAttachment = &depthAttachmentReference;
        }
        
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments    = nullptr;
        
        VkSubpassDependency dependency{};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        VkRenderPassCreateInfo createInfo = {};
        createInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext                  = nullptr;
        createInfo.flags                  = 0;
        createInfo.attachmentCount        = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments           = attachments.data();
        createInfo.subpassCount           = 1;
        createInfo.pSubpasses             = &subpass;
        createInfo.dependencyCount        = 1;
        createInfo.pDependencies          = &dependency;

        return vkCreateRenderPass(m_device->GetHandle(), &createInfo, nullptr, _outRenderPass);
    }

    IContext* Factory::GetContext() { return m_context.get(); }

} // namespace Vulkan
} // namespace RHI
