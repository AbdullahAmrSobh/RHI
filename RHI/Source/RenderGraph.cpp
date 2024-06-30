#include "RHI/RenderGraph.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/RGInternals.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/Attachment.hpp"

#include <graaflib/algorithm/cycle_detection/dfs_cycle_detection.h>
#include <graaflib/algorithm/topological_sorting/dfs_topological_sorting.h>
#include <graaflib/algorithm/shortest_path/bfs_shortest_path.h>
#include <graaflib/algorithm/graph_traversal/breadth_first_search.h>
#include <graaflib/algorithm/utils.h>

namespace RHI
{
    RenderGraph::RenderGraph(Context* context)
        : m_context(context)
        , m_graph()
        , m_passToVertexLut()
        , m_vertexToPassLut()
        , m_state()
        , m_frameCounter(0ull)
        , m_passPool()
        , m_imageAttachmentPool()
        , m_bufferAttachmentPool()
        , m_passes()
        , m_importedSwapchainImageAttachments()
        , m_importedImageAttachments()
        , m_importedBufferAttachments()
        , m_transientImageAttachments()
        , m_transientBufferAttachments()
        , m_imageAttachments()
        , m_bufferAttachments()
        , m_transientAliasingAllocator(CreatePtr<TransientAliasingAllocator>(this))
        , m_frameContext(CreatePtr<FrameContext>())
    {
    }

    RenderGraph::~RenderGraph() = default;

    Handle<Pass> RenderGraph::CreatePass(const PassCreateInfo& createInfo)
    {
        Pass pass{};
        pass.name = createInfo.name;
        pass.flags = createInfo.flags;
        auto handle = m_passPool.Emplace(std::move(pass));
        m_passes.push_back(handle);
        return handle;
    }

    void RenderGraph::PassResize(Handle<Pass> pass, ImageSize2D size)
    {
        m_passPool[pass]->renderingAttachments.size = size;
    }

    RenderTargetLayoutDesc RenderGraph::PassGetRenderTargetLayout(Handle<Pass> pass) const
    {
        return m_passPool[pass]->renderingAttachments.layout;
    }

    Handle<ImageAttachment> RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain)
    {
        auto imageID = m_frameContext->AddSwapchain(swapchain);
        auto handle = m_imageAttachmentPool.Emplace(ImageAttachment(name, imageID, swapchain));
        m_importedSwapchainImageAttachments.push_back(handle);
        return handle;
    }

    Handle<ImageAttachment> RenderGraph::ImportImage(const char* name, Handle<Image> image)
    {
        auto imageID = m_frameContext->AddImage(image);
        auto handle = m_imageAttachmentPool.Emplace(ImageAttachment(name, imageID));
        m_importedImageAttachments.push_back(handle);
        return handle;
    }

    Handle<BufferAttachment> RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        auto bufferID = m_frameContext->AddBuffer(buffer);
        auto handle = m_bufferAttachmentPool.Emplace(BufferAttachment(name, bufferID));
        m_importedBufferAttachments.push_back(handle);
        return handle;
    }

    Handle<ImageAttachment> RenderGraph::CreateImage(const ImageCreateInfo& createInfo)
    {
        auto handle = m_imageAttachmentPool.Emplace(ImageAttachment(createInfo.name, createInfo));
        m_transientImageAttachments.push_back(handle);
        return handle;
    }

    Handle<BufferAttachment> RenderGraph::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto handle = m_bufferAttachmentPool.Emplace(BufferAttachment(createInfo.name, createInfo));
        m_transientBufferAttachments.push_back(handle);
        return handle;
    }

    void RenderGraph::PassUseImage(Handle<Pass> pass, Handle<ImageAttachment> attachment, ImageUsage usage, Flags<ShaderStage> stage, Access access)
    {
        PassUseImage(pass, attachment, ImageViewInfo(), usage, stage, access);
    }

    void RenderGraph::PassUseImage(Handle<Pass> pass, Handle<ImageAttachment> attachmentHandle, const ImageViewInfo& viewInfo, ImageUsage usage, Flags<ShaderStage> stage, Access access)
    {
        AddDependency(pass, attachmentHandle);
        auto attachment = m_imageAttachmentPool[attachmentHandle];
        auto passAttachment = attachment->Emplace(pass, attachmentHandle, viewInfo, usage, access, stage);
        m_passPool[pass]->imagesAttachments.push_back(passAttachment);
    }

    void RenderGraph::PassUseBuffer(Handle<Pass> pass, Handle<BufferAttachment> attachment, BufferUsage usage, Flags<ShaderStage> stage, Access access)
    {
        PassUseBuffer(pass, attachment, BufferViewInfo(), usage, stage, access);
    }

    void RenderGraph::PassUseBuffer(Handle<Pass> pass, Handle<BufferAttachment> attachmentHandle, const BufferViewInfo& viewInfo, BufferUsage usage, Flags<ShaderStage> stage, Access access)
    {
        AddDependency(pass, attachmentHandle);
        auto attachment = m_bufferAttachmentPool[attachmentHandle];
        auto passAttachment = attachment->Emplace(pass, attachmentHandle, viewInfo, usage, access, stage);
        m_passPool[pass]->bufferAttachments.push_back(passAttachment);
    }

    Handle<Image> RenderGraph::PassGetImage(Handle<ImageAttachment> attachmentHandle) const
    {
        auto attachment = m_imageAttachmentPool[attachmentHandle];
        return m_frameContext->GetImage(attachment->m_resource);
    }

    Handle<Buffer> RenderGraph::PassGetBuffer(Handle<BufferAttachment> attachmentHandle) const
    {
        auto attachment = m_bufferAttachmentPool[attachmentHandle];
        return m_frameContext->GetBuffer(attachment->m_resource);
    }

    Handle<ImageView> RenderGraph::PassGetImageView(Handle<Pass> pass, Handle<ImageAttachment> attachment) const
    {
        auto id = *m_imageAttachmentPool[attachment]->m_passToViews.at(pass);
        return m_frameContext->GetImageView(id->view);
    }

    Handle<BufferView> RenderGraph::PassGetBufferView(Handle<Pass> pass, Handle<BufferAttachment> attachment) const
    {
        auto id = *m_bufferAttachmentPool[attachment]->m_passToViews.at(pass);
        return m_frameContext->GetBufferView(id->view);
    }

    // void  RenderGraph::Submit(Handle<Pass> pass, TL::Span<const CommandList*> commandList, Fence* signalFence)
    // {}

    // private:

    RenderGraph::State RenderGraph::GetGraphState() const
    {
        return m_state;
    }

    void RenderGraph::AddDependency(Handle<Pass> pass, Handle<ImageAttachment> attachment)
    {
        // if (m_imageAttachmentPool[attachment]->end() == )
            // return;
        auto srcPass = m_passToVertexLut[pass];
        auto dstPass = m_passToVertexLut[m_imageAttachmentPool[attachment]->end()->pass];
        m_graph.add_edge(srcPass, dstPass, PassAttachment());
    }

    void RenderGraph::AddDependency(Handle<Pass> pass, Handle<BufferAttachment> attachment)
    {
        // if (m_bufferAttachmentPool[attachment]->end() == nullptr)
            // return;
        auto srcPass = m_passToVertexLut[pass];
        auto dstPass = m_passToVertexLut[m_bufferAttachmentPool[attachment]->end()->pass];
        m_graph.add_edge(srcPass, dstPass, PassAttachment());
    }

} // namespace RHI