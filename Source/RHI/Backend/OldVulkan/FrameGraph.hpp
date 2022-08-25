#pragma once
#include "RHI/FrameGraph.hpp"
#include "RHI/RenderPass.hpp"

#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/RenderPass.hpp"
#include "RHI/Backend/Vulkan/SwapChain.hpp"

#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Image.hpp"

#include "RHI/Backend/Vulkan/DescriptorPool.hpp"
#include "RHI/Backend/Vulkan/DescriptorSet.hpp"
#include "RHI/Backend/Vulkan/DescriptorSetLayout.hpp"

#include "RHI/Backend/Vulkan/CommandList.hpp"

namespace RHI
{
namespace Vulkan
{
    class AttachmentResourceAllocator final : public IAttachmentResourceAllocator
    {
    public:
    };
    
    class BufferAttachmentResource final : public IBufferAttachmentResource
    {
    public:
    };
    
    class ImageAttachmentResource final : public IImageAttachmentResource
    {
    public:
    };
    
    class SwapChainAttachmentResource final : public ISwapChainAttachmentResource
    {
    public:
    };
    
    class FrameGraph final
        : public IFrameGraph
        , private DeviceObject<void>
    {
        virtual void CreateRenderPass(std::string name, ERenderPassQueueType queueType, RenderPassExecuter& executer) override;

        virtual BufferAttachmentId CreateBufferAttachment(const BufferAttachmentDesc& desc) override;

        virtual ImageAttachmentId CreateImageAttachment(const ImageAttachmentDesc& desc) override;

        virtual BufferAttachmentId ImportBufferAsAttachment(const BufferAttachmentDesc& desc, IBuffer& buffer) override;

        virtual ImageAttachmentId ImportImageAsAttachment(const ImageAttachmentDesc& desc, IImage& image) override;

        virtual ImageAttachmentId ImportSwapchainAsAttachment(const ImageAttachmentDesc& desc, const ISwapChain& swapchain) override;

    private:
        Unique<DescriptorPool>                        m_descriptorPool;
        Unique<DescriptorSetLayout>                   m_descriptorSetLayout;
        Unique<CommandPool>                           m_commandPool;
        std::vector<Unique<RenderPass>>               m_renderPasses;
        std::unordered_map<RenderPassId, RenderPass*> m_renderPassLookup;
        
        std::vector<BufferAttachmentResource>    m_bufferAttachments;
        std::vector<ImageAttachmentResource>     m_imageAttachments;
        std::vector<SwapChainAttachmentResource> m_swapChainAttachments;

        std::unordered_map<BufferAttachmentId, BufferAttachmentResource*>       m_bufferAttachmentLookup;
        std::unordered_map<ImageAttachmentId, ImageAttachmentResource*>         m_imageAttachmentLookup;
        std::unordered_map<SwapChainAttachmentId, SwapChainAttachmentResource*> m_swapChainAttachmentLoopup;

    private:
        inline RenderPass* GetRenderPass(const RenderPassId& id)
        {
            auto it = m_renderPassLookup.find(id);
            return it == m_renderPassLookup.end() ? nullptr : it->second;
        }
        
        inline BufferAttachmentResource* GetBufferAttachment(const BufferAttachmentId& id)
        {
            auto it = m_bufferAttachmentLookup.find(id);
            return it == m_bufferAttachmentLookup.end() ? nullptr : it->second;
        }

        inline ImageAttachmentResource* GetImageAttachment(const ImageAttachmentId& id)
        {
            auto it = m_imageAttachmentLookup.find(id);
            return it == m_imageAttachmentLookup.end() ? nullptr : it->second;
        }
    };

} // namespace Vulkan
} // namespace RHI
