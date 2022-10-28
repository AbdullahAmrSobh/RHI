#pragma once
#include "RHI/FrameGraphPass.hpp"

#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/FrameGraphPass.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{

namespace Vulkan
{

    class CommandAllocator;
    class CommandBuffer;
    class Pass;

    class RenderPass final : public DeviceObject<VkRenderPass>
    {
    public:
        static Result<Unique<RenderPass>> Create(const Device& device, const Pass& pass);

        RenderPass(const Device& device)
            : DeviceObject(&device)
        {
        }

        ~RenderPass();

        VkResult Init(const Pass& pass);
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

        Framebuffer(const Device& device)
            : DeviceObject(&device)
        {
        }
        ~Framebuffer();

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
        
        virtual EResultCode Compile() override;

        virtual EResultCode Submit() override;

        inline std::optional<std::pair<const RenderPass&, uint32_t>> GetRenderPass() const
        {
            if (m_queueType == EHardwareQueueType::Graphics)
            {
                return {};
            }

            return std::make_pair(*m_renderPass, 0);
        }

    private:
        const Device* m_pDevice;

        Unique<RenderPass> m_renderPass;

        Unique<Framebuffer> m_framebuffer;

        Unique<CommandAllocator> m_commandAllocator;

        std::vector<Unique<CommandBuffer>> m_commandBuffers;

        uint32_t m_currentBackbufferIndex;

        std::vector<Semaphore*> m_pWaitSemaphores;

        Unique<Semaphore> m_signalSemaphore;
    };

} // namespace Vulkan
} // namespace RHI