#pragma once
#include <expected>
#include <span>
#include <vulkan/vulkan.h>

namespace Vulkan
{
class Instance
{
public:
private:
    VkInstance               instance;
    VkDebugUtilsMessengerEXT debugMessenger;
};

} // namespace Vulkan