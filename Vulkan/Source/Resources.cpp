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

    // TODO: remove or assert
    inline static constexpr uint32_t c_MaxRenderTargetAttachmentsCount = 16u;
    inline static constexpr uint32_t c_MaxPipelineVertexBindings       = 32u;
    inline static constexpr uint32_t c_MaxPipelineVertexAttributes     = 32u;
    inline static constexpr uint32_t c_MaxPipelineBindGroupsCount      = 4u;

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

    ////////////////////////////////////////////////////////////////////////
    // BindGroupAllocator
    ////////////////////////////////////////////////////////////////////////

    BindGroupAllocator::BindGroupAllocator()  = default;
    BindGroupAllocator::~BindGroupAllocator() = default;

    ResultCode BindGroupAllocator::Init(IDevice* device)
    {
        m_device = device;

        /// @todo: (don't do this)
        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                1024},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1024},
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
        Validate(vkCreateDescriptorPool(m_device->m_device, &createInfo, nullptr, &m_descriptorPool));
        return ResultCode::Success;
    }

    void BindGroupAllocator::Shutdown()
    {
        vkResetDescriptorPool(m_device->m_device, m_descriptorPool, 0);
        vkDestroyDescriptorPool(m_device->m_device, m_descriptorPool, nullptr);
    }

    ResultCode BindGroupAllocator::InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout)
    {
        bindGroup->bindlessCount = bindGroupLayout->bindlessCount;
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorInfo =
            {
                .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
                .pNext              = nullptr,
                .descriptorSetCount = 1,
                .pDescriptorCounts  = &bindGroup->bindlessCount,
            };
        VkDescriptorSetAllocateInfo allocateInfo =
            {
                .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext              = bindGroupLayout->bindlessCount ? &variableDescriptorInfo : nullptr,
                .descriptorPool     = m_descriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts        = &bindGroupLayout->handle,
            };
        auto result = vkAllocateDescriptorSets(m_device->m_device, &allocateInfo, &bindGroup->descriptorSet);
        if (result != VK_SUCCESS)
            return ResultCode::ErrorOutOfMemory;
        return ResultCode::Success;
    }

    void BindGroupAllocator::ShutdownBindGroup(IBindGroup* bindGroup)
    {
        // m_device->m_destroyQueue->Push(
        //     m_device->m_frameIndex,
        //     [=](IDevice* device)
        //     {
        //         vkFreeDescriptorSets(device->m_device, m_descriptorPool, 1, &bindGroup->descriptorSet);
        //     });
    }

    ////////////////////////////////////////////////////////////////////////
    // IBindGroupLayout
    ////////////////////////////////////////////////////////////////////////

    ResultCode IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        uint32_t                 bindingFlagsCount = 0;
        VkDescriptorBindingFlags bindingFlags[32]  = {};

        uint32_t                     bindingLayoutsCount = 0;
        VkDescriptorSetLayoutBinding bindingLayouts[32]  = {};

        uint32_t bindingCount = 0;
        for (size_t i = 0; i < createInfo.bindings.size(); ++i)
        {
            const auto& binding          = createInfo.bindings[i];
            shaderBindings[bindingCount] = binding; // Store ShaderBinding in IBindGroupLayout

            // Check if this binding is bindless (array count is large or other criteria)
            bool isBindless = binding.arrayCount == BindlessArraySize;

            // Assign bindless descriptor binding flags if necessary
            if (isBindless)
            {
                bindingFlags[bindingFlagsCount++] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                                    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

                bindlessCount = 1024;
            }
            else
            {
                bindingFlags[bindingFlagsCount++] = 0;
            }

            // Fill out VkDescriptorSetLayoutBinding
            bindingLayouts[bindingLayoutsCount++] = {
                .binding            = bindingCount++,
                .descriptorType     = ConvertDescriptorType(binding.type),
                .descriptorCount    = binding.arrayCount, // Bindless types will have large counts or variable counts
                .stageFlags         = ConvertShaderStage(binding.stages),
                .pImmutableSamplers = nullptr,
            };
        }

        // If bindless, set up VkDescriptorSetLayoutBindingFlagsCreateInfo
        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext         = nullptr,
            .bindingCount  = bindingFlagsCount,
            .pBindingFlags = bindingFlags,
        };

        VkDescriptorSetLayoutCreateInfo setLayoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = bindlessCount ? &bindingFlagsInfo : nullptr,
            .flags = static_cast<VkDescriptorSetLayoutCreateFlags>(
                bindlessCount ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT : 0),
            .bindingCount = bindingLayoutsCount,
            .pBindings    = bindingLayouts,
        };

        // Create the descriptor set layout
        auto result = vkCreateDescriptorSetLayout(device->m_device, &setLayoutCI, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }

        return ConvertResult(result);
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
        auto layoutObject = device->m_bindGroupLayoutsOwner.Get(createInfo.layout);

        std::copy(std::begin(layoutObject->shaderBindings), std::end(layoutObject->shaderBindings), std::begin(shaderBindings));

        return allocator->InitBindGroup(this, layoutObject);
    }

    void IBindGroup::Shutdown(IDevice* device)
    {
        auto allocator = device->m_bindGroupAllocator.get();
        allocator->ShutdownBindGroup(this);
    }

    void IBindGroup::Write(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        uint32_t writeInfosCount = 0;

        uint32_t              imageInfosCount = 0;
        VkDescriptorImageInfo imageInfos[128] = {};

        uint32_t               bufferInfosCount = 0;
        VkDescriptorBufferInfo bufferInfos[128] = {};

        uint32_t              samplerInfosCount = 0;
        VkDescriptorImageInfo samplerInfos[128] = {};

        VkWriteDescriptorSet writeInfos[128 * 3]{};

        for (const auto& imageUpdate : updateInfo.images)
        {
            const auto&      binding        = shaderBindings[imageUpdate.dstBinding];
            VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

            for (size_t i = 0; i < imageUpdate.images.size(); ++i)
            {
                imageInfos[imageInfosCount] = {
                    .sampler     = VK_NULL_HANDLE,
                    .imageView   = device->m_imageOwner.Get(imageUpdate.images[i])->viewHandle,
                    .imageLayout = (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ? VK_IMAGE_LAYOUT_GENERAL
                                                                                        : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                };

                writeInfos[writeInfosCount++] = {
                    .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext            = nullptr,
                    .dstSet           = descriptorSet,
                    .dstBinding       = imageUpdate.dstBinding,
                    .dstArrayElement  = imageUpdate.dstArrayElement + static_cast<uint32_t>(i),
                    .descriptorCount  = 1,
                    .descriptorType   = descriptorType,
                    .pImageInfo       = &imageInfos[imageInfosCount++],
                    .pBufferInfo      = nullptr,
                    .pTexelBufferView = nullptr,
                };
            }
        }

        for (const auto& bufferUpdate : updateInfo.buffers)
        {
            TL_ASSERT(bufferUpdate.buffers.size() == bufferUpdate.subregions.size());

            const auto&      binding        = shaderBindings[bufferUpdate.dstBinding];
            VkDescriptorType descriptorType = ConvertDescriptorType(binding.type);

            for (size_t i = 0; i < bufferUpdate.buffers.size(); ++i)
            {
                auto buffer    = device->m_bufferOwner.Get(bufferUpdate.buffers[i]);
                auto subregion = bufferUpdate.subregions[i];

                bufferInfos[bufferInfosCount] = {
                    .buffer = buffer->handle,
                    .offset = subregion.offset,
                    .range  = subregion.size,
                };

                writeInfos[writeInfosCount++] = {
                    .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext            = nullptr,
                    .dstSet           = descriptorSet,
                    .dstBinding       = bufferUpdate.dstBinding,
                    .dstArrayElement  = bufferUpdate.dstArrayElement + static_cast<uint32_t>(i),
                    .descriptorCount  = 1,
                    .descriptorType   = descriptorType,
                    .pImageInfo       = nullptr,
                    .pBufferInfo      = &bufferInfos[bufferInfosCount++],
                    .pTexelBufferView = nullptr,
                };
            }
        }

        for (const auto& samplerUpdate : updateInfo.samplers)
        {
            [[maybe_unused]] const auto& binding        = shaderBindings[samplerUpdate.dstBinding];
            VkDescriptorType             descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

            for (size_t i = 0; i < samplerUpdate.samplers.size(); ++i)
            {
                samplerInfos[samplerInfosCount] = {
                    .sampler     = device->m_samplerOwner.Get(samplerUpdate.samplers[i])->handle,
                    .imageView   = VK_NULL_HANDLE,
                    .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                };

                writeInfos[writeInfosCount++] = {
                    .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext            = nullptr,
                    .dstSet           = descriptorSet,
                    .dstBinding       = samplerUpdate.dstBinding,
                    .dstArrayElement  = samplerUpdate.dstArrayElement + static_cast<uint32_t>(i),
                    .descriptorCount  = 1,
                    .descriptorType   = descriptorType,
                    .pImageInfo       = &samplerInfos[samplerInfosCount++],
                    .pBufferInfo      = nullptr,
                    .pTexelBufferView = nullptr,
                };
            }
        }

        vkUpdateDescriptorSets(device->m_device, writeInfosCount, writeInfos, 0, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////
    // IShaderModule
    ////////////////////////////////////////////////////////////////////////
    IShaderModule::IShaderModule()  = default;
    IShaderModule::~IShaderModule() = default;

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        m_device = device;
        VkShaderModuleCreateInfo shaderModuleCI =
            {
                .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext    = nullptr,
                .flags    = {},
                .codeSize = createInfo.code.size_bytes(),
                .pCode    = createInfo.code.data(),
            };
        auto result = vkCreateShaderModule(device->m_device, &shaderModuleCI, nullptr, &m_shaderModule);
        return ConvertResult(result);
    }

    void IShaderModule::Shutdown()
    {
        vkDestroyShaderModule(m_device->m_device, m_shaderModule, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////
    // IPipelineLayout
    ////////////////////////////////////////////////////////////////////////
    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        TL::Vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (auto bindGroupLayout : createInfo.layouts)
        {
            if (bindGroupLayout == RHI::NullHandle)
            {
                break;
            }

            auto layout = device->m_bindGroupLayoutsOwner.Get(bindGroupLayout);
            descriptorSetLayouts.push_back(layout->handle);
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
        // auto scopeAllocator = TL::ScopeAllocator(TL::TempAllocator);

        uint32_t                        stagesCreateInfoCount = 2;
        VkPipelineShaderStageCreateInfo stagesCreateInfos[4];
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
            stagesCreateInfos[0] = vertexStageCI;

            VkPipelineShaderStageCreateInfo pixelStageCI{
                .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext               = nullptr,
                .flags               = 0,
                .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module              = static_cast<IShaderModule*>(createInfo.pixelShaderModule)->m_shaderModule,
                .pName               = createInfo.pixelShaderName,
                .pSpecializationInfo = nullptr,
            };
            stagesCreateInfos[1] = pixelStageCI;
        }

        uint32_t vertexBindingsCount   = 0;
        uint32_t vertexAttributesCount = 0;

        VkVertexInputBindingDescription   vertexBindings[c_MaxPipelineVertexBindings]     = {};
        VkVertexInputAttributeDescription vertexAttributes[c_MaxPipelineVertexAttributes] = {};

        uint32_t bindingIndex = 0;
        for (const auto& bindingDesc : createInfo.vertexBufferBindings)
        {
            // Set up vertex binding
            auto& binding     = vertexBindings[vertexBindingsCount];
            binding.binding   = bindingIndex;
            binding.stride    = bindingDesc.stride;
            binding.inputRate = ConvertVertexInputRate(bindingDesc.stepRate);

            // Iterate through vertex attributes for this binding
            for (const auto& attributeDesc : bindingDesc.attributes)
            {
                // Set up vertex attribute
                auto& attribute    = vertexAttributes[vertexAttributesCount];
                attribute.location = vertexAttributesCount;
                attribute.binding  = bindingIndex;
                attribute.format   = ConvertFormat(attributeDesc.format);
                attribute.offset   = attributeDesc.offset;

                vertexAttributesCount++;
            }

            vertexBindingsCount++;
            bindingIndex++;
        }

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext                           = nullptr,
            .flags                           = 0,
            .vertexBindingDescriptionCount   = vertexBindingsCount,
            .pVertexBindingDescriptions      = vertexBindings,
            .vertexAttributeDescriptionCount = vertexAttributesCount,
            .pVertexAttributeDescriptions    = vertexAttributes,
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

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCI{
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext             = nullptr,
            .flags             = 0,
            .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
            .pDynamicStates    = dynamicStates,
        };

        uint32_t colorAttachmentFormatCount                                = 0;
        VkFormat colorAttachmentFormats[c_MaxRenderTargetAttachmentsCount] = {};

        for (uint32_t formatIndex = 0; formatIndex < createInfo.renderTargetLayout.colorAttachmentsFormats.size(); formatIndex++)
        {
            auto format = createInfo.renderTargetLayout.colorAttachmentsFormats[formatIndex];
            if (format == Format::Unknown)
            {
                break;
            }
            colorAttachmentFormatCount++;
            colorAttachmentFormats[formatIndex] = ConvertFormat(format);
        }

        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentStates[c_MaxRenderTargetAttachmentsCount] = {};

        uint32_t location = 0;
        for (auto blendState : createInfo.colorBlendState.blendStates)
        {
            if (location >= colorAttachmentFormatCount)
            {
                break;
            }

            auto& state = pipelineColorBlendAttachmentStates[location++] = {
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
        }

        VkPipelineColorBlendStateCreateInfo colorBlendStateCI{
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .logicOpEnable   = VK_FALSE,
            .logicOp         = VK_LOGIC_OP_SET,
            .attachmentCount = colorAttachmentFormatCount,
            .pAttachments    = pipelineColorBlendAttachmentStates,
            .blendConstants  = {
                                createInfo.colorBlendState.blendConstants[0],
                                createInfo.colorBlendState.blendConstants[1],
                                createInfo.colorBlendState.blendConstants[2],
                                createInfo.colorBlendState.blendConstants[3],
                                },
        };

        VkPipelineRenderingCreateInfo renderTargetLayout{
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext                   = nullptr,
            .viewMask                = 0,
            .colorAttachmentCount    = colorAttachmentFormatCount,
            .pColorAttachmentFormats = colorAttachmentFormats,
            .depthAttachmentFormat   = ConvertFormat(createInfo.renderTargetLayout.depthAttachmentFormat),
            .stencilAttachmentFormat = ConvertFormat(createInfo.renderTargetLayout.stencilAttachmentFormat),
        };

        layout = device->m_pipelineLayoutOwner.Get(createInfo.layout)->handle;

        VkGraphicsPipelineCreateInfo graphicsPipelineCI{
            .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext               = &renderTargetLayout,
            .flags               = 0,
            .stageCount          = stagesCreateInfoCount,
            .pStages             = stagesCreateInfos,
            .pVertexInputState   = &vertexInputStateCI,
            .pInputAssemblyState = &inputAssemblyStateCI,
            .pTessellationState  = &tessellationStateCI,
            .pViewportState      = &viewportStateCI,
            .pRasterizationState = &rasterizationStateCI,
            .pMultisampleState   = &multisampleStateCI,
            .pDepthStencilState  = &depthStencilStateCI,
            .pColorBlendState    = &colorBlendStateCI,
            .pDynamicState       = &dynamicStateCI,
            .layout              = layout,
            .renderPass          = VK_NULL_HANDLE,
            .subpass             = 0,
            .basePipelineHandle  = VK_NULL_HANDLE,
            .basePipelineIndex   = 0,
        };
        auto result = vkCreateGraphicsPipelines(device->m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void IGraphicsPipeline::Shutdown(IDevice* device)
    {
        auto frame = (IFrame*)device->GetCurrentFrame();

        if (handle)
            device->m_destroyQueue->Push(frame->m_timeline, handle);
    }

    ////////////////////////////////////////////////////////////////////////
    // IComputePipeline
    ////////////////////////////////////////////////////////////////////////
    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        auto shaderModule = static_cast<IShaderModule*>(createInfo.shaderModule);

        layout = device->m_pipelineLayoutOwner.Get(createInfo.layout)->handle;

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
            .layout             = layout,
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
            device->m_destroyQueue->Push(frame->m_timeline, handle);
    }

    ////////////////////////////////////////////////////////////////////////
    // IBuffer
    ////////////////////////////////////////////////////////////////////////
    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        VmaAllocationCreateInfo allocationCI =
            {
                .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
                .usage          = createInfo.hostMapped ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .requiredFlags  = 0,
                .preferredFlags = 0,
                .memoryTypeBits = 0,
                .pool           = VK_NULL_HANDLE,
                .pUserData      = nullptr,
                .priority       = 0.0f,
            };
        VkBufferCreateInfo bufferCI =
            {
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
        auto frame = (IFrame*)device->GetCurrentFrame();

        if (handle)
            device->m_destroyQueue->Push(frame->m_timeline, handle);

        if (allocation)
            device->m_destroyQueue->Push(frame->m_timeline, handle);
    }

    VkMemoryRequirements IBuffer::GetMemoryRequirements(IDevice* device) const
    {
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(device->m_device, handle, &requirements);
        return requirements;
    }

    DeviceMemoryPtr IBuffer::Map(IDevice* device)
    {
        DeviceMemoryPtr ptr = nullptr;
        auto            res = vmaMapMemory(device->m_deviceAllocator, allocation, &ptr);
        Validate(res);
        return ptr;
    }

    void IBuffer::Unmap(IDevice* device)
    {
        vmaUnmapMemory(device->m_deviceAllocator, allocation);
    }

    ////////////////////////////////////////////////////////////////////////
    // IImage
    ////////////////////////////////////////////////////////////////////////
    ResultCode IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        VkResult result;

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
        result = vmaAllocateMemoryForImage(device->m_deviceAllocator, handle, &allocationInfo, &allocation, nullptr);
        vmaBindImageMemory(device->m_deviceAllocator, allocation, handle);

        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }

        VkImageViewCreateInfo imageViewCI{
            .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = 0,
            .image    = handle,
            .viewType = VK_IMAGE_VIEW_TYPE_1D,
            .format   = imageCI.format,
            .components{
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        },
            .subresourceRange =
                {
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

        this->size   = createInfo.size;
        this->format = createInfo.format;
        this->subresources =
            {
                .imageAspects  = GetFormatAspects(format),
                .mipBase       = 0,
                .mipLevelCount = (uint8_t)createInfo.mipLevels,
                .arrayBase     = 0,
                .arrayCount    = (uint8_t)createInfo.arrayCount,
            };

        return ConvertResult(result);
    }

    ResultCode IImage::Init(IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCI)
    {
        this->handle = image;
        this->size   = {swapchainCI.imageExtent.width, swapchainCI.imageExtent.height, 1};
        this->format = ConvertFormat(swapchainCI.imageFormat);
        this->subresources =
            {
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
        // VkResult result = vkCreateImageView(device->m_device, &imageViewCI, nullptr, &viewHandle);
        return ResultCode::Success;
    }

    void IImage::Shutdown(IDevice* device)
    {
        auto frame = (IFrame*)device->GetCurrentFrame();

        if (handle)
            device->m_destroyQueue->Push(frame->m_timeline, handle);

        if (viewHandle)
            device->m_destroyQueue->Push(frame->m_timeline, handle);

        if (allocation)
            device->m_destroyQueue->Push(frame->m_timeline, handle);
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
        device->m_destroyQueue->Push(frame->m_timeline, handle);
    }

} // namespace RHI::Vulkan