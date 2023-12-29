#pragma once

#include <vk_mem_alloc.h>

#include <RHI/Resources.hpp>
#include <RHI/FrameScheduler.hpp>

namespace Vulkan
{

    inline static bool IsWriteAccess(RHI::AttachmentAccess access)
    {
        return (access & RHI::AttachmentAccess::Write) == RHI::AttachmentAccess::Write;
    }

    inline static bool IsRenderTarget(RHI::AttachmentUsage usage)
    {
        return usage == RHI::AttachmentUsage::RenderTarget | usage == RHI::AttachmentUsage::Depth;
    }

    inline static VkAttachmentLoadOp ConvertLoadOp(RHI::ImageLoadOperation op)
    {
        switch (op)
        {
        case RHI::ImageLoadOperation::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case RHI::ImageLoadOperation::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
        case RHI::ImageLoadOperation::Discard:  return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default:                                RHI_UNREACHABLE(); return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }

    inline static VkAttachmentStoreOp ConvertStoreOp(RHI::ImageStoreOperation op)
    {
        switch (op)
        {
        case RHI::ImageStoreOperation::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case RHI::ImageStoreOperation::Store:    return VK_ATTACHMENT_STORE_OP_STORE;
        case RHI::ImageStoreOperation::Discard:  return VK_ATTACHMENT_STORE_OP_NONE;
        default:                                 RHI_UNREACHABLE(); return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
        }
    }

    inline static VkImageLayout ConvertImageLayout(RHI::AttachmentUsage usage, RHI::AttachmentAccess access)
    {
        if (usage == RHI::AttachmentUsage::ShaderStorage && IsWriteAccess(access))
        {
            return VK_IMAGE_LAYOUT_GENERAL;
        }

        switch (usage)
        {
        case RHI::AttachmentUsage::ShaderResource: return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        case RHI::AttachmentUsage::RenderTarget:   return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case RHI::AttachmentUsage::Depth:          return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
        case RHI::AttachmentUsage::Copy:           return IsWriteAccess(access) ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        default:
            {
                RHI_UNREACHABLE();
            }
        }

        return VK_IMAGE_LAYOUT_MAX_ENUM;
    }

    inline static VkAccessFlags2 ConvertPipelineAccess(RHI::AttachmentUsage usage, RHI::AttachmentAccess access, bool access2)
    {
        switch (usage)
        {
        case RHI::AttachmentUsage::ShaderResource: return VK_IMAGE_USAGE_SAMPLED_BIT;
        case RHI::AttachmentUsage::ShaderStorage:  return IsWriteAccess(access) ? VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_WRITE_BIT;
        case RHI::AttachmentUsage::RenderTarget:   return access2 ? VK_ACCESS_2_NONE : VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        case RHI::AttachmentUsage::Depth:          return access2 ? VK_ACCESS_2_NONE : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case RHI::AttachmentUsage::Copy:           return IsWriteAccess(access) ? VK_ACCESS_2_TRANSFER_READ_BIT : VK_ACCESS_2_TRANSFER_WRITE_BIT;
        default:                                   RHI_UNREACHABLE(); return {};
        }
    }

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(RHI::ShaderStage stage)
    {
        switch (stage)
        {
        case RHI::ShaderStage::Vertex:  return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        case RHI::ShaderStage::Pixel:   return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        case RHI::ShaderStage::Compute: return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        default:
            {
                RHI_UNREACHABLE();
            }
        }

        return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    }

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(RHI::AttachmentUsage usage, RHI::ShaderStage stage)
    {
        switch (usage)
        {
        case RHI::AttachmentUsage::None:           return VK_PIPELINE_STAGE_2_NONE;
        case RHI::AttachmentUsage::ShaderResource: return ConvertPipelineStageFlags(stage);
        case RHI::AttachmentUsage::ShaderStorage:  return ConvertPipelineStageFlags(stage);
        case RHI::AttachmentUsage::RenderTarget:   return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case RHI::AttachmentUsage::Depth:          return VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        case RHI::AttachmentUsage::Copy:           return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        default:                                   RHI_UNREACHABLE();
        }

        return VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
    }

    inline static VkSampleCountFlagBits ConvertSampleCount(RHI::SampleCount sampleCount)
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

    inline static VkSampleCountFlags ConvertSampleCountFlags(RHI::Flags<RHI::SampleCount> sampleCountFlags)
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

    inline static VkImageUsageFlagBits ConvertImageUsage(RHI::ImageUsage imageUsage)
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

    inline static VkImageUsageFlags ConvertImageUsageFlags(RHI::Flags<RHI::ImageUsage> imageUsageFlags)
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

    inline static VkImageType ConvertImageType(RHI::ImageType imageType)
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

    inline static VkImageAspectFlagBits ConvertImageAspect(RHI::Flags<RHI::ImageAspect> imageAspect)
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

    inline static VkImageAspectFlags ConvertImageAspect(RHI::ImageAspect imageAspect)
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

    inline static VkComponentSwizzle ConvertComponentSwizzle(RHI::ComponentSwizzle componentSwizzle)
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

    inline static VkBufferUsageFlagBits ConvertBufferUsage(RHI::BufferUsage bufferUsage)
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

    inline static VkBufferUsageFlags ConvertBufferUsageFlags(RHI::Flags<RHI::BufferUsage> bufferUsageFlags)
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

    inline static VkShaderStageFlagBits ConvertShaderStage(RHI::ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
        case RHI::ShaderStage::Vertex:  return VK_SHADER_STAGE_VERTEX_BIT;
        case RHI::ShaderStage::Pixel:   return VK_SHADER_STAGE_FRAGMENT_BIT;
        case RHI::ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                        RHI_UNREACHABLE(); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkShaderStageFlags ConvertShaderStage(RHI::Flags<RHI::ShaderStage> shaderStageFlags)
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

    inline static VkDescriptorType ConvertDescriptorType(RHI::ShaderBindingType bindingType)
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

    inline static VkAccessFlags ConvertAccessFlags(RHI::ShaderBindingAccess bindingAccess)
    {
        switch (bindingAccess)
        {
        case RHI::ShaderBindingAccess::OnlyRead:  return VK_ACCESS_SHADER_READ_BIT;
        case RHI::ShaderBindingAccess::ReadWrite: return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        default:                                  RHI_UNREACHABLE(); return VK_ACCESS_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkVertexInputRate ConvertVertexInputRate(RHI::PipelineVertexInputRate inputRate)
    {
        switch (inputRate)
        {
        case RHI::PipelineVertexInputRate::PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case RHI::PipelineVertexInputRate::PerVertex:   return VK_VERTEX_INPUT_RATE_VERTEX;
        default:                                        RHI_UNREACHABLE(); return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    inline static VkCullModeFlags ConvertCullModeFlags(RHI::PipelineRasterizerStateCullMode cullMode)
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

    inline static VkPolygonMode ConvertPolygonMode(RHI::PipelineRasterizerStateFillMode fillMode)
    {
        switch (fillMode)
        {
        case RHI::PipelineRasterizerStateFillMode::Point:    return VK_POLYGON_MODE_POINT;
        case RHI::PipelineRasterizerStateFillMode::Triangle: return VK_POLYGON_MODE_FILL;
        case RHI::PipelineRasterizerStateFillMode::Line:     return VK_POLYGON_MODE_LINE;
        default:                                             RHI_UNREACHABLE(); return VK_POLYGON_MODE_MAX_ENUM;
        }
    }

    inline static VkPrimitiveTopology ConvertPrimitiveTopology(RHI::PipelineTopologyMode topologyMode)
    {
        switch (topologyMode)
        {
        case RHI::PipelineTopologyMode::Points:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case RHI::PipelineTopologyMode::Lines:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case RHI::PipelineTopologyMode::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:                                   RHI_UNREACHABLE(); return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }
    }

    inline static VkFrontFace ConvertFrontFace(RHI::PipelineRasterizerStateFrontFace frontFace)
    {
        switch (frontFace)
        {
        case RHI::PipelineRasterizerStateFrontFace::Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
        case RHI::PipelineRasterizerStateFrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:                                                      RHI_UNREACHABLE(); return VK_FRONT_FACE_MAX_ENUM;
        }
    }

    inline static VkCompareOp ConvertCompareOp(RHI::CompareOperator compareOperator)
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

    inline static VkFilter ConvertFilter(RHI::SamplerFilter samplerFilter)
    {
        switch (samplerFilter)
        {
        case RHI::SamplerFilter::Point:  return VK_FILTER_NEAREST;
        case RHI::SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default:                         RHI_UNREACHABLE(); return VK_FILTER_MAX_ENUM;
        }
    }

    inline static VkSamplerAddressMode ConvertSamplerAddressMode(RHI::SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case RHI::SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case RHI::SamplerAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:                              RHI_UNREACHABLE(); return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    inline static VkCompareOp ConvertCompareOp(RHI::SamplerCompareOperation compareOperation)
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

    inline static VkBlendFactor ConvertBlendFactor(RHI::BlendFactor blendFactor)
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

    inline static VkBlendOp ConvertBlendOp(RHI::BlendEquation blendEquation)
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

    inline static VkImageSubresource ConvertSubresource(const RHI::ImageSubresource& subresource)
    {
        auto vkSubresource = VkImageSubresource{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel = subresource.mipLevel;
        vkSubresource.arrayLayer = subresource.arrayLayer;
        return vkSubresource;
    }

    inline static VkImageSubresourceLayers ConvertSubresourceLayer(const RHI::ImageSubresourceLayers& subresource)
    {
        auto vkSubresource = VkImageSubresourceLayers{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel = subresource.mipLevel;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount = subresource.arrayCount;
        return vkSubresource;
    }

    inline static VkImageSubresourceRange ConvertSubresourceRange(const RHI::ImageSubresourceRange& subresource)
    {
        auto vkSubresource = VkImageSubresourceRange{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.baseMipLevel = subresource.mipBase;
        vkSubresource.levelCount = subresource.mipLevelCount;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount = subresource.arrayCount;
        return vkSubresource;
    }

    inline static VkExtent3D ConvertExtent3D(RHI::ImageSize3D size)
    {
        return { size.width, size.height, size.depth };
    }

    inline static VkExtent2D ConvertExtent2D(RHI::ImageSize3D size)
    {
        RHI_ASSERT(size.depth == 0);
        return { size.width, size.height };
    }

    inline static VkOffset3D ConvertOffset3D(RHI::ImageOffset offset)
    {
        return { offset.x, offset.y, offset.z };
    }

    inline static VkOffset3D ConvertOffset2D(RHI::ImageOffset offset)
    {
        RHI_ASSERT(offset.z == 0);
        return { offset.x, offset.y, offset.z };
    }

} // namespace Vulkan