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

    } // namespace Utils
} // namespace Vulkan
} // namespace RHI
