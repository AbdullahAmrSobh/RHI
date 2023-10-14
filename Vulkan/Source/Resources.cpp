#include <RHI/Pass.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#include "Context.hpp"
#include "Format.inl"
#include "Resources.hpp"

#include <Windows.h>

namespace Vulkan
{
    VkSampleCountFlagBits ConvertToVkSampleCount(RHI::SampleCount sampleCount)
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

    VkSampleCountFlags ConvertToVkSampleCountFlags(RHI::Flags<RHI::SampleCount> sampleCountFlags)
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

    VkImageUsageFlagBits ConvertToVkImageUsage(RHI::ImageUsage imageUsage)
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

    VkImageUsageFlags ConvertToVkImageUsageFlags(RHI::Flags<RHI::ImageUsage> imageUsageFlags)
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

    VkImageType ConvertToVkImageType(RHI::ImageType imageType)
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

    VkImageAspectFlagBits ConvertToVkImageAspect(RHI::Flags<RHI::ImageAspect> imageAspect)
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

    VkImageAspectFlags ConvertToVkImageAspect(RHI::ImageAspect imageAspect)
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

    VkComponentSwizzle ConvertToVkComponentSwizzle(RHI::ComponentSwizzle componentSwizzle)
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

    VkBufferUsageFlagBits ConvertToVkBufferUsage(RHI::BufferUsage bufferUsage)
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

    VkBufferUsageFlags ConvertToVkBufferUsageFlags(RHI::Flags<RHI::BufferUsage> bufferUsageFlags)
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

