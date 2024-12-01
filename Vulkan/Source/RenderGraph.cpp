#include "RenderGraph.hpp"

#include <vulkan/vulkan.h>

#include "CommandList.hpp"
#include "Device.hpp"

namespace RHI::Vulkan
{
    struct BarrierStage
    {
        VkPipelineStageFlags2 stageMask        = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2        accessMask       = VK_ACCESS_2_NONE;
        VkImageLayout         layout           = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t              queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(TL::Flags<PipelineStage> pipelineStages)
    {
        VkPipelineStageFlags2 stageFlags = {};
        if (pipelineStages & PipelineStage::TopOfPipe) stageFlags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        if (pipelineStages & PipelineStage::DrawIndirect) stageFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        if (pipelineStages & PipelineStage::VertexInput) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
        if (pipelineStages & PipelineStage::VertexShader) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        if (pipelineStages & PipelineStage::TessellationControlShader) stageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;
        if (pipelineStages & PipelineStage::TessellationEvaluationShader) stageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
        if (pipelineStages & PipelineStage::PixelShader) stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        if (pipelineStages & PipelineStage::EarlyFragmentTests) stageFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        if (pipelineStages & PipelineStage::LateFragmentTests) stageFlags |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        if (pipelineStages & PipelineStage::ColorAttachmentOutput) stageFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        if (pipelineStages & PipelineStage::ComputeShader) stageFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        if (pipelineStages & PipelineStage::Transfer) stageFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        if (pipelineStages & PipelineStage::BottomOfPipe) stageFlags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        if (pipelineStages & PipelineStage::Host) stageFlags |= VK_PIPELINE_STAGE_2_HOST_BIT;
        if (pipelineStages & PipelineStage::AllGraphics) stageFlags |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        if (pipelineStages & PipelineStage::AllCommands) stageFlags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        if (pipelineStages & PipelineStage::Copy) stageFlags |= VK_PIPELINE_STAGE_2_COPY_BIT;
        if (pipelineStages & PipelineStage::Resolve) stageFlags |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
        if (pipelineStages & PipelineStage::Blit) stageFlags |= VK_PIPELINE_STAGE_2_BLIT_BIT;
        if (pipelineStages & PipelineStage::Clear) stageFlags |= VK_PIPELINE_STAGE_2_CLEAR_BIT;
        if (pipelineStages & PipelineStage::IndexInput) stageFlags |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
        if (pipelineStages & PipelineStage::VertexAttributeInput) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
        if (pipelineStages & PipelineStage::PreRasterizationShaders) stageFlags |= VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT;
        if (pipelineStages & PipelineStage::TransformFeedback) stageFlags |= VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT;
        if (pipelineStages & PipelineStage::ConditionalRendering) stageFlags |= VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT;
        if (pipelineStages & PipelineStage::FragmentShadingRateAttachment) stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
        if (pipelineStages & PipelineStage::AccelerationStructureBuild) stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
        if (pipelineStages & PipelineStage::RayTracingShader) stageFlags |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
        if (pipelineStages & PipelineStage::TaskShader) stageFlags |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
        if (pipelineStages & PipelineStage::MeshShader) stageFlags |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
        if (pipelineStages & PipelineStage::AccelerationStructureCopy) stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR;
        return stageFlags;
    }

    inline static VkAccessFlags2 GetAccessFlags2(ImageUsage usage, TL::Flags<Access> access)
    {
        switch (usage)
        {
        case ImageUsage::ShaderResource:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case ImageUsage::StorageResource:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case ImageUsage::CopySrc:
        case ImageUsage::CopyDst:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        default: TL_UNREACHABLE(); return {};
        };
    }

    inline static VkAccessFlags2 GetAccessFlags2(BufferUsage usage, TL::Flags<Access> access)
    {
        switch (usage)
        {
        case BufferUsage::None: return {};
        case BufferUsage::Uniform:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case BufferUsage::Storage:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case BufferUsage::CopySrc:
        case BufferUsage::CopyDst:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;

        default: TL_UNREACHABLE(); return {};
        };
    }

    inline static VkImageLayout GetImageLayout(ImageUsage usage, TL::Flags<Access> access, TL::Flags<ImageAspect> aspect)
    {
        switch (usage)
        {
        case ImageUsage::ShaderResource:
            {
                bool isReadOnly = access == Access::Read;

                if (aspect & ImageAspect::Color)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
                }
                else if (aspect & ImageAspect::DepthStencil)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else if (aspect & ImageAspect::Depth)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
                                      : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                }
                else if (aspect & ImageAspect::Stencil)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
                                      : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                }

