#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>

#include <cstdint>
#include <tracy/Tracy.hpp>

namespace RHI
{
    namespace Colors
    {
        constexpr uint32_t Red   = 0x7F000000;
        constexpr uint32_t Green = 0x007F7F00;
        constexpr uint32_t Blue  = 0x00007FFF;

        constexpr ClearValue DebugMarker_Graphics{
            .f32{0.7f, 0.2f, 0.2f, 1.0f}
        };

        constexpr ClearValue DebugMarker_Compute{
            .f32{0.2f, 0.7f, 0.2f, 1.0f}
        };

        constexpr ClearValue DebugMarker_Transfer{
            .f32{0.2f, 0.4f, 0.7f, 1.0f}
        };

    }; // namespace Colors

    class RenderGraph::TransientResourceAllocator
    {
    public:
        void Create(RGImage* rgImage)
        {
            TL_LOG_WARNNING("Recreating resource {}", rgImage->m_name);
        }

    private:
        TL::Map<size_t, Handle<Image>>  m_images;
        TL::Map<size_t, Handle<Buffer>> m_buffers;
    };

    class RenderGraph::DependencyLevel
    {
    public:
        DependencyLevel(uint32_t m_index = 0)
            : m_levelIndex(m_index)
        {
        }

        void AddPass(RGPass* pass) { m_passes.push_back(pass); }

        TL::Span<RGPass* const> GetPasses() const { return m_passes; }

        uint32_t m_levelIndex;

    private:
        TL::Vector<RGPass*> m_passes;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Pass Resource types
    ///////////////////////////////////////////////////////////////////////////

    RGImage::RGImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples)
        : m_name(name)
    {
    }

    RGImage::RGImage(const char* name, Handle<Image> handle, Format format)
        : m_name(name)
    {
    }

    RGBuffer::RGBuffer(const char* name, size_t size)
        : m_name(name)
    {
    }

    RGBuffer::RGBuffer(const char* name, Handle<Buffer> handle)
        : m_name(name)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Builder Interface
    ///////////////////////////////////////////////////////////////////////////

    RenderGraphBuilder::RenderGraphBuilder(RenderGraph* rg, RGPass* pass)
        : m_rg(rg)
        , m_pass(pass)
    {
    }

    void RenderGraphBuilder::ReadImage(Handle<RGImage> imageHandle, ImageUsage usage, PipelineStage stage)
    {
        ReadImage(imageHandle, ImageSubresourceRange::All(), usage, stage);
    }

    void RenderGraphBuilder::ReadImage(Handle<RGImage> imageHandle, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        TL_ASSERT(m_rg->CheckHandleIsValid(imageHandle));

        RGImageDependency dep{imageHandle, subresource, usage, stage, Access::Read};
        m_pass->m_imageDependencies.push_back(dep);
    }

    Handle<RGImage> RenderGraphBuilder::WriteImage(Handle<RGImage> imageHandle, ImageUsage usage, PipelineStage stage)
    {
        return WriteImage(imageHandle, ImageSubresourceRange::All(), usage, stage);
    }

    Handle<RGImage> RenderGraphBuilder::WriteImage(Handle<RGImage> imageHandle, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        TL_ASSERT(m_rg->CheckHandleIsValid(imageHandle));

        auto image     = m_rg->GetImage(imageHandle);
        image->m_valid = false; //

        RGImageDependency dep{imageHandle, subresource, usage, stage, Access::Write};
        m_pass->m_imageDependencies.push_back(dep);

        // Mark this pass depends on the last pass that wrote to this resource.
        // this means, when the graph is sorted, current pass will execute after the last
        // pass wrote to this resource.
        m_rg->AddDependency(image->m_pass, m_pass);

        auto newHandle         = m_rg->CreateImage(image->m_name.c_str(), ImageType::None, {}, Format::Unknown);
        auto newImage          = m_rg->GetImage(newHandle);
        newImage->m_prevHandle = imageHandle;
        image->m_nextHandle    = newHandle;
        return newHandle;
    }