    VkShaderStageFlagBits ConvertToVkShaderStage(RHI::ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
        case RHI::ShaderStage::Vertex:  return VK_SHADER_STAGE_VERTEX_BIT;
        case RHI::ShaderStage::Pixel:   return VK_SHADER_STAGE_FRAGMENT_BIT;
        case RHI::ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                        RHI_UNREACHABLE(); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkShaderStageFlags ConvertToVkShaderStage(RHI::Flags<RHI::ShaderStage> shaderStageFlags)
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

    VkDescriptorType ConvertToVkDescriptorType(RHI::ShaderBindingType bindingType)
    {
        switch (bindingType)
        {
        case RHI::ShaderBindingType::None:    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        case RHI::ShaderBindingType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
        case RHI::ShaderBindingType::Image:   return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case RHI::ShaderBindingType::Buffer:  return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        default:                              RHI_UNREACHABLE(); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    VkAccessFlags ConvertToVkAccessFlags(RHI::ShaderBindingAccess bindingAccess)
    {
        switch (bindingAccess)
        {
        case RHI::ShaderBindingAccess::OnlyRead:  return VK_ACCESS_SHADER_READ_BIT;
        case RHI::ShaderBindingAccess::ReadWrite: return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        default:                                  RHI_UNREACHABLE(); return VK_ACCESS_FLAG_BITS_MAX_ENUM;
        }
    }

    VkVertexInputRate ConvertToVkVertexInputRate(RHI::PipelineVertexInputRate inputRate)
    {
        switch (inputRate)
        {
        case RHI::PipelineVertexInputRate::PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case RHI::PipelineVertexInputRate::PerVertex:   return VK_VERTEX_INPUT_RATE_VERTEX;
        default:                                        RHI_UNREACHABLE(); return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    VkCullModeFlags ConvertToVkCullModeFlags(RHI::PipelineRasterizerStateCullMode cullMode)
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

    VkPolygonMode ConvertToVkPolygonMode(RHI::PipelineRasterizerStateFillMode fillMode)
    {
        switch (fillMode)
        {
        case RHI::PipelineRasterizerStateFillMode::Point:    return VK_POLYGON_MODE_POINT;
        case RHI::PipelineRasterizerStateFillMode::Triangle: return VK_POLYGON_MODE_FILL;
        case RHI::PipelineRasterizerStateFillMode::Line:     return VK_POLYGON_MODE_LINE;
        default:                                             RHI_UNREACHABLE(); return VK_POLYGON_MODE_MAX_ENUM;
        }
    }

    VkPrimitiveTopology ConvertToVkPrimitiveTopology(RHI::PipelineTopologyMode topologyMode)
    {
        switch (topologyMode)
        {
        case RHI::PipelineTopologyMode::Points:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case RHI::PipelineTopologyMode::Lines:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case RHI::PipelineTopologyMode::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:                                   RHI_UNREACHABLE(); return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }
    }

    VkFrontFace ConvertToVkFrontFace(RHI::PipelineRasterizerStateFrontFace frontFace)
    {
        switch (frontFace)
        {
        case RHI::PipelineRasterizerStateFrontFace::Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
        case RHI::PipelineRasterizerStateFrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:                                                      RHI_UNREACHABLE(); return VK_FRONT_FACE_MAX_ENUM;
        }
    }

    VkCompareOp ConvertToVkCompareOp(RHI::CompareOperator compareOperator)
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

    VkFilter ConvertToVkFilter(RHI::SamplerFilter samplerFilter)
    {
        switch (samplerFilter)
        {
        case RHI::SamplerFilter::Point:  return VK_FILTER_NEAREST;
        case RHI::SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default:                         RHI_UNREACHABLE(); return VK_FILTER_MAX_ENUM;
        }
    }

    VkSamplerAddressMode ConvertToVkSamplerAddressMode(RHI::SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case RHI::SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case RHI::SamplerAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:                              RHI_UNREACHABLE(); return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    VkCompareOp ConvertToVkCompareOp(RHI::SamplerCompareOperation compareOperation)
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

    VkBlendFactor ConvertToVkBlendFactor(RHI::BlendFactor blendFactor)
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

    VkBlendOp ConvertToVkBlendOp(RHI::BlendEquation blendEquation)
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

    ShaderModule::~ShaderModule()
    {
        auto context = static_cast<Context*>(m_context);

        vkDestroyShaderModule(context->m_device, m_shaderModule, nullptr);
    }

    VkResult ShaderModule::Init(const RHI::ShaderModuleCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.pNext = nullptr;
        moduleCreateInfo.flags = {};
        moduleCreateInfo.codeSize = createInfo.size * 4;
        moduleCreateInfo.pCode = (uint32_t*)createInfo.code;

        RHI_ASSERT(moduleCreateInfo.codeSize % 4 == 0);

        VkResult result = vkCreateShaderModule(context->m_device, &moduleCreateInfo, nullptr, &m_shaderModule);
        RHI_ASSERT(result == VK_SUCCESS);
        return result;
    }

    ShaderBindGroupAllocator::~ShaderBindGroupAllocator()
    {
    }

    VkResult ShaderBindGroupAllocator::Init()
    {
        return VK_SUCCESS;
    }

    std::vector<RHI::Handle<RHI::ShaderBindGroup>> ShaderBindGroupAllocator::AllocateShaderBindGroups(RHI::TL::Span<const RHI::ShaderBindGroupLayout> layouts)
    {
        return {};
    }

    void ShaderBindGroupAllocator::Free(RHI::TL::Span<RHI::Handle<RHI::ShaderBindGroup>> groups)
    {
        auto context = static_cast<Context*>(m_context);

        // vkFreeDescriptorSets()
    }

    void ShaderBindGroupAllocator::Update(RHI::Handle<RHI::ShaderBindGroup> group, const RHI::ShaderBindGroupData& data)
    {
        auto context = static_cast<Context*>(m_context);
        auto resourcesManager = context->m_resourceManager.get();
        auto descriptorSet = resourcesManager->m_descriptorSetOwner.Get(group);

        std::vector<std::vector<VkDescriptorImageInfo>> descriptorImageInfos;
        std::vector<std::vector<VkDescriptorBufferInfo>> descriptorBufferInfos;
        std::vector<std::vector<VkBufferView>> descriptorBufferViews;

        std::vector<VkWriteDescriptorSet> writeInfos;
        // writeInfos.reserve(data.m_imageBindings.size() + data.m_bufferBindings.size() + data.m_samplerBindings.size());

        // // bind image resources
        // for (auto binding : data.m_imageBindings)
        // {
        //     std::vector<VkDescriptorImageInfo> imageInfos;

        //     for (auto view : binding.views)
        //     {
        //         auto view = resourcesManager->m_imageViewOwner.Get(view)

        //         VkDescriptorImageInfo imageInfo{};
        //         imageInfo.imageView;
        //         imageInfo.imageLayout;
        //         imageInfos.push_back(imageInfo);
        //     }

        //     VkWriteDescriptorSet writeInfo{};
        //     writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //     writeInfo.pNext = nullptr;
        //     writeInfo.dstSet = descriptorSet->handle;
        //     writeInfo.dstBinding = binding.index;
        //     writeInfo.dstArrayElement = binding.arrayOffset;
        //     writeInfo.descriptorCount = binding.elementsCount;
        //     writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        //     writeInfo.pImageInfo = imageInfos.data();

        //     descriptorImageInfos.push_back(imageInfos);
        //     writeInfos.push_back(writeInfo);
        // }

        // // bind buffer resources
        // for (auto binding : data.m_bufferBindings)
        // {
        //     std::vector<VkDescriptorBufferInfo> bufferInfos;
        //     for (auto view : binding.views)
        //     {
        //         VkDescriptorBufferInfo bufferInfo{};

        //         bufferInfos.push_back(bufferInfo);
        //     }

        //     VkWriteDescriptorSet writeInfo{};
        //     writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //     writeInfo.pNext = nullptr;
        //     writeInfo.dstSet = descriptorSet->handle;
        //     writeInfo.dstBinding = binding.index;
        //     writeInfo.dstArrayElement = binding.arrayOffset;
        //     writeInfo.descriptorCount = binding.elementsCount;
        //     writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        //     writeInfo.pBufferInfo = bufferInfos.data();

        //     descriptorBufferInfos.push_back(bufferInfos);
        //     writeInfos.push_back(writeInfo);
        // }

        // // bind sampler resources
        // for (auto binding : data.m_samplerBindings)
        // {
        //     std::vector<VkDescriptorImageInfo> samplerInfos;
        //     for (auto view : binding.samplers)
        //     {
        //         VkDescriptorImageInfo sampler{};
        //         sampler.sampler;
        //         samplerInfos.push_back(sampler);
        //     }

        //     VkWriteDescriptorSet writeInfo{};
        //     writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //     writeInfo.pNext = nullptr;
        //     writeInfo.dstSet = descriptorSet->handle;
        //     writeInfo.dstBinding = binding.index;
        //     writeInfo.dstArrayElement = binding.arrayOffset;
        //     writeInfo.descriptorCount = binding.elementsCount;
        //     writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        //     writeInfo.pImageInfo = samplerInfos.data();

        //     descriptorImageInfos.push_back(samplerInfos);
        //     writeInfos.push_back(writeInfo);
        // }

        vkUpdateDescriptorSets(context->m_device, writeInfos.size(), writeInfos.data(), 0, nullptr);
    }

    ResourcePool::~ResourcePool()
    {
        auto context = static_cast<Context*>(m_context);
        auto allocator = context->m_allocator;

        vmaDestroyPool(allocator, m_pool);
    }

    VkResult ResourcePool::Init(const RHI::ResourcePoolCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        m_poolInfo = createInfo;

        VmaPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.flags = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT;
        poolCreateInfo.blockSize = createInfo.blockSize;
        poolCreateInfo.minBlockCount = createInfo.minBlockCount;
        poolCreateInfo.maxBlockCount = createInfo.maxBlockCount;
        poolCreateInfo.priority = 1.0f;
        poolCreateInfo.minAllocationAlignment = createInfo.minBlockAlignment;
        poolCreateInfo.pMemoryAllocateNext = nullptr;

        VkResult result = vmaCreatePool(context->m_allocator, &poolCreateInfo, &m_pool);
        RHI_ASSERT(result == VK_SUCCESS);
        return result;
    }

    RHI::Result<RHI::Handle<RHI::Image>> ResourcePool::Allocate(const RHI::ImageCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.pool = m_pool;
        // allocationCreateInfo.flags = VMA_ALLOCATION_F;

        VkImageCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.imageType = ConvertToVkImageType(createInfo.type);
        vkCreateInfo.format = GetFormat(createInfo.format);
        vkCreateInfo.extent.width = createInfo.size.width;
        vkCreateInfo.extent.height = createInfo.size.height;
        vkCreateInfo.mipLevels = createInfo.mipLevels;
        vkCreateInfo.arrayLayers = createInfo.arrayCount;
        vkCreateInfo.samples = ConvertToVkSampleCount(createInfo.sampleCount);
        vkCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        vkCreateInfo.usage = ConvertToVkImageUsageFlags(createInfo.usageFlags);
        vkCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateInfo.queueFamilyIndexCount = 0;
        vkCreateInfo.pQueueFamilyIndices = nullptr;
        vkCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        Image resource{};
        resource.pool = this;
        resource.swapchain = nullptr;

        auto result = vmaCreateImage(context->m_allocator, &vkCreateInfo, &allocationCreateInfo, &resource.handle, &resource.allocationHandle, &resource.allocationInfo);

        if (result == VK_SUCCESS)
        {
            auto handle = context->m_resourceManager->m_imageOwner.Insert(resource);
            return { handle, RHI::ResultCode::Success };
        }

        RHI_UNREACHABLE();

        return { {}, RHI::ResultCode::ErrorOutOfMemory };
    }

    RHI::Result<RHI::Handle<RHI::Buffer>> ResourcePool::Allocate(const RHI::BufferCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.pool = m_pool;
        // allocationCreateInfo.flags = VMA_ALLOCATION_F;

        VkBufferCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.size = createInfo.byteSize;
        vkCreateInfo.usage = ConvertToVkBufferUsageFlags(createInfo.usageFlags);
        vkCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateInfo.queueFamilyIndexCount = 0;
        vkCreateInfo.pQueueFamilyIndices = nullptr;

        Buffer resource{};
        resource.pool = this;

        auto result = vmaCreateBuffer(context->m_allocator, &vkCreateInfo, &allocationCreateInfo, &resource.handle, &resource.allocationHandle, &resource.allocationInfo);

        if (result == VK_SUCCESS)
        {
            auto handle = context->m_resourceManager->m_bufferOwner.Insert(resource);
            return { handle, RHI::ResultCode::Success };
        }

        RHI_UNREACHABLE();

        return { {}, RHI::ResultCode::ErrorOutOfMemory };
    }

    void ResourcePool::Free(RHI::Handle<RHI::Image> image)
    {
        auto context = static_cast<Context*>(m_context);

        auto resource = context->m_resourceManager->m_imageOwner.Get(image);
        RHI_ASSERT(resource);

        vmaDestroyImage(context->m_allocator, resource->handle, resource->allocationHandle);
    }

    void ResourcePool::Free(RHI::Handle<RHI::Buffer> buffer)
    {
        auto context = static_cast<Context*>(m_context);

        auto resource = context->m_resourceManager->m_bufferOwner.Get(buffer);
        RHI_ASSERT(resource);

        vmaDestroyBuffer(context->m_allocator, resource->handle, resource->allocationHandle);
    }

    size_t ResourcePool::GetSize(RHI::Handle<RHI::Image> image) const
    {
        auto context = static_cast<Context*>(m_context);
        return context->m_resourceManager->m_imageOwner.Get(image)->allocationInfo.size;
    }

    size_t ResourcePool::GetSize(RHI::Handle<RHI::Buffer> buffer) const
    {
        auto context = static_cast<Context*>(m_context);
        return context->m_resourceManager->m_bufferOwner.Get(buffer)->allocationInfo.size;
    }

    Swapchain::~Swapchain()
    {
        auto context = static_cast<Context*>(m_context);

        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);
        vkDestroySurfaceKHR(context->m_instance, m_surface, nullptr);
    }

    VkResult Swapchain::Init(const RHI::SwapchainCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

#ifdef RHI_PLATFORM_WINDOWS
        // create win32 surface
        VkWin32SurfaceCreateInfoKHR vkCreateInfo;
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        // vkCreateInfo.hinstance = static_cast<HINSTANCE>(createInfo.win32Window.hinstance);
        vkCreateInfo.hwnd = static_cast<HWND>(createInfo.win32Window.hwnd);
        VkResult result = vkCreateWin32SurfaceKHR(context->m_instance, &vkCreateInfo, nullptr, &m_surface);
        RHI_ASSERT(result == VK_SUCCESS);
#endif

        VkBool32 surfaceSupportPresent = VK_FALSE;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(
            context->m_physicalDevice, context->m_graphicsQueueFamilyIndex, m_surface, &surfaceSupportPresent);
        RHI_ASSERT(result == VK_SUCCESS && surfaceSupportPresent == VK_TRUE);

        result = CreateNativeSwapchain();
        RHI_ASSERT(result == VK_SUCCESS);
        return result;
    }

    RHI::ResultCode Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        auto context = static_cast<Context*>(m_context);

        m_swapchainInfo.imageSize = { newWidth, newHeight };

        VkResult result = vkQueueWaitIdle(context->m_graphicsQueue);

        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);

        result = CreateNativeSwapchain();

        return {};
    }

    RHI::ResultCode Swapchain::SetExclusiveFullScreenMode(bool enableFullscreen)
    {
        auto context = static_cast<Context*>(m_context);

        if (enableFullscreen && !m_fullscreenMode)
        {
            // auto result = vkAcquireFullScreenExclusiveModeEXT(context->m_device, m_swapchain);

            m_fullscreenMode = true;
        }
        else if (!enableFullscreen && m_fullscreenMode)
        {
            // auto result = vkReleaseFullScreenExclusiveModeEXT(context->m_device, m_swapchain);

            m_fullscreenMode = true;
        }

        return RHI::ResultCode::Success;
    }

    RHI::ResultCode Swapchain::Present()
    {
        auto context = static_cast<Context*>(m_context);

        // Present current image to be rendered.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount;
        presentInfo.pWaitSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.pImageIndices = &m_currentImageIndex;
        presentInfo.pResults = &m_lastPresentResult;
        VkResult result = vkQueuePresentKHR(context->m_graphicsQueue, &presentInfo);
        RHI_ASSERT(result == VK_SUCCESS);

        result = vkAcquireNextImageKHR(
            context->m_device, m_swapchain, SwapchainAcquireTime, VK_NULL_HANDLE, VK_NULL_HANDLE, &m_currentImageIndex);
        RHI_ASSERT(result == VK_SUCCESS);

        return {};
    }

    VkResult Swapchain::CreateNativeSwapchain()
    {
        auto context = static_cast<Context*>(m_context);

        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->m_physicalDevice, m_surface, &surfaceCapabilities);
        RHI_ASSERT(result == VK_SUCCESS);

        auto surfaceFormat = GetSurfaceFormat(GetFormat(m_swapchainInfo.imageFormat));
        auto minImageCount = std::clamp(m_swapchainInfo.imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        auto minImageWidth =
            std::clamp(m_swapchainInfo.imageSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        auto minImageHeight = std::clamp(
            m_swapchainInfo.imageSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = m_surface;
        createInfo.minImageCount = minImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent.width = minImageWidth;
        createInfo.imageExtent.height = minImageHeight;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = ConvertToVkImageUsageFlags(m_swapchainInfo.imageUsage);
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha = GetCompositeAlpha(surfaceCapabilities);
        createInfo.presentMode = GetPresentMode();
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_swapchain;

        result = vkCreateSwapchainKHR(context->m_device, &createInfo, nullptr, &m_swapchain);
        RHI_ASSERT(result == VK_SUCCESS);
        if (result == VK_SUCCESS)
            return result;

        uint32_t imagesCount;
        result = vkGetSwapchainImagesKHR(context->m_device, m_swapchain, &imagesCount, nullptr);
        std::vector<VkImage> images;
        images.resize(imagesCount);
        result = vkGetSwapchainImagesKHR(context->m_device, m_swapchain, &imagesCount, images.data());
        RHI_ASSERT(result == VK_SUCCESS);
        if (result == VK_SUCCESS)
            return result;

        for (auto imageHandles : images)
        {
            Vulkan::Image image {};
            image.handle = imageHandles;
            image.swapchain = this;
            image.format = createInfo.imageFormat;
            image.type = VK_IMAGE_TYPE_2D;

            auto handle = context->m_resourceManager->m_imageOwner.Insert(image);
            m_images.push_back(handle);
        }

        return result;
    }

    VkSurfaceFormatKHR Swapchain::GetSurfaceFormat(VkFormat format)
    {
        auto context = static_cast<Context*>(m_context);

        uint32_t formatsCount;

        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(context->m_physicalDevice, m_surface, &formatsCount, nullptr);

        std::vector<VkSurfaceFormatKHR> formats{};
        formats.resize(formatsCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(context->m_physicalDevice, m_surface, &formatsCount, formats.data());

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
        auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, nullptr);
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

        // m_context->GetDebugCallbacks().LogWarnning("Could not find preferred presentation mode");

        RHI_UNREACHABLE();

        return presentModes[0];
    }

    ResourceManager::ResourceManager(Context* context)
        : m_context(context)
        , m_imageOwner(512u)
        , m_bufferOwner(512u)
        , m_imageViewOwner(512u)
        , m_bufferViewOwner(512u)
        , m_descriptorSetLayoutOwner(512u)
        , m_descriptorSetOwner(512u)
        , m_pipelineLayoutOwner(512u)
        , m_graphicsPipelineOwner(512u)
        , m_computePipelineOwner(512u)
        , m_samplerOwner(512u)
        , m_fenceOwner(512u)
    {
    }

    ResourceManager::~ResourceManager()
    {
    }

    RHI::Result<RHI::Handle<Image>> ResourceManager::CreateImage(const VmaAllocationCreateInfo allocationInfo, const RHI::ImageCreateInfo& createInfo)
    {
        VkImageCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = {};
        vkCreateInfo.imageType = ConvertToVkImageType(createInfo.type);
        vkCreateInfo.format = GetFormat(createInfo.format);
        vkCreateInfo.extent.width = createInfo.size.width;
        vkCreateInfo.extent.height = createInfo.size.height;
        vkCreateInfo.extent.depth = createInfo.size.depth;
        vkCreateInfo.mipLevels = createInfo.mipLevels;
        vkCreateInfo.arrayLayers = createInfo.arrayCount;
        vkCreateInfo.samples = GetSampleCount(createInfo.sampleCount);
        vkCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        vkCreateInfo.usage = ConvertToVkImageUsageFlags(createInfo.usageFlags);
        vkCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateInfo.queueFamilyIndexCount = 0;
        vkCreateInfo.pQueueFamilyIndices = nullptr;
        vkCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        Image resource{};
        auto result = vmaCreateImage(m_context->m_allocator, &vkCreateInfo, &allocationInfo, &resource.handle, &resource.allocationHandle, &resource.allocationInfo);
        auto handle = m_imageOwner.Insert(resource);
        return { handle, ConvertToRhiResult(result) };
    }

    RHI::Result<RHI::Handle<Buffer>> ResourceManager::CreateBuffer(const VmaAllocationCreateInfo allocationInfo, const RHI::BufferCreateInfo& createInfo)
    {
        VkBufferCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = {};
        vkCreateInfo.size = createInfo.byteSize;
        vkCreateInfo.usage = ConvertToVkBufferUsageFlags(createInfo.usageFlags);
        vkCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateInfo.queueFamilyIndexCount = 0;
        vkCreateInfo.pQueueFamilyIndices = nullptr;

        Buffer resource{};
        auto result = vmaCreateBuffer(m_context->m_allocator, &vkCreateInfo, &allocationInfo, &resource.handle, &resource.allocationHandle, &resource.allocationInfo);
        auto handle = m_bufferOwner.Insert(resource);
        return { handle, ConvertToRhiResult(result) };
    }

    RHI::Result<RHI::Handle<ImageView>> ResourceManager::CreateImageView(RHI::Handle<Image> imageHandle, const RHI::ImageAttachmentUseInfo& useInfo)
    {
        auto image = m_imageOwner.Get(imageHandle);

        VkImageViewCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.image = image->handle;

        switch (image->type)
        {
        case VK_IMAGE_TYPE_1D: vkCreateInfo.viewType = useInfo.subresource.arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case VK_IMAGE_TYPE_2D: vkCreateInfo.viewType = useInfo.subresource.arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case VK_IMAGE_TYPE_3D: vkCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        default:               RHI_UNREACHABLE();
        }

        vkCreateInfo.format = image->format;
        vkCreateInfo.components.r = ConvertToVkComponentSwizzle(useInfo.components.r);
        vkCreateInfo.components.g = ConvertToVkComponentSwizzle(useInfo.components.g);
        vkCreateInfo.components.b = ConvertToVkComponentSwizzle(useInfo.components.b);
        vkCreateInfo.components.a = ConvertToVkComponentSwizzle(useInfo.components.a);
        vkCreateInfo.subresourceRange.aspectMask = ConvertToVkImageAspect(useInfo.subresource.imageAspects);
        vkCreateInfo.subresourceRange.baseMipLevel = useInfo.subresource.mipBase;
        vkCreateInfo.subresourceRange.levelCount = useInfo.subresource.mipCount;
        vkCreateInfo.subresourceRange.baseArrayLayer = useInfo.subresource.arrayBase;
        vkCreateInfo.subresourceRange.layerCount = useInfo.subresource.arrayCount;

        ImageView resource{};

        auto result = vkCreateImageView(m_context->m_device, &vkCreateInfo, nullptr, &resource.handle);
        RHI_ASSERT(result == VK_SUCCESS);
        return { m_imageViewOwner.Insert(resource), RHI::ResultCode::Success };
    }

    RHI::Result<RHI::Handle<BufferView>> ResourceManager::CreateBufferView(RHI::Handle<Buffer> bufferHandle, const RHI::BufferAttachmentUseInfo& useInfo)
    {
        auto buffer = m_bufferOwner.Get(bufferHandle);

        VkBufferViewCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.buffer = buffer->handle;
        vkCreateInfo.format = GetFormat(useInfo.format);
        vkCreateInfo.offset = useInfo.byteOffset;
        vkCreateInfo.range = useInfo.byteSize;

        BufferView resource{};

        auto result = vkCreateBufferView(m_context->m_device, &vkCreateInfo, nullptr, &resource.handle);
        RHI_ASSERT(result == VK_SUCCESS);
        return { m_bufferViewOwner.Insert(resource), RHI::ResultCode::Success };
    }

    RHI::Result<RHI::Handle<DescriptorSetLayout>> ResourceManager::CreateDescriptorSetLayout(const RHI::ShaderBindGroupLayout& layout)
    {
        uint32_t bindingsCount = 0;
        VkDescriptorSetLayoutBinding bindings[32];

        for (auto shaderBinding : layout.bindings)
        {
            auto& binding = bindings[bindingsCount];
            binding.binding = bindingsCount++;
            binding.descriptorType = ConvertToVkDescriptorType(shaderBinding.type);
            binding.descriptorCount = shaderBinding.arrayCount;
            binding.stageFlags = ConvertToVkShaderStage(shaderBinding.stages);
            binding.pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.bindingCount = bindingsCount;
        createInfo.pBindings = bindings;

        DescriptorSetLayout resource;

        auto result = vkCreateDescriptorSetLayout(m_context->m_device, &createInfo, nullptr, &resource.handle);
        RHI_ASSERT(result == VK_SUCCESS);
        return { m_descriptorSetLayoutOwner.Insert(resource), RHI::ResultCode::Success };
    }

    RHI::Result<RHI::Handle<DescriptorSet>> ResourceManager::CreateDescriptorSet(RHI::Handle<DescriptorSetLayout> dsl)
    {
        VkDescriptorPool descriptorPool;

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.descriptorPool = descriptorPool;
        // allocateInfos.descriptorSetCount = dsl.size();
        // allocateInfos.pSetLayouts        = dsl.data();

        DescriptorSet resource{};

        auto result = vkAllocateDescriptorSets(m_context->m_device, &allocateInfo, &resource.handle);
        RHI_ASSERT(result == VK_SUCCESS);
        return { m_descriptorSetOwner.Insert(resource), RHI::ResultCode::Success };
    }

    RHI::Result<RHI::Handle<PipelineLayout>> ResourceManager::CreatePipelineLayout(RHI::TL::Span<RHI::ShaderBindGroupLayout> layouts)
    {
        VkDescriptorSetLayout* descriptorSetLayouts;

        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.setLayoutCount = layouts.size();
        createInfo.pSetLayouts = descriptorSetLayouts;
        createInfo.pushConstantRangeCount = 0;
        createInfo.pPushConstantRanges = nullptr;

        PipelineLayout resource{};
        auto result = vkCreatePipelineLayout(m_context->m_device, &createInfo, nullptr, &resource.handle);
        RHI_ASSERT(result == VK_SUCCESS);
        return { m_pipelineLayoutOwner.Insert(resource), RHI::ResultCode::Success };
    }

    RHI::Result<RHI::Handle<GraphicsPipeline>> ResourceManager::CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo)
    {
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        uint32_t stagesCreateInfoCount = 2;
        VkPipelineShaderStageCreateInfo stagesCreateInfos[4];
        {
            VkPipelineShaderStageCreateInfo stageInfo{};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.pNext = nullptr;
            stageInfo.flags = 0;
            stageInfo.pSpecializationInfo = nullptr;
            stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            stageInfo.module = static_cast<ShaderModule*>(createInfo.vertexShaderModule)->m_shaderModule;
            stageInfo.pName = createInfo.vertexShaderName;
            stagesCreateInfos[0] = stageInfo;

            stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            stageInfo.module = static_cast<ShaderModule*>(createInfo.pixelShaderModule)->m_shaderModule;
            stageInfo.pName = createInfo.pixelShaderName;
            stagesCreateInfos[1] = stageInfo;
        }

        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;

        for (auto attributeDesc : createInfo.inputAssemblerState.attributes)
        {
            VkVertexInputAttributeDescription attribute{};
            attribute.location = attributeDesc.location;
            attribute.binding = attributeDesc.binding;
            attribute.format = GetFormat(attributeDesc.format);
            attribute.offset = attributeDesc.offset;
            inputAttributeDescriptions.push_back(attribute);
        }

        for (auto bindingDesc : createInfo.inputAssemblerState.bindings)
        {
            VkVertexInputBindingDescription binding{};
            binding.binding = bindingDesc.binding;
            binding.stride = bindingDesc.stride;
            binding.inputRate = bindingDesc.stepRate == RHI::PipelineVertexInputRate::PerVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
            vertexInputBindingDescriptions.push_back(binding);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.pNext = nullptr;
        vertexInputStateCreateInfo.flags = 0;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size());
        vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributeDescriptions.size());
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.pNext = nullptr;
        inputAssemblyStateCreateInfo.flags = 0;
        inputAssemblyStateCreateInfo.topology = ConvertToVkPrimitiveTopology(createInfo.topologyMode);
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{};
        tessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationStateCreateInfo.pNext = nullptr;
        tessellationStateCreateInfo.flags = 0;
        // tessellationStateCreateInfo.patchControlPoints;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.pNext = nullptr;
        viewportStateCreateInfo.flags = 0;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = nullptr;
        viewportStateCreateInfo.pViewports = nullptr;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.pNext = nullptr;
        rasterizationStateCreateInfo.flags = 0;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = ConvertToVkPolygonMode(createInfo.rasterizationState.fillMode);
        rasterizationStateCreateInfo.cullMode = ConvertToVkCullModeFlags(createInfo.rasterizationState.cullMode);
        rasterizationStateCreateInfo.frontFace = ConvertToVkFrontFace(createInfo.rasterizationState.frontFace);
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
        rasterizationStateCreateInfo.lineWidth = createInfo.rasterizationState.lineWidth;

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.pNext = nullptr;
        multisampleStateCreateInfo.flags = 0;
        multisampleStateCreateInfo.rasterizationSamples = ConvertToVkSampleCount(createInfo.multisampleState.sampleCount);
        multisampleStateCreateInfo.sampleShadingEnable = createInfo.multisampleState.sampleShading ? VK_TRUE : VK_FALSE;
        multisampleStateCreateInfo.minSampleShading = multisampleStateCreateInfo.rasterizationSamples / 2;
        multisampleStateCreateInfo.pSampleMask = nullptr;
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.pNext = nullptr;
        depthStencilStateCreateInfo.flags = 0;
        depthStencilStateCreateInfo.depthTestEnable = createInfo.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
        depthStencilStateCreateInfo.depthWriteEnable = createInfo.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
        depthStencilStateCreateInfo.depthCompareOp = ConvertToVkCompareOp(createInfo.depthStencilState.compareOperator);
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.stencilTestEnable = createInfo.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;
        // depthStencilStateCreateInfo.front;
        // depthStencilStateCreateInfo.back;
        depthStencilStateCreateInfo.minDepthBounds = 0.0;
        depthStencilStateCreateInfo.maxDepthBounds = 1.0;

        uint32_t pipelineColorBlendAttachmentStateCount = 0;
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentStates[8];

        for (auto blendState : createInfo.colorBlendState.blendStates)
        {
            auto& state = pipelineColorBlendAttachmentStates[pipelineColorBlendAttachmentStateCount++];
            state.blendEnable = blendState.blendEnable ? VK_TRUE : VK_FALSE;
            state.srcColorBlendFactor = ConvertToVkBlendFactor(blendState.srcColor);
            state.dstColorBlendFactor = ConvertToVkBlendFactor(blendState.dstColor);
            state.colorBlendOp = ConvertToVkBlendOp(blendState.colorBlendOp);
            state.srcAlphaBlendFactor = ConvertToVkBlendFactor(blendState.srcAlpha);
            state.dstAlphaBlendFactor = ConvertToVkBlendFactor(blendState.dstAlpha);
            state.alphaBlendOp = ConvertToVkBlendOp(blendState.alphaBlendOp);
            state.colorWriteMask = 0;
            if (blendState.writeMask & RHI::ColorWriteMask::Red)
                state.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;

            if (blendState.writeMask & RHI::ColorWriteMask::Green)
                state.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;

            if (blendState.writeMask & RHI::ColorWriteMask::Blue)
                state.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;

            if (blendState.writeMask & RHI::ColorWriteMask::Alpha)
                state.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
        }

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.pNext = nullptr;
        colorBlendStateCreateInfo.flags = 0;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_SET;
        colorBlendStateCreateInfo.attachmentCount = pipelineColorBlendAttachmentStateCount;
        colorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStates;
        colorBlendStateCreateInfo.blendConstants[0] = createInfo.colorBlendState.blendConstants[0];
        colorBlendStateCreateInfo.blendConstants[1] = createInfo.colorBlendState.blendConstants[1];
        colorBlendStateCreateInfo.blendConstants[2] = createInfo.colorBlendState.blendConstants[2];
        colorBlendStateCreateInfo.blendConstants[3] = createInfo.colorBlendState.blendConstants[3];

        uint32_t dynamicStatesCount = 0;
        VkDynamicState dynamicStates[64] = {};

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pNext = nullptr;
        dynamicStateCreateInfo.flags = 0;
        dynamicStateCreateInfo.dynamicStateCount = dynamicStatesCount;
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

        uint32_t colorAttachmentFormatCount = static_cast<uint32_t>(createInfo.renderTargetLayout.colorAttachmentsFormats.size());
        VkFormat colorAttachmentFormats[8] = {};

        uint32_t index = 0;
        for (auto format : createInfo.renderTargetLayout.colorAttachmentsFormats)
            colorAttachmentFormats[index++] = GetFormat(format);

        VkPipelineRenderingCreateInfo renderTargetLayout{};
        renderTargetLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderTargetLayout.pNext = nullptr;
        // renderTargetLayout.viewMask;
        renderTargetLayout.colorAttachmentCount = colorAttachmentFormatCount;
        renderTargetLayout.pColorAttachmentFormats = colorAttachmentFormats;
        renderTargetLayout.depthAttachmentFormat = GetFormat(createInfo.renderTargetLayout.depthAttachmentFormat);
        renderTargetLayout.stencilAttachmentFormat = GetFormat(createInfo.renderTargetLayout.stencilAttachmentFormat);

        VkGraphicsPipelineCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        vkCreateInfo.pNext = &renderTargetLayout;
        vkCreateInfo.flags = 0;
        vkCreateInfo.stageCount = stagesCreateInfoCount;
        vkCreateInfo.pStages = stagesCreateInfos;
        vkCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        vkCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        vkCreateInfo.pTessellationState = &tessellationStateCreateInfo;
        vkCreateInfo.pViewportState = &viewportStateCreateInfo;
        vkCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        vkCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        vkCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        vkCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        vkCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        vkCreateInfo.layout = pipelineLayout;
        vkCreateInfo.renderPass = VK_NULL_HANDLE;
        vkCreateInfo.subpass = 0;
        vkCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        vkCreateInfo.basePipelineIndex = 0;

        GraphicsPipeline pipeline{};

        auto result = vkCreateGraphicsPipelines(m_context->m_device, VK_NULL_HANDLE, 1, &vkCreateInfo, nullptr, &pipeline.handle);

        if (result == VK_SUCCESS)
            return { m_graphicsPipelineOwner.Insert(pipeline), RHI::ResultCode::Success };

        return { {}, RHI::ResultCode::ErrorUnkown };
    }

    RHI::Result<RHI::Handle<ComputePipeline>> ResourceManager::CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo)
    {
        auto shaderModule = static_cast<ShaderModule*>(createInfo.shaderModule);

        VkPipelineLayout layout;

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.pNext = nullptr;
        shaderStage.flags = 0;
        shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.module = shaderModule->m_shaderModule;
        shaderStage.pName = createInfo.shaderName;
        shaderStage.pSpecializationInfo = nullptr;

        VkComputePipelineCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = {};
        vkCreateInfo.stage = shaderStage;
        vkCreateInfo.layout = layout;
        vkCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        vkCreateInfo.basePipelineIndex = 0;

        ComputePipeline pipeline{};

        auto result = vkCreateComputePipelines(m_context->m_device, VK_NULL_HANDLE, 1, &vkCreateInfo, nullptr, &pipeline.handle);

        if (result == VK_SUCCESS)
            return { m_computePipelineOwner.Insert(pipeline), RHI::ResultCode::Success };

        return { {}, RHI::ResultCode::ErrorUnkown };
    }

    RHI::Result<RHI::Handle<Sampler>> ResourceManager::CreateSampler(const RHI::SamplerCreateInfo& createInfo)
    {
        VkSamplerCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.magFilter = ConvertToVkFilter(createInfo.filterMag);
        vkCreateInfo.minFilter = ConvertToVkFilter(createInfo.filterMin);
        vkCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // ConvertToVkFilter(createInfo.filterMip);
        vkCreateInfo.addressModeU = ConvertToVkSamplerAddressMode(createInfo.addressU);
        vkCreateInfo.addressModeV = ConvertToVkSamplerAddressMode(createInfo.addressV);
        vkCreateInfo.addressModeW = ConvertToVkSamplerAddressMode(createInfo.addressW);
        vkCreateInfo.mipLodBias = createInfo.mipLodBias;
        vkCreateInfo.anisotropyEnable = VK_TRUE;
        vkCreateInfo.maxAnisotropy = 1.0f;
        vkCreateInfo.compareEnable = VK_TRUE;
        vkCreateInfo.compareOp = ConvertToVkCompareOp(createInfo.compare);
        vkCreateInfo.minLod = createInfo.minLod;
        vkCreateInfo.maxLod = createInfo.maxLod;
        vkCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        vkCreateInfo.unnormalizedCoordinates = VK_FALSE;

        Sampler sampler{};

        auto result = vkCreateSampler(m_context->m_device, &vkCreateInfo, nullptr, &sampler.handle);

        if (result == VK_SUCCESS)
            return { m_samplerOwner.Insert(sampler), RHI::ResultCode::Success };

        return { {}, RHI::ResultCode::ErrorUnkown };
    }

    RHI::Result<RHI::Handle<Fence>> ResourceManager::CreateFence()
    {
        Fence fence{};

        VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, {} };

        auto result = vkCreateFence(m_context->m_device, &createInfo, nullptr, &fence.handle);

        if (result == VK_SUCCESS)
            return { m_fenceOwner.Insert(fence), RHI::ResultCode::Success };

        return { {}, RHI::ResultCode::ErrorUnkown };
    }

    void ResourceManager::FreeImage(RHI::Handle<Image> handle)
    {
        auto image = m_imageOwner.Get(handle);
        RHI_ASSERT(image);

        image->pool->Free(RHI::Handle<RHI::Image>(handle));
    }

    void ResourceManager::FreeBuffer(RHI::Handle<Buffer> handle)
    {
        auto buffer = m_bufferOwner.Get(handle);
        RHI_ASSERT(buffer);

        buffer->pool->Free(RHI::Handle<RHI::Buffer>(handle));
    }

    void ResourceManager::FreeImageView(RHI::Handle<ImageView> handle)
    {
        auto imageView = m_imageViewOwner.Get(handle);
        RHI_ASSERT(imageView);

        vkDestroyImageView(m_context->m_device, imageView->handle, nullptr);
    }

    void ResourceManager::FreeBufferView(RHI::Handle<BufferView> handle)
    {
        auto bufferView = m_bufferViewOwner.Get(handle);
        RHI_ASSERT(bufferView);

        vkDestroyBufferView(m_context->m_device, bufferView->handle, nullptr);
    }

    void ResourceManager::FreeDescriptorSetLayout(RHI::Handle<DescriptorSetLayout> handle)
    {
        auto dsl = m_descriptorSetLayoutOwner.Get(handle);
        RHI_ASSERT(dsl);

        vkDestroyDescriptorSetLayout(m_context->m_device, dsl->handle, nullptr);
    }

    void ResourceManager::FreeDescriptorSet(RHI::Handle<DescriptorSet> handle)
    {
        auto descriptorSet = m_descriptorSetOwner.Get(handle);
    }

    void ResourceManager::FreePipelineLayout(RHI::Handle<PipelineLayout> handle)
    {
        auto layout = m_pipelineLayoutOwner.Get(handle);
        RHI_ASSERT(layout);

        vkDestroyPipelineLayout(m_context->m_device, layout->handle, nullptr);
    }

    void ResourceManager::FreeGraphicsPipeline(RHI::Handle<GraphicsPipeline> handle)
    {
        auto pipeline = m_graphicsPipelineOwner.Get(handle);
        RHI_ASSERT(pipeline);

        vkDestroyPipeline(m_context->m_device, pipeline->handle, nullptr);
    }

    void ResourceManager::FreeComputePipeline(RHI::Handle<ComputePipeline> handle)
    {
        auto pipeline = m_computePipelineOwner.Get(handle);
        RHI_ASSERT(pipeline);

        vkDestroyPipeline(m_context->m_device, pipeline->handle, nullptr);
    }

    void ResourceManager::FreeSampler(RHI::Handle<Sampler> handle)
    {
        auto sampler = m_samplerOwner.Get(handle);
        RHI_ASSERT(sampler);

        vkDestroySampler(m_context->m_device, sampler->handle, nullptr);
    }

    void ResourceManager::FreeFence(RHI::Handle<Fence> handle)
    {
        auto fence = m_fenceOwner.Get(handle);
        RHI_ASSERT(fence);

        vkDestroyFence(m_context->m_device, fence->handle, nullptr);
    }

} // namespace Vulkan