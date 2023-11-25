#include <RHI/Assert.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#include "Common.hpp"
#include "Context.hpp"
#include "Format.inl"
#include "FrameGraph.hpp"
#include "Resources.hpp"

#include <Windows.h>
#include <algorithm>

namespace Vulkan
{
    ///////////////////////////////////////////////////////////////////////////
    /// Utility functions
    ///////////////////////////////////////////////////////////////////////////

    VkSampleCountFlagBits ConvertSampleCount(RHI::SampleCount sampleCount)
    {
        switch (sampleCount)
        {
        case RHI::SampleCount::None:      return VK_SAMPLE_COUNT_1_BIT;
        case RHI::SampleCount::Samples1:  return VK_SAMPLE_COUNT_1_BIT;
        case RHI::SampleCount::Samples2:  return VK_SAMPLE_COUNT_2_BIT;
        case RHI::SampleCount::Samples4:  return VK_SAMPLE_COUNT_4_BIT;
        case RHI::SampleCount::Samples8:  return VK_SAMPLE_COUNT_8_BIT;
        case RHI::SampleCount::Samples16: return VK_SAMPLE_COUNT_16_BIT;
        case RHI::SampleCount::Samples32: return VK_SAMPLE_COUNT_32_BIT;
        case RHI::SampleCount::Samples64: return VK_SAMPLE_COUNT_64_BIT;
        default:                          RHI_UNREACHABLE(); return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        }
    }

    VkSampleCountFlags ConvertSampleCountFlags(RHI::Flags<RHI::SampleCount> sampleCountFlags)
    {
        VkSampleCountFlags result = 0;
        if (sampleCountFlags & RHI::SampleCount::Samples1)
            result |= VK_SAMPLE_COUNT_1_BIT;
        if (sampleCountFlags & RHI::SampleCount::Samples2)
            result |= VK_SAMPLE_COUNT_2_BIT;
        if (sampleCountFlags & RHI::SampleCount::Samples4)
            result |= VK_SAMPLE_COUNT_4_BIT;
        if (sampleCountFlags & RHI::SampleCount::Samples8)
            result |= VK_SAMPLE_COUNT_8_BIT;
        if (sampleCountFlags & RHI::SampleCount::Samples16)
            result |= VK_SAMPLE_COUNT_16_BIT;
        if (sampleCountFlags & RHI::SampleCount::Samples32)
            result |= VK_SAMPLE_COUNT_32_BIT;
        if (sampleCountFlags & RHI::SampleCount::Samples64)
            result |= VK_SAMPLE_COUNT_64_BIT;
        return result;
    }

    VkImageUsageFlagBits ConvertImageUsage(RHI::ImageUsage imageUsage)
    {
        switch (imageUsage)
        {
        case RHI::ImageUsage::None:           return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        case RHI::ImageUsage::ShaderResource: return VK_IMAGE_USAGE_SAMPLED_BIT;
        case RHI::ImageUsage::Color:          return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        case RHI::ImageUsage::Depth:          return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case RHI::ImageUsage::Stencil:        return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case RHI::ImageUsage::CopySrc:        return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        case RHI::ImageUsage::CopyDst:        return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        default:                              RHI_UNREACHABLE(); return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkImageUsageFlags ConvertImageUsageFlags(RHI::Flags<RHI::ImageUsage> imageUsageFlags)
    {
        VkImageUsageFlags result = 0;
        if (imageUsageFlags & RHI::ImageUsage::ShaderResource)
            result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (imageUsageFlags & RHI::ImageUsage::Color)
            result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (imageUsageFlags & RHI::ImageUsage::Depth)
            result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & RHI::ImageUsage::Stencil)
            result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & RHI::ImageUsage::CopySrc)
            result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (imageUsageFlags & RHI::ImageUsage::CopyDst)
            result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        return result;
    }

    VkImageType ConvertImageType(RHI::ImageType imageType)
    {
        switch (imageType)
        {
        case RHI::ImageType::None:    return VK_IMAGE_TYPE_MAX_ENUM;
        case RHI::ImageType::Image1D: return VK_IMAGE_TYPE_1D;
        case RHI::ImageType::Image2D: return VK_IMAGE_TYPE_2D;
        case RHI::ImageType::Image3D: return VK_IMAGE_TYPE_3D;
        default:                      RHI_UNREACHABLE(); return VK_IMAGE_TYPE_MAX_ENUM;
        }
    }

    VkImageAspectFlagBits ConvertImageAspect(RHI::Flags<RHI::ImageAspect> imageAspect)
    {
        if (imageAspect & RHI::ImageAspect::Color)
            return VK_IMAGE_ASPECT_COLOR_BIT;
        if (imageAspect & RHI::ImageAspect::Depth)
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        if (imageAspect & RHI::ImageAspect::Stencil)
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        RHI_UNREACHABLE();
        return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    }

    VkImageAspectFlags ConvertImageAspect(RHI::ImageAspect imageAspect)
    {
        switch (imageAspect)
        {
        case RHI::ImageAspect::None:         return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
        case RHI::ImageAspect::Color:        return VK_IMAGE_ASPECT_COLOR_BIT;
        case RHI::ImageAspect::Depth:        return VK_IMAGE_ASPECT_DEPTH_BIT;
        case RHI::ImageAspect::Stencil:      return VK_IMAGE_ASPECT_STENCIL_BIT;
        case RHI::ImageAspect::DepthStencil: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case RHI::ImageAspect::All:          return VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:                             RHI_UNREACHABLE(); return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
        }
    }

    VkComponentSwizzle ConvertComponentSwizzle(RHI::ComponentSwizzle componentSwizzle)
    {
        switch (componentSwizzle)
        {
        case RHI::ComponentSwizzle::Identity: return VK_COMPONENT_SWIZZLE_IDENTITY;
        case RHI::ComponentSwizzle::Zero:     return VK_COMPONENT_SWIZZLE_ZERO;
        case RHI::ComponentSwizzle::One:      return VK_COMPONENT_SWIZZLE_ONE;
        case RHI::ComponentSwizzle::R:        return VK_COMPONENT_SWIZZLE_R;
        case RHI::ComponentSwizzle::G:        return VK_COMPONENT_SWIZZLE_G;
        case RHI::ComponentSwizzle::B:        return VK_COMPONENT_SWIZZLE_B;
        case RHI::ComponentSwizzle::A:        return VK_COMPONENT_SWIZZLE_A;
        default:                              RHI_UNREACHABLE(); return VK_COMPONENT_SWIZZLE_IDENTITY;
        }
    }