    void RenderGraphBuilder::ReadBuffer(Handle<RGBuffer> bufferHandle, BufferUsage usage, PipelineStage stage)
    {
        ReadBuffer(bufferHandle, BufferSubregion{.offset = 0, .size = WholeSize}, usage, stage);
    }

    void RenderGraphBuilder::ReadBuffer(Handle<RGBuffer> bufferHandle, const BufferSubregion& subregion, BufferUsage usage, PipelineStage stage)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        TL_ASSERT(m_rg->CheckHandleIsValid(bufferHandle));

        RGBufferDependency dep{bufferHandle, subregion, usage, stage, Access::Read};
        m_pass->m_bufferDependencies.push_back(dep);
    }

    Handle<RGBuffer> RenderGraphBuilder::WriteBuffer(Handle<RGBuffer> bufferHandle, BufferUsage usage, PipelineStage stage)
    {
        return WriteBuffer(bufferHandle, BufferSubregion{.offset = 0, .size = WholeSize}, usage, stage);
    }

    Handle<RGBuffer> RenderGraphBuilder::WriteBuffer(Handle<RGBuffer> bufferHandle, const BufferSubregion& subregion, BufferUsage usage, PipelineStage stage)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        TL_ASSERT(m_rg->CheckHandleIsValid(bufferHandle));

        RGBufferDependency dep{bufferHandle, subregion, usage, stage, Access::Write};
        m_pass->m_bufferDependencies.push_back(dep);

        auto buffer     = m_rg->GetBuffer(bufferHandle);
        buffer->m_valid = false;

        // Mark this pass depends on the last pass that wrote to this resource.
        // this means, when the graph is sorted, current pass will execute after the last
        // pass wrote to this resource.
        m_rg->AddDependency(buffer->m_pass, m_pass);

        auto newHandle         = m_rg->CreateBuffer(buffer->m_name.c_str(), 0);
        auto newImage          = m_rg->GetBuffer(newHandle);
        newImage->m_prevHandle = bufferHandle;
        buffer->m_nextHandle   = newHandle;
        return newHandle;
    }

    Handle<RGImage> RenderGraphBuilder::AddColorAttachment(RGColorAttachment attachment)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        TL_ASSERT(m_rg->CheckHandleIsValid(attachment.color));
        if (attachment.resolveView) TL_ASSERT(m_rg->CheckHandleIsValid(attachment.resolveView));

        m_pass->m_gfxPassInfo.m_colorAttachments.push_back(attachment);

        auto color = WriteImage(attachment.color, attachment.colorRange, ImageUsage::Color, PipelineStage::ColorAttachmentOutput);
        if (attachment.resolveView)
        {
            return WriteImage(attachment.resolveView, attachment.resolveRange, ImageUsage::Resolve, PipelineStage::Resolve);
        }
        return color;
    }

    Handle<RGImage> RenderGraphBuilder::SetDepthStencil(RGDepthStencilAttachment attachment)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        TL_ASSERT(m_rg->CheckHandleIsValid(attachment.depthStencil));

        // TODO: fix this function

        m_pass->m_gfxPassInfo.m_depthStencilAttachment = attachment;
        return WriteImage(attachment.depthStencil, attachment.depthStencilRange, ImageUsage::DepthStencil, PipelineStage::LateFragmentTests);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Context Interface
    ///////////////////////////////////////////////////////////////////////////

    RenderGraphContext::RenderGraphContext(RenderGraph* rg, RGPass* pass)
        : m_rg(rg)
        , m_pass(pass)
    {
    }

    Handle<Image> RenderGraphContext::GetImage(Handle<RGImage> handle) const
    {
        return m_rg->GetImageHandle(handle);
    }

    Handle<Buffer> RenderGraphContext::GetBuffer(Handle<RGBuffer> handle) const
    {
        return m_rg->GetBufferHandle(handle);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Pass
    ///////////////////////////////////////////////////////////////////////////

    RGPass::RGPass() = default;

    RGPass::~RGPass()
    {
        Shutdown();
    }

    ResultCode RGPass::Init(RenderGraph* rg, const PassCreateInfo& ci)
    {
        m_renderGraph   = rg;
        m_name          = ci.name;
        m_type          = ci.type;
        m_setupCallback = ci.setupCallback;

        if (ci.compileCallback)
        {
            m_compileCallback     = ci.compileCallback;
            m_state.shouldCompile = true;
        }
        m_executeCallback = ci.executeCallback;
        // m_globalExecutionIndex
        // m_dependencyLevelIndex
        // m_localToDependencyLevelExecutionIndex
        // m_localToQueueExecutionIndex
        // m_indexInUnorderedList
        // m_producers
        // m_imageWrites
        // m_imageReads
        // m_bufferWrites
        // m_bufferReads
        return ResultCode::Success;
    }

    void RGPass::Shutdown()
    {
    }

    void RGPass::Setup(RenderGraphBuilder& builder)
    {
        ZoneScopedN("RGPass::Setup");
        TL_ASSERT(m_setupCallback);
        m_setupCallback(builder);
    }

    void RGPass::Compile(RenderGraphContext& context)
    {
        ZoneScopedN("RGPass::Compile");
        TL_ASSERT(m_compileCallback);
        m_compileCallback(context);
    }

    void RGPass::Execute(CommandList& commandList)
    {
        ZoneScopedN("RGPass::Execute");
        TL_ASSERT(m_executeCallback);
        m_executeCallback(commandList);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    RenderGraph::RenderGraph()  = default;
    RenderGraph::~RenderGraph() = default;

    ResultCode RenderGraph::Init(Device* device, const RenderGraphCreateInfo& ci)
    {
        m_device        = device;
        m_allocator     = (TL::IAllocator*)ci.allocator;
        m_tempAllocator = new TL::Arena();
        return ResultCode::Success;
    }

    void RenderGraph::Shutdown()
    {
        delete m_allocator;
        delete m_tempAllocator;
    }

    void RenderGraph::BeginFrame(ImageSize2D frameSize)
    {
        ZoneScopedC(Colors::Red);

        m_state.frameRecording = true;
        m_frameSize            = frameSize;
    }

    void RenderGraph::EndFrame()
    {
        ZoneScopedC(Colors::Red);

        TL_ASSERT(m_state.frameRecording == true);
        m_state.frameRecording = false;

        Compile();
        Execute();

        {
            // Print RenderGraph state: dependency levels, passes, inputs, outputs
            TL_LOG_INFO("=== RenderGraph State ===");
            for (size_t levelIdx = 0; levelIdx < m_dependencyLevels.size(); ++levelIdx)
            {
            const auto& level = m_dependencyLevels[levelIdx];
            TL_LOG_INFO("Dependency Level {}:", levelIdx);
            for (auto pass : level.GetPasses())
            {
                TL_LOG_INFO("  Pass: {}", pass->GetName());

                // Print previous producers
                if (!pass->m_producers.empty())
                {
                TL_LOG_INFO("    Previous Producers:");
                for (auto producer : pass->m_producers)
                {
                    TL_LOG_INFO("      - {}", producer ? producer->GetName() : "null");
                }
                }

                // Print input images
                if (!pass->m_imageDependencies.empty())
                {
                TL_LOG_INFO("    Image Inputs:");
                for (const auto& dep : pass->m_imageDependencies)
                {
                    if (dep.access == Access::Read)
                    {
                    auto img = GetImage(dep.image);
                    TL_LOG_INFO("      - {} (usage: {}, stage: {})", img ? img->m_name.c_str() : "null", (uint32_t)dep.usage, (uint32_t)dep.stage);
                    }
                }
                }
                // Print output images
                if (!pass->m_imageDependencies.empty())
                {
                TL_LOG_INFO("    Image Outputs:");
                for (const auto& dep : pass->m_imageDependencies)
                {
                    if (dep.access == Access::Write)
                    {
                    auto img = GetImage(dep.image);
                    TL_LOG_INFO("      - {} (usage: {}, stage: {})", img ? img->m_name.c_str() : "null", (uint32_t)dep.usage, (uint32_t)dep.stage);
                    }
                }
                }
                // Print input buffers
                if (!pass->m_bufferDependencies.empty())
                {
                TL_LOG_INFO("    Buffer Inputs:");
                for (const auto& dep : pass->m_bufferDependencies)
                {
                    if (dep.access == Access::Read)
                    {
                    auto buf = GetBuffer(dep.buffer);
                    TL_LOG_INFO("      - {} (usage: {}, stage: {})", buf ? buf->m_name.c_str() : "null", (uint32_t)dep.usage, (uint32_t)dep.stage);
                    }
                }
                }
                // Print output buffers
                if (!pass->m_bufferDependencies.empty())
                {
                TL_LOG_INFO("    Buffer Outputs:");
                for (const auto& dep : pass->m_bufferDependencies)
                {
                    if (dep.access == Access::Write)
                    {
                    auto buf = GetBuffer(dep.buffer);
                    TL_LOG_INFO("      - {} (usage: {}, stage: {})", buf ? buf->m_name.c_str() : "null", (uint32_t)dep.usage, (uint32_t)dep.stage);
                    }
                }
                }
            }
            }
            TL_LOG_INFO("=========================");
        }

        {
            m_passPool.clear();
            m_imagePool.Clear();
            m_bufferPool.Clear();
            m_dependencyLevels.clear();
            m_dependencyLevels.clear();

            // Finally collect temp arena allocations
            m_tempAllocator->Collect();
        }

        m_frameIndex++;
    }

    Handle<RGImage> RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain, Format format)
    {
        TL_ASSERT(m_state.frameRecording == true);
        TL_ASSERT(IsNameValid(name));

        m_swapchain = &swapchain;
        auto handle = m_imagePool.Emplace(RGImage(name, swapchain.GetImage(), format));
        return handle;
    }

    Handle<RGImage> RenderGraph::ImportImage(const char* name, Handle<Image> image, Format format)
    {
        TL_ASSERT(m_state.frameRecording == true);
        TL_ASSERT(IsNameValid(name));

        auto handle = m_imagePool.Emplace(RGImage(name, image, format));
        return handle;
    }

    Handle<RGBuffer> RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        TL_ASSERT(m_state.frameRecording == true);
        TL_ASSERT(IsNameValid(name));

        auto handle = m_bufferPool.Emplace(RGBuffer(name, buffer));
        return handle;
    }

    Handle<RGImage> RenderGraph::CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples)
    {
        TL_ASSERT(m_state.frameRecording == true);
        TL_ASSERT(IsNameValid(name));

        auto handle = m_imagePool.Emplace(RGImage(name, type, size, format, mipLevels, arrayCount, samples));
        return handle;
    }

    Handle<RGImage> RenderGraph::CreateRenderTarget(const char* name, ImageSize2D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples)
    {
        TL_ASSERT(m_state.frameRecording == true);
        return CreateImage(name, ImageType::Image2D, {size.width, size.height}, format, mipLevels, arrayCount, samples);
    }

    Handle<RGBuffer> RenderGraph::CreateBuffer(const char* name, size_t size)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto handle = m_bufferPool.Emplace(RGBuffer(name, size));
        return handle;
    }

    RGPass* RenderGraph::AddPass(const PassCreateInfo& createInfo)
    {
        ZoneScoped;
        TL_ASSERT(m_state.frameRecording == true);
        uint32_t   indexInUnorderedList = m_passPool.size();
        RGPass*    pass                 = m_passPool.emplace_back(TL::CreatePtr<RGPass>()).get();
        ResultCode result               = pass->Init(this, createInfo);
        TL_ASSERT(IsSuccess(result));
        auto builder = RenderGraphBuilder(this, pass);
        pass->Setup(builder);
        pass->m_indexInUnorderedList = indexInUnorderedList;
        return pass;
    }

    void RenderGraph::QueueBufferRead(TL_MAYBE_UNUSED Handle<RGBuffer> buffer, TL_MAYBE_UNUSED uint32_t offset, TL_MAYBE_UNUSED TL::Block data)
    {
        TL_ASSERT(m_state.frameRecording == true);
        TL_UNREACHABLE_MSG("TODO: Implement");
    }

    Handle<RGBuffer> RenderGraph::QueueBufferWrite(TL_MAYBE_UNUSED Handle<RGBuffer> buffer, TL_MAYBE_UNUSED uint32_t offset, TL_MAYBE_UNUSED TL::Block data)
    {
        TL_ASSERT(m_state.frameRecording == true);
        TL_UNREACHABLE_MSG("TODO: Implement");
    }

    void RenderGraph::QueueImageRead(TL_MAYBE_UNUSED Handle<RGImage> image, TL_MAYBE_UNUSED ImageOffset3D offset, TL_MAYBE_UNUSED ImageSize3D size, TL_MAYBE_UNUSED ImageSubresourceLayers dstLayers, TL_MAYBE_UNUSED TL::Block block)
    {
        TL_ASSERT(m_state.frameRecording == true);
        TL_UNREACHABLE_MSG("TODO: Implement");
    }

    Handle<RGImage> RenderGraph::QueueImageWrite(TL_MAYBE_UNUSED Handle<RGImage> image, TL_MAYBE_UNUSED ImageOffset3D offset, TL_MAYBE_UNUSED ImageSize3D size, TL_MAYBE_UNUSED ImageSubresourceLayers dstLayers, TL_MAYBE_UNUSED TL::Block block)
    {
        TL_ASSERT(m_state.frameRecording == true);
        TL_UNREACHABLE_MSG("TODO: Implement");
    }

    // private:

    ImageSize2D RenderGraph::GetFrameSize() const
    {
        TL_ASSERT(m_state.compiled);
        return m_frameSize;
    }

    const RGImage* RenderGraph::GetImage(Handle<RGImage> handle) const
    {
        // TL_ASSERT(m_state.compiled);
        return m_imagePool.Get(handle);
    }

    RGImage* RenderGraph::GetImage(Handle<RGImage> handle)
    {
        // TL_ASSERT(m_state.compiled);
        return m_imagePool.Get(handle);
    }

    const RGBuffer* RenderGraph::GetBuffer(Handle<RGBuffer> handle) const
    {
        // TL_ASSERT(m_state.compiled);
        return m_bufferPool.Get(handle);
    }

    RGBuffer* RenderGraph::GetBuffer(Handle<RGBuffer> handle)
    {
        // TL_ASSERT(m_state.compiled);
        return m_bufferPool.Get(handle);
    }

    Handle<Image> RenderGraph::GetImageHandle(Handle<RGImage> handle) const
    {
        TL_ASSERT(m_state.compiled);
        auto image = GetImage(handle);
        TL_ASSERT(image->m_handle);
        return image->m_handle;
    }

    Handle<Buffer> RenderGraph::GetBufferHandle(Handle<RGBuffer> handle) const
    {
        TL_ASSERT(m_state.compiled);
        auto buffer = GetBuffer(handle);
        TL_ASSERT(buffer->m_handle);
        return buffer->m_handle;
    }

    bool RenderGraph::IsNameValid(TL_MAYBE_UNUSED const char* name) const
    {
        TL_LOG_WARNNING("!TODO: Update this method to ensure graph names are unique");
        return true;
    }

    bool RenderGraph::CheckHandleIsValid(Handle<RGImage> imageHandle) const
    {
        return GetImage(imageHandle)->m_valid;
    }

    bool RenderGraph::CheckHandleIsValid(Handle<RGBuffer> bufferHandle) const
    {
        return GetBuffer(bufferHandle)->m_valid;
    }

    bool RenderGraph::CheckDependency(const RGPass* producer, const RGPass* consumer) const
    {
        auto passesCount = m_passPool.size();
        return m_dependencyTable[producer->m_indexInUnorderedList + consumer->m_indexInUnorderedList * passesCount] == true;
    }

    void RenderGraph::AddDependency(const RGPass* producer, RGPass* consumer)
    {
        auto passesCount = m_passPool.size();

        if (producer == nullptr)
            return; // consumer is actually the first producer of this resource
        TL_ASSERT(producer->m_indexInUnorderedList != consumer->m_indexInUnorderedList);

        consumer->m_producers.push_back((RGPass*)producer);

        m_dependencyTable[producer->m_indexInUnorderedList + consumer->m_indexInUnorderedList * passesCount] = true;
    }

    void RenderGraph::Compile()
    {
        ZoneScoped;
        m_dependencyTable.resize(m_passPool.size() * m_passPool.size(), false);

        TL::Vector<TL::Vector<uint32_t>> adjacencyLists(m_passPool.size());
        BuildAdjacencyLists(adjacencyLists);

        TL::Vector<uint32_t> sortedPasses;
        TopologicalSort(adjacencyLists, sortedPasses);

        uint32_t                    detectedQueueCount = 0;
        // TL::Vector<DependencyLevel> dependencyLevels;
        BuildDependencyLevels(sortedPasses, adjacencyLists, m_dependencyLevels, detectedQueueCount);

        // TODO: Move to new function
        // for (auto& pass : m_passPool)
        // {
        //     if (pass->m_state.culled)
        //         continue;
        //     // Look up the pass hash in a map
        //     if (auto state = GetPassState(pass.get()); state->shouldCompile)
        //     {
        //         RenderGraphContext ctx(this, pass.get());
        //         pass->Compile(ctx);
        //         state->shouldCompile = false;
        //     }
        // }
    }

    void RenderGraph::BuildAdjacencyLists(TL::Vector<TL::Vector<uint32_t>>& adjacencyLists)
    {
        for (size_t nodeIdx = 0; nodeIdx < m_passPool.size(); ++nodeIdx)
        {
            auto  pass                = m_passPool[nodeIdx].get();
            auto& adjacentNodeIndices = adjacencyLists[nodeIdx];
            for (size_t otherNodeIdx = 0; otherNodeIdx < m_passPool.size(); ++otherNodeIdx)
            {
                // Do not check dependencies on itself
                if (nodeIdx == otherNodeIdx) continue;
                auto otherNode = m_passPool[otherNodeIdx].get();
                if (CheckDependency(pass, otherNode))
                {
                    adjacentNodeIndices.push_back(otherNodeIdx);
                }
            }
        }
    }

    void RenderGraph::DepthFirstSearch(
        uint32_t                                nodeIndex,
        TL::Vector<bool>&                       visited,
        TL::Vector<bool>&                       onStack,
        bool&                                   isCyclic,
        const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists,
        TL::Vector<uint32_t>&                   sortedPasses)
    {
        if (isCyclic) return;
        visited[nodeIndex]          = true;
        onStack[nodeIndex]          = true;
        uint64_t adjacencyListIndex = m_passPool[nodeIndex]->m_indexInUnorderedList;
        for (uint32_t neighbour : adjacencyLists[adjacencyListIndex])
        {
            if (visited[neighbour] && onStack[neighbour])
            {
                isCyclic = true;
                return;
            }
            if (!visited[neighbour])
            {
                DepthFirstSearch(neighbour, visited, onStack, isCyclic, adjacencyLists, sortedPasses);
            }
        }
        onStack[nodeIndex] = false;
        sortedPasses.push_back(m_passPool[nodeIndex]->m_indexInUnorderedList);
    }

    void RenderGraph::TopologicalSort(const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses)
    {
        TL::Vector<bool> visitedNodes(m_passPool.size(), false);
        TL::Vector<bool> onStackNodes(m_passPool.size(), false);
        bool             isCyclic = false;
        for (uint32_t nodeIndex = 0; nodeIndex < (uint32_t)m_passPool.size(); ++nodeIndex)
        {
            if (!visitedNodes[nodeIndex])
            {
                DepthFirstSearch(nodeIndex, visitedNodes, onStackNodes, isCyclic, adjacencyLists, sortedPasses);
                TL_ASSERT(!isCyclic, "Detected cyclic dependency in pass: ", m_passPool[nodeIndex]->GetName());
            }
        }
        std::reverse(sortedPasses.begin(), sortedPasses.end());
    }

    void RenderGraph::BuildDependencyLevels(TL::Span<const uint32_t> sortedPasses, const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<DependencyLevel>& dependencyLevels, uint32_t& detectedQueueCount)
    {
        TL::Vector<int32_t> longestDistances(sortedPasses.size(), 0);
        uint64_t            dependencyLevelCount = 1;
        // Perform longest node distance search
        for (auto nodeIndex = 0; nodeIndex < sortedPasses.size(); ++nodeIndex)
        {
            uint64_t originalIndex      = m_passPool[sortedPasses[nodeIndex]]->m_indexInUnorderedList;
            uint64_t adjacencyListIndex = originalIndex;

            for (uint64_t adjacentNodeIndex : adjacencyLists[adjacencyListIndex])
            {
                if (longestDistances[adjacentNodeIndex] < longestDistances[originalIndex] + 1)
                {
                    int32_t newLongestDistance          = longestDistances[originalIndex] + 1;
                    longestDistances[adjacentNodeIndex] = newLongestDistance;
                    dependencyLevelCount                = std::max(uint64_t(newLongestDistance + 1), dependencyLevelCount);
                }
            }
        }
        dependencyLevels.resize(dependencyLevelCount);
        detectedQueueCount = 1;
        // Dispatch nodes to corresponding dependency levels.
        // Iterate through unordered nodes because adjacency lists contain indices to
        // initial unordered list of nodes and longest distances also correspond to them.
        for (uint32_t nodeIndex = 0; nodeIndex < (uint32_t)m_passPool.size(); nodeIndex++)
        {
            RGPass*          pass            = m_passPool[nodeIndex].get();
            auto             levelIndex      = longestDistances[nodeIndex];
            DependencyLevel& dependencyLevel = dependencyLevels[levelIndex];
            dependencyLevel.m_levelIndex     = levelIndex;
            dependencyLevel.AddPass(pass);
            pass->m_dependencyLevelIndex = levelIndex;
            detectedQueueCount           = std::max(detectedQueueCount, pass->m_executionQueueIndex + 1);
        }
    }

    void RenderGraph::Execute()
    {
        ZoneScoped;

        // TODO: Handle resource transition barriers
        // // for every pass in topologically sorted passes
        // for (auto& level : m_dependencyLevels)
        // {
        //     for (auto pass : level.GetPasses())
        //     {
        //         if (pass->m_state.culled)
        //             continue;

        //         for (auto dep : pass->m_imageDependencies)
        //         {
        //             // last known state
        //         }
        //         for (auto dep : pass->m_bufferDependencies)
        //         {
        //         }
        //     }

        //     // for every dep in pass
        //     // if current state = dep.state (skip)
        //     // if resource.last_pass = dep.pass (skip)
        //     // dep.resource.push_back(transition)
        // }

        // ExecuteSerialSingleThreaded();

        m_frameIndex++;
    }

    void RenderGraph::ExecutePass(RGPass* pass, CommandList* commandList)
    {
        ZoneScoped;

        bool isCompute = pass->m_type == PassType::Compute || pass->m_type == PassType::AsyncCompute;

        if (pass->m_type == PassType::Graphics)
            commandList->PushDebugMarker(pass->GetName(), Colors::DebugMarker_Graphics);
        else if (isCompute)
            commandList->PushDebugMarker(pass->GetName(), Colors::DebugMarker_Compute);
        else
            commandList->PushDebugMarker(pass->GetName(), Colors::DebugMarker_Transfer);

        const auto& [prebarriers, preImageBarriers, preBufferBarriers] = pass->m_barriers[RGPass::Prilogue];

        commandList->AddPipelineBarrier(prebarriers, preImageBarriers, preBufferBarriers);
        if (pass->m_type == PassType::Graphics)
        {
            TL::Vector<ColorAttachment>          attachments;
            TL::Optional<DepthStencilAttachment> dsAttachment;
            attachments.reserve(pass->m_gfxPassInfo.m_colorAttachments.size());

            for (auto attachment : pass->m_gfxPassInfo.m_colorAttachments)
            {
                auto color   = GetImageHandle(attachment.color);
                auto resolve = attachment.resolveView ? GetImageHandle(attachment.resolveView) : NullHandle;
                attachments.push_back({
                    .view        = color,
                    .loadOp      = attachment.loadOp,
                    .storeOp     = attachment.storeOp,
                    .clearValue  = attachment.clearValue,
                    .resolveMode = attachment.resolveMode,
                    .resolveView = resolve,
                });
            }
            if (auto dsv = pass->m_gfxPassInfo.m_depthStencilAttachment)
            {
                auto depthStencil = GetImageHandle(dsv->depthStencil);
                dsAttachment      = DepthStencilAttachment{
                         .view           = depthStencil,
                         .depthLoadOp    = dsv->depthLoadOp,
                         .depthStoreOp   = dsv->depthStoreOp,
                         .stencilLoadOp  = dsv->stencilLoadOp,
                         .stencilStoreOp = dsv->stencilStoreOp,
                         .clearValue     = dsv->clearValue,
                };
            }
            RenderPassBeginInfo beginInfo{
                .name                   = pass->GetName(),
                .size                   = pass->m_gfxPassInfo.m_size,
                .offset                 = pass->m_gfxPassInfo.m_offset,
                .colorAttachments       = attachments,
                .depthStencilAttachment = dsAttachment,
            };
            commandList->BeginRenderPass(beginInfo);
        }
        else if (isCompute)
        {
            ComputePassBeginInfo beginInfo{
                .name = pass->GetName(),
            };
            commandList->BeginComputePass(beginInfo);
        }

        pass->Execute(*commandList);

        if (pass->m_type == PassType::Graphics)
            commandList->EndRenderPass();
        else if (isCompute)
            commandList->EndComputePass();

        const auto& [postbarriers, postImageBarriers, postBufferBarriers] = pass->m_barriers[RGPass::Epilogue];
        commandList->AddPipelineBarrier(postbarriers, postImageBarriers, postBufferBarriers);
        commandList->PopDebugMarker();
    }

    void RenderGraph::ExecuteSerialSingleThreaded()
    {
        CommandListCreateInfo cmdCI{
            .name      = "RG-CMD",
            .queueType = QueueType::Graphics,
        };
        auto commandList = m_device->CreateCommandList(cmdCI);

        commandList->Begin();
        for (const auto& level : m_dependencyLevels)
        {
            for (auto pass : level.GetPasses())
            {
                ExecutePass(pass, commandList);
            }
        }

        if (m_swapchain)
        {
            // Add barrier to blits swapchain to its original
        }

        commandList->End();

        QueueSubmitInfo submitInfo{
            .queueType            = QueueType::Graphics,
            .commandLists         = commandList,
            .signalStage          = PipelineStage::BottomOfPipe,
            .waitInfos            = {},
            .m_swapchainToAcquire = m_swapchain,
            .m_swapchainToSignal  = m_swapchain,
        };
        TL_MAYBE_UNUSED auto timeline = m_device->QueueSubmit(submitInfo);
    }

} // namespace RHI