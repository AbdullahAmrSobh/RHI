#pragma once
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace RHI
{
namespace Vulkan
{
    EResultCode ConvertResult(VkResult result);
    VkResult    ConvertResult(EResultCode result);
    
    VkFormat ConvertFormat(EFormat format);
    EFormat  ConvertFormat(VkFormat format);

} // namespace Vulkan
} // namespace RHI