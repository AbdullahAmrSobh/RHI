#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"
#include "RHI/Reflect.hpp"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>
#include <TL/Utils.hpp>
#include <TL/Defer.hpp>

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

        constexpr ClearValue DebugMarker_AsyncCompute{
            .f32{0.3f, 0.6f, 0.24f, 1.0f}
        };

        constexpr ClearValue DebugMarker_Transfer{
            .f32{0.2f, 0.4f, 0.7f, 1.0f}
        };

    }; // namespace Colors

    // ///////////////////////////////////////////////////////////////////////////
    // /// Pass Resource types
    // ///////////////////////////////////////////////////////////////////////////

    RGFrameImage* RenderGraph::CreateFrameImage(const char* name)
    {
        auto image  = TL::ConstructFrom<RGFrameImage>(&m_arena);
        image->name = name;
        return image;
    }

    RGFrameBuffer* RenderGraph::CreateFrameBuffer(const char* name)
    {
        auto buffer  = TL::ConstructFrom<RGFrameBuffer>(&m_arena);
        buffer->name = name;
        return buffer;
    }

    RGImage* RenderGraph::EmplacePassImage(RGFrameImage* frameImage, RGPass* pass, ImageBarrierState initialState)
    {
        auto image             = TL::ConstructFrom<RGImage>(&m_arena);
        image->m_frameResource = frameImage;
        image->m_state         = initialState;
        image->m_producer      = pass;
        image->m_prevHandle    = frameImage->lastProducer;

        if (frameImage->lastProducer)
        {
            frameImage->lastProducer->m_nextHandle = image;
        }

        frameImage->lastProducer = image;
        return image;
    }

    RGBuffer* RenderGraph::EmplacePassBuffer(RGFrameBuffer* frameBuffer, RGPass* pass, BufferBarrierState initialState)
    {
        auto buffer             = TL::ConstructFrom<RGBuffer>(&m_arena);
        buffer->m_frameResource = frameBuffer;
        buffer->m_state         = initialState;
        buffer->m_producer      = pass;
        buffer->m_prevHandle    = frameBuffer->lastProducer;

        if (frameBuffer->lastProducer)
        {
            frameBuffer->lastProducer->m_nextHandle = buffer;
        }

        frameBuffer->lastProducer = buffer;
        return buffer;
    }

    TL::IAllocator& RenderGraph::GetFrameAllocator()
    {
        return m_activeFrame->GetAllocator();
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Builder Interface
    ///////////////////////////////////////////////////////////////////////////

    RenderGraphBuilder::RenderGraphBuilder(RenderGraph* rg, RGPass* pass)
        : m_rg(rg)
        , m_pass(pass)
    {
    }

    void RenderGraphBuilder::ReadImage(RGImage* imageHandle, ImageUsage usage, PipelineStage stage)
    {
        ReadImage(imageHandle, ImageSubresourceRange::All(), usage, stage);
    }

    void RenderGraphBuilder::ReadImage(RGImage* imageHandle, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage)
    {
        UseImageInternal(imageHandle, Access::Read, subresource, usage, stage);
    }

    RGImage* RenderGraphBuilder::WriteImage(RGImage* imageHandle, ImageUsage usage, PipelineStage stage)
    {
        return WriteImage(imageHandle, ImageSubresourceRange::All(), usage, stage);
    }

    RGImage* RenderGraphBuilder::WriteImage(RGImage* imageHandle, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage)
    {
        return UseImageInternal(imageHandle, Access::Write, subresource, usage, stage);
    }

    void RenderGraphBuilder::ReadBuffer(RGBuffer* bufferHandle, BufferUsage usage, PipelineStage stage)
    {
        ReadBuffer(bufferHandle, BufferSubregion{.offset = 0, .size = WholeSize}, usage, stage);
    }

    void RenderGraphBuilder::ReadBuffer(RGBuffer* bufferHandle, const BufferSubregion& subregion, BufferUsage usage, PipelineStage stage)
    {
        UseBufferInternal(bufferHandle, Access::Read, subregion, usage, stage);
    }

    RGBuffer* RenderGraphBuilder::WriteBuffer(RGBuffer* bufferHandle, BufferUsage usage, PipelineStage stage)
    {
        return WriteBuffer(bufferHandle, BufferSubregion{.offset = 0, .size = WholeSize}, usage, stage);
    }

    RGBuffer* RenderGraphBuilder::WriteBuffer(RGBuffer* bufferHandle, const BufferSubregion& subregion, BufferUsage usage, PipelineStage stage)
    {
        return UseBufferInternal(bufferHandle, Access::Write, subregion, usage, stage);
    }

    RGImage* RenderGraphBuilder::AddColorAttachment(RGColorAttachment attachment)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        // TL_ASSERT(m_rg->CheckHandleIsValid(attachment.color));
        // if (attachment.resolveView) TL_ASSERT(m_rg->CheckHandleIsValid(attachment.resolveView));

        m_pass->m_gfxPassInfo.m_colorAttachments.push_back(attachment);

        auto color = WriteImage(attachment.color, attachment.colorRange, ImageUsage::Color, PipelineStage::ColorAttachmentOutput);
        if (attachment.resolveView)
        {
            return WriteImage(attachment.resolveView, attachment.resolveRange, ImageUsage::Resolve, PipelineStage::Resolve);
        }
        return color;
    }

    RGImage* RenderGraphBuilder::SetDepthStencil(RGDepthStencilAttachment attachment)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        // TL_ASSERT(m_rg->CheckHandleIsValid(attachment.depthStencil));

        // TODO: fix this function

        m_pass->m_gfxPassInfo.m_depthStencilAttachment = attachment;
        return WriteImage(attachment.depthStencil, attachment.depthStencilRange, ImageUsage::DepthStencil, PipelineStage::LateFragmentTests);
    }

    RGImage* RenderGraphBuilder::UseImageInternal(RGImage* resource, Access access, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);

        // Add dependency from the resource's producer to this pass
        if (resource->m_producer && resource->m_producer != m_pass)
            m_rg->AddDependency(resource->m_producer, m_pass);

        if (resource->m_producer == nullptr)
        {
            TL_ASSERT(access & Access::Write);
            resource->m_producer = m_pass;
        }
        else if (access & Access::Write)
        {
            resource = m_rg->EmplacePassImage(resource->m_frameResource, m_pass, ImageBarrierState{usage, stage, access});
        }

        // Add resource dependency to the pass
        resource->m_frameResource->usageFlags |= usage; // extend actual usage
        RGImageDependency dep{
            .image  = resource,
            .viewID = 0,
            .state  = ImageBarrierState{usage, stage, access},
        };
        m_pass->m_imageDependencies.push_back(dep);

        return resource;
    }

    RGBuffer* RenderGraphBuilder::UseBufferInternal(RGBuffer* resource, Access access, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);

        // Add dependency from the resource's producer to this pass
        if (resource->m_producer && resource->m_producer != m_pass)
            m_rg->AddDependency(resource->m_producer, m_pass);

        if (resource->m_producer == nullptr)
        {
            TL_ASSERT(access & Access::Write);
            resource->m_producer = m_pass;
        }
        else if (access & Access::Write)
        {
            resource = m_rg->EmplacePassBuffer(resource->m_frameResource, m_pass, BufferBarrierState{usage, stage, access});
        }

        // Add resource dependency to the pass
        resource->m_frameResource->usageFlags |= usage; // extend actual usage
        RGBufferDependency dep{
            .buffer    = resource,
            .subregion = subresource,
            .state     = BufferBarrierState{usage, stage, access},
        };
        m_pass->m_bufferDependencies.push_back(dep);

        return resource;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Pass
    ///////////////////////////////////////////////////////////////////////////

    RGPass::RGPass(RenderGraph* rg, const PassCreateInfo& ci)
        : m_renderGraph{rg}
        , m_name("UNNAMED")
        , m_type{ci.type}
        , m_setupCallback{ci.setupCallback}
        , m_executeCallback{ci.executeCallback}
        , m_imageDependencies(rg->GetFrameAllocator())
        , m_bufferDependencies(rg->GetFrameAllocator())
        , m_producers(rg->GetFrameAllocator())
        , m_gfxPassInfo(rg->GetFrameAllocator())
        , m_barriers{rg->GetFrameAllocator()}
    {
    }

    RGPass::~RGPass()
    {
    }

    void RGPass::Setup(RenderGraphBuilder& builder)
    {
        ZoneScopedN("RGPass::Setup");
        TL_ASSERT(m_setupCallback);
        m_setupCallback(builder);
    }

    void RGPass::Execute(CommandList& commandList)
    {
        ZoneScopedN("RGPass::Execute");
        TL_ASSERT(m_executeCallback);
        m_executeCallback(commandList);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Resource Pool
    ///////////////////////////////////////////////////////////////////////////

    inline static bool CompareImageCreateInfo(const ImageCreateInfo& ci1, const ImageCreateInfo& ci2)
    {
        return (ci1.usageFlags == ci2.usageFlags) &&
               (ci1.type == ci2.type) &&
               (ci1.size == ci2.size) &&
               (ci1.format == ci2.format) &&
               (ci1.sampleCount == ci2.sampleCount) &&
               (ci1.mipLevels == ci2.mipLevels) &&
               (ci1.arrayCount == ci2.arrayCount);
    }

    inline static bool CompareBufferCreateInfo(const BufferCreateInfo& ci1, const BufferCreateInfo& ci2)
    {
        return (ci1.hostMapped == ci2.hostMapped) &&
               (ci1.usageFlags == ci2.usageFlags) &&
               (ci1.byteSize == ci2.byteSize);
    }

    ResultCode RenderGraphResourcePool::Init(Device* device)
    {
        m_device = device;

        // m_imageCache.clear();
        // m_bufferCache.clear();
        return ResultCode::Success;
    }

    void RenderGraphResourcePool::Shutdown()
    {
        for (const auto& [_, handle] : m_imageCache)
        {
            m_device->DestroyImage(handle.second);
        }
        for (const auto& [_, handle] : m_bufferCache)
        {
            m_device->DestroyBuffer(handle.second);
        }

        m_imageCache.clear();
        m_bufferCache.clear();
    }

    Handle<Image> RenderGraphResourcePool::InitTransientImage(RGFrameImage* rgImage)
    {
        auto it = m_imageCache.find(rgImage->name);

        if (rgImage->isImported)
            return rgImage->handle;

        ImageCreateInfo imageCI{
            .name        = rgImage->name.c_str(),
            .usageFlags  = rgImage->usageFlags,
            .type        = rgImage->type,
            .size        = rgImage->size,
            .format      = rgImage->format,
            .sampleCount = rgImage->sampleCount,
            .mipLevels   = rgImage->mipLevels,
            .arrayCount  = rgImage->arrayCount,
        };

        // First try to find a in cache
        if (it != m_imageCache.end())
        {
            const auto& [cachedCI, handle] = it->second;
            if (CompareImageCreateInfo(cachedCI, imageCI))
                return handle;
            // Resource with same name, but different properties were found, so recreate the resource.
            TL_LOG_INFO("...Recreating resource {}\n {}", rgImage->name, Debug::ToString(imageCI));
            m_device->DestroyImage(handle);
            m_imageCache.erase(it); // No need (I think) as
        }
        else
        {
            TL_LOG_INFO("Creating image {}", Debug::ToString(imageCI));
        }

        auto [_, handle] = m_imageCache[rgImage->name] = std::pair{imageCI, m_device->CreateImage(imageCI)};
        return handle;
    }

    Handle<Buffer> RenderGraphResourcePool::InitTransientBuffer(RGFrameBuffer* rgBuffer)
    {
        auto it = m_bufferCache.find(rgBuffer->name);

        if (rgBuffer->isImported)
        {
            return rgBuffer->handle;
        }

        BufferCreateInfo ci{
            .name       = rgBuffer->name.c_str(),
            .hostMapped = true, // TODO: Should be I don't care?
            .usageFlags = rgBuffer->usageFlags,
            .byteSize   = rgBuffer->size,
        };

        if (it != m_bufferCache.end())
        {
            const auto& [cachedCI, handle] = it->second;
            if (CompareBufferCreateInfo(cachedCI, ci))
                return handle;

            // Resource with same name, but different properties were found, so recreate the resource.
            TL_LOG_INFO("...Recreating resource {}\n {}", rgBuffer->name, Debug::ToString(ci));
            m_device->DestroyBuffer(handle);
            m_bufferCache.erase(it);
        }
        else
        {
            TL_LOG_INFO("Creating buffer {}", Debug::ToString(ci));
        }

        auto [_, handle] = m_bufferCache[rgBuffer->name] = std::pair{ci, m_device->CreateBuffer(ci)};
        return handle;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    RenderGraph::RenderGraph()  = default;
    RenderGraph::~RenderGraph() = default;

    ResultCode RenderGraph::Init(Device* device, const RenderGraphCreateInfo& ci)
    {
        m_device       = device;
        m_resourcePool = TL::CreatePtr<RenderGraphResourcePool>();

        auto result = m_resourcePool->Init(device);

        return result;
    }

    void RenderGraph::Shutdown()
    {
        m_resourcePool->Shutdown();
        m_arena.Collect();
        m_passPool.clear();
        m_imagePool.clear();
        m_bufferPool.clear();
    }

    void RenderGraph::Debug_CaptureNextFrame()
    {
        m_state.debug_triggerNextFrameCapture = true;
    }

    void RenderGraph::BeginFrame(ImageSize2D frameSize)
    {
        ZoneScopedC(Colors::Red);

        m_state.frameRecording = true;
        m_frameSize            = frameSize;

        m_activeFrame = m_device->GetCurrentFrame();
    }

    void RenderGraph::EndFrame()
    {
        ZoneScopedC(Colors::Red);

        TL_ASSERT(m_state.frameRecording == true);
        m_state.frameRecording = false;

        Compile();
        Execute();

        if (m_state.dumpGraphviz)
        {
            TL::String output;
            output += "====== RenderGraph State ======\n";
            for (size_t levelIdx = 0; levelIdx < m_dependencyLevels.size(); ++levelIdx)
            {
                const auto& level = m_dependencyLevels[levelIdx];
                output += std::format("Dependency Level {}:\n", levelIdx);
                for (auto pass : level.GetPasses())
                {
                    output += std::format("  Pass: {}\n", pass->GetName());
                    // Print image transitions
                    for (const auto& dep : pass->m_imageDependencies)
                    {
                        auto img = dep.image;
                        output += std::format("    Image: {} | Usage: {}\n", img->m_frameResource->name, Debug::ToString(dep.state.usage));
                    }
                    // Print buffer transitions
                    for (const auto& dep : pass->m_bufferDependencies)
                    {
                        auto buf = dep.buffer;
                        output += std::format("    Buffer: {} | Usage: {}\n", buf->m_frameResource->name, Debug::ToString(dep.state.usage));
                    }
                }
            }
            output += "================================\n";
            TL_LOG_INFO("{}", output);
            m_state.dumpGraphviz = false;
        }

        {
            ZoneScopedN("Clear");
            m_passPool.clear();
            m_imagePool.clear();
            m_bufferPool.clear();
            m_dependencyLevels.clear();
            m_swapchain.clear();
            m_swapchainAcquireStage.clear();
            m_arena.Collect();
            memset(&m_state, 0, sizeof(State));
        }

        m_frameIndex++;
    }

    RGImage* RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain, Format format)
    {
        TL_ASSERT(m_state.frameRecording == true);
        m_swapchain.push_back(&swapchain);
        m_swapchainAcquireStage.push_back(PipelineStage::BottomOfPipe);

        auto frameResource        = TL::ConstructFrom<RGFrameImage>(&m_arena);
        frameResource->name       = name;
        frameResource->handle     = swapchain.GetImage();
        frameResource->format     = format;
        frameResource->isImported = true;
        return EmplacePassImage(frameResource, nullptr, {});
    }

    RGImage* RenderGraph::ImportImage(const char* name, Handle<Image> image, Format format)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto frameImage        = TL::ConstructFrom<RGFrameImage>(&m_arena);
        frameImage->name       = name;
        frameImage->handle     = image;
        frameImage->format     = format;
        frameImage->isImported = true;
        m_imagePool.push_back(frameImage);
        return EmplacePassImage(frameImage, nullptr, {});
    }

    RGBuffer* RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto frameBuffer        = TL::ConstructFrom<RGFrameBuffer>(&m_arena);
        frameBuffer->name       = name;
        frameBuffer->handle     = buffer;
        frameBuffer->isImported = true;
        m_bufferPool.push_back(frameBuffer);
        return EmplacePassBuffer(frameBuffer, nullptr, {});
    }

    RGImage* RenderGraph::CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto frameImage         = TL::ConstructFrom<RGFrameImage>(&m_arena);
        frameImage->name        = name;
        frameImage->type        = type;
        frameImage->size        = size;
        frameImage->format      = format;
        frameImage->mipLevels   = mipLevels;
        frameImage->arrayCount  = arrayCount;
        frameImage->sampleCount = samples;
        m_imagePool.push_back(frameImage);
        return EmplacePassImage(frameImage, nullptr, {});
    }

    RGImage* RenderGraph::CreateRenderTarget(const char* name, ImageSize2D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples)
    {
        TL_ASSERT(m_state.frameRecording == true);
        return CreateImage(name, ImageType::Image2D, {size.width, size.height, 1}, format, mipLevels, arrayCount, samples);
    }

    RGBuffer* RenderGraph::CreateBuffer(const char* name, size_t size)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto frameBuffer  = TL::ConstructFrom<RGFrameBuffer>(&m_arena);
        frameBuffer->name = name;
        frameBuffer->size = size;
        m_bufferPool.push_back(frameBuffer);
        return EmplacePassBuffer(frameBuffer, nullptr, {});
    }

    RGPass* RenderGraph::AddPass(const PassCreateInfo& createInfo)
    {
        ZoneScoped;
        TL_ASSERT(m_state.frameRecording == true);
        uint32_t indexInUnorderedList = m_passPool.size();
        RGPass*  pass                 = m_passPool.emplace_back(TL::ConstructFrom<RGPass>(&GetFrameAllocator(), this, createInfo));

        auto builder = RenderGraphBuilder(this, pass);
        pass->Setup(builder);
        pass->m_gfxPassInfo.m_size   = createInfo.size;
        pass->m_indexInUnorderedList = indexInUnorderedList;
        return pass;
    }

    void RenderGraph::Dump()
    {
        m_state.dumpGraphviz = true;
    }

    // private:

    ImageSize2D RenderGraph::GetFrameSize() const
    {
        // TL_ASSERT(m_state.compiled);
        return m_frameSize;
    }

    Handle<Image> RenderGraph::GetImageHandle(RGImage* image) const
    {
        TL_ASSERT(m_state.compiled);
        TL_ASSERT(image->m_frameResource->handle);
        return image->m_frameResource->handle;
    }

    Handle<Buffer> RenderGraph::GetBufferHandle(RGBuffer* buffer) const
    {
        TL_ASSERT(m_state.compiled);
        TL_ASSERT(buffer->m_frameResource->handle);
        return buffer->m_frameResource->handle;
    }

    bool RenderGraph::CheckDependency(const RGPass* producer, const RGPass* consumer) const
    {
        return consumer->m_producers.contains(producer->m_indexInUnorderedList);
    }

    void RenderGraph::AddDependency(const RGPass* producer, RGPass* consumer)
    {
        if (producer == nullptr)
            return; // consumer is actually the first producer of this resource

        if (producer == consumer)
            return;

        consumer->m_producers.insert(producer->m_indexInUnorderedList);
    }

    void RenderGraph::Compile()
    {
        ZoneScoped;

        TL::Vector<TL::Vector<uint32_t>> adjacencyLists(GetFrameAllocator());
        adjacencyLists.resize(m_passPool.size(), TL::Vector<uint32_t>{GetFrameAllocator()});
        for (size_t nodeIdx = 0; nodeIdx < m_passPool.size(); ++nodeIdx)
        {
            auto pass = m_passPool[nodeIdx];
            for (uint32_t otherNodeIdx = 0; otherNodeIdx < m_passPool.size(); ++otherNodeIdx)
            {
                if (nodeIdx == otherNodeIdx) continue;
                auto otherNode = m_passPool[otherNodeIdx];
                if (CheckDependency(pass, otherNode))
                {
                    adjacencyLists[nodeIdx].push_back(otherNodeIdx);
                }
            }
        }

        TL::Vector<uint32_t> sortedPasses(GetFrameAllocator());
        TopologicalSort(adjacencyLists, sortedPasses);

        uint32_t detectedQueueCount = 0;
        {
            TL::Vector<int32_t> longestDistances(sortedPasses.size(), 0, GetFrameAllocator());
            uint64_t            dependencyLevelCount = 1;
            // Perform longest node distance search
            for (uint32_t nodeIndex = 0; nodeIndex < sortedPasses.size(); ++nodeIndex)
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
            m_dependencyLevels.resize(dependencyLevelCount, {GetFrameAllocator()});
            detectedQueueCount = 1;
            // Dispatch nodes to corresponding dependency levels.
            // Iterate through unordered nodes because adjacency lists contain indices to
            // initial unordered list of nodes and longest distances also correspond to them.
            for (uint32_t nodeIndex = 0; nodeIndex < (uint32_t)m_passPool.size(); nodeIndex++)
            {
                RGPass*          pass            = m_passPool[nodeIndex];
                auto             levelIndex      = longestDistances[nodeIndex];
                DependencyLevel& dependencyLevel = m_dependencyLevels[levelIndex];
                dependencyLevel.m_levelIndex     = (uint32_t)levelIndex;
                dependencyLevel.AddPass(pass);
                pass->m_dependencyLevelIndex = levelIndex;
                detectedQueueCount           = std::max(detectedQueueCount, pass->m_executionQueueIndex + 1);
            }
        }

        CreateTransientResources();
        PassBuildBarriers();

        m_state.compiled = true;
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
        TL::Vector<bool> visitedNodes(m_passPool.size(), false, GetFrameAllocator());
        TL::Vector<bool> onStackNodes(m_passPool.size(), false, GetFrameAllocator());
        bool             isCyclic = false;
        for (uint32_t nodeIndex = 0; nodeIndex < (uint32_t)m_passPool.size(); ++nodeIndex)
        {
            if (!visitedNodes[nodeIndex])
            {
                DepthFirstSearch(nodeIndex, visitedNodes, onStackNodes, isCyclic, adjacencyLists, sortedPasses);
                TL_ASSERT(!isCyclic, "Detected cyclic dependency in pass: ", m_passPool[nodeIndex]->GetName());
            }
        }
        // I don't know why/how but it flips the expected order
        std::reverse(sortedPasses.begin(), sortedPasses.end());
    }

    void RenderGraph::CreateTransientResources()
    {
        ZoneScoped;

        for (auto frameImage : m_imagePool)
        {
            if (frameImage->isImported)
                continue;

            frameImage->handle = m_resourcePool->InitTransientImage(frameImage);
        }

        for (auto frameBuffer : m_bufferPool)
        {
            if (frameBuffer->isImported)
                continue;

            frameBuffer->handle = m_resourcePool->InitTransientBuffer(frameBuffer);
        }
    }

    void RenderGraph::PassBuildBarriers()
    {
        ZoneScoped;

        auto TransitionImageResource = [this](RGPass* pass, RGImageDependency dep)
        {
            auto resource = dep.image;
            if (resource->m_state == dep.state)
                return;

            auto& passImageBarriers = pass->m_barriers.imageBarriers;
            passImageBarriers.push_back({
                .image    = resource->m_frameResource->handle,
                .srcState = resource->m_state,
                .dstState = dep.state,
            });

            resource->m_state = dep.state;
        };

        auto TransitionBufferResource = [this](RGPass* pass, RGBufferDependency dep)
        {
            auto resource = dep.buffer;
            if (resource->m_state == dep.state)
                return;

            auto& passBufferBarriers = pass->m_barriers.bufferBarriers;
            passBufferBarriers.push_back({
                .buffer   = resource->m_frameResource->handle,
                .srcState = resource->m_state,
                .dstState = dep.state,
            });

            resource->m_state = dep.state;
        };

        for (auto level : m_dependencyLevels)
        {
            for (auto pass : level.GetPasses())
            {
                if (pass->m_state.culled)
                    continue;

                for (auto dep : pass->m_imageDependencies)
                    TransitionImageResource(pass, dep);
                for (auto dep : pass->m_bufferDependencies)
                    TransitionBufferResource(pass, dep);
            }
        }
    }

    void RenderGraph::Execute()
    {
        ZoneScoped;

#if RHI_RG_EXECUTE_MULTITHREADED
    #error "TODO: Implement"
#elif 1
        auto frame = m_device->GetCurrentFrame();

        if (m_state.debug_triggerNextFrameCapture)
        {
            frame->CaptureNextFrame();
        }
        frame->Begin(m_swapchain);

        CommandListCreateInfo cmdCI{
            .name      = "cmd-rendergraph",
            .queueType = QueueType::Graphics,
        };
        auto commandList = frame->CreateCommandList(cmdCI);

        commandList->Begin();
        for (const auto& level : m_dependencyLevels)
        {
            for (auto pass : level.GetPasses())
            {
                ExecutePass(pass, commandList);
            }
        }

        for (auto swapchain : m_swapchain)
        {
            // FIXME: This is hardcoded for now
            RHI::ImageBarrierInfo barrier{
                .image    = swapchain->GetImage(),
                .srcState = {
                             .usage  = ImageUsage::Color,
                             .stage  = PipelineStage::ColorAttachmentOutput,
                             .access = Access::ReadWrite,
                             },
                .dstState = {
                             .usage  = ImageUsage::Present,
                             .stage  = PipelineStage::BottomOfPipe,
                             .access = Access::None,
                             },
            };
            commandList->AddPipelineBarrier({}, barrier, {});
        }

        commandList->End();

        std::reverse(m_swapchain.begin(), m_swapchain.end());

        QueueSubmitInfo submitInfo{
            .commandLists          = commandList,
            .signalStage           = PipelineStage::BottomOfPipe,
            .waitInfos             = {},
            .m_swapchainToAcquire  = m_swapchain,
            .m_swapchainWaitStages = m_swapchainAcquireStage,
            .signalPresent         = true,
        };
        TL_MAYBE_UNUSED auto timeline = frame->QueueSubmit(QueueType::Graphics, submitInfo);

        m_frameIndex = frame->End();
#endif

        m_frameIndex++;
    }

    void RenderGraph::ExecutePass(RGPass* pass, CommandList* commandList)
    {
        ZoneScoped;

        bool isCompute = pass->m_type == PassType::Compute || pass->m_type == PassType::AsyncCompute;

        ClearValue markerColor{};
        switch (pass->m_type)
        {
        case PassType::Graphics:     markerColor = Colors::DebugMarker_Graphics; break;
        case PassType::Compute:      markerColor = Colors::DebugMarker_Compute; break;
        case PassType::AsyncCompute: markerColor = Colors::DebugMarker_AsyncCompute; break;
        case PassType::Transfer:     markerColor = Colors::DebugMarker_Transfer; break;
        }
        commandList->PushDebugMarker(pass->GetName(), markerColor);

        // const auto& [prebarriers, preImageBarriers, preBufferBarriers] = pass->m_barriers[RGPass::Prilogue];
        const auto& [prebarriers, preImageBarriers, preBufferBarriers] = pass->m_barriers;
        commandList->AddPipelineBarrier(prebarriers, preImageBarriers, preBufferBarriers);

        if (pass->m_type == PassType::Graphics)
        {
            TL::Vector<ColorAttachment>          attachments(GetFrameAllocator());
            TL::Optional<DepthStencilAttachment> dsAttachment;
            attachments.reserve(pass->m_gfxPassInfo.m_colorAttachments.size());

            // auto [passWidth, passHeight]    = pass->m_gfxPassInfo.m_size;
            for (auto attachment : pass->m_gfxPassInfo.m_colorAttachments)
            {
                auto [imgWidth, imgHeight, imgDepth] = attachment.color->m_frameResource->size;
                // TL_ASSERT(imgWidth == passWidth && imgHeight == passHeight);
                if (attachment.resolveView)
                {
                    auto [resWidth, resHeight, resDepth] = attachment.resolveView->m_frameResource->size;
                    // TL_ASSERT(resWidth == passWidth && resHeight == passHeight);
                }

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
            if (auto attachment = pass->m_gfxPassInfo.m_depthStencilAttachment)
            {
                auto [dsWidth, dsHeight, dsDepth] = attachment->depthStencil->m_frameResource->size;
                // TL_ASSERT(dsWidth == passWidth && dsHeight == passHeight);
                auto depthStencil                 = GetImageHandle(attachment->depthStencil);
                dsAttachment                      = DepthStencilAttachment{
                                         .view           = depthStencil,
                                         .depthLoadOp    = attachment->depthLoadOp,
                                         .depthStoreOp   = attachment->depthStoreOp,
                                         .stencilLoadOp  = attachment->stencilLoadOp,
                                         .stencilStoreOp = attachment->stencilStoreOp,
                                         .clearValue     = attachment->clearValue,
                };
            }
            RenderPassBeginInfo beginInfo{
                // .name                   = pass->GetName(),
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

        // const auto& [postbarriers, postImageBarriers, postBufferBarriers] = pass->m_barriers[RGPass::Epilogue];
        // commandList->AddPipelineBarrier(postbarriers, postImageBarriers, postBufferBarriers);
        commandList->PopDebugMarker();
    }
} // namespace RHI