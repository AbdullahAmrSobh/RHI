#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/RenderPass.hpp"
#include "RHI/RenderPassExecuter.hpp"

#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/RenderingLayout.hpp"
#include "RHI/Backend/Vulkan/Semaphore.hpp"
#include <RHI/Backend/Vulkan/DescriptorSet.hpp>
#include <RHI/Backend/Vulkan/FrameBuffer.hpp>

namespace RHI
{
namespace Vulkan
{

    class RenderPass final : public IRenderPass
    {
    public:
        struct UsedResource
        {
            UsedResource() = default;

            UsedResource(EAttachmentUsage usage, EAttachmentAccess access)
                : usage(usage)
                , access(access)
            {
            }

            EAttachmentUsage     usage                  = EAttachmentUsage::Undefined;
            EAttachmentAccess    access                 = EAttachmentAccess::Undefined;
            VkPipelineStageFlags accessStage            = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            Unique<Semaphore>    writeFinishedSemaphore = nullptr;
			bool 				 isFirstUse             = false;
        };

        struct UsedSwapChainResource : UsedResource
        {
            UsedSwapChainResource() = default;
            
            SwapChainAttachmentId id;
        };

        struct UsedImageResource : UsedResource
        {
            UsedImageResource() = default;
            
			EPixelFormat format;
            std::vector<ImageAttachmentId> id;
        };

        struct UsedBufferResource : UsedResource
        {
            UsedBufferResource() = default;

            std::vector<BufferAttachmentId> id;
        };

        virtual RenderTargetLayout GetRenderTargetLayout() const override;

        virtual void Invalidate() override;
        
        virtual void UseSwapChainAttachment(SwapChainAttachmentId id, EAttachmentUsage usage, EAttachmentAccess access) override;
        
        virtual void UseImageAttachments(Span<ImageAttachmentId> ids, EAttachmentUsage usage, EAttachmentAccess access) override;
        
        virtual void UseBufferAttachments(Span<BufferAttachmentId> ids, EAttachmentUsage usage, EAttachmentAccess access) override;
        
        struct SubmitInfo CompileSubmitInfo();
		
		struct PresentInfo CompilePresentInfo();

		void BuildResourceBarriers();
    
    private:
        inline void Setup()
        {
            m_pExecuter->Setup(*this);
        }

        inline void BuildCommandLists()
        {
            m_pExecuter->Execute(this->m_executeContext);
        }

    private:
        Unique<RenderingLayout> m_renderingLayout;
        Unique<FrameBuffer>     m_frameBuffer;
        Unique<DescriptorSet>   m_descriptorSet;
        ExecuteContext          m_executeContext;
        
        Unique<Semaphore> m_renderPassFinishedSemaphore;

        bool m_isDepthStencilEnabled = false;
        bool m_isSwapChainWriter     = false;

        UsedImageResource               m_usedDepthStencilAttachment;
        UsedSwapChainResource           m_usedSwapChainAttachment;
        std::vector<UsedImageResource>  m_usedImageResources;
        std::vector<UsedBufferResource> m_usedBufferResources;

    };

} // namespace Vulkan
} // namespace RHI