    VkBufferUsageFlagBits ConvertBufferUsage(RHI::BufferUsage bufferUsage)
    {
        switch (bufferUsage)
        {
        case RHI::BufferUsage::None:    return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        case RHI::BufferUsage::Storage: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case RHI::BufferUsage::Uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case RHI::BufferUsage::Vertex:  return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case RHI::BufferUsage::Index:   return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case RHI::BufferUsage::CopySrc: return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        case RHI::BufferUsage::CopyDst: return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        default:                        RHI_UNREACHABLE(); return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkBufferUsageFlags ConvertBufferUsageFlags(RHI::Flags<RHI::BufferUsage> bufferUsageFlags)
    {
        VkBufferUsageFlags result = 0;
        if (bufferUsageFlags & RHI::BufferUsage::Storage)
            result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (bufferUsageFlags & RHI::BufferUsage::Uniform)
            result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (bufferUsageFlags & RHI::BufferUsage::Vertex)
            result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (bufferUsageFlags & RHI::BufferUsage::Index)
            result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (bufferUsageFlags & RHI::BufferUsage::CopySrc)
            result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (bufferUsageFlags & RHI::BufferUsage::CopyDst)
            result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        return result;
    }

    VkShaderStageFlagBits ConvertShaderStage(RHI::ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
        case RHI::ShaderStage::Vertex:  return VK_SHADER_STAGE_VERTEX_BIT;
        case RHI::ShaderStage::Pixel:   return VK_SHADER_STAGE_FRAGMENT_BIT;
        case RHI::ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                        RHI_UNREACHABLE(); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkShaderStageFlags ConvertShaderStage(RHI::Flags<RHI::ShaderStage> shaderStageFlags)
    {
        VkShaderStageFlags result = 0;
        if (shaderStageFlags & RHI::ShaderStage::Vertex)
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        if (shaderStageFlags & RHI::ShaderStage::Pixel)
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (shaderStageFlags & RHI::ShaderStage::Compute)
            result |= VK_SHADER_STAGE_COMPUTE_BIT;
        return result;
    }

    VkDescriptorType ConvertDescriptorType(RHI::ShaderBindingType bindingType)
    {
        switch (bindingType)
        {
        case RHI::ShaderBindingType::None:    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        case RHI::ShaderBindingType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
        case RHI::ShaderBindingType::Image:   return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case RHI::ShaderBindingType::Buffer:  return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // todo add storage buffers
        default: RHI_UNREACHABLE(); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    VkAccessFlags ConvertAccessFlags(RHI::ShaderBindingAccess bindingAccess)
    {
        switch (bindingAccess)
        {
        case RHI::ShaderBindingAccess::OnlyRead:  return VK_ACCESS_SHADER_READ_BIT;
        case RHI::ShaderBindingAccess::ReadWrite: return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        default:                                  RHI_UNREACHABLE(); return VK_ACCESS_FLAG_BITS_MAX_ENUM;
        }
    }

    VkVertexInputRate ConvertVertexInputRate(RHI::PipelineVertexInputRate inputRate)
    {
        switch (inputRate)
        {
        case RHI::PipelineVertexInputRate::PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case RHI::PipelineVertexInputRate::PerVertex:   return VK_VERTEX_INPUT_RATE_VERTEX;
        default:                                        RHI_UNREACHABLE(); return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    VkCullModeFlags ConvertCullModeFlags(RHI::PipelineRasterizerStateCullMode cullMode)
    {
        switch (cullMode)
        {
        case RHI::PipelineRasterizerStateCullMode::None:      return VK_CULL_MODE_NONE;
        case RHI::PipelineRasterizerStateCullMode::FrontFace: return VK_CULL_MODE_FRONT_BIT;
        case RHI::PipelineRasterizerStateCullMode::BackFace:  return VK_CULL_MODE_BACK_BIT;
        case RHI::PipelineRasterizerStateCullMode::Discard:   return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        default:                                              RHI_UNREACHABLE(); return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkPolygonMode ConvertPolygonMode(RHI::PipelineRasterizerStateFillMode fillMode)
    {
        switch (fillMode)
        {
        case RHI::PipelineRasterizerStateFillMode::Point:    return VK_POLYGON_MODE_POINT;
        case RHI::PipelineRasterizerStateFillMode::Triangle: return VK_POLYGON_MODE_FILL;
        case RHI::PipelineRasterizerStateFillMode::Line:     return VK_POLYGON_MODE_LINE;
        default:                                             RHI_UNREACHABLE(); return VK_POLYGON_MODE_MAX_ENUM;
        }
    }

    VkPrimitiveTopology ConvertPrimitiveTopology(RHI::PipelineTopologyMode topologyMode)
    {
        switch (topologyMode)
        {
        case RHI::PipelineTopologyMode::Points:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case RHI::PipelineTopologyMode::Lines:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case RHI::PipelineTopologyMode::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:                                   RHI_UNREACHABLE(); return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }
    }

    VkFrontFace ConvertFrontFace(RHI::PipelineRasterizerStateFrontFace frontFace)
    {
        switch (frontFace)
        {
        case RHI::PipelineRasterizerStateFrontFace::Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
        case RHI::PipelineRasterizerStateFrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:                                                      RHI_UNREACHABLE(); return VK_FRONT_FACE_MAX_ENUM;
        }
    }

    VkCompareOp ConvertCompareOp(RHI::CompareOperator compareOperator)
    {
        switch (compareOperator)
        {
        case RHI::CompareOperator::Never:          return VK_COMPARE_OP_NEVER;
        case RHI::CompareOperator::Equal:          return VK_COMPARE_OP_EQUAL;
        case RHI::CompareOperator::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case RHI::CompareOperator::Greater:        return VK_COMPARE_OP_GREATER;
        case RHI::CompareOperator::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case RHI::CompareOperator::Less:           return VK_COMPARE_OP_LESS;
        case RHI::CompareOperator::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case RHI::CompareOperator::Always:         return VK_COMPARE_OP_ALWAYS;
        default:                                   RHI_UNREACHABLE(); return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    VkFilter ConvertFilter(RHI::SamplerFilter samplerFilter)
    {
        switch (samplerFilter)
        {
        case RHI::SamplerFilter::Point:  return VK_FILTER_NEAREST;
        case RHI::SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default:                         RHI_UNREACHABLE(); return VK_FILTER_MAX_ENUM;
        }
    }

    VkSamplerAddressMode ConvertSamplerAddressMode(RHI::SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case RHI::SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case RHI::SamplerAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:                              RHI_UNREACHABLE(); return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    VkCompareOp ConvertCompareOp(RHI::SamplerCompareOperation compareOperation)
    {
        switch (compareOperation)
        {
        case RHI::SamplerCompareOperation::Never:     return VK_COMPARE_OP_NEVER;
        case RHI::SamplerCompareOperation::Equal:     return VK_COMPARE_OP_EQUAL;
        case RHI::SamplerCompareOperation::NotEqual:  return VK_COMPARE_OP_NOT_EQUAL;
        case RHI::SamplerCompareOperation::Always:    return VK_COMPARE_OP_ALWAYS;
        case RHI::SamplerCompareOperation::Less:      return VK_COMPARE_OP_LESS;
        case RHI::SamplerCompareOperation::LessEq:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case RHI::SamplerCompareOperation::Greater:   return VK_COMPARE_OP_GREATER;
        case RHI::SamplerCompareOperation::GreaterEq: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        default:                                      RHI_UNREACHABLE(); return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    VkBlendFactor ConvertBlendFactor(RHI::BlendFactor blendFactor)
    {
        switch (blendFactor)
        {
        case RHI::BlendFactor::Zero:                  return VK_BLEND_FACTOR_ZERO;
        case RHI::BlendFactor::One:                   return VK_BLEND_FACTOR_ONE;
        case RHI::BlendFactor::SrcColor:              return VK_BLEND_FACTOR_SRC_COLOR;
        case RHI::BlendFactor::OneMinusSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case RHI::BlendFactor::DstColor:              return VK_BLEND_FACTOR_DST_COLOR;
        case RHI::BlendFactor::OneMinusDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case RHI::BlendFactor::SrcAlpha:              return VK_BLEND_FACTOR_SRC_ALPHA;
        case RHI::BlendFactor::OneMinusSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case RHI::BlendFactor::DstAlpha:              return VK_BLEND_FACTOR_DST_ALPHA;
        case RHI::BlendFactor::OneMinusDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case RHI::BlendFactor::ConstantColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case RHI::BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case RHI::BlendFactor::ConstantAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case RHI::BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        default:                                      RHI_UNREACHABLE(); return VK_BLEND_FACTOR_MAX_ENUM;
        }
    }

    VkBlendOp ConvertBlendOp(RHI::BlendEquation blendEquation)
    {
        switch (blendEquation)
        {
        case RHI::BlendEquation::Add:             return VK_BLEND_OP_ADD;
        case RHI::BlendEquation::Subtract:        return VK_BLEND_OP_SUBTRACT;
        case RHI::BlendEquation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case RHI::BlendEquation::Min:             return VK_BLEND_OP_MIN;
        case RHI::BlendEquation::Max:             return VK_BLEND_OP_MAX;
        default:                                  RHI_UNREACHABLE(); return VK_BLEND_OP_MAX_ENUM;
        }
    }

    VkDescriptorType DescriptorTypeFromAttachmentUsage(RHI::AttachmentUsage usage, RHI::AttachmentAccess access)
    {
        (void)access;
        switch (usage)
        {
        case RHI::AttachmentUsage::None:
        case RHI::AttachmentUsage::VertexInputBuffer:
        case RHI::AttachmentUsage::RenderTarget:
        case RHI::AttachmentUsage::Depth:
        case RHI::AttachmentUsage::Copy:
        case RHI::AttachmentUsage::Resolve:
        case RHI::AttachmentUsage::ShaderStorage:     return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case RHI::AttachmentUsage::ShaderResource:    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    VkImageSubresource ConvertSubresource(const RHI::ImageSubresource& subresource)
    {
        auto vkSubresource       = VkImageSubresource{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel   = subresource.mipBase;
        vkSubresource.arrayLayer = subresource.arrayBase;
        return vkSubresource;
    }

    VkImageSubresourceLayers ConvertSubresourceLayer(const RHI::ImageSubresource& subresource)
    {
        auto vkSubresource           = VkImageSubresourceLayers{};
        vkSubresource.aspectMask     = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel       = subresource.mipBase;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount     = subresource.arrayCount;
        return vkSubresource;
    }

    VkImageSubresourceRange ConvertSubresourceRange(const RHI::ImageSubresource& subresource)
    {
        auto vkSubresource           = VkImageSubresourceRange{};
        vkSubresource.aspectMask     = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.baseMipLevel   = subresource.mipBase;
        vkSubresource.levelCount     = subresource.mipCount;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount     = subresource.arrayCount;
        return vkSubresource;
    }

    VkExtent3D ConvertExtent3D(RHI::ImageSize size)
    {
        return { size.width, size.height, size.depth };
    }

    VkExtent2D ConvertExtent2D(RHI::ImageSize size)
    {
        RHI_ASSERT(size.depth == 0);
        return { size.width, size.height };
    }

    VkOffset3D ConvertOffset3D(RHI::ImageOffset offset)
    {
        return { offset.x, offset.y, offset.z };
    }

    VkOffset3D ConvertOffset2D(RHI::ImageOffset offset)
    {
        RHI_ASSERT(offset.z == 0);
        return { offset.x, offset.y, offset.z };
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Image
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode Image::Init(Context* context, const VmaAllocationCreateInfo allocationInfo, const RHI::ImageCreateInfo& createInfo, ResourcePool* parentPool, bool isTransientResource)
    {
        (void)parentPool;
        VkImageCreateInfo vkCreateInfo{};
        vkCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        vkCreateInfo.pNext                 = nullptr;
        vkCreateInfo.flags                 = {};
        vkCreateInfo.imageType             = ConvertImageType(createInfo.type);
        vkCreateInfo.format                = ConvertFormat(createInfo.format);
        vkCreateInfo.extent.width          = createInfo.size.width;
        vkCreateInfo.extent.height         = createInfo.size.height;
        vkCreateInfo.extent.depth          = createInfo.size.depth;
        vkCreateInfo.mipLevels             = createInfo.mipLevels;
        vkCreateInfo.arrayLayers           = createInfo.arrayCount;
        vkCreateInfo.samples               = ConvertSampleCount(createInfo.sampleCount);
        vkCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
        vkCreateInfo.usage                 = ConvertImageUsageFlags(createInfo.usageFlags);
        vkCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateInfo.queueFamilyIndexCount = 0;
        vkCreateInfo.pQueueFamilyIndices   = nullptr;
        vkCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

        this->createInfo = vkCreateInfo;
        VkResult result  = VK_ERROR_UNKNOWN;

        if (isTransientResource)
        {
            allocation.type = AllocationType::Aliasing;
            result          = vkCreateImage(context->m_device, &vkCreateInfo, nullptr, &handle);
        }
        else
        {
            allocation.type = AllocationType::Default;
            result          = vmaCreateImage(context->m_allocator, &vkCreateInfo, &allocationInfo, &handle, &allocation.handle, &allocation.info);
        }

        return ConvertResult(result);
    }

    void Image::Shutdown(Context* context)
    {
        if (pool)
        {
            vmaDestroyImage(context->m_allocator, handle, allocation.handle);
        }
        else
        {
            vkDestroyImage(context->m_device, handle, nullptr);
        }
    }

    VkMemoryRequirements Image::GetMemoryRequirements(VkDevice device) const
    {
        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Buffer
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode Buffer::Init(Context* context, const VmaAllocationCreateInfo allocationInfo, const RHI::BufferCreateInfo& createInfo, ResourcePool* parentPool, bool isTransientResource)
    {
        (void)parentPool;
        VkBufferCreateInfo vkCreateInfo{};
        vkCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkCreateInfo.pNext                 = nullptr;
        vkCreateInfo.flags                 = {};
        vkCreateInfo.size                  = createInfo.byteSize;
        vkCreateInfo.usage                 = ConvertBufferUsageFlags(createInfo.usageFlags);
        vkCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateInfo.queueFamilyIndexCount = 0;
        vkCreateInfo.pQueueFamilyIndices   = nullptr;

        VkResult result = VK_ERROR_UNKNOWN;
        if (isTransientResource)
        {
            allocation.type = AllocationType::Aliasing;
            result          = vkCreateBuffer(context->m_device, &vkCreateInfo, nullptr, &handle);
        }
        else
        {
            allocation.type = AllocationType::Default;
            result          = vmaCreateBuffer(context->m_allocator, &vkCreateInfo, &allocationInfo, &handle, &allocation.handle, &allocation.info);
        }

        return ConvertResult(result);
    }

    void Buffer::Shutdown(Context* context)
    {
        if (pool)
        {
            vmaDestroyBuffer(context->m_allocator, handle, allocation.handle);
        }
        else
        {
            vkDestroyBuffer(context->m_device, handle, nullptr);
        }
    }

    VkMemoryRequirements Buffer::GetMemoryRequirements(VkDevice device) const
    {
        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ImageView
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode ImageView::Init(Context* context, RHI::Handle<Image> imageHandle, const RHI::ImageAttachmentUseInfo& useInfo)
    {
        auto image = context->m_imageOwner.Get(imageHandle);
        RHI_ASSERT(image);

        VkImageViewCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.image = image->handle;

        switch (image->createInfo.imageType)
        {
        case VK_IMAGE_TYPE_1D: vkCreateInfo.viewType = useInfo.subresource.arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case VK_IMAGE_TYPE_2D: vkCreateInfo.viewType = useInfo.subresource.arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case VK_IMAGE_TYPE_3D: vkCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        default:               RHI_UNREACHABLE(); break;
        }

        vkCreateInfo.format                          = image->createInfo.format;
        vkCreateInfo.components.r                    = ConvertComponentSwizzle(useInfo.components.r);
        vkCreateInfo.components.g                    = ConvertComponentSwizzle(useInfo.components.g);
        vkCreateInfo.components.b                    = ConvertComponentSwizzle(useInfo.components.b);
        vkCreateInfo.components.a                    = ConvertComponentSwizzle(useInfo.components.a);
        vkCreateInfo.subresourceRange.aspectMask     = ConvertImageAspect(useInfo.subresource.imageAspects);
        vkCreateInfo.subresourceRange.baseMipLevel   = useInfo.subresource.mipBase;
        vkCreateInfo.subresourceRange.levelCount     = useInfo.subresource.mipCount;
        vkCreateInfo.subresourceRange.baseArrayLayer = useInfo.subresource.arrayBase;
        vkCreateInfo.subresourceRange.layerCount     = useInfo.subresource.arrayCount;

        auto result = vkCreateImageView(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void ImageView::Shutdown(Context* context)
    {
        vkDestroyImageView(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BufferView
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode BufferView::Init(Context* context, RHI::Handle<Buffer> bufferHandle, const RHI::BufferAttachmentUseInfo& useInfo)
    {
        auto buffer = context->m_bufferOwner.Get(bufferHandle);
        RHI_ASSERT(buffer);

        VkBufferViewCreateInfo vkCreateInfo{};
        vkCreateInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        vkCreateInfo.pNext  = nullptr;
        vkCreateInfo.flags  = 0;
        vkCreateInfo.buffer = buffer->handle;
        vkCreateInfo.format = ConvertFormat(useInfo.format);
        vkCreateInfo.offset = useInfo.byteOffset;
        vkCreateInfo.range  = useInfo.byteSize;

        auto result = vkCreateBufferView(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void BufferView::Shutdown(Context* context)
    {
        vkDestroyBufferView(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BindGroupLayout
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode BindGroupLayout::Init(Context* context, const RHI::BindGroupLayoutCreateInfo& createInfo)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (auto shaderBinding : createInfo.bindings)
        {
            auto& binding              = bindings.emplace_back<VkDescriptorSetLayoutBinding>({});
            binding.binding            = bindings.size() - 1;
            binding.descriptorType     = ConvertDescriptorType(shaderBinding.type);
            binding.descriptorCount    = shaderBinding.arrayCount;
            binding.stageFlags         = ConvertShaderStage(shaderBinding.stages);
            binding.pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo vkCreateInfo;
        vkCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        vkCreateInfo.pNext        = nullptr;
        vkCreateInfo.flags        = 0;
        vkCreateInfo.bindingCount = bindings.size();
        vkCreateInfo.pBindings    = bindings.data();

        auto result = vkCreateDescriptorSetLayout(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void BindGroupLayout::Shutdown(Context* context)
    {
        vkDestroyDescriptorSetLayout(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BindGroup
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode BindGroup::Init(Context* context, VkDescriptorSetLayout layout, VkDescriptorPool pool)
    {
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext              = nullptr;
        allocateInfo.descriptorPool     = pool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts        = &layout;

        auto result = vkAllocateDescriptorSets(context->m_device, &allocateInfo, &handle);
        return ConvertResult(result);
    }

    void BindGroup::Shutdown(Context* context)
    {
        (void)context;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// PipelineLayout
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode PipelineLayout::Init(Context* context, const RHI::PipelineLayoutCreateInfo& createInfo)
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (auto bindGroupLayout : createInfo.layouts)
        {
            auto layout = context->m_bindGroupLayoutsOwner.Get(bindGroupLayout);
            descriptorSetLayouts.push_back(layout->handle);
        }

        VkPipelineLayoutCreateInfo vkCreateInfo{};
        vkCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        vkCreateInfo.pNext                  = nullptr;
        vkCreateInfo.flags                  = 0;
        vkCreateInfo.setLayoutCount         = descriptorSetLayouts.size();
        vkCreateInfo.pSetLayouts            = descriptorSetLayouts.data();
        vkCreateInfo.pushConstantRangeCount = 0;
        vkCreateInfo.pPushConstantRanges    = nullptr;

        auto result = vkCreatePipelineLayout(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void PipelineLayout::Shutdown(Context* context)
    {
        vkDestroyPipelineLayout(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// GraphicsPipeline
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode GraphicsPipeline::Init(Context* context, const RHI::GraphicsPipelineCreateInfo& createInfo)
    {
        uint32_t                        stagesCreateInfoCount = 2;
        VkPipelineShaderStageCreateInfo stagesCreateInfos[4];
        {
            VkPipelineShaderStageCreateInfo stageInfo{};
            stageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.pNext               = nullptr;
            stageInfo.flags               = 0;
            stageInfo.pSpecializationInfo = nullptr;
            stageInfo.stage               = VK_SHADER_STAGE_VERTEX_BIT;
            stageInfo.module              = static_cast<ShaderModule*>(createInfo.vertexShaderModule)->m_shaderModule;
            stageInfo.pName               = createInfo.vertexShaderName;
            stagesCreateInfos[0]          = stageInfo;

            stageInfo.stage      = VK_SHADER_STAGE_FRAGMENT_BIT;
            stageInfo.module     = static_cast<ShaderModule*>(createInfo.pixelShaderModule)->m_shaderModule;
            stageInfo.pName      = createInfo.pixelShaderName;
            stagesCreateInfos[1] = stageInfo;
        }

        std::vector<VkVertexInputBindingDescription>   vertexInputBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;

        for (auto attributeDesc : createInfo.inputAssemblerState.attributes)
        {
            VkVertexInputAttributeDescription attribute{};
            attribute.location = attributeDesc.location;
            attribute.binding  = attributeDesc.binding;
            attribute.format   = ConvertFormat(attributeDesc.format);
            attribute.offset   = attributeDesc.offset;
            inputAttributeDescriptions.push_back(attribute);
        }

        for (auto bindingDesc : createInfo.inputAssemblerState.bindings)
        {
            VkVertexInputBindingDescription binding{};
            binding.binding   = bindingDesc.binding;
            binding.stride    = bindingDesc.stride;
            binding.inputRate = bindingDesc.stepRate == RHI::PipelineVertexInputRate::PerVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
            vertexInputBindingDescriptions.push_back(binding);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.pNext                           = nullptr;
        vertexInputStateCreateInfo.flags                           = 0;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexInputBindingDescriptions.size());
        vertexInputStateCreateInfo.pVertexBindingDescriptions      = vertexInputBindingDescriptions.data();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributeDescriptions.size());
        vertexInputStateCreateInfo.pVertexAttributeDescriptions    = inputAttributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.pNext                  = nullptr;
        inputAssemblyStateCreateInfo.flags                  = 0;
        inputAssemblyStateCreateInfo.topology               = ConvertPrimitiveTopology(createInfo.topologyMode);
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{};
        tessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationStateCreateInfo.pNext = nullptr;
        tessellationStateCreateInfo.flags = 0;
        // tessellationStateCreateInfo.patchControlPoints;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.pNext         = nullptr;
        viewportStateCreateInfo.flags         = 0;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount  = 1;
        viewportStateCreateInfo.pScissors     = nullptr;
        viewportStateCreateInfo.pViewports    = nullptr;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
        rasterizationStateCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.pNext                   = nullptr;
        rasterizationStateCreateInfo.flags                   = 0;
        rasterizationStateCreateInfo.depthClampEnable        = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode             = ConvertPolygonMode(createInfo.rasterizationState.fillMode);
        rasterizationStateCreateInfo.cullMode                = ConvertCullModeFlags(createInfo.rasterizationState.cullMode);
        rasterizationStateCreateInfo.frontFace               = ConvertFrontFace(createInfo.rasterizationState.frontFace);
        rasterizationStateCreateInfo.depthBiasEnable         = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp          = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor    = 0.0f;
        rasterizationStateCreateInfo.lineWidth               = createInfo.rasterizationState.lineWidth;

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.pNext                 = nullptr;
        multisampleStateCreateInfo.flags                 = 0;
        multisampleStateCreateInfo.rasterizationSamples  = ConvertSampleCount(createInfo.multisampleState.sampleCount);
        multisampleStateCreateInfo.sampleShadingEnable   = createInfo.multisampleState.sampleShading ? VK_TRUE : VK_FALSE;
        multisampleStateCreateInfo.minSampleShading      = multisampleStateCreateInfo.rasterizationSamples / 2;
        multisampleStateCreateInfo.pSampleMask           = nullptr;
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreateInfo.alphaToOneEnable      = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.pNext                 = nullptr;
        depthStencilStateCreateInfo.flags                 = 0;
        depthStencilStateCreateInfo.depthTestEnable       = createInfo.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
        depthStencilStateCreateInfo.depthWriteEnable      = createInfo.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
        depthStencilStateCreateInfo.depthCompareOp        = ConvertCompareOp(createInfo.depthStencilState.compareOperator);
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.stencilTestEnable     = createInfo.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;
        // depthStencilStateCreateInfo.front;
        // depthStencilStateCreateInfo.back;
        depthStencilStateCreateInfo.minDepthBounds        = 0.0;
        depthStencilStateCreateInfo.maxDepthBounds        = 1.0;

        uint32_t                            pipelineColorBlendAttachmentStateCount = 0;
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentStates[8];

        for (auto blendState : createInfo.colorBlendState.blendStates)
        {
            auto& state               = pipelineColorBlendAttachmentStates[pipelineColorBlendAttachmentStateCount++];
            state.blendEnable         = blendState.blendEnable ? VK_TRUE : VK_FALSE;
            state.srcColorBlendFactor = ConvertBlendFactor(blendState.srcColor);
            state.dstColorBlendFactor = ConvertBlendFactor(blendState.dstColor);
            state.colorBlendOp        = ConvertBlendOp(blendState.colorBlendOp);
            state.srcAlphaBlendFactor = ConvertBlendFactor(blendState.srcAlpha);
            state.dstAlphaBlendFactor = ConvertBlendFactor(blendState.dstAlpha);
            state.alphaBlendOp        = ConvertBlendOp(blendState.alphaBlendOp);
            state.colorWriteMask      = 0;
            if (blendState.writeMask & RHI::ColorWriteMask::Red)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
            }

            if (blendState.writeMask & RHI::ColorWriteMask::Green)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
            }

            if (blendState.writeMask & RHI::ColorWriteMask::Blue)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
            }

            if (blendState.writeMask & RHI::ColorWriteMask::Alpha)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
            }
        }

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.pNext             = nullptr;
        colorBlendStateCreateInfo.flags             = 0;
        colorBlendStateCreateInfo.logicOpEnable     = VK_FALSE;
        colorBlendStateCreateInfo.logicOp           = VK_LOGIC_OP_SET;
        colorBlendStateCreateInfo.attachmentCount   = pipelineColorBlendAttachmentStateCount;
        colorBlendStateCreateInfo.pAttachments      = pipelineColorBlendAttachmentStates;
        colorBlendStateCreateInfo.blendConstants[0] = createInfo.colorBlendState.blendConstants[0];
        colorBlendStateCreateInfo.blendConstants[1] = createInfo.colorBlendState.blendConstants[1];
        colorBlendStateCreateInfo.blendConstants[2] = createInfo.colorBlendState.blendConstants[2];
        colorBlendStateCreateInfo.blendConstants[3] = createInfo.colorBlendState.blendConstants[3];

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pNext             = nullptr;
        dynamicStateCreateInfo.flags             = 0;
        dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
        dynamicStateCreateInfo.pDynamicStates    = dynamicStates.data();

        uint32_t colorAttachmentFormatCount = static_cast<uint32_t>(createInfo.renderTargetLayout.colorAttachmentsFormats.size());
        VkFormat colorAttachmentFormats[8]  = {};

        uint32_t index = 0;
        for (auto format : createInfo.renderTargetLayout.colorAttachmentsFormats)
            colorAttachmentFormats[index++] = ConvertFormat(format);

        VkPipelineRenderingCreateInfo renderTargetLayout{};
        renderTargetLayout.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderTargetLayout.pNext                   = nullptr;
        // renderTargetLayout.viewMask;
        renderTargetLayout.colorAttachmentCount    = colorAttachmentFormatCount;
        renderTargetLayout.pColorAttachmentFormats = colorAttachmentFormats;
        renderTargetLayout.depthAttachmentFormat   = ConvertFormat(createInfo.renderTargetLayout.depthAttachmentFormat);
        renderTargetLayout.stencilAttachmentFormat = ConvertFormat(createInfo.renderTargetLayout.stencilAttachmentFormat);

        layout = context->m_pipelineLayoutOwner.Get(createInfo.layout)->handle;

        VkGraphicsPipelineCreateInfo vkCreateInfo{};
        vkCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        vkCreateInfo.pNext               = &renderTargetLayout;
        vkCreateInfo.flags               = 0;
        vkCreateInfo.stageCount          = stagesCreateInfoCount;
        vkCreateInfo.pStages             = stagesCreateInfos;
        vkCreateInfo.pVertexInputState   = &vertexInputStateCreateInfo;
        vkCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        vkCreateInfo.pTessellationState  = &tessellationStateCreateInfo;
        vkCreateInfo.pViewportState      = &viewportStateCreateInfo;
        vkCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        vkCreateInfo.pMultisampleState   = &multisampleStateCreateInfo;
        vkCreateInfo.pDepthStencilState  = &depthStencilStateCreateInfo;
        vkCreateInfo.pColorBlendState    = &colorBlendStateCreateInfo;
        vkCreateInfo.pDynamicState       = &dynamicStateCreateInfo;
        vkCreateInfo.layout              = layout;
        vkCreateInfo.renderPass          = VK_NULL_HANDLE;
        vkCreateInfo.subpass             = 0;
        vkCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
        vkCreateInfo.basePipelineIndex   = 0;

        auto result = vkCreateGraphicsPipelines(context->m_device, VK_NULL_HANDLE, 1, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void GraphicsPipeline::Shutdown(Context* context)
    {
        vkDestroyPipeline(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ComputePipeline
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode ComputePipeline::Init(Context* context, const RHI::ComputePipelineCreateInfo& createInfo)
    {
        auto shaderModule = static_cast<ShaderModule*>(createInfo.shaderModule);

        layout = context->m_pipelineLayoutOwner.Get(createInfo.layout)->handle;

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.pNext               = nullptr;
        shaderStage.flags               = 0;
        shaderStage.stage               = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.module              = shaderModule->m_shaderModule;
        shaderStage.pName               = createInfo.shaderName;
        shaderStage.pSpecializationInfo = nullptr;

        VkComputePipelineCreateInfo vkCreateInfo{};
        vkCreateInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        vkCreateInfo.pNext              = nullptr;
        vkCreateInfo.flags              = {};
        vkCreateInfo.stage              = shaderStage;
        vkCreateInfo.layout             = layout;
        vkCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        vkCreateInfo.basePipelineIndex  = 0;

        ComputePipeline pipeline{};

        auto result = vkCreateComputePipelines(context->m_device, VK_NULL_HANDLE, 1, &vkCreateInfo, nullptr, &pipeline.handle);
        return ConvertResult(result);
    }

    void ComputePipeline::Shutdown(Context* context)
    {
        vkDestroyPipeline(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Sampler
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode Sampler::Init(Context* context, const RHI::SamplerCreateInfo& createInfo)
    {
        VkSamplerCreateInfo vkCreateInfo{};
        vkCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        vkCreateInfo.pNext                   = nullptr;
        vkCreateInfo.flags                   = 0;
        vkCreateInfo.magFilter               = ConvertFilter(createInfo.filterMag);
        vkCreateInfo.minFilter               = ConvertFilter(createInfo.filterMin);
        vkCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR; // ConvertFilter(createInfo.filterMip);
        vkCreateInfo.addressModeU            = ConvertSamplerAddressMode(createInfo.addressU);
        vkCreateInfo.addressModeV            = ConvertSamplerAddressMode(createInfo.addressV);
        vkCreateInfo.addressModeW            = ConvertSamplerAddressMode(createInfo.addressW);
        vkCreateInfo.mipLodBias              = createInfo.mipLodBias;
        vkCreateInfo.anisotropyEnable        = VK_TRUE;
        vkCreateInfo.maxAnisotropy           = 1.0f;
        vkCreateInfo.compareEnable           = VK_TRUE;
        vkCreateInfo.compareOp               = ConvertCompareOp(createInfo.compare);
        vkCreateInfo.minLod                  = createInfo.minLod;
        vkCreateInfo.maxLod                  = createInfo.maxLod;
        vkCreateInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        vkCreateInfo.unnormalizedCoordinates = VK_FALSE;

        auto result = vkCreateSampler(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void Sampler::Shutdown(Context* context)
    {
        vkDestroySampler(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ShaderModule
    ///////////////////////////////////////////////////////////////////////////

    ShaderModule::~ShaderModule()
    {
        vkDestroyShaderModule(m_context->m_device, m_shaderModule, nullptr);
    }

    VkResult ShaderModule::Init(const RHI::ShaderModuleCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.pNext    = nullptr;
        moduleCreateInfo.flags    = {};
        moduleCreateInfo.codeSize = createInfo.size * 4;
        moduleCreateInfo.pCode    = (uint32_t*)createInfo.code;
        RHI_ASSERT(moduleCreateInfo.codeSize % 4 == 0);

        return vkCreateShaderModule(context->m_device, &moduleCreateInfo, nullptr, &m_shaderModule);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BindGroupAllocator
    ///////////////////////////////////////////////////////////////////////////

    BindGroupAllocator::~BindGroupAllocator()
    {
        auto context = static_cast<Context*>(m_context);

        for (auto pool : m_descriptorPools)
            vkDestroyDescriptorPool(context->m_device, pool, nullptr);
    }

    VkResult BindGroupAllocator::Init()
    {
        return VK_SUCCESS;
    }

    std::vector<RHI::Handle<RHI::BindGroup>> BindGroupAllocator::AllocateBindGroups(TL::Span<RHI::Handle<RHI::BindGroupLayout>> bindGroupLayouts)
    {
        auto context = static_cast<Context*>(m_context);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (auto bindGroupLayoutHandle : bindGroupLayouts)
        {
            auto bindGroupLayout = m_context->m_bindGroupLayoutsOwner.Get(bindGroupLayoutHandle);
            descriptorSetLayouts.push_back(bindGroupLayout->handle);
        }

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext              = nullptr;
        allocateInfo.descriptorSetCount = descriptorSetLayouts.size();
        allocateInfo.pSetLayouts        = descriptorSetLayouts.data();

        bool                         success = false;
        std::vector<VkDescriptorSet> descriptorSets;
        descriptorSets.resize(descriptorSetLayouts.size());
        for (auto descriptorPool : m_descriptorPools)
        {
            allocateInfo.descriptorPool = descriptorPool;

            auto result = vkAllocateDescriptorSets(context->m_device, &allocateInfo, descriptorSets.data());

            if (result == VK_SUCCESS)
            {
                success = true;
                break;
            }
        }

        if (success == false)
        {
            VkDescriptorPoolSize poolSizes[] = {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 16 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 16 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 16 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 16 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 16 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 16 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 16 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 16 },
            };

            VkDescriptorPoolCreateInfo poolCreateInfo{};
            poolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.pNext         = nullptr;
            poolCreateInfo.flags         = 0;
            poolCreateInfo.maxSets       = 8;
            poolCreateInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize); // todo recheck this
            poolCreateInfo.pPoolSizes    = poolSizes;

            VkDescriptorPool newDescriptorPool{};
            auto             result = vkCreateDescriptorPool(context->m_device, &poolCreateInfo, nullptr, &newDescriptorPool);
            VULKAN_ASSERT_SUCCESS(result);
            m_descriptorPools.push_back(newDescriptorPool);

            allocateInfo.descriptorPool = newDescriptorPool;

            result = vkAllocateDescriptorSets(context->m_device, &allocateInfo, descriptorSets.data());
            VULKAN_ASSERT_SUCCESS(result);
        }

        std::vector<RHI::Handle<RHI::BindGroup>> bindGroups;
        for (auto descriptorSet : descriptorSets)
        {
            auto [handle, bindGroup] = m_context->m_bindGroupOwner.InsertZerod();
            bindGroup.handle         = descriptorSet;
            bindGroup.pool           = allocateInfo.descriptorPool;
            bindGroups.push_back(handle);
        }

        return bindGroups;
    }

    void BindGroupAllocator::Free(TL::Span<RHI::Handle<RHI::BindGroup>> bindGroups)
    {
        std::vector<VkDescriptorSet> descriptorSetHandles;
        descriptorSetHandles.reserve(bindGroups.size());

        for (auto group : bindGroups)
        {
            auto set = m_context->m_bindGroupOwner.Get(group);
            descriptorSetHandles.push_back(set->handle);
        }

        // vkFreeDescriptorSets(m_context->m_device, );
        RHI_UNREACHABLE();
    }

    void BindGroupAllocator::Update(RHI::Handle<RHI::BindGroup> _bindGroup, const RHI::BindGroupData& data)
    {
        auto context   = static_cast<Context*>(m_context);
        auto bindGroup = m_context->m_bindGroupOwner.Get(_bindGroup);

        std::vector<std::vector<VkDescriptorImageInfo>>  descriptorImageInfos;
        std::vector<std::vector<VkDescriptorBufferInfo>> descriptorBufferInfos;
        std::vector<std::vector<VkBufferView>>           descriptorBufferViews;

        std::vector<VkWriteDescriptorSet> writeInfos;

        for (auto [binding, resourceVarient] : data.m_bindings)
        {
            VkWriteDescriptorSet writeInfo{};
            writeInfo.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext      = nullptr;
            writeInfo.dstSet     = bindGroup->handle;
            writeInfo.dstBinding = binding;
            if (auto resources = std::get_if<0>(&resourceVarient))
            {
                auto& imageInfos = descriptorImageInfos.emplace_back();

                for (auto passAttachment : resources->views)
                {
                    auto view = context->m_imageViewOwner.Get(passAttachment->view);

                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView   = view->handle;
                    imageInfos.push_back(imageInfo);
                }

                writeInfo.dstArrayElement = resources->arrayOffset;
                writeInfo.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writeInfo.descriptorCount = imageInfos.size();
                writeInfo.pImageInfo      = imageInfos.data();
            }
            else if (auto resources = std::get_if<1>(&resourceVarient))
            {
                auto& bufferInfos = descriptorBufferInfos.emplace_back();

                for (auto passAttachment : resources->views)
                {
                    auto buffer = m_context->m_bufferOwner.Get(passAttachment->attachment->handle);

                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = buffer->handle;
                    bufferInfo.offset = passAttachment->info.byteOffset;
                    bufferInfo.range  = passAttachment->info.byteSize == 0 ? VK_WHOLE_SIZE : passAttachment->info.byteSize;
                    bufferInfos.push_back(bufferInfo);
                }

                writeInfo.dstArrayElement = resources->arrayOffset;
                writeInfo.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // todo support storage buffers, and more complex types
                writeInfo.descriptorCount = bufferInfos.size();
                writeInfo.pBufferInfo     = bufferInfos.data();
            }
            else if (auto resources = std::get_if<2>(&resourceVarient))
            {
                auto& imageInfos = descriptorImageInfos.emplace_back();

                for (auto samplerHandle : resources->samplers)
                {
                    auto sampler = m_context->m_samplerOwner.Get(samplerHandle);

                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.sampler = sampler->handle;
                    imageInfos.push_back(imageInfo);
                }

                writeInfo.dstArrayElement = resources->arrayOffset;
                writeInfo.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writeInfo.descriptorCount = imageInfos.size();
                writeInfo.pImageInfo      = imageInfos.data();
            }
            else
            {
                RHI_UNREACHABLE();
            }

            // writeInfo.pTexelBufferView; todo

            writeInfos.push_back(writeInfo);
        }

        vkUpdateDescriptorSets(m_context->m_device, writeInfos.size(), writeInfos.data(), 0, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ResourcePool
    ///////////////////////////////////////////////////////////////////////////

    ResourcePool::~ResourcePool()
    {
        vmaDestroyPool(m_context->m_allocator, m_pool);
    }

    VkResult ResourcePool::Init(const RHI::ResourcePoolCreateInfo& createInfo)
    {
        m_poolInfo = createInfo;

        VmaPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.flags                  = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        poolCreateInfo.blockSize              = createInfo.blockSize;
        poolCreateInfo.minBlockCount          = createInfo.minBlockCount;
        poolCreateInfo.maxBlockCount          = createInfo.maxBlockCount;
        poolCreateInfo.priority               = 1.0f;
        poolCreateInfo.minAllocationAlignment = createInfo.minBlockAlignment;
        poolCreateInfo.pMemoryAllocateNext    = nullptr;
        poolCreateInfo.memoryTypeIndex        = m_context->GetMemoryTypeIndex(createInfo.heapType);

        return vmaCreatePool(m_context->m_allocator, &poolCreateInfo, &m_pool);
    }

    RHI::Result<RHI::Handle<RHI::Image>> ResourcePool::Allocate(const RHI::ImageCreateInfo& createInfo)
    {
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.pool = m_pool;

        auto [handle, image] = m_context->m_imageOwner.InsertZerod();
        if (auto result = image.Init(m_context, allocationInfo, createInfo, this, false); RHI::IsError(result))
        {
            return result;
        }

        return RHI::Handle<RHI::Image>(handle);
    }

    RHI::Result<RHI::Handle<RHI::Buffer>> ResourcePool::Allocate(const RHI::BufferCreateInfo& createInfo)
    {
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.pool = m_pool;

        auto [handle, buffer] = m_context->m_bufferOwner.InsertZerod();
        if (auto result = buffer.Init(m_context, allocationInfo, createInfo, this, false); RHI::IsError(result))
        {
            return result;
        }

        return RHI::Handle<RHI::Buffer>(handle);
    }

    void ResourcePool::Free(RHI::Handle<RHI::Image> image)
    {
        auto resource = m_context->m_imageOwner.Get(image);
        RHI_ASSERT(resource);

        vmaDestroyImage(m_context->m_allocator, resource->handle, resource->allocation.handle);
    }

    void ResourcePool::Free(RHI::Handle<RHI::Buffer> buffer)
    {
        auto resource = m_context->m_bufferOwner.Get(buffer);
        RHI_ASSERT(resource);

        vmaDestroyBuffer(m_context->m_allocator, resource->handle, resource->allocation.handle);
    }

    size_t ResourcePool::GetSize(RHI::Handle<RHI::Image> image) const
    {
        auto& owner = m_context->m_imageOwner;
        return owner.Get(image)->GetMemoryRequirements(m_context->m_device).size;
    }

    size_t ResourcePool::GetSize(RHI::Handle<RHI::Buffer> buffer) const
    {
        auto& owner = m_context->m_bufferOwner;
        return owner.Get(buffer)->GetMemoryRequirements(m_context->m_device).size;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Swapchain
    ///////////////////////////////////////////////////////////////////////////

    Swapchain::~Swapchain()
    {
        auto context = static_cast<Context*>(m_context);

        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);
        vkDestroySurfaceKHR(context->m_instance, m_surface, nullptr);
    }

    VkResult Swapchain::Init(const RHI::SwapchainCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        m_swapchainInfo = createInfo;

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;
        VkResult result           = vkCreateSemaphore(context->m_device, &semaphoreCreateInfo, nullptr, &m_imageReadySemaphore);
        VULKAN_RETURN_VKERR_CODE(result);

#ifdef RHI_PLATFORM_WINDOWS
        // create win32 surface
        VkWin32SurfaceCreateInfoKHR vkCreateInfo;
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        // vkCreateInfo.hinstance = static_cast<HINSTANCE>(createInfo.win32Window.hinstance);
        vkCreateInfo.hwnd  = static_cast<HWND>(createInfo.win32Window.hwnd);
        result             = vkCreateWin32SurfaceKHR(context->m_instance, &vkCreateInfo, nullptr, &m_surface);
        VULKAN_RETURN_VKERR_CODE(result);
#endif

        VkBool32 surfaceSupportPresent = VK_FALSE;
        result                         = vkGetPhysicalDeviceSurfaceSupportKHR(
            context->m_physicalDevice, context->m_graphicsQueueFamilyIndex, m_surface, &surfaceSupportPresent);
        RHI_ASSERT(result == VK_SUCCESS && surfaceSupportPresent == VK_TRUE);

        result = CreateNativeSwapchain();
        VULKAN_RETURN_VKERR_CODE(result);
        return result;
    }

    RHI::ResultCode Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        auto context = static_cast<Context*>(m_context);

        m_swapchainInfo.imageSize = { newWidth, newHeight, 0 };

        VkResult result = vkQueueWaitIdle(context->m_graphicsQueue);
        VULKAN_ASSERT_SUCCESS(result);

        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);

        result = CreateNativeSwapchain();

        return ConvertResult(result);
    }

    RHI::ResultCode Swapchain::Present(RHI::Pass& passBase)
    {
        auto& pass = static_cast<Pass&>(passBase);

        // Present current image to be rendered.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext              = nullptr;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &pass.m_signalSemaphore;
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &m_swapchain;
        presentInfo.pImageIndices      = &m_currentImageIndex;
        presentInfo.pResults           = &m_lastPresentResult;
        VkResult result                = vkQueuePresentKHR(m_context->m_graphicsQueue, &presentInfo);
        VULKAN_RETURN_ERR_CODE(result);

        result = vkAcquireNextImageKHR(m_context->m_device, m_swapchain, 1000000, m_imageReadySemaphore, VK_NULL_HANDLE, &m_currentImageIndex);
        VULKAN_RETURN_ERR_CODE(result);

        return RHI::ResultCode::Success;
    }

    VkResult Swapchain::CreateNativeSwapchain()
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        VkResult                 result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->m_physicalDevice, m_surface, &surfaceCapabilities);
        VULKAN_RETURN_VKERR_CODE(result);

        auto surfaceFormat  = GetSurfaceFormat(ConvertFormat(m_swapchainInfo.imageFormat));
        auto minImageCount  = std::clamp(m_swapchainInfo.imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        auto minImageWidth  = std::clamp(m_swapchainInfo.imageSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        auto minImageHeight = std::clamp(m_swapchainInfo.imageSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;
        createInfo.surface               = m_surface;
        createInfo.minImageCount         = minImageCount;
        createInfo.imageFormat           = surfaceFormat.format;
        createInfo.imageColorSpace       = surfaceFormat.colorSpace;
        createInfo.imageExtent.width     = minImageWidth;
        createInfo.imageExtent.height    = minImageHeight;
        createInfo.imageArrayLayers      = 1;
        createInfo.imageUsage            = ConvertImageUsageFlags(m_swapchainInfo.imageUsage);
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        createInfo.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha        = GetCompositeAlpha(surfaceCapabilities);
        createInfo.presentMode           = GetPresentMode();
        createInfo.clipped               = VK_TRUE;
        createInfo.oldSwapchain          = m_swapchain;

        result = vkCreateSwapchainKHR(m_context->m_device, &createInfo, nullptr, &m_swapchain);
        VULKAN_RETURN_VKERR_CODE(result);
        if (result != VK_SUCCESS)
            return result;

        uint32_t imagesCount;
        result = vkGetSwapchainImagesKHR(m_context->m_device, m_swapchain, &imagesCount, nullptr);
        std::vector<VkImage> images;
        images.resize(imagesCount);
        result = vkGetSwapchainImagesKHR(m_context->m_device, m_swapchain, &imagesCount, images.data());
        VULKAN_RETURN_VKERR_CODE(result);
        if (result != VK_SUCCESS)
            return result;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext                 = nullptr;
        imageInfo.flags                 = 0;
        imageInfo.imageType             = VK_IMAGE_TYPE_2D;
        imageInfo.format                = surfaceFormat.format;
        imageInfo.extent.width          = createInfo.imageExtent.width;
        imageInfo.extent.height         = createInfo.imageExtent.height;
        imageInfo.extent.depth          = 1;
        imageInfo.mipLevels             = 1;
        imageInfo.arrayLayers           = createInfo.imageArrayLayers;
        imageInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage                 = createInfo.imageUsage;
        imageInfo.sharingMode           = createInfo.imageSharingMode;
        imageInfo.queueFamilyIndexCount = createInfo.queueFamilyIndexCount;
        imageInfo.pQueueFamilyIndices   = createInfo.pQueueFamilyIndices;
        imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

        for (auto imageHandles : images)
        {
            Vulkan::Image image{};
            image.handle     = imageHandles;
            image.createInfo = imageInfo;
            image.swapchain  = this;

            auto handle = m_context->m_imageOwner.Insert(image);
            m_images.push_back(handle);
        }

        result = vkAcquireNextImageKHR(m_context->m_device, m_swapchain, 1000000, m_imageReadySemaphore, VK_NULL_HANDLE, &m_currentImageIndex);
        VULKAN_RETURN_VKERR_CODE(result);

        return result;
    }

    VkSurfaceFormatKHR Swapchain::GetSurfaceFormat(VkFormat format)
    {
        uint32_t formatsCount;

        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->m_physicalDevice, m_surface, &formatsCount, nullptr);
        VULKAN_ASSERT_SUCCESS(result);

        std::vector<VkSurfaceFormatKHR> formats{};
        formats.resize(formatsCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->m_physicalDevice, m_surface, &formatsCount, formats.data());

        for (auto surfaceFormat : formats)
        {
            if (surfaceFormat.format == format)
                return surfaceFormat;
        }

        RHI_UNREACHABLE();

        return formats.front();
    }

    VkCompositeAlphaFlagBitsKHR Swapchain::GetCompositeAlpha(VkSurfaceCapabilitiesKHR surfaceCapabilities)
    {
        VkCompositeAlphaFlagBitsKHR preferedModes[] = { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                                        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
                                                        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                                                        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR };

        for (VkCompositeAlphaFlagBitsKHR mode : preferedModes)
        {
            if (surfaceCapabilities.supportedCompositeAlpha & mode)
            {
                return mode;
            }
        }

        RHI_UNREACHABLE();

        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    VkPresentModeKHR Swapchain::GetPresentMode()
    {
        auto context = static_cast<Context*>(m_context);

        uint32_t presentModesCount;
        auto     result = vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, nullptr);
        VULKAN_ASSERT_SUCCESS(result);
        std::vector<VkPresentModeKHR> presentModes{};
        presentModes.resize(presentModesCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, presentModes.data());

        VkPresentModeKHR preferredModes[] = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR };

        for (VkPresentModeKHR preferredMode : preferredModes)
        {
            for (VkPresentModeKHR supportedMode : presentModes)
            {
                if (supportedMode == preferredMode)
                {
                    return supportedMode;
                }
            }
        }

        // context->GetDebugMessenger().LogWarnning("Could not find preferred presentation mode");

        return presentModes[0];
    }

} // namespace Vulkan