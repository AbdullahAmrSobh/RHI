#include "Backend/Vulkan/Common.hpp"

namespace RHI
{
namespace Vulkan
{

    EResultCode ConvertResult(VkResult resultCode)
    {
        switch (resultCode)
        {
        case VK_SUCCESS: return EResultCode::Success;
        case VK_TIMEOUT: return EResultCode::Timeout;
        case VK_NOT_READY: return EResultCode::NotReady;
        case VK_ERROR_OUT_OF_HOST_MEMORY: return EResultCode::HostOutOfMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return EResultCode::DeviceOutOfMemory;
        case VK_ERROR_EXTENSION_NOT_PRESENT: return EResultCode::ExtensionNotAvailable;
        case VK_ERROR_FEATURE_NOT_PRESENT: return EResultCode::FeatureNotAvailable;
        default: return EResultCode::Fail;
        }
    }

    VkImageViewType ConvertImageType(EImageType imageType, bool array)
    {
        static VkImageViewType lookup[] =  {
            VK_IMAGE_VIEW_TYPE_1D,
            VK_IMAGE_VIEW_TYPE_1D_ARRAY,
            VK_IMAGE_VIEW_TYPE_2D, 
            VK_IMAGE_VIEW_TYPE_2D_ARRAY, 
            VK_IMAGE_VIEW_TYPE_3D,
            VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
            VK_IMAGE_VIEW_TYPE_MAX_ENUM
        };
        uint32_t index = static_cast<uint32_t>(imageType);
        return lookup[index + (array ? 0 : 1)];
    }

    VkFormat ConvertFormat(EFormat format);

    uint32_t GetTexelSize(EFormat format);

    uint32_t GetTexelSize(VkFormat format);

    VkShaderStageFlags CovnertShaderStages(EShaderStageFlagBits stages);
    VkShaderStageFlags CovnertShaderStages(ShaderStageFlags stages);

    // PipelineState

    VkCullModeFlags ConvertCullMode(PipelineStateDesc::Rasterization::ECullMode cullMode);

    VkPolygonMode ConvertFillMode(PipelineStateDesc::Rasterization::EFillMode fillMode);

    VkImageAspectFlags ConvertViewAspect(ImageViewAspectFlags aspectFlags);

    VkSamplerAddressMode ConvertAddressMode(SamplerDesc::EAddressMode addressMode);

    // Sampler Utility functions

    VkBool32 IsCompareEnable(SamplerDesc::ECompareOp comapreOp);

    VkCompareOp ConvertCompareOp(SamplerDesc::ECompareOp compareOp);

    VkSamplerMipmapMode ConvertFilterToSamplerMipmapMode(SamplerDesc::EFilter filter);

    VkFilter ConvertFilter(SamplerDesc::EFilter filter);

    // Buffer Utility functions

    VkBufferUsageFlags ConvertBufferUsage(BufferUsageFlags usageFlags);

    // Image Utility functions

    VkExtent3D ConvertExtent(Extent3D extent);

    VkExtent2D ConvertExtent(Extent2D extent);

    VkSampleCountFlagBits ConvertSampleCount(ESampleCount sampleCount);

    VkImageUsageFlags ConvertImageUsage(ImageUsageFlags usageFlags);

    VkImageType ConvertImageType(Extent3D extent);

    VmaMemoryUsage ConvertMemoryUsage(EMemoryUsage usage);

    VkViewport ConvertViewport(const Viewport& viewport);

    VkRect2D ConvertRect(const Rect& rect)
    {
        return { {rect.x, rect.y}, {rect.sizeX, rect.sizeY} };
    }

} // namespace Vulkan
} // namespace RHI