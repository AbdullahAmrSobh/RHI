#pragma once
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{

struct RenderTargetLayout;

namespace Vulkan
{

    class CommandAllocator;
    class CommandBuffer;

    class RenderPass final : public DeviceObject<VkRenderPass>
    {
    public:
        static Result<Unique<RenderPass>> Create(const Device& device, const RenderTargetLayout& renderTargetLayout);
        
        RenderPass(const Device& device)
            : DeviceObject(&device)
        {
        }
        
        ~RenderPass();
        
        VkResult Init(const Pass& pass);
    };
    
    class RenderPassManager
    {
    public:
        struct SubpassDesc
        {
            VkRenderPass renderPass;
            uint32_t     subpassIndex;
        };

        SubpassDesc& GetPass(const RenderTargetLayout& renderTargetLayout) const;

    private:
        mutable std::unordered_map<size_t, Unique<RenderPass>> m_renderPasses;
    };
    
    class Framebuffer final : public DeviceObject<VkFramebuffer>
    {
    public:
        struct AttachmentsDesc
        {
            uint32_t         colorAttachmentsCount;
            const ImageView* pColorAttachments;
            const ImageView* pDepthStencilAttachment;
        };
        
        static Result<Unique<Framebuffer>> Create(Device& device, VkExtent2D extent, const AttachmentsDesc& attachments, const RenderPass& renderPass);
        
        VkResult Init(VkExtent2D extent, const AttachmentsDesc& attachmentsDesc, const RenderPass& renderPass);
        
        inline const RenderPass& GetRenderPass() const
        {
            return *m_pRenderPass;
        }
    
    private:
        const RenderPass* m_pRenderPass;
    };
    
    class Pass final : public IPass
    {
    public:
        Pass();
        ~Pass();

        VkResult Init();
        
        virtual EResultCode Submit() override;
    
    private:
        const Device*                        m_pDevice;
        Unique<RenderPass>                   m_pRenderPass;
        Unique<Framebuffer>                  m_framebuffer;
        Unique<CommandAllocator>             m_commandAllocator;

        std::vector<Unique<CommandBuffer>>   m_commandBuffers; 
        
        uint32_t m_currentBackbufferIndex;

        std::vector<Semaphore*> m_pWaitSemaphores;
        Unique<Semaphore> m_signalSemaphore;
    };

} // namespace Vulkan
} // namespace RHI