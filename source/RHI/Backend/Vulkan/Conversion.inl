#pragma once

#include "RHI/Format.hpp"
#include "RHI/Pipeline.hpp"
#include "RHI/Result.hpp"
#include "RHI/ShaderResourceGroup.hpp"

#include <vulkan/vulkan.hpp>

namespace Vulkan
{

inline static vk::Format ConvertFormat(RHI::Format format)
{
    switch (format)
    {
        case RHI::Format::RGBA8: return vk::Format::eR8G8B8A8Srgb;
        case RHI::Format::D24S8: return vk::Format::eD24UnormS8Uint;
        case RHI::Format::D32: return vk::Format::eD32Sfloat;
        default: RHI_ASSERT_MSG(false, "Invalid format");
    }

    return vk::Format::eUndefined;
}

inline static vk::Extent3D ConvertExtent3D(RHI::ImageSize imageSize)
{
    vk::Extent3D extent {};
    extent.setWidth(imageSize.width);
    extent.setHeight(imageSize.height);
    extent.setDepth(imageSize.depth);
    return extent;
}

inline static vk::Offset3D ConverOffset3D(RHI::ImageOffset imageOffset)
{
    vk::Offset3D offset {};
    offset.setX(imageOffset.x);
    offset.setY(imageOffset.y);
    offset.setZ(imageOffset.z);
    return offset;
}

inline static vk::DescriptorType GetDescriptorType(RHI::ShaderResourceType resourceType)
{
    switch (resourceType)
    {
        case RHI::ShaderResourceType::UniformBuffer: return vk::DescriptorType::eUniformBuffer;
        case RHI::ShaderResourceType::StorageBuffer: return vk::DescriptorType::eStorageImage;
        case RHI::ShaderResourceType::UniformTexelBuffer: return vk::DescriptorType::eUniformTexelBuffer;
        case RHI::ShaderResourceType::StorageTexelBuffer: return vk::DescriptorType::eStorageTexelBuffer;
        case RHI::ShaderResourceType::Image: return vk::DescriptorType::eSampledImage;
        case RHI::ShaderResourceType::Sampler: return vk::DescriptorType::eSampler;
        default: RHI_ASSERT_MSG(false, "Invalid enum");
    }

    return {};
}

inline static vk::ShaderStageFlagBits GetShaderStage(RHI::ShaderStage shaderType)
{
    switch (shaderType)
    {
        case RHI::ShaderStage::Vertex: return vk::ShaderStageFlagBits::eVertex;
        case RHI::ShaderStage::TessellationControl: return vk::ShaderStageFlagBits::eTessellationControl;
        case RHI::ShaderStage::TessellationEvaluation: return vk::ShaderStageFlagBits::eTessellationEvaluation;
        case RHI::ShaderStage::Pixel: return vk::ShaderStageFlagBits::eFragment;
        default: RHI_ASSERT_MSG(false, "Invalid enum");
    }

    return {};
}

inline static vk::ImageLayout GetAttachmentLayout(RHI::AttachmentUsage usage, RHI::AttachmentAccess access)
{
    switch (usage)
    {
        case RHI::AttachmentUsage::Resolve:
        case RHI::AttachmentUsage::RenderTarget: return vk::ImageLayout::eColorAttachmentOptimal;
        case RHI::AttachmentUsage::ShaderResource: return vk::ImageLayout::eAttachmentOptimal;
        case RHI::AttachmentUsage::Depth: {
            switch (access)
            {
                case RHI::AttachmentAccess::Read: return vk::ImageLayout::eDepthReadOnlyOptimal;
                case RHI::AttachmentAccess::ReadWrite: return vk::ImageLayout::eDepthAttachmentOptimal;
                default: RHI_UNREACHABLE();
            }
            break;
        }
        case RHI::AttachmentUsage::Stencil: {
            switch (access)
            {
                case RHI::AttachmentAccess::Read: return vk::ImageLayout::eStencilReadOnlyOptimal;
                case RHI::AttachmentAccess::ReadWrite: return vk::ImageLayout::eStencilAttachmentOptimal;
                default: RHI_UNREACHABLE();
            }
            break;
        }
        case RHI::AttachmentUsage::DepthStencil: {
            switch (access)
            {
                case RHI::AttachmentAccess::Read: return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
                case RHI::AttachmentAccess::ReadWrite: return vk::ImageLayout::eDepthStencilAttachmentOptimal;
                default: RHI_UNREACHABLE();
            }
            break;
        }
        case RHI::AttachmentUsage::Copy: {
            switch (access)
            {
                case RHI::AttachmentAccess::Read: return vk::ImageLayout::eTransferSrcOptimal;
                case RHI::AttachmentAccess::ReadWrite: return vk::ImageLayout::eTransferDstOptimal;
                default: RHI_UNREACHABLE();
            }
            break;
        }
    }

    RHI_WARN("Invalid attachment access requested");
    return vk::ImageLayout::eGeneral;
}

inline static vk::PipelineStageFlags2 GetAttachmentPipelineStageFlags(RHI::AttachmentUsage usage, RHI::ShaderStage shaderStage = RHI::ShaderStage::None)
{
    switch (usage)
    {
        case RHI::AttachmentUsage::RenderTarget: return vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        case RHI::AttachmentUsage::Resolve: return vk::PipelineStageFlagBits2::eResolve;
        case RHI::AttachmentUsage::Depth:
        case RHI::AttachmentUsage::Stencil:
        case RHI::AttachmentUsage::DepthStencil:
            return vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
        case RHI::AttachmentUsage::ShaderResource: {
            switch (shaderStage)
            {
                case RHI::ShaderStage::Pixel: return vk::PipelineStageFlagBits2::eFragmentShader;
                case RHI::ShaderStage::Vertex: return vk::PipelineStageFlagBits2::eVertexShader;
                case RHI::ShaderStage::TessellationControl: return vk::PipelineStageFlagBits2::eTessellationControlShader;
                case RHI::ShaderStage::TessellationEvaluation: return vk::PipelineStageFlagBits2::eTessellationEvaluationShader;
                case RHI::ShaderStage::Compute: return vk::PipelineStageFlagBits2::eComputeShader;
                default: RHI_UNREACHABLE(); break;
            }
            break;
        }
        case RHI::AttachmentUsage::Copy: return vk::PipelineStageFlagBits2::eTransfer;
    }

    RHI_WARN("Invalid attachment access requested");
    return vk::PipelineStageFlagBits2::eAllCommands;
}

inline static vk::AccessFlags2 GetAttachmentAccessFlags(RHI::AttachmentUsage usage, RHI::AttachmentAccess access)
{
    switch (usage)
    {
        case RHI::AttachmentUsage::RenderTarget: {
            switch (access)
            {
                case RHI::AttachmentAccess::Read: return vk::AccessFlagBits2::eColorAttachmentRead;
                case RHI::AttachmentAccess::Write: return vk::AccessFlagBits2::eColorAttachmentWrite;
                case RHI::AttachmentAccess::ReadWrite:
                    return vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
                default: RHI_UNREACHABLE(); break;
            }

            break;
        }
        case RHI::AttachmentUsage::Resolve: return vk::AccessFlagBits2::eTransferWrite;
        case RHI::AttachmentUsage::Stencil:
        case RHI::AttachmentUsage::Depth:
        case RHI::AttachmentUsage::DepthStencil: {
            switch (access)
            {
                case RHI::AttachmentAccess::Read: return vk::AccessFlagBits2::eDepthStencilAttachmentRead;
                case RHI::AttachmentAccess::Write: return vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
                case RHI::AttachmentAccess::ReadWrite:
                    return vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
                default: RHI_UNREACHABLE(); break;
            }

            break;
        }
        case RHI::AttachmentUsage::ShaderResource: {
            switch (access)
            {
                case RHI::AttachmentAccess::Read: return vk::AccessFlagBits2::eShaderRead;
                case RHI::AttachmentAccess::Write: return vk::AccessFlagBits2::eShaderWrite;
                case RHI::AttachmentAccess::ReadWrite: return vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite;
                default: RHI_UNREACHABLE(); break;
            }

            break;
        }
        case RHI::AttachmentUsage::Copy: {
            switch (access)
            {
                case RHI::AttachmentAccess::Read: return vk::AccessFlagBits2::eTransferRead;
                case RHI::AttachmentAccess::Write: return vk::AccessFlagBits2::eTransferWrite;
                case RHI::AttachmentAccess::ReadWrite: return vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eTransferWrite;
                default: RHI_UNREACHABLE(); break;
            }

            break;
        }
    }

    RHI_UNREACHABLE_MSG("Invalid attachment access requested");
    return {};
}

inline static vk::ShaderStageFlags ConvertShaderStages(RHI::Flags<RHI::ShaderStage> stages)
{
    vk::ShaderStageFlags flags {};

    if (stages & RHI::ShaderStage::Vertex)
    {
        flags &= vk::ShaderStageFlagBits::eVertex;
    }

    if (stages & RHI::ShaderStage::TessellationControl)
    {
        flags &= vk::ShaderStageFlagBits::eTessellationControl;
    }

    if (stages & RHI::ShaderStage::TessellationEvaluation)
    {
        flags &= vk::ShaderStageFlagBits::eTessellationEvaluation;
    }

    if (stages & RHI::ShaderStage::Pixel)
    {
        flags &= vk::ShaderStageFlagBits::eFragment;
    }

    return flags;
}

}  // namespace Vulkan