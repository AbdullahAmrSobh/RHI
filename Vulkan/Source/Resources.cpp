#include "Resources.hpp"
#include "Common.hpp"
#include "Device.hpp"

#include <algorithm>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    inline static VkQueryType ConvertQueryType(QueryType queryType)
    {
        switch (queryType)
        {
        case QueryType::Occlusion:                          return VK_QUERY_TYPE_OCCLUSION;
        case QueryType::Timestamp:                          return VK_QUERY_TYPE_TIMESTAMP;
        case QueryType::AccelerationStructureSize:          return VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR;
        case QueryType::AccelerationStructureCompactedSize: return VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
        case QueryType::PipelineStatistics:                 return VK_QUERY_TYPE_PIPELINE_STATISTICS;
        default:                                            TL_UNREACHABLE(); return VK_QUERY_TYPE_MAX_ENUM;
        }
    }

    inline static VkBufferUsageFlags ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags)
    {
        VkBufferUsageFlags result = 0;
        if (bufferUsageFlags & BufferUsage::Storage) result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Uniform) result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Vertex) result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Index) result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::CopySrc) result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (bufferUsageFlags & BufferUsage::CopyDst) result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (bufferUsageFlags & BufferUsage::Indirect) result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::DeviceBufferAddress) result |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        if (bufferUsageFlags & BufferUsage::Indirect) result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        return result;
    }

    inline static VkImageUsageFlags ConvertImageUsageFlags(TL::Flags<ImageUsage> imageUsageFlags)
    {
        VkImageUsageFlags result = 0;
        if (imageUsageFlags & ImageUsage::ShaderResource) result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (imageUsageFlags & ImageUsage::StorageResource) result |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (imageUsageFlags & ImageUsage::Color) result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::Depth) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::Stencil) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::CopySrc) result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (imageUsageFlags & ImageUsage::CopyDst) result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        // if (imageUsageFlags & ImageUsage::Present) TL_UNREACHABLE(); // TODO: Handle present usage;
        return result;
    }

    inline static VkImageType ConvertImageType(ImageType imageType)
    {
        switch (imageType)
        {
        case ImageType::None:    return VK_IMAGE_TYPE_MAX_ENUM;
        case ImageType::Image1D: return VK_IMAGE_TYPE_1D;
        case ImageType::Image2D: return VK_IMAGE_TYPE_2D;
        case ImageType::Image3D: return VK_IMAGE_TYPE_3D;
        default:                 TL_UNREACHABLE(); return VK_IMAGE_TYPE_MAX_ENUM;
        }
    }

    inline static VkImageViewType ConvertImageViewType(ImageViewType imageType)
    {
        switch (imageType)
        {
        case ImageViewType::View1D:      return VK_IMAGE_VIEW_TYPE_1D;
        case ImageViewType::View1DArray: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case ImageViewType::View2D:      return VK_IMAGE_VIEW_TYPE_2D;
        case ImageViewType::View2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case ImageViewType::View3D:      return VK_IMAGE_VIEW_TYPE_3D;
        case ImageViewType::CubeMap:     return VK_IMAGE_VIEW_TYPE_CUBE;
        case ImageViewType::None:
        default:
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        }
    }

    VkImageAspectFlags ConvertImageAspect(TL::Flags<ImageAspect> imageAspect, Format format)
    {
        auto formatInfo = GetFormatInfo(format);
        imageAspect &= GetFormatAspects(format);

        VkImageAspectFlags vkAspectFlags = 0;

        if (formatInfo.hasDepth || formatInfo.hasStencil)
        {
            if (imageAspect & ImageAspect::Depth) vkAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
            if (imageAspect & ImageAspect::Stencil) vkAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        else
        {
            if (imageAspect & ImageAspect::Color) vkAspectFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
        }

        TL_ASSERT(vkAspectFlags != 0);
        return vkAspectFlags;
    }

    inline static VkComponentSwizzle ConvertComponentSwizzle(ComponentSwizzle componentSwizzle)
    {
        switch (componentSwizzle)
        {
        case ComponentSwizzle::Identity: return VK_COMPONENT_SWIZZLE_IDENTITY;
        case ComponentSwizzle::Zero:     return VK_COMPONENT_SWIZZLE_ZERO;
        case ComponentSwizzle::One:      return VK_COMPONENT_SWIZZLE_ONE;
        case ComponentSwizzle::R:        return VK_COMPONENT_SWIZZLE_R;
        case ComponentSwizzle::G:        return VK_COMPONENT_SWIZZLE_G;
        case ComponentSwizzle::B:        return VK_COMPONENT_SWIZZLE_B;
        case ComponentSwizzle::A:        return VK_COMPONENT_SWIZZLE_A;
        default:                         TL_UNREACHABLE(); return VK_COMPONENT_SWIZZLE_IDENTITY;
        }
    }

    VkImageSubresourceRange ConvertSubresourceRange(const ImageSubresourceRange& subresource, Format format)
    {
        return VkImageSubresourceRange{
            .aspectMask     = ConvertImageAspect(subresource.imageAspects, format),
            .baseMipLevel   = subresource.mipBase,
            .levelCount     = subresource.mipLevelCount,
            .baseArrayLayer = subresource.arrayBase,
            .layerCount     = subresource.arrayCount,
        };
    }

    inline static VkComponentMapping ConvertComponentMapping(ComponentMapping componentMapping)
    {
        VkComponentMapping mapping{
            .r = ConvertComponentSwizzle(componentMapping.r),
            .g = ConvertComponentSwizzle(componentMapping.g),
            .b = ConvertComponentSwizzle(componentMapping.b),
            .a = ConvertComponentSwizzle(componentMapping.a),
        };
        return mapping;
    }

    inline static VkFilter ConvertFilter(SamplerFilter samplerFilter)
    {
        switch (samplerFilter)
        {
        case SamplerFilter::Point:  return VK_FILTER_NEAREST;
        case SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default:                    TL_UNREACHABLE(); return VK_FILTER_MAX_ENUM;
        }
    }

    inline static VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:                         TL_UNREACHABLE(); return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    inline static VkCompareOp ConvertCompareOp(CompareOperator compareOperator)
    {
        switch (compareOperator)
        {
        case CompareOperator::Undefined:      return VK_COMPARE_OP_NEVER;
        case CompareOperator::Never:          return VK_COMPARE_OP_NEVER;
        case CompareOperator::Equal:          return VK_COMPARE_OP_EQUAL;
        case CompareOperator::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOperator::Greater:        return VK_COMPARE_OP_GREATER;
        case CompareOperator::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOperator::Less:           return VK_COMPARE_OP_LESS;
        case CompareOperator::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOperator::Always:         return VK_COMPARE_OP_ALWAYS;
        default:                              TL_UNREACHABLE(); return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    inline static VkBool32 ConvertBool(bool value)
    {
        return value ? VK_TRUE : VK_FALSE;
    }

    inline static VkShaderStageFlagBits ConvertShaderStage(ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
        case ShaderStage::Vertex:  return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Mesh:    return VK_SHADER_STAGE_MESH_BIT_EXT;
        case ShaderStage::Pixel:   return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                   TL_UNREACHABLE(); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkShaderStageFlags ConvertShaderStage(TL::Flags<ShaderStage> shaderStageFlags)
    {
        VkShaderStageFlags result = 0;
        if (shaderStageFlags & ShaderStage::Vertex) result |= VK_SHADER_STAGE_VERTEX_BIT;
        if (shaderStageFlags & ShaderStage::Pixel) result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (shaderStageFlags & ShaderStage::Compute) result |= VK_SHADER_STAGE_COMPUTE_BIT;
        return result;
    }

    inline static VkVertexInputRate ConvertVertexInputRate(PipelineVertexInputRate inputRate)
    {
        switch (inputRate)
        {
        case PipelineVertexInputRate::PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case PipelineVertexInputRate::PerVertex:   return VK_VERTEX_INPUT_RATE_VERTEX;
        default:                                   TL_UNREACHABLE(); return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    inline static VkCullModeFlags ConvertCullModeFlags(PipelineRasterizerStateCullMode cullMode)
    {
        switch (cullMode)
        {
        case PipelineRasterizerStateCullMode::None:      return VK_CULL_MODE_NONE;
        case PipelineRasterizerStateCullMode::FrontFace: return VK_CULL_MODE_FRONT_BIT;
        case PipelineRasterizerStateCullMode::BackFace:  return VK_CULL_MODE_BACK_BIT;
        case PipelineRasterizerStateCullMode::Discard:   return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        default:                                         TL_UNREACHABLE(); return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkPolygonMode ConvertPolygonMode(PipelineRasterizerStateFillMode fillMode)
    {
        switch (fillMode)
        {
        case PipelineRasterizerStateFillMode::Point:    return VK_POLYGON_MODE_POINT;
        case PipelineRasterizerStateFillMode::Triangle: return VK_POLYGON_MODE_FILL;
        case PipelineRasterizerStateFillMode::Line:     return VK_POLYGON_MODE_LINE;
        default:                                        TL_UNREACHABLE(); return VK_POLYGON_MODE_MAX_ENUM;
        }
    }

    inline static VkPrimitiveTopology ConvertPrimitiveTopology(PipelineTopologyMode topologyMode)
    {
        switch (topologyMode)
        {
        case PipelineTopologyMode::Points:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PipelineTopologyMode::Lines:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PipelineTopologyMode::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:                              TL_UNREACHABLE(); return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }
    }

    inline static VkFrontFace ConvertFrontFace(PipelineRasterizerStateFrontFace frontFace)
    {
        switch (frontFace)
        {
        case PipelineRasterizerStateFrontFace::Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
        case PipelineRasterizerStateFrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:                                                 TL_UNREACHABLE(); return VK_FRONT_FACE_MAX_ENUM;
        }
    }

    inline static VkBlendFactor ConvertBlendFactor(BlendFactor blendFactor)
    {
        switch (blendFactor)
        {
        case BlendFactor::Zero:                  return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One:                   return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor:              return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:              return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::OneMinusDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:              return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:              return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        default:                                 TL_UNREACHABLE(); return VK_BLEND_FACTOR_MAX_ENUM;
        }
    }

    inline static VkBlendOp ConvertBlendOp(BlendEquation blendEquation)
    {
        switch (blendEquation)
        {
        case BlendEquation::Add:             return VK_BLEND_OP_ADD;
        case BlendEquation::Subtract:        return VK_BLEND_OP_SUBTRACT;
        case BlendEquation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendEquation::Min:             return VK_BLEND_OP_MIN;
        case BlendEquation::Max:             return VK_BLEND_OP_MAX;
        default:                             TL_UNREACHABLE(); return VK_BLEND_OP_MAX_ENUM;
        }
    }

    inline static VkDescriptorType ConvertDescriptorType(BindingType bindingType)
    {
        switch (bindingType)
        {
        case BindingType::Sampler:              return VK_DESCRIPTOR_TYPE_SAMPLER;
        case BindingType::SampledImage:         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case BindingType::StorageImage:         return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case BindingType::UniformBuffer:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case BindingType::StorageBuffer:        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case BindingType::DynamicUniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case BindingType::DynamicStorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case BindingType::BufferView:           return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case BindingType::StorageBufferView:    return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        default:                                TL_UNREACHABLE(); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    inline static VkPresentModeKHR ConvertToPresentMode(SwapchainPresentMode presentMode)
    {
        switch (presentMode)
        {
        case SwapchainPresentMode::Immediate:   return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case SwapchainPresentMode::Fifo:        return VK_PRESENT_MODE_FIFO_KHR;
        case SwapchainPresentMode::FifoRelaxed: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        case SwapchainPresentMode::Mailbox:     return VK_PRESENT_MODE_MAILBOX_KHR;
        default:                                return VK_PRESENT_MODE_MAX_ENUM_KHR;
        }
    }

    inline static VkCompositeAlphaFlagBitsKHR ConvertToAlphaMode(SwapchainAlphaMode alphaMode)
    {
        switch (alphaMode)
        {
        case SwapchainAlphaMode::None:           return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        case SwapchainAlphaMode::PreMultiplied:  return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        case SwapchainAlphaMode::PostMultiplied: return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        default:                                 TL_UNREACHABLE(); return VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
        }
    }

    DescriptorSetWriter::DescriptorSetWriter(IDevice* device, VkDescriptorSet descriptorSet, IBindGroupLayout* layout, TL::IAllocator& allocator)
        : m_device(device)
        , m_allocator(&allocator)
        , m_bindGroupLayout(layout)
        , m_descriptorSet(descriptorSet)
        , m_images(allocator)
        , m_sampler(allocator)
        , m_buffers(allocator)
        , m_bufferViews(allocator)
        , m_writes(allocator)
    {
    }

    VkWriteDescriptorSet DescriptorSetWriter::BindImages(uint32_t dstBinding, uint32_t dstArray, TL::Span<Image* const> images)
    {
        auto layout        = (IBindGroupLayout*)(m_bindGroupLayout);
        auto shaderBinding = layout->GetBinding(dstBinding);
        auto isStorage     = shaderBinding.type == BindingType::StorageImage;
        auto hasWrite      = (shaderBinding.access & Access::Write) == Access::Write;

        VkImageLayout    imageLayout    = (isStorage && hasWrite) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkDescriptorType descriptorType = isStorage ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

        TL::Vector<VkDescriptorImageInfo>& descriptorImageInfos = m_images.emplace_back(*m_allocator);
        descriptorImageInfos.reserve(images.size());
        for (auto imageHandle : images)
        {
            auto                  image          = (IImage*)(imageHandle);
            VkDescriptorImageInfo descriptorInfo = {
                .sampler     = VK_NULL_HANDLE,
                .imageView   = image->viewHandle,
                .imageLayout = imageLayout,
            };
            descriptorImageInfos.push_back(descriptorInfo);
        }

        VkWriteDescriptorSet writeInfo{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext           = nullptr,
            .dstSet          = m_descriptorSet,
            .dstBinding      = dstBinding,
            .dstArrayElement = dstArray,
            .descriptorCount = (uint32_t)descriptorImageInfos.size(),
            .descriptorType  = descriptorType,
            .pImageInfo      = descriptorImageInfos.data(),
        };
        return m_writes.emplace_back(writeInfo);
    }

    VkWriteDescriptorSet DescriptorSetWriter::BindSamplers(uint32_t dstBinding, uint32_t dstArray, TL::Span<Sampler* const> samplers)
    {
        TL::Vector<VkDescriptorImageInfo>& descriptorImageInfos = m_sampler.emplace_back(*m_allocator);
        descriptorImageInfos.reserve(samplers.size());
        for (auto samplerHandle : samplers)
        {
            auto sampler = (ISampler*)(samplerHandle);

            VkDescriptorImageInfo descriptorInfo = {
                .sampler     = sampler->handle,
                .imageView   = VK_NULL_HANDLE,
                .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            };
            descriptorImageInfos.push_back(descriptorInfo);
        }

        VkWriteDescriptorSet writeInfo{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext           = nullptr,
            .dstSet          = m_descriptorSet,
            .dstBinding      = dstBinding,
            .dstArrayElement = dstArray,
            .descriptorCount = (uint32_t)descriptorImageInfos.size(),
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo      = descriptorImageInfos.data(),
        };
        return m_writes.emplace_back(writeInfo);
    }

    VkWriteDescriptorSet DescriptorSetWriter::BindBuffers(uint32_t dstBinding, uint32_t dstArray, TL::Span<const BufferBindingInfo> bufferBindings)
    {
        auto layout         = (IBindGroupLayout*)(m_bindGroupLayout);
        auto shaderBinding  = layout->GetBinding(dstBinding);
        auto descriptorType = ConvertDescriptorType(shaderBinding.type);

        TL::Vector<VkDescriptorBufferInfo>& descriptorBufferInfos = m_buffers.emplace_back(*m_allocator);
        descriptorBufferInfos.reserve(bufferBindings.size());
        for (const auto& bufferBinding : bufferBindings)
        {
            auto buffer = (IBuffer*)(bufferBinding.buffer);
            auto offset = bufferBinding.offset;
            auto range  = (bufferBinding.range == RemainingSize) ? VK_WHOLE_SIZE : bufferBinding.range;

            VkDescriptorBufferInfo descriptorInfo = {
                .buffer = buffer->handle,
                .offset = offset,
                .range  = range,
            };
            descriptorBufferInfos.push_back(descriptorInfo);
        }

        VkWriteDescriptorSet writeInfo{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext           = nullptr,
            .dstSet          = m_descriptorSet,
            .dstBinding      = dstBinding,
            .dstArrayElement = dstArray,
            .descriptorCount = (uint32_t)descriptorBufferInfos.size(),
            .descriptorType  = descriptorType,
            .pBufferInfo     = descriptorBufferInfos.data(),
        };
        return m_writes.emplace_back(writeInfo);
    }

    ////////////////////////////////////////////////////////////////////////
    // BindGroupAllocator
    ////////////////////////////////////////////////////////////////////////

    BindGroupAllocator::BindGroupAllocator()  = default;
    BindGroupAllocator::~BindGroupAllocator() = default;

    VkResult BindGroupAllocator::Init(IDevice* device)
    {
        m_device = device;

        /// @todo: (don't do this)
        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 2048},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2048},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2048},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2048},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2048},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2048},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2048},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2048},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 2048},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2048},
        };

        VkDescriptorPoolCreateFlags poolFlags =
            VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        VkDescriptorPoolCreateInfo createInfo{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext         = nullptr,
            .flags         = poolFlags,
            .maxSets       = 8,
            .poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize),
            .pPoolSizes    = poolSizes,
        };

        VulkanResult result;
        result = vkCreateDescriptorPool(m_device->m_device, &createInfo, nullptr, &m_descriptorPool);
        return result;
    }

    void BindGroupAllocator::Shutdown()
    {
        vkResetDescriptorPool(m_device->m_device, m_descriptorPool, 0);
        vkDestroyDescriptorPool(m_device->m_device, m_descriptorPool, nullptr);
    }

    ResultCode BindGroupAllocator::InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout, uint32_t bindlessResourcesCount)
    {
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountInfo{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .pNext              = nullptr,
            .descriptorSetCount = 1,
            .pDescriptorCounts  = &bindlessResourcesCount,
        };

        VkDescriptorSetAllocateInfo allocateInfo{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext              = bindGroupLayout->hasBindless ? &variableDescriptorCountInfo : nullptr,
            .descriptorPool     = m_descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts        = &bindGroupLayout->handle,
        };

        VulkanResult result;
        result = vkAllocateDescriptorSets(m_device->m_device, &allocateInfo, &bindGroup->descriptorSet);
        return result;
    }

    void BindGroupAllocator::ShutdownBindGroup(IBindGroup* bindGroup)
    {
        // todo this should be deleted throuh deletion queue
        vkFreeDescriptorSets(m_device->m_device, m_descriptorPool, 1, &bindGroup->descriptorSet);
    }

    ////////////////////////////////////////////////////////////////////////
    // IBindGroupLayout
    ////////////////////////////////////////////////////////////////////////

    ResultCode IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        this->shaderBindings = {createInfo.bindings.begin(), createInfo.bindings.end()};

        TL::Vector<VkDescriptorBindingFlags>     bindingFlags{device->m_arena};
        TL::Vector<VkDescriptorSetLayoutBinding> setLayoutBindings{device->m_arena};

        // Query the physical device limits for descriptor counts
        VkPhysicalDeviceProperties2 properties{};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        VkPhysicalDeviceDescriptorIndexingProperties descriptorIndexingProps{};
        descriptorIndexingProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
        properties.pNext              = &descriptorIndexingProps;

        vkGetPhysicalDeviceProperties2(device->m_physicalDevice, &properties);

        // Example: retrieve max descriptor counts
        TL_MAYBE_UNUSED uint32_t maxSampledImages         = properties.properties.limits.maxPerStageDescriptorSampledImages;
        TL_MAYBE_UNUSED uint32_t maxStorageImages         = properties.properties.limits.maxPerStageDescriptorStorageImages;
        TL_MAYBE_UNUSED uint32_t maxSamplers              = properties.properties.limits.maxPerStageDescriptorSamplers;
        TL_MAYBE_UNUSED uint32_t maxUniformBuffers        = properties.properties.limits.maxPerStageDescriptorUniformBuffers;
        TL_MAYBE_UNUSED uint32_t maxStorageBuffers        = properties.properties.limits.maxPerStageDescriptorStorageBuffers;
        TL_MAYBE_UNUSED uint32_t maxCombinedImageSamplers = properties.properties.limits.maxPerStageDescriptorSampledImages + properties.properties.limits.maxPerStageDescriptorSamplers;

        /// @todo: subtract 100 to reserve for pass inputs
        uint32_t                 maxBindlessSampledImages  = descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindSampledImages - 100;
        TL_MAYBE_UNUSED uint32_t maxBindlessStorageImages  = descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindStorageImages;
        TL_MAYBE_UNUSED uint32_t maxBindlessSamplers       = descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindSamplers;
        TL_MAYBE_UNUSED uint32_t maxBindlessUniformBuffers = descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
        TL_MAYBE_UNUSED uint32_t maxBindlessStorageBuffers = descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
        // Use these values as needed for validation or allocation

        for (uint32_t bindingIndex = 0; bindingIndex < createInfo.bindings.size(); bindingIndex++)
        {
            auto binding = createInfo.bindings[bindingIndex];

            auto isBindless = binding.arrayCount == BindlessArraySize;

            VkDescriptorSetLayoutBinding layoutBinding{
                .binding            = bindingIndex,
                .descriptorType     = ConvertDescriptorType(binding.type),
                .descriptorCount    = isBindless ? maxBindlessSampledImages : binding.arrayCount,
                .stageFlags         = ConvertShaderStage(binding.stages),
                .pImmutableSamplers = nullptr,
            };
            setLayoutBindings.push_back(layoutBinding);

            if (isBindless)
            {
                VkDescriptorBindingFlags flags =
                    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                    VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
                bindingFlags.push_back(flags);
                hasBindless = true;
            }
            else
            {
                bindingFlags.push_back(0);
            }
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlagsCI{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext         = nullptr,
            .bindingCount  = (uint32_t)bindingFlags.size(),
            .pBindingFlags = bindingFlags.data(),
        };

        VkDescriptorSetLayoutCreateInfo layoutCI{
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = hasBindless ? &layoutBindingFlagsCI : nullptr,
            .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = (uint32_t)setLayoutBindings.size(),
            .pBindings    = setLayoutBindings.data(),
        };

        VulkanResult result;
        result = vkCreateDescriptorSetLayout(device->m_device, &layoutCI, nullptr, &handle);
        TL_ASSERT(result, "vkCreateDescriptorSetLayout failed with error: {}", result.AsString());
        if (result && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return result;
    }

    void IBindGroupLayout::Shutdown(IDevice* device)
    {
        vkDestroyDescriptorSetLayout(device->m_device, handle, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////
    // IBindGroup
    ////////////////////////////////////////////////////////////////////////

    ResultCode IBindGroup::Init(IDevice* device, const BindGroupCreateInfo& createInfo)
    {
        IBindGroupLayout* bindGroupLayout = (IBindGroupLayout*)(createInfo.layout);

        this->bindGroupLayout = (IBindGroupLayout*)createInfo.layout;

        return device->m_bindGroupAllocator.InitBindGroup(this, bindGroupLayout, createInfo.bindlessArrayCount);
    }

    void IBindGroup::Shutdown(IDevice* device)
    {
        device->m_bindGroupAllocator.ShutdownBindGroup(this);
    }

    void IBindGroup::Update(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        DescriptorSetWriter writer(device, this->descriptorSet, this->bindGroupLayout, device->m_arena);

        for (auto [dstBindings, dstArrayelements, buffers] : updateInfo.buffers)
        {
            writer.BindBuffers(dstBindings, dstArrayelements, buffers);
        }

        for (auto [dstBindings, dstArrayelements, images] : updateInfo.images)
        {
            writer.BindImages(dstBindings, dstArrayelements, images);
        }

        for (auto [dstBindings, dstArrayelements, samplers] : updateInfo.samplers)
        {
            writer.BindSamplers(dstBindings, dstArrayelements, samplers);
        }

        vkUpdateDescriptorSets(device->m_device, (uint32_t)writer.GetWrites().size(), writer.GetWrites().data(), 0, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////
    // IFence
    ////////////////////////////////////////////////////////////////////////

    ResultCode IFence::Init(IDevice* device, const FenceCreateInfo& createInfo)
    {
        VkSemaphoreTypeCreateInfo semaphoreTypeCI = {
            .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue  = createInfo.initialValue,
        };

        VkSemaphoreCreateInfo semaphoreCI{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphoreTypeCI,
        };

        VulkanResult result = vkCreateSemaphore(device->m_device, &semaphoreCI, nullptr, &this->semaphore);
        if (result.IsSuccess() && createInfo.name)
        {
            device->SetDebugName(semaphore, createInfo.name);
        }
        return result;
    }

    void IFence::Shutdown(IDevice* device)
    {
    }

    bool IFence::waitValue(IDevice* device, uint64_t value)
    {
        VkSemaphoreWaitInfo semaphoreWaitInfo = {
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1,
            .pSemaphores    = &semaphore,
            .pValues        = &value,
        };
        VulkanResult result = vkWaitSemaphores(device->m_device, &semaphoreWaitInfo, UINT64_MAX);
        TL_ASSERT(result.IsSuccess());
        return true;
    }

    ////////////////////////////////////////////////////////////////////////
    // IShaderModule
    ////////////////////////////////////////////////////////////////////////

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        VkShaderModuleCreateInfo shaderModuleCI{
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = {},
            .codeSize = createInfo.code.size_bytes(),
            .pCode    = createInfo.code.data(),
        };
        auto result = vkCreateShaderModule(device->m_device, &shaderModuleCI, nullptr, &m_shaderModule);
        return ConvertResult(result);
    }

    void IShaderModule::Shutdown(IDevice* device)
    {
        vkDestroyShaderModule(device->m_device, m_shaderModule, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////
    // IPipelineLayout
    ////////////////////////////////////////////////////////////////////////

    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        TL::Vector<VkDescriptorSetLayout> descriptorSetLayouts{device->m_arena};
        uint32_t                          index = 0;
        for (auto bindGroupLayout : createInfo.layouts)
        {
            auto layout = (IBindGroupLayout*)(bindGroupLayout);
            descriptorSetLayouts.push_back(layout->handle);
            this->bindGroupLayouts[index++] = (IBindGroupLayout*)bindGroupLayout;
        }

        TL::Vector<VkPushConstantRange> pushConstantRanges{device->m_arena};
        for (auto range : createInfo.pushConstants)
        {
            pushConstantRanges.push_back({
                .stageFlags = ConvertShaderStage(range.stages),
                .offset     = range.offset,
                .size       = range.size,
            });
        }

        VkPipelineLayoutCreateInfo pipelineLayouCI{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext                  = nullptr,
            .flags                  = 0,
            .setLayoutCount         = uint32_t(descriptorSetLayouts.size()),
            .pSetLayouts            = descriptorSetLayouts.data(),
            .pushConstantRangeCount = uint32_t(pushConstantRanges.size()),
            .pPushConstantRanges    = pushConstantRanges.data(),
        };
        auto result = vkCreatePipelineLayout(device->m_device, &pipelineLayouCI, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void IPipelineLayout::Shutdown(IDevice* device)
    {
        if (release())
        {
            vkDestroyPipelineLayout(device->m_device, handle, nullptr);
        }
    }

    ////////////////////////////////////////////////////////////////////////
    // IGraphicsPipeline
    ////////////////////////////////////////////////////////////////////////

    VkPipelineShaderStageCreateInfo convertShaderStage(PipelineShaderStage stage)
    {
        auto shaderModule = (IShaderModule*)stage.module;

        return {
            .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext               = nullptr,
            .flags               = 0,
            .stage               = ConvertShaderStage(stage.stage),
            .module              = shaderModule->m_shaderModule,
            .pName               = stage.name,
            .pSpecializationInfo = nullptr,
        };
    }

    ResultCode IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        TL::Vector<VkPipelineShaderStageCreateInfo> shaderStageCIs{device->m_arena};
        for (const auto& stage : createInfo.shaderStages)
        {
            shaderStageCIs.push_back(convertShaderStage(stage));
        }

        TL::Vector<VkVertexInputBindingDescription>   vertexBindings{device->m_arena};
        TL::Vector<VkVertexInputAttributeDescription> vertexAttributes{device->m_arena};
        for (const auto& bindingDesc : createInfo.vertexBufferBindings)
        {
            VkVertexInputBindingDescription binding{
                .binding   = (uint32_t)vertexBindings.size(),
                .stride    = bindingDesc.stride,
                .inputRate = ConvertVertexInputRate(bindingDesc.stepRate),
            };
            for (const auto& attributeDesc : bindingDesc.attributes)
            {
                VkVertexInputAttributeDescription attribute{
                    .location = (uint32_t)vertexAttributes.size(),
                    .binding  = (uint32_t)vertexBindings.size(),
                    .format   = ConvertFormat(attributeDesc.format),
                    .offset   = attributeDesc.offset,
                };
                vertexAttributes.push_back(attribute);
            }
            vertexBindings.push_back(binding);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext                           = nullptr,
            .flags                           = 0,
            .vertexBindingDescriptionCount   = (uint32_t)vertexBindings.size(),
            .pVertexBindingDescriptions      = vertexBindings.data(),
            .vertexAttributeDescriptionCount = (uint32_t)vertexAttributes.size(),
            .pVertexAttributeDescriptions    = vertexAttributes.data(),
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext                  = nullptr,
            .flags                  = 0,
            .topology               = ConvertPrimitiveTopology(createInfo.topologyMode),
            .primitiveRestartEnable = VK_FALSE,
        };

        VkPipelineTessellationStateCreateInfo tessellationStateCI{
            .sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .pNext              = nullptr,
            .flags              = 0,
            .patchControlPoints = 0,
        };

        VkPipelineViewportStateCreateInfo viewportStateCI{
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext         = nullptr,
            .flags         = 0,
            .viewportCount = 1,
            .pViewports    = nullptr,
            .scissorCount  = 1,
            .pScissors     = nullptr,
        };

        VkPipelineRasterizationStateCreateInfo rasterizationStateCI{
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = ConvertPolygonMode(createInfo.rasterizationState.fillMode),
            .cullMode                = ConvertCullModeFlags(createInfo.rasterizationState.cullMode),
            .frontFace               = ConvertFrontFace(createInfo.rasterizationState.frontFace),
            .depthBiasEnable         = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp          = 0.0f,
            .depthBiasSlopeFactor    = 0.0f,
            .lineWidth               = createInfo.rasterizationState.lineWidth,
        };

        VkPipelineMultisampleStateCreateInfo multisampleStateCI{
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .rasterizationSamples  = ConvertSampleCount(createInfo.multisampleState.sampleCount),
            .sampleShadingEnable   = ConvertBool(createInfo.multisampleState.sampleShading),
            .minSampleShading      = float(uint32_t(multisampleStateCI.rasterizationSamples)) / 2.0f,
            .pSampleMask           = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable      = VK_FALSE,
        };

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .depthTestEnable       = ConvertBool(createInfo.depthStencilState.depthTestEnable),
            .depthWriteEnable      = ConvertBool(createInfo.depthStencilState.depthWriteEnable),
            .depthCompareOp        = ConvertCompareOp(createInfo.depthStencilState.compareOperator),
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable     = ConvertBool(createInfo.depthStencilState.stencilTestEnable),
            .front                 = {},
            .back                  = {},
            .minDepthBounds        = 0.0,
            .maxDepthBounds        = 1.0,
        };

        VkDynamicState                   dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCI{
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext             = nullptr,
            .flags             = 0,
            .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
            .pDynamicStates    = dynamicStates,
        };

        TL::Vector<VkFormat> colorAttachmentFormats{device->m_arena};
        colorAttachmentFormats.reserve(createInfo.renderTargetLayout.colorAttachmentsFormats.size());
        for (auto format : createInfo.renderTargetLayout.colorAttachmentsFormats)
        {
            colorAttachmentFormats.push_back(ConvertFormat(format));
        }

        TL::Vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates{device->m_arena};
        pipelineColorBlendAttachmentStates.reserve(colorAttachmentFormats.size());

        for (auto blendState : createInfo.colorBlendState.blendStates)
        {
            VkPipelineColorBlendAttachmentState state{
                .blendEnable         = blendState.blendEnable ? VK_TRUE : VK_FALSE,
                .srcColorBlendFactor = ConvertBlendFactor(blendState.srcColor),
                .dstColorBlendFactor = ConvertBlendFactor(blendState.dstColor),
                .colorBlendOp        = ConvertBlendOp(blendState.colorBlendOp),
                .srcAlphaBlendFactor = ConvertBlendFactor(blendState.srcAlpha),
                .dstAlphaBlendFactor = ConvertBlendFactor(blendState.dstAlpha),
                .alphaBlendOp        = ConvertBlendOp(blendState.alphaBlendOp),
                .colorWriteMask      = 0,
            };
            if (blendState.writeMask & ColorWriteMask::Red) state.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
            if (blendState.writeMask & ColorWriteMask::Green) state.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
            if (blendState.writeMask & ColorWriteMask::Blue) state.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
            if (blendState.writeMask & ColorWriteMask::Alpha) state.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

            pipelineColorBlendAttachmentStates.push_back(state);
        }

        auto [r, g, b, a] = createInfo.colorBlendState.blendConstants;
        VkPipelineColorBlendStateCreateInfo colorBlendStateCI{
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .logicOpEnable   = VK_FALSE,
            .logicOp         = VK_LOGIC_OP_SET,
            .attachmentCount = (uint32_t)colorAttachmentFormats.size(),
            .pAttachments    = pipelineColorBlendAttachmentStates.data(),
            .blendConstants  = {r, g, b, a},
        };

        VkPipelineRenderingCreateInfo renderTargetLayout{
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext                   = nullptr,
            .viewMask                = 0,
            .colorAttachmentCount    = (uint32_t)colorAttachmentFormats.size(),
            .pColorAttachmentFormats = colorAttachmentFormats.data(),
            .depthAttachmentFormat   = ConvertFormat(createInfo.renderTargetLayout.depthAttachmentFormat),
            .stencilAttachmentFormat = ConvertFormat(createInfo.renderTargetLayout.stencilAttachmentFormat),
        };

        this->layout = (IPipelineLayout*)createInfo.layout;

        VkGraphicsPipelineCreateInfo graphicsPipelineCI{
            .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext               = &renderTargetLayout,
            .flags               = 0,
            .stageCount          = (uint32_t)shaderStageCIs.size(),
            .pStages             = shaderStageCIs.data(),
            .pVertexInputState   = &vertexInputStateCI,
            .pInputAssemblyState = &inputAssemblyStateCI,
            .pTessellationState  = &tessellationStateCI,
            .pViewportState      = &viewportStateCI,
            .pRasterizationState = &rasterizationStateCI,
            .pMultisampleState   = &multisampleStateCI,
            .pDepthStencilState  = &depthStencilStateCI,
            .pColorBlendState    = &colorBlendStateCI,
            .pDynamicState       = &dynamicStateCI,
            .layout              = this->layout->handle,
            .renderPass          = VK_NULL_HANDLE,
            .subpass             = 0,
            .basePipelineHandle  = VK_NULL_HANDLE,
            .basePipelineIndex   = 0,
        };

        VulkanResult result;
        result = vkCreateGraphicsPipelines(device->m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &handle);
        TL_ASSERT(result, "vkCreateGraphicsPipelines failed with error: {}", result.AsString());
        if (result && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return result;
    }

    void IGraphicsPipeline::Shutdown(IDevice* device)
    {
        auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();

        if (handle)
            device->m_destroyQueue->Push(frame, handle);
    }

    ////////////////////////////////////////////////////////////////////////
    // IComputePipeline
    ////////////////////////////////////////////////////////////////////////

    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        this->layout     = (IPipelineLayout*)createInfo.layout;
        auto shaderStage = convertShaderStage(createInfo.computeShader);

        VkComputePipelineCreateInfo computePipelineCI{
            .sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext              = nullptr,
            .flags              = {},
            .stage              = shaderStage,
            .layout             = layout->handle,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex  = 0,
        };

        auto result = vkCreateComputePipelines(device->m_device, VK_NULL_HANDLE, 1, &computePipelineCI, nullptr, &handle);
        return ConvertResult(result);
    }

    void IComputePipeline::Shutdown(IDevice* device)
    {
        auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();

        if (handle)
            device->m_destroyQueue->Push(frame, handle);
    }

    ResultCode IRayTracingPipeline::Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo)
    {
        this->layout = (IPipelineLayout*)createInfo.layout;

        TL::Vector<VkPipelineShaderStageCreateInfo>      shaderStagesCI{device->m_arena};
        TL::Vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroupsCI{device->m_arena};

        for (const auto& stage : createInfo.shaderStages)
        {
            shaderStagesCI.push_back(convertShaderStage(stage));
        }

        // VkPipelineLibraryCreateInfoKHR
        // VkRayTracingPipelineInterfaceCreateInfoKHR
        // VkPipelineDynamicStateCreateInfo

        VkDynamicState                   dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCI{
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext             = nullptr,
            .flags             = 0,
            .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
            .pDynamicStates    = dynamicStates,
        };

        VkRayTracingPipelineCreateInfoKHR pipelineCI{
            .sType                        = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
            .pNext                        = nullptr,
            .flags                        = 0,
            .stageCount                   = (uint32_t)shaderStagesCI.size(),
            .pStages                      = shaderStagesCI.data(),
            .groupCount                   = (uint32_t)shaderGroupsCI.size(),
            .pGroups                      = shaderGroupsCI.data(),
            .maxPipelineRayRecursionDepth = createInfo.maxRecursionDepth,
            .pLibraryInfo                 = nullptr,
            .pLibraryInterface            = nullptr,
            .pDynamicState                = &dynamicStateCI,
            .layout                       = layout->handle,
            .basePipelineHandle           = VK_NULL_HANDLE,
            .basePipelineIndex            = 0,
        };

        VulkanResult result;
        result = device->m_pfn.vkCreateRayTracingPipelinesKHR(device->m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &handle);
        return result;
    }

    void IRayTracingPipeline::Shutdown(IDevice* device)
    {
        auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();

        if (handle)
            device->m_destroyQueue->Push(frame, handle);
    }

    ////////////////////////////////////////////////////////////////////////
    // QueryPool
    /////////////////////////////////////////////////////////////////////

    ResultCode IQueryPool::Init(IDevice* device, const QueryPoolCreateInfo& createInfo)
    {
        VkQueryPoolCreateInfo queryPoolCI{
            .sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
            .pNext              = nullptr,
            .flags              = 0,
            .queryType          = ConvertQueryType(createInfo.type),
            .queryCount         = createInfo.count,
            .pipelineStatistics = 0,
        };

        // clang-format off
        if (queryPoolCI.queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS)
        {
            queryPoolCI.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
        }
        // clang-format on

        VulkanResult result;
        result = vkCreateQueryPool(device->m_device, &queryPoolCI, nullptr, &handle);
        return ConvertResult(result);
    }

    void IQueryPool::Shutdown(IDevice* device)
    {
        auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();

        if (handle)
            device->m_destroyQueue->Push(frame, handle);
    }

    ////////////////////////////////////////////////////////////////////////
    // IBuffer
    ////////////////////////////////////////////////////////////////////////

    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        VkMemoryPropertyFlags requiredFlags = 0;
        if (createInfo.usageFlags & BufferUsage::HostMapped)
        {
            requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        }

        VmaAllocationCreateInfo allocationCI{
            .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            .usage          = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags  = requiredFlags,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = VK_NULL_HANDLE,
            .pUserData      = nullptr,
            .priority       = 0.0f,
        };
        VkBufferCreateInfo bufferCI{
            .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .size                  = createInfo.byteSize,
            .usage                 = ConvertBufferUsageFlags(createInfo.usageFlags),
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
        };
        auto result = vmaCreateBuffer(device->m_deviceAllocator, &bufferCI, &allocationCI, &handle, &allocation, nullptr);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
            vmaSetAllocationName(device->m_deviceAllocator, allocation, createInfo.name);
        }

        if (createInfo.usageFlags & BufferUsage::DeviceBufferAddress)
        {
            VkBufferDeviceAddressInfo info{
                .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .pNext  = nullptr,
                .buffer = this->handle,
            };
            this->address = vkGetBufferDeviceAddress(device->m_device, &info);
        }

        return ConvertResult(result);
    }

    void IBuffer::Shutdown(IDevice* device)
    {
        TL_ASSERT(mapped == false, "Unmap buffer first");

        auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();

        if (handle)
            device->m_destroyQueue->Push(frame, handle);

        if (allocation)
            device->m_destroyQueue->Push(frame, allocation);
    }

    VkMemoryRequirements IBuffer::GetMemoryRequirements(IDevice* device) const
    {
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(device->m_device, handle, &requirements);
        return requirements;
    }

    DeviceMemoryPtr IBuffer::Map(IDevice* device)
    {
        TL_ASSERT(mapped == false, "Buffer is already mapped");
        mapped = true;

        DeviceMemoryPtr ptr = nullptr;
        VulkanResult    result;
        result = vmaMapMemory(device->m_deviceAllocator, allocation, &ptr);
        TL_ASSERT(result)
        return ptr;
    }

    void IBuffer::Unmap(IDevice* device)
    {
        TL_ASSERT(mapped == true, "Buffer is already unmapped");
        mapped = false;
        vmaUnmapMemory(device->m_deviceAllocator, allocation);
    }

    ////////////////////////////////////////////////////////////////////////
    // IImage
    ////////////////////////////////////////////////////////////////////////

    ResultCode IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        VulkanResult result;

        VmaAllocationCreateInfo allocationInfo{
            .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            .usage          = VMA_MEMORY_USAGE_GPU_ONLY,
            .requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = VK_NULL_HANDLE,
            .pUserData      = nullptr,
            .priority       = 0.0f,
        };
        VkImageCreateInfo imageCI{
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .imageType             = ConvertImageType(createInfo.type),
            .format                = ConvertFormat(createInfo.format),
            .extent                = ConvertExtent3D(createInfo.size),
            .mipLevels             = createInfo.mipLevels,
            .arrayLayers           = createInfo.arrayCount,
            .samples               = ConvertSampleCount(createInfo.sampleCount),
            .tiling                = VK_IMAGE_TILING_OPTIMAL,
            .usage                 = ConvertImageUsageFlags(createInfo.usageFlags),
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        result = vkCreateImage(device->m_device, &imageCI, nullptr, &handle); //, &allocation, nullptr);
        TL_ASSERT(result, "vkCreateImage failed with error: {}", result.AsString());

        result = vmaAllocateMemoryForImage(device->m_deviceAllocator, handle, &allocationInfo, &allocation, nullptr);
        TL_ASSERT(result, "vmaAllocateMemoryForImage failed with error: {}", result.AsString());

        vmaBindImageMemory(device->m_deviceAllocator, allocation, handle);

        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
            vmaSetAllocationName(device->m_deviceAllocator, allocation, createInfo.name);
        }

        VkImageViewCreateInfo imageViewCI{
            .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext      = nullptr,
            .flags      = 0,
            .image      = handle,
            .viewType   = VK_IMAGE_VIEW_TYPE_1D,
            .format     = imageCI.format,
            .components = {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask     = ConvertImageAspect(GetFormatAspects(createInfo.format), createInfo.format),
                .baseMipLevel   = 0,
                .levelCount     = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount     = VK_REMAINING_ARRAY_LAYERS,
            },
        };

        switch (imageCI.imageType)
        {
        case VK_IMAGE_TYPE_1D: imageViewCI.viewType = imageCI.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case VK_IMAGE_TYPE_2D: imageViewCI.viewType = imageCI.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case VK_IMAGE_TYPE_3D: imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        default:               TL_UNREACHABLE(); break;
        }

        result = vkCreateImageView(device->m_device, &imageViewCI, nullptr, &viewHandle);
        TL_ASSERT(result, "vkCreateImageView failed with error: {}", result.AsString());

        this->size         = createInfo.size;
        this->format       = createInfo.format;
        this->subresources = {
            .imageAspects  = GetFormatAspects(format),
            .mipBase       = 0,
            .mipLevelCount = (uint8_t)createInfo.mipLevels,
            .arrayBase     = 0,
            .arrayCount    = (uint8_t)createInfo.arrayCount,
        };

        return ConvertResult(result);
    }

    ResultCode IImage::Init(IDevice* device, const ImageViewCreateInfo& createInfo)
    {
        TL_UNREACHABLE_MSG("TODO: Implement!");
        return ResultCode::ErrorUnknown;
    }

    ResultCode IImage::Init(TL_MAYBE_UNUSED IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCI)
    {
        this->handle       = image;
        this->size         = {swapchainCI.imageExtent.width, swapchainCI.imageExtent.height, 1};
        this->format       = ConvertFormat(swapchainCI.imageFormat);
        this->subresources = {
            .imageAspects  = GetFormatAspects(format),
            .mipBase       = 0,
            .mipLevelCount = 1,
            .arrayBase     = 0,
            .arrayCount    = 1,
        };

        // VkImageViewCreateInfo imageViewCI{
        //     .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        //     .pNext      = nullptr,
        //     .flags      = 0,
        //     .image      = handle,
        //     .viewType   = VK_IMAGE_VIEW_TYPE_2D,
        //     .format     = swapchainCI.imageFormat,
        //     .components = {
        //                    VK_COMPONENT_SWIZZLE_IDENTITY,
        //                    VK_COMPONENT_SWIZZLE_IDENTITY,
        //                    VK_COMPONENT_SWIZZLE_IDENTITY,
        //                    VK_COMPONENT_SWIZZLE_IDENTITY,
        //                    },
        //     .subresourceRange = {
        //                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        //                    .baseMipLevel   = 0,
        //                    .levelCount     = VK_REMAINING_MIP_LEVELS,
        //                    .baseArrayLayer = 0,
        //                    .layerCount     = VK_REMAINING_ARRAY_LAYERS,
        //                    },
        // };
        // VulkanResult result = vkCreateImageView(device->m_device, &imageViewCI, nullptr, &viewHandle);
        return ResultCode::Success;
    }

    void IImage::Shutdown(IDevice* device)
    {
        auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();

        if (handle)
            device->m_destroyQueue->Push(frame, handle);

        if (viewHandle)
            device->m_destroyQueue->Push(frame, viewHandle);

        if (allocation)
            device->m_destroyQueue->Push(frame, allocation);
    }

    VkMemoryRequirements IImage::GetMemoryRequirements(IDevice* device) const
    {
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device->m_device, handle, &requirements);
        return requirements;
    }

    VkImageAspectFlags IImage::SelectImageAspect(ImageAspect aspect)
    {
        return ConvertImageAspect(aspect, format);
    }

    ////////////////////////////////////////////////////////////////////////
    // ISampler
    ////////////////////////////////////////////////////////////////////////

    ResultCode ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        VkSamplerCreateInfo samplerCI{
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .magFilter               = ConvertFilter(createInfo.filterMag),
            .minFilter               = ConvertFilter(createInfo.filterMin),
            .mipmapMode              = createInfo.filterMip == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU            = ConvertSamplerAddressMode(createInfo.addressU),
            .addressModeV            = ConvertSamplerAddressMode(createInfo.addressV),
            .addressModeW            = ConvertSamplerAddressMode(createInfo.addressW),
            .mipLodBias              = createInfo.mipLodBias,
            .anisotropyEnable        = VK_TRUE,
            .maxAnisotropy           = 1.0f,
            .compareEnable           = VK_TRUE,
            .compareOp               = ConvertCompareOp(createInfo.compare),
            .minLod                  = createInfo.minLod,
            .maxLod                  = createInfo.maxLod,
            .borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE,
        };
        auto result = vkCreateSampler(device->m_device, &samplerCI, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void ISampler::Shutdown(IDevice* device)
    {
        auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();
        device->m_destroyQueue->Push(frame, handle);
    }

    ////////////////////////////////////////////////////////////////////////
    // ISampler
    ////////////////////////////////////////////////////////////////////////

    ISwapchain::ISwapchain()  = default;
    ISwapchain::~ISwapchain() = default;

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        m_device = device;
        if (createInfo.name)
            m_name = createInfo.name;

        VulkanResult result = CreateSurface(*m_device, createInfo, m_surface);
        if (!result)
        {
            Shutdown(device);
            return result;
        }

        m_imageHandle = TL::construct<IImage>();

        for (uint32_t i = 0; i < MaxImageCount; ++i)
        {
            VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_acquireSemaphore[i]);
            TL_ASSERT(result, "Failed to create swapchain semaphore");
            result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_presentSemaphore[i]);
            TL_ASSERT(result, "Failed to create swapchain semaphore");
            if (!m_name.empty())
            {
                m_device->SetDebugName(m_acquireSemaphore[i], "swapchain: {} - acquire[{}]", m_name, i);
                m_device->SetDebugName(m_presentSemaphore[i], "swapchain: {} - present[{}]", m_name, i);
            }
        }
        return result;
    }

    void ISwapchain::Shutdown(IDevice* device)
    {
        auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();

        for (uint32_t i = 0; i < MaxImageCount; ++i)
        {
            if (m_acquireSemaphore[i])
            {
                m_device->m_destroyQueue->Push(frame, m_acquireSemaphore[i]);
                m_acquireSemaphore[i] = VK_NULL_HANDLE;
            }
        }

        for (uint32_t i = 0; i < MaxImageCount; ++i)
        {
            if (m_imageViews[i])
            {
                m_device->m_destroyQueue->Push(frame, m_imageViews[i]);
                m_imageViews[i] = VK_NULL_HANDLE;
            }
        }

        if (m_swapchain)
        {
            m_device->m_destroyQueue->Push(frame, m_swapchain);
            m_swapchain = VK_NULL_HANDLE;
        }

        if (m_surface)
        {
            m_device->m_destroyQueue->Push(frame, m_surface);
            m_surface = VK_NULL_HANDLE;
        }

        for (uint32_t i = 0; i < MaxImageCount; ++i)
        {
            m_images[i] = VK_NULL_HANDLE;
        }

        TL::free(m_imageHandle);
    }

    VkResult ISwapchain::CreateSurface(IDevice& device, const SwapchainCreateInfo& createInfo, VkSurfaceKHR& outSurface)
    {
        VulkanResult result;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        VkWin32SurfaceCreateInfoKHR win32SurfaceCI =
            {
                .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .pNext     = nullptr,
                .flags     = 0,
                .hinstance = static_cast<HINSTANCE>(createInfo.win32Window.hinstance),
                .hwnd      = static_cast<HWND>(createInfo.win32Window.hwnd),
            };
        result = vkCreateWin32SurfaceKHR(device.m_instance, &win32SurfaceCI, nullptr, &outSurface);
#endif

        if (!result)
        {
            TL_LOG_ERROR("Failed to create swapchain surface with error: {}", result.AsString());
        }

        IQueue*  queue = (IQueue*)device.GetQueue(QueueType::Graphics);
        VkBool32 surfaceSupportPresent;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(device.m_physicalDevice, queue->m_familyIndex, outSurface, &surfaceSupportPresent);
        TL_ASSERT(result);

        if (surfaceSupportPresent != VK_TRUE)
        {
            TL_LOG_ERROR("surface does not support present: {}", result.AsString());
            vkDestroySurfaceKHR(device.m_instance, outSurface, nullptr);
            outSurface = VK_NULL_HANDLE;
        }

        return VK_SUCCESS;
    }

    uint32_t ISwapchain::GetImagesCount() const
    {
        return m_configuration.imageCount;
    }

    Image* ISwapchain::GetImage() const
    {
        return m_imageHandle;
    }

    SurfaceCapabilities ISwapchain::GetSurfaceCapabilities() const
    {
        TL_ASSERT(m_surface);

        VulkanResult result;

        VkSurfaceCapabilitiesKHR capabilities;
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &capabilities);
        TL_ASSERT(result);

        SurfaceCapabilities output{};

        // Add image size and count limits
        {
            output.minImageSize  = {capabilities.minImageExtent.width, capabilities.minImageExtent.height};
            output.maxImageSize  = {capabilities.maxImageExtent.width, capabilities.maxImageExtent.height};
            output.minImageCount = capabilities.minImageCount;
            output.maxImageCount = capabilities.maxImageCount;
        }

        // Add image usage flags
        {
            if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) output.usages |= ImageUsage::ShaderResource;
            if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) output.usages |= ImageUsage::StorageResource;
            if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) output.usages |= ImageUsage::Color;
            if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) output.usages |= ImageUsage::CopyDst;
        }

        // Add formats
        {
            uint32_t formatCount{0};
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, nullptr);
            TL_ASSERT(result);
            TL::Vector<VkSurfaceFormatKHR> formats(formatCount);
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, formats.data());
            TL_ASSERT(result);
            for (const auto& surfaceFormat : formats)
            {
                output.formats.push_back(ConvertFormat(surfaceFormat.format));
            }
        }

        // Add composite alpha modes
        {
            auto alpha = capabilities.supportedCompositeAlpha;
            if (alpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) output.alphaModes |= SwapchainAlphaMode::None;
            if (alpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) output.alphaModes |= SwapchainAlphaMode::PreMultiplied;
            if (alpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) output.alphaModes |= SwapchainAlphaMode::PostMultiplied;
        }

        // Add present modes
        {
            uint32_t presentModeCount{0};
            result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->m_physicalDevice, m_surface, &presentModeCount, nullptr);
            TL_ASSERT(result);
            TL::Vector<VkPresentModeKHR> presentModes(presentModeCount);
            result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->m_physicalDevice, m_surface, &presentModeCount, presentModes.data());
            TL_ASSERT(result);
            for (VkPresentModeKHR mode : presentModes)
            {
                switch (mode)
                {
                case VK_PRESENT_MODE_IMMEDIATE_KHR:    output.presentModes |= SwapchainPresentMode::Immediate; break;
                case VK_PRESENT_MODE_MAILBOX_KHR:      output.presentModes |= SwapchainPresentMode::Mailbox; break;
                case VK_PRESENT_MODE_FIFO_KHR:         output.presentModes |= SwapchainPresentMode::Fifo; break;
                case VK_PRESENT_MODE_FIFO_RELAXED_KHR: output.presentModes |= SwapchainPresentMode::FifoRelaxed; break;
                default:                               TL_UNREACHABLE(); break;
                }
            }
        }
        return output;
    }

    ResultCode ISwapchain::Resize(const ImageSize2D& size)
    {
        m_configuration.size = size;
        return Configure(m_configuration);
    }

    ResultCode ISwapchain::Configure(const SwapchainConfigureInfo& configInfo)
    {
        TL_ASSERT(configInfo.imageCount <= MaxImageCount, "Swapchain returned more images than supported.");

        m_configuration = configInfo;

        VulkanResult result;

        // Query surface capabilities
        VkSurfaceCapabilitiesKHR surfaceCaps;
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCaps);
        if (!result) return ConvertResult(result);

        // Validate image usage flags
        VkImageUsageFlags imageUsage = ConvertImageUsageFlags(configInfo.imageUsage);
        if ((imageUsage & surfaceCaps.supportedUsageFlags) != imageUsage)
        {
            TL_LOG_ERROR("Failed to create swapchain, image usage flags not supported");
            return ResultCode::ErrorInvalidArguments;
        }

        // Clamp extent to supported range
        VkExtent2D extent = ConvertExtent2D(configInfo.size);
        extent.width      = std::clamp(extent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        extent.height     = std::clamp(extent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
        if (extent.width != configInfo.size.width || extent.height != configInfo.size.height)
        {
            TL_LOG_WARNNING(
                "Swapchain size clamped from requested ({}, {}) to supported ({}, {})",
                configInfo.size.width,
                configInfo.size.height,
                extent.width,
                extent.height);
        }

        // Clamp image count to supported range
        uint32_t imageCount = std::clamp(configInfo.imageCount, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);
        if (imageCount != configInfo.imageCount)
        {
            TL_LOG_WARNNING(
                "Swapchain image count clamped from requested ({}) to supported ({}, min: {}, max: {})",
                configInfo.imageCount,
                imageCount,
                surfaceCaps.minImageCount,
                surfaceCaps.maxImageCount);
        }

        // Select surface format
        VkSurfaceFormatKHR surfaceFormat{};
        {
            uint32_t formatCount{0};
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, nullptr);
            TL_ASSERT(result);

            TL::Vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->m_physicalDevice, m_surface, &formatCount, surfaceFormats.data());
            TL_ASSERT(result);

            VkFormat desiredFormat = ConvertFormat(m_configuration.format);
            bool     found         = false;
            for (const auto& currSurfaceFormat : surfaceFormats)
            {
                if (currSurfaceFormat.format == desiredFormat)
                {
                    surfaceFormat = currSurfaceFormat;
                    found         = true;
                    break;
                }
            }
            TL_ASSERT(found);
        }

        VkSwapchainCreateInfoKHR createInfo{
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext                 = nullptr,
            .flags                 = 0,
            .surface               = m_surface,
            .minImageCount         = imageCount,
            .imageFormat           = surfaceFormat.format,
            .imageColorSpace       = surfaceFormat.colorSpace,
            .imageExtent           = extent,
            .imageArrayLayers      = 1,
            .imageUsage            = ConvertImageUsageFlags(m_configuration.imageUsage),
            .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha        = ConvertToAlphaMode(m_configuration.alphaMode),
            .presentMode           = ConvertToPresentMode(m_configuration.presentMode),
            .clipped               = VK_TRUE,
            .oldSwapchain          = m_swapchain,
        };
        result = vkCreateSwapchainKHR(m_device->m_device, &createInfo, nullptr, &m_swapchain);
        TL_ASSERT(result);

        if (m_name.empty() == false)
            m_device->SetDebugName(m_swapchain, m_name.c_str());

        // Destroy old image views and old swapchain if present
        {
            // auto frame = ((IQueue*)device->GetQueue(QueueType::Graphics))->m_lastSubmitValue.load();
            for (uint32_t i = 0; i < MaxImageCount; i++)
            {
                if (m_imageViews[i] != VK_NULL_HANDLE)
                {
                    // m_device->m_destroyQueue->Push(frame, m_imageViews[i]);
                    m_imageViews[i] = VK_NULL_HANDLE;
                }
            }

            if (createInfo.oldSwapchain)
            {
                m_device->m_destroyQueue->Push(0, createInfo.oldSwapchain);
            }
        }

        result = vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, nullptr);
        TL_ASSERT(result, "Failed to get swapchain images count");

        result = vkGetSwapchainImagesKHR(m_device->m_device, m_swapchain, &m_imageCount, m_images);
        TL_ASSERT(result, "Failed to get swapchain images count");

        for (uint32_t i = 0; i < m_imageCount; i++)
        {
            VkImageViewCreateInfo imageViewCI{
                .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext            = nullptr,
                .flags            = 0,
                .image            = m_images[i],
                .viewType         = VK_IMAGE_VIEW_TYPE_2D,
                .format           = surfaceFormat.format,
                .components       = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS},
            };
            result = vkCreateImageView(m_device->m_device, &imageViewCI, nullptr, &m_imageViews[i]);
            TL_ASSERT(result, "Failed to create swapchain image view");
            if (m_name.empty() == false)
            {
                m_device->SetDebugName(m_images[i], "swapchain: {} - image [{}]", m_name, i);
                m_device->SetDebugName(m_imageViews[i], "swapchain: {} - view [{}]", m_name, i);
            }
        }

        AcquireNextImage();

        return ConvertResult(result);
    }

    VkResult ISwapchain::AcquireNextImage()
    {
        VkSemaphore acquireSemaphore = m_acquireSemaphore[m_acquireSemaphoreIndex];

        VulkanResult              result;
        VkAcquireNextImageInfoKHR acquireInfo{
            .sType      = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
            .pNext      = nullptr,
            .swapchain  = m_swapchain,
            .timeout    = UINT64_MAX,
            .semaphore  = acquireSemaphore,
            .fence      = VK_NULL_HANDLE,
            .deviceMask = 0x00000001,
        };
        result = vkAcquireNextImage2KHR(m_device->m_device, &acquireInfo, &m_imageIndex);

        auto updateCurrentImage = [this]()
        {
            auto [width, height] = m_configuration.size;
            if (auto image = (IImage*)(m_imageHandle))
            {
                image->handle     = m_images[m_imageIndex];
                image->viewHandle = m_imageViews[m_imageIndex];
                image->format     = m_configuration.format;
                image->size       = {width, height};
            }
        };

        if (result.IsSwapchainSuccess())
        {
            updateCurrentImage();
        }
        else if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            TL_LOG_WARNNING("Swapchain is out of date, attempting to reconfigure (result: {})", result.AsString());
            if (Configure(m_configuration) == ResultCode::Success)
            {
                acquireInfo.swapchain = m_swapchain;
                result                = vkAcquireNextImage2KHR(m_device->m_device, &acquireInfo, &m_imageIndex);
                if (result.IsSwapchainSuccess())
                {
                    updateCurrentImage();
                }
                else
                {
                    TL_LOG_ERROR("Failed to acquire next image after swapchain reconfiguration (result: {})", result.AsString());
                }
            }
            else
            {
                TL_LOG_ERROR("Failed to reconfigure swapchain after out-of-date/suboptimal error.");
            }
        }
        else
        {
            TL_LOG_ERROR("Failed to acquire next swapchain image (result: {})", result.AsString());
        }

        if (result.IsSuccess())
        {
            // m_acquireSemaphoreIndex = (m_acquireSemaphoreIndex + 1) % MaxImageCount;
        }

        return result;
    }

    VkResult ISwapchain::Present(TL::Span<Fence* const> fences)
    {
        TL::Vector<VkSemaphore> waitSemaphores{m_device->m_arena};
        for (auto& _fence : fences)
        {
            auto fence = (IFence*)_fence;
            waitSemaphores.push_back(fence->semaphore);
            // TL_ASSERT(fence->value == SWAPCHAIN_VALUE)
        }

        VkResult presentResult;

        IQueue*          graphicsQueue = (IQueue*)m_device->GetQueue(QueueType::Graphics);
        VkPresentInfoKHR presentInfo{
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = nullptr,
            .waitSemaphoreCount = (uint32_t)waitSemaphores.size(),
            .pWaitSemaphores    = waitSemaphores.data(),
            .swapchainCount     = 1,
            .pSwapchains        = &m_swapchain,
            .pImageIndices      = &m_imageIndex,
            .pResults           = &presentResult,
        };
        VkResult presentSubmitResult = vkQueuePresentKHR(graphicsQueue->m_queue, &presentInfo);

        return presentResult;
    }

} // namespace RHI::Vulkan