#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Image.hpp"

#include <vulkan/vulkan.h>

namespace RHI
{
namespace Vulkan
{
    namespace Utils
    {
        inline bool IsSuccess(VkResult result) { return result <= VK_INCOMPLETE; }
        
        VkFormat              ConvertPixelFormat(EPixelFormat format);
        VkFormat              ConvertBufferFormat(EVertexAttributeFormat format);
        VkFormat              ConvertBufferFormat(EBufferFormat format);
        size_t                GetFormatTaxelSize(VkFormat format);
        VkSampleCountFlagBits ConvertSampleCount(ESampleCount sampleCount);
        
        inline VkShaderStageFlags ConvertShaderStage(ShaderStageFlags stages)
        {
            return static_cast<VkShaderStageFlags>(static_cast<ShaderStageFlags::MaskType>(stages));
        }
        
        VkDescriptorType GetDescriptorType(EDescriptorType type, EDescriptorAccessType access);
    
    } // namespace Utils
} // namespace Vulkan
} // namespace RHI