                TL_UNREACHABLE();
                return VK_IMAGE_LAYOUT_GENERAL;
            }
        case ImageUsage::CopySrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageUsage::CopyDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        default:                  return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    inline static BarrierStage GetBarrierStage(const PassAccessedResource* accessedResource)
    {
        if (accessedResource == nullptr) return {};

        switch (accessedResource->type)
        {
        case RenderGraphResourceAccessType::None:
            {
                TL_UNREACHABLE();
                return {};
            }
            break;
        case RenderGraphResourceAccessType::Image:
            {
                auto usage  = accessedResource->asImage.usage;
                auto stage  = accessedResource->asImage.stage;
                auto access = accessedResource->asImage.access;
                return {
                    .stageMask        = ConvertPipelineStageFlags(stage),
                    .accessMask       = GetAccessFlags2(usage, access),
                    // TODO: might have stencil!!
                    .layout           = GetImageLayout(usage, access, ImageAspect::Color),
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        case RenderGraphResourceAccessType::Buffer:
            {
                auto usage  = accessedResource->asBuffer.usage;
                auto stage  = accessedResource->asBuffer.stage;
                auto access = accessedResource->asBuffer.access;
                return {
                    .stageMask        = ConvertPipelineStageFlags(stage),
                    .accessMask       = GetAccessFlags2(usage, access),
                    .layout           = VK_IMAGE_LAYOUT_UNDEFINED,
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        case RenderGraphResourceAccessType::RenderTarget:
            {
                auto           format = GetFormatInfo(accessedResource->asImage.image->GetFormat());
                // TODO: optimize layout based on load/store operations
                VkImageLayout  layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                VkAccessFlags2 access = VK_ACCESS_2_NONE;
                if (accessedResource->asRenderTarget.loadOperation == LoadOperation::Load)
                    access |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
                if (accessedResource->asRenderTarget.storeOperation == StoreOperation::Store)
                    access |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                if (format.hasDepth && format.hasStencil)
                {
                    layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                    if (accessedResource->asRenderTarget.loadOperation == LoadOperation::Load)
                        access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                    if (accessedResource->asRenderTarget.storeOperation == StoreOperation::Store)
                        access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                }
                else if (format.hasDepth)
                {
                    layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                    if (accessedResource->asRenderTarget.loadOperation == LoadOperation::Load)
                        access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                    if (accessedResource->asRenderTarget.storeOperation == StoreOperation::Store)
                        access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                }
                else if (format.hasStencil)
                {
                    layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                    if (accessedResource->asRenderTarget.stencilLoadOperation == LoadOperation::Load)
                        access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                    if (accessedResource->asRenderTarget.stencilStoreOperation == StoreOperation::Store)
                        access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                }
                return {
                    .stageMask        = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    .accessMask       = access,
                    .layout           = layout,
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        case RenderGraphResourceAccessType::Resolve:
            {
                TL_UNREACHABLE();
            }
            break;
        case RenderGraphResourceAccessType::SwapchainPresent:
            {
                // TODO: Assumes swapchain is created with color usage only!
                return {
                    .stageMask        = ConvertPipelineStageFlags(PipelineStage::BottomOfPipe),
                    .accessMask       = GetAccessFlags2(ImageUsage::Color, Access::ReadWrite),
                    .layout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        default:
            TL_UNREACHABLE();
            break;
        }
    }

    IRenderGraph::IRenderGraph()  = default;
    IRenderGraph::~IRenderGraph() = default;

    ResultCode IRenderGraph::Init([[maybe_unused]] IDevice* device)
    {
        this->m_device = device;
        return ResultCode::Success;
    }

    void IRenderGraph::Shutdown()
    {
    }

    void IRenderGraph::OnBeginPassExecute(Pass& pass, CommandList& _commandList)
    {
        auto  device      = (IDevice*)m_device;
        auto& commandList = (ICommandList&)_commandList;

        commandList.Begin();

        TL::Vector<VkImageMemoryBarrier2>  imageBarriers;
        TL::Vector<VkBufferMemoryBarrier2> bufferBarriers;

        for (const auto& accessedResource : pass.GetAccessedResources())
        {
            auto previousAccess = accessedResource->prev;

            auto [srcStageMask, srcAccessMask, srcLayout, srcQfi] = GetBarrierStage(previousAccess);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQfi] = GetBarrierStage(accessedResource);

            // TODO: Optimize this to only use overlapped resources

            bool isImage = accessedResource->type != RenderGraphResourceAccessType::Buffer;

            if (isImage)
            {
                auto graphImage = accessedResource->asImage.image;
                auto image      = device->m_imageOwner.Get(graphImage->GetImage());
                auto aspects = ConvertImageAspect(GetFormatAspects(graphImage->GetFormat()));
                imageBarriers.push_back({
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .pNext               = nullptr,
                    .srcStageMask        = srcStageMask,
                    .srcAccessMask       = srcAccessMask,
                    .dstStageMask        = dstStageMask,
                    .dstAccessMask       = dstAccessMask,
                    .oldLayout           = srcLayout,
                    .newLayout           = dstLayout,
                    .srcQueueFamilyIndex = srcQfi,
                    .dstQueueFamilyIndex = dstQfi,
                    .image               = image->handle,
                    .subresourceRange    = {
                           .aspectMask     = aspects,
                           .baseMipLevel   = 0,
                           .levelCount     = VK_REMAINING_MIP_LEVELS,
                           .baseArrayLayer = 0,
                           .layerCount     = VK_REMAINING_ARRAY_LAYERS,
                    },
                });
            }
            else
            {
                auto graphBuffer = accessedResource->asBuffer.buffer;
                auto buffer      = device->m_bufferOwner.Get(graphBuffer->GetBuffer());
                bufferBarriers.push_back({
                    .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                    .pNext               = nullptr,
                    .srcStageMask        = srcStageMask,
                    .srcAccessMask       = srcAccessMask,
                    .dstStageMask        = dstStageMask,
                    .dstAccessMask       = dstAccessMask,
                    .srcQueueFamilyIndex = srcQfi,
                    .dstQueueFamilyIndex = dstQfi,
                    .buffer              = buffer->handle,
                    .offset              = 0,
                    .size                = VK_WHOLE_SIZE,
                });
            }
        }

        commandList.AddPipelineBarriers({
            .imageBarriers  = imageBarriers,
            .bufferBarriers = bufferBarriers,
        });
        commandList.BeginPass(pass);
    }

    void IRenderGraph::OnEndPassExecute([[maybe_unused]] Pass& pass, CommandList& _commandList)
    {
        auto  device      = (IDevice*)m_device;
        auto& commandList = (ICommandList&)_commandList;

        TL::Vector<VkImageMemoryBarrier2>  imageBarriers;
        TL::Vector<VkBufferMemoryBarrier2> bufferBarriers;

        auto                  swapchain            = m_graphImportedSwapchainsLookup.begin();
        RenderGraphImage*     swapchainImage       = nullptr;
        PassAccessedResource* swapchainImageAccess = nullptr;
        for (const auto& accessedResource : pass.GetAccessedResources())
        {
            if (accessedResource->type == RenderGraphResourceAccessType::Image)
            {
                if (swapchain->second == accessedResource->asImage.image)
                {
                    swapchainImage       = accessedResource->asImage.image;
                    swapchainImageAccess = accessedResource;
                }
            }
            else if (accessedResource->type == RenderGraphResourceAccessType::RenderTarget)
            {
                if (swapchain->second == accessedResource->asImage.image)
                {
                    swapchainImage       = accessedResource->asRenderTarget.attachment;
                    swapchainImageAccess = accessedResource;
                }
            }
        }
        if (swapchainImage)
        {
            auto [srcStageMask, srcAccessMask, srcLayout, srcQfi] = GetBarrierStage(swapchainImageAccess);
            auto image                                            = device->m_imageOwner.Get(swapchain->second->GetImage());
            imageBarriers.push_back({
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = srcStageMask,
                .srcAccessMask       = srcAccessMask,
                .dstStageMask        = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                .dstAccessMask       = VK_ACCESS_2_NONE,
                .oldLayout           = srcLayout,
                .newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = srcQfi,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = image->handle,
                .subresourceRange    = {
                       .aspectMask     = ConvertImageAspect(GetFormatAspects(swapchainImage->GetFormat())),
                       .baseMipLevel   = 0,
                       .levelCount     = VK_REMAINING_MIP_LEVELS,
                       .baseArrayLayer = 0,
                       .layerCount     = VK_REMAINING_ARRAY_LAYERS,
                },
            });
        }

        commandList.EndPass();
        commandList.AddPipelineBarriers({
            .imageBarriers  = imageBarriers,
            .bufferBarriers = bufferBarriers,
        });
        commandList.End();
    }

} // namespace RHI::Vulkan