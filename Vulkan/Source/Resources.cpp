#include "Resources.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "Frame.hpp"

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    VkBufferUsageFlags ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags)
    {
        VkBufferUsageFlags result = 0;
        if (bufferUsageFlags & BufferUsage::Storage) result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Uniform) result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Vertex) result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Index) result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::CopySrc) result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (bufferUsageFlags & BufferUsage::CopyDst) result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (bufferUsageFlags & BufferUsage::Indirect) result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        return result;
    }

    VkImageUsageFlags ConvertImageUsageFlags(TL::Flags<ImageUsage> imageUsageFlags)
    {
        VkImageUsageFlags result = 0;
        if (imageUsageFlags & ImageUsage::ShaderResource) result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (imageUsageFlags & ImageUsage::StorageResource) result |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (imageUsageFlags & ImageUsage::Color) result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::Depth) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::Stencil) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::CopySrc) result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (imageUsageFlags & ImageUsage::CopyDst) result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (imageUsageFlags & ImageUsage::Present) TL_UNREACHABLE(); // TODO: Handle present usage;
        return result;
    }

    VkImageType ConvertImageType(ImageType imageType)
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

    VkImageViewType ConvertImageViewType(ImageViewType imageType)
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

    VkComponentSwizzle ConvertComponentSwizzle(ComponentSwizzle componentSwizzle)
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
        auto vkSubresource           = VkImageSubresourceRange{};
        vkSubresource.aspectMask     = ConvertImageAspect(subresource.imageAspects, format);
        vkSubresource.baseMipLevel   = subresource.mipBase;
        vkSubresource.levelCount     = subresource.mipLevelCount;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount     = subresource.arrayCount;
        return vkSubresource;
    }

    VkExtent2D ConvertExtent2D(ImageSize2D size)
    {
        return {size.width, size.height};
    }

    VkExtent3D ConvertExtent3D(ImageSize3D size)
    {
        return {size.width, size.height, size.depth};
    }

    VkOffset2D ConvertOffset2D(ImageOffset2D offset)
    {
        return {offset.x, offset.y};
    }

    VkOffset3D ConvertOffset3D(ImageOffset3D offset)
    {
        return {offset.x, offset.y, offset.z};
    }

    VkComponentMapping ConvertComponentMapping(ComponentMapping componentMapping)
    {
        VkComponentMapping mapping{};
        mapping.r = ConvertComponentSwizzle(componentMapping.r);
        mapping.g = ConvertComponentSwizzle(componentMapping.g);
        mapping.b = ConvertComponentSwizzle(componentMapping.b);
        mapping.a = ConvertComponentSwizzle(componentMapping.a);
        return mapping;
    }

    VkFilter ConvertFilter(SamplerFilter samplerFilter)
    {
        switch (samplerFilter)
        {
        case SamplerFilter::Point:  return VK_FILTER_NEAREST;
        case SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default:                    TL_UNREACHABLE(); return VK_FILTER_MAX_ENUM;
        }
    }

    VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:                         TL_UNREACHABLE(); return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    VkCompareOp ConvertCompareOp(CompareOperator compareOperator)
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
        case ShaderStage::Pixel:   return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                   TL_UNREACHABLE(); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkShaderStageFlags ConvertShaderStage(TL::Flags<ShaderStage> shaderStageFlags)
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

    VkDescriptorType ConvertDescriptorType(BindingType bindingType)
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

        TL::Vector<VkDescriptorBindingFlags>     bindingFlags{device->GetTempAllocator()};
        TL::Vector<VkDescriptorSetLayoutBinding> setLayoutBindings{device->GetTempAllocator()};

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
        auto allocator    = device->m_bindGroupAllocator.get();
        auto layoutObject = (IBindGroupLayout*)(createInfo.layout);

        this->bindGroupLayout = (IBindGroupLayout*)createInfo.layout;

        return allocator->InitBindGroup(this, layoutObject, createInfo.bindlessArrayCount);
    }

    void IBindGroup::Shutdown(IDevice* device)
    {
        auto allocator = device->m_bindGroupAllocator.get();
        allocator->ShutdownBindGroup(this);
    }

    void IBindGroup::Update(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        DescriptorSetWriter writer(device, this->descriptorSet, this->bindGroupLayout, device->GetTempAllocator());

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
    // IShaderModule
    ////////////////////////////////////////////////////////////////////////

    IShaderModule::IShaderModule()  = default;
    IShaderModule::~IShaderModule() = default;

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        m_device = device;
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
        vkDestroyShaderModule(m_device->m_device, m_shaderModule, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////
    // IPipelineLayout
    ////////////////////////////////////////////////////////////////////////

    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        TL::Vector<VkDescriptorSetLayout> descriptorSetLayouts{device->GetTempAllocator()};
        uint32_t                          index = 0;
        for (auto bindGroupLayout : createInfo.layouts)
        {
            auto layout = (IBindGroupLayout*)(bindGroupLayout);
            descriptorSetLayouts.push_back(layout->handle);
            this->bindGroupLayouts[index++] = bindGroupLayout;
        }

        VkPipelineLayoutCreateInfo pipelineLayouCI{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext                  = nullptr,
            .flags                  = 0,
            .setLayoutCount         = uint32_t(descriptorSetLayouts.size()),
            .pSetLayouts            = descriptorSetLayouts.data(),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr,
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
        vkDestroyPipelineLayout(device->m_device, handle, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////
    // IGraphicsPipeline
    ////////////////////////////////////////////////////////////////////////

    ResultCode IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        TL::Vector<VkPipelineShaderStageCreateInfo> stagesCreateInfos{device->GetTempAllocator()};
        {
            VkPipelineShaderStageCreateInfo vertexStageCI{
                .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext               = nullptr,
                .flags               = 0,
                .stage               = VK_SHADER_STAGE_VERTEX_BIT,
                .module              = static_cast<IShaderModule*>(createInfo.vertexShaderModule)->m_shaderModule,
                .pName               = createInfo.vertexShaderName,
                .pSpecializationInfo = nullptr,
            };
            stagesCreateInfos.push_back(vertexStageCI);

            VkPipelineShaderStageCreateInfo pixelStageCI{
                .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext               = nullptr,
                .flags               = 0,
                .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module              = static_cast<IShaderModule*>(createInfo.pixelShaderModule)->m_shaderModule,
                .pName               = createInfo.pixelShaderName,
                .pSpecializationInfo = nullptr,
            };
            stagesCreateInfos.push_back(pixelStageCI);
        }

        TL::Vector<VkVertexInputBindingDescription>   vertexBindings{device->GetTempAllocator()};
        TL::Vector<VkVertexInputAttributeDescription> vertexAttributes{device->GetTempAllocator()};
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

        TL::Vector<VkFormat> colorAttachmentFormats{device->GetTempAllocator()};
        colorAttachmentFormats.reserve(createInfo.renderTargetLayout.colorAttachmentsFormats.size());
        for (auto format : createInfo.renderTargetLayout.colorAttachmentsFormats)
        {
            colorAttachmentFormats.push_back(ConvertFormat(format));
        }

        TL::Vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates{device->GetTempAllocator()};
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
            .stageCount          = (uint32_t)stagesCreateInfos.size(),
            .pStages             = stagesCreateInfos.data(),
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
        auto frame = (IFrame*)device->GetCurrentFrame();

        if (handle)
            device->m_destroyQueue->Push(frame->GetTimelineValue(), handle);
    }

    ////////////////////////////////////////////////////////////////////////
    // IComputePipeline
    ////////////////////////////////////////////////////////////////////////

    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        auto shaderModule = static_cast<IShaderModule*>(createInfo.shaderModule);

        this->layout = (IPipelineLayout*)createInfo.layout;

        VkPipelineShaderStageCreateInfo shaderStageCI{
            .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext               = nullptr,
            .flags               = 0,
            .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
            .module              = shaderModule->m_shaderModule,
            .pName               = createInfo.shaderName,
            .pSpecializationInfo = nullptr,
        };

        VkComputePipelineCreateInfo computePipelineCI{
            .sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext              = nullptr,
            .flags              = {},
            .stage              = shaderStageCI,
            .layout             = layout->handle,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex  = 0,
        };

        auto result = vkCreateComputePipelines(device->m_device, VK_NULL_HANDLE, 1, &computePipelineCI, nullptr, &handle);
        return ConvertResult(result);
    }

    void IComputePipeline::Shutdown(IDevice* device)
    {
        auto frame = (IFrame*)device->GetCurrentFrame();

        if (handle)
            device->m_destroyQueue->Push(frame->GetTimelineValue(), handle);
    }

    ////////////////////////////////////////////////////////////////////////
    // IBuffer
    ////////////////////////////////////////////////////////////////////////

    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        VmaAllocationCreateInfo allocationCI{
            .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            .usage          = createInfo.hostMapped ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags  = 0,
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
        }
        return ConvertResult(result);
    }

    void IBuffer::Shutdown(IDevice* device)
    {
        TL_ASSERT(mapped == false, "Unmap buffer first");

        auto frame = (IFrame*)device->GetCurrentFrame();

        if (handle)
            device->m_destroyQueue->Push(frame->GetTimelineValue(), handle);

        if (allocation)
            device->m_destroyQueue->Push(frame->GetTimelineValue(), allocation);
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
        auto frame = (IFrame*)device->GetCurrentFrame();

        if (handle)
            device->m_destroyQueue->Push(frame->GetTimelineValue(), handle);

        if (viewHandle)
            device->m_destroyQueue->Push(frame->GetTimelineValue(), viewHandle);

        if (allocation)
            device->m_destroyQueue->Push(frame->GetTimelineValue(), allocation);
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
        auto frame = (IFrame*)device->GetCurrentFrame();
        device->m_destroyQueue->Push(frame->GetTimelineValue(), handle);
    }

} // namespace RHI::Vulkan