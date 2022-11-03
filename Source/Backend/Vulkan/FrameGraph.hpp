#pragma once
#include "RHI/FrameGraph.hpp"

#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

    class AttachmentsRegistry final : public IAttachmentsRegistry
    {
    public:
        virtual ImageAttachmentReference ImportSwapchain(std::string name, ISwapchain& swapchain) override;
        
        virtual ImageAttachmentReference ImportImageResource(std::string name, Unique<IImage>& image) override;
        
        virtual BufferAttachmentReference ImportBufferResource(std::string name, Unique<IBuffer>& buffer) override;
        
        virtual ImageAttachmentReference CreateTransientImageAttachment(const ImageFrameAttachmentDesc& attachmentDesc) override;

        virtual BufferAttachmentReference CreateTransientBufferAttachment(const BufferFrameAttachmentDesc& attachmentDesc) override;
        
        virtual void Compile(IFrameGraph& frameGraph) override;

    private:
        Device* m_pDevice;
    };

    class FrameGraph final : public IFrameGraph
    {
        friend class IPassCallbacks; 
    public:
        FrameGraph(const Device& device);
        ~FrameGraph();
        
        VkResult Init();
    
    protected:
        virtual Unique<IPass> CreatePass(std::string name, EPassType passType) = 0;
    
    private:
        const Device* m_pDevice;
    };

} // namespace Vulkan
} // namespace RHI