#include "RHI/Backend/Vulkan/Utils.hpp"

namespace RHI
{
namespace Vulkan
{
    namespace Utils
    {

        VkFormat ConvertTextureFormat(EPixelFormat format)
        {
            switch (format)
            {
            case EPixelFormat::None:         return VK_FORMAT_UNDEFINED;
            case EPixelFormat::RGB:          return VK_FORMAT_R8G8B8A8_SRGB;
            case EPixelFormat::RGBA:         return VK_FORMAT_R8G8B8A8_SRGB;
            case EPixelFormat::BGRA:         return VK_FORMAT_B8G8R8A8_SRGB;
            case EPixelFormat::Depth:        return VK_FORMAT_D32_SFLOAT;
            case EPixelFormat::DepthStencil: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            default:                         return VK_FORMAT_UNDEFINED;
            }
        }

        VkFormat ConvertBufferFormat(EVertexAttributeFormat format)
        {
            switch (format)
            {
            case EVertexAttributeFormat::Float:  return VK_FORMAT_R32_SFLOAT;
            case EVertexAttributeFormat::Float2: return VK_FORMAT_R32G32_SFLOAT;
            case EVertexAttributeFormat::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
            case EVertexAttributeFormat::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case EVertexAttributeFormat::UInt:   return VK_FORMAT_R32_UINT;
            case EVertexAttributeFormat::UInt2:  return VK_FORMAT_R32G32_UINT;
            case EVertexAttributeFormat::UInt3:  return VK_FORMAT_R32G32B32_UINT;
            case EVertexAttributeFormat::UInt4:  return VK_FORMAT_R32G32B32A32_UINT;
            case EVertexAttributeFormat::Int:    return VK_FORMAT_R32_SINT;
            case EVertexAttributeFormat::Int2:   return VK_FORMAT_R32G32_SINT;
            case EVertexAttributeFormat::Int3:   return VK_FORMAT_R32G32B32_SINT;
            case EVertexAttributeFormat::Int4:   return VK_FORMAT_R32G32B32A32_SINT;
            default:                             return VK_FORMAT_UNDEFINED;
            };
        }
        
        VkDescriptorType GetDescriptorType(EDescriptorType type, EDescriptorAccessType access)
        {
            switch (type)
            {
            case EDescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
            case EDescriptorType::Image:
            {
                switch (access)
                {
                case RHI::EDescriptorAccessType::ReadOnly: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                case RHI::EDescriptorAccessType::Unoredered: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                default: assert(false); break;
                }
            }
            break;
            case EDescriptorType::UniformBuffer:
            {
                switch (access)
                {
                case RHI::EDescriptorAccessType::ReadOnly: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case RHI::EDescriptorAccessType::Unoredered: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                default: assert(false); break;
                }
            }
            break;
            case EDescriptorType::TexelBuffer:
            {
                switch (access)
                {
                case RHI::EDescriptorAccessType::ReadOnly: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                case RHI::EDescriptorAccessType::Unoredered: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                default: assert(false); break;
                };
            }
            break;
            };
            
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }

    } // namespace Utils
} // namespace Vulkan
} // namespace RHI
