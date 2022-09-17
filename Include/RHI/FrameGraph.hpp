#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "RHI/Commands.hpp"
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{
    enum class EHardwareQueueType
    {
        Graphics,
        Compute,
        Transfer    
    };  
    
    using AttachmentID = std::string_view; 
    
    class IPass
    {
    public:
        class Callbacks; 
        
        const RenderTargetLayout& GetRenderTargetLayout() const;
        
        void SetCallbacks(Callbacks& callbacks);
    
        virtual EResultCode Submit() = 0;
    };
    
    class IFrameGraph
    {
    public:
        class Builder; 
        
        Unique<IPass> AddRenderPass(std::string_view name, EHardwareQueueType queueType); 

        void BeginFrame();
        void EndFrame();
        
        virtual void BeginFrameInternal() {}
        virtual void EndFrameInternal() {}
    };
    
    class AttachmentsContext
    {
    public:
        IImageView* GetImageView(AttachmentID id);
    
        IBufferView* GetBufferView(AttachmentID id);
    
        IImage* GetImageResource(AttachmentID id);
    
        IBuffer* GetBufferResource(AttachmentID id);
        
        EResultCode CreateTransientImageAttachment(AttachmentID id, const ImageDesc& desc);
        
        EResultCode CreateTransientBufferAttachment(AttachmentID id, const BufferDesc& desc);
        
        EResultCode ImportSwapchain(AttachmentID id, const ImageViewDesc& viewDesc, ISwapchain& swapchain);
        
        EResultCode ImportImageResource(AttachmentID id, Unique<IImage> image);
        
        EResultCode ImportBufferResource(AttachmentID id, Unique<IBuffer> buffer);
    };
    
    class IFrameGraph::Builder
    {
    public:
        void UseOutputAttachment(std::string_view name, ImageViewDesc& view);
        
        void UseDepthStencilAttachment(std::string_view name, ImageViewDesc& view);
        
        void UseShaderResource(std::string_view name, ImageViewDesc& view, EAccess access);
        
        void UseShaderResource(std::string_view name, BufferViewDesc& view, EAccess access);
        
        void UseShaderResource(std::string_view name, BufferViewRange& range, EAccess access);
        
        void AddSignalFence(Unique<IFence> fence);
        
        void WaitPass(std::string_view passName);
    };
    
    class IPass::Callbacks
    {
    public:
        virtual void SetupRenderPass(IFrameGraph::Builder& builder) = 0;
        
        virtual void BindShaderAttachments(AttachmentsContext& context) = 0;
        
        virtual void BuildCommandList(ICommandBuffer& commandBuffer) = 0;
    };

} // namespace RHI