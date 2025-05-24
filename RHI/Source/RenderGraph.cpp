#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"
#include "RHI/Reflect.hpp"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>
#include <TL/Utils.hpp>

#include <cstdint>
#include <tracy/Tracy.hpp>

namespace std
{
    // Add hash functions for ImageCreateInfo, ImageViewCreateInfo, and BufferView ...
}

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

    // ///////////////////////////////////////////////////////////////////////////
    // /// Pass Resource types
    // ///////////////////////////////////////////////////////////////////////////

    // size_t RGImage::GetHash() const
    // {
    //     uint64_t hash = 0;
    //     TL::HashCombine(hash, (uint64_t)m_desc.usageFlags.GetHash());
    //     TL::HashCombine(hash, m_desc.size.width);
    //     TL::HashCombine(hash, m_desc.size.height);
    //     TL::HashCombine(hash, m_desc.size.depth);
    //     TL::HashCombine(hash, (uint64_t)m_desc.format);
    //     TL::HashCombine(hash, m_desc.mipLevels);
    //     TL::HashCombine(hash, m_desc.arrayCount);
    //     TL::HashCombine(hash, (uint64_t)m_desc.sampleCount);
    //     return hash;
    // }

    // size_t RGBuffer::GetHash() const
    // {
    //     uint64_t hash = 0;
    //     TL::HashCombine(hash, (uint64_t)m_desc.usageFlags.GetHash());
    //     TL::HashCombine(hash, m_desc.size);
    //     return hash;
    // }

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
        UseImageInternal(imageHandle, m_rg->m_imagePool, Access::Read, subresource, usage, stage);
    }

    Handle<RGImage> RenderGraphBuilder::WriteImage(Handle<RGImage> imageHandle, ImageUsage usage, PipelineStage stage)
    {
        return WriteImage(imageHandle, ImageSubresourceRange::All(), usage, stage);
    }

    Handle<RGImage> RenderGraphBuilder::WriteImage(Handle<RGImage> imageHandle, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage)
    {
        return UseImageInternal(imageHandle, m_rg->m_imagePool, Access::Write, subresource, usage, stage);
    }

    void RenderGraphBuilder::ReadBuffer(Handle<RGBuffer> bufferHandle, BufferUsage usage, PipelineStage stage)
    {
        ReadBuffer(bufferHandle, BufferSubregion{.offset = 0, .size = WholeSize}, usage, stage);
    }

    void RenderGraphBuilder::ReadBuffer(Handle<RGBuffer> bufferHandle, const BufferSubregion& subregion, BufferUsage usage, PipelineStage stage)
    {
        UseBufferInternal(bufferHandle, m_rg->m_bufferPool, Access::Read, subregion, usage, stage);
    }

    Handle<RGBuffer> RenderGraphBuilder::WriteBuffer(Handle<RGBuffer> bufferHandle, BufferUsage usage, PipelineStage stage)
    {
        return WriteBuffer(bufferHandle, BufferSubregion{.offset = 0, .size = WholeSize}, usage, stage);
    }

    Handle<RGBuffer> RenderGraphBuilder::WriteBuffer(Handle<RGBuffer> bufferHandle, const BufferSubregion& subregion, BufferUsage usage, PipelineStage stage)
    {
        return UseBufferInternal(bufferHandle, m_rg->m_bufferPool, Access::Write, subregion, usage, stage);
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

    Handle<RGImage> RenderGraphBuilder::UseImageInternal(Handle<RGImage> handle, HandlePool<RGImage>& pool, Access access, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        TL_ASSERT(m_rg->CheckHandleIsValid(handle));

        RGImage* resource = pool.Get(handle);

        if (resource->m_producer == nullptr)
        {
            TL_ASSERT(access & Access::Write);
            resource->m_producer = m_pass;
        }

        // Add dependency from the resource's producer to this pass
        if (resource->m_producer && resource->m_producer != m_pass)
            m_rg->AddDependency(resource->m_producer, m_pass);

        // Add resource dependency to the pass
        m_rg->ExtendImageUsage(resource, usage);
        RGImageDependency dep{
            .image  = handle,
            .viewID = 0,
            .state  = ImageBarrierState{usage, stage, access},
        };
        m_pass->m_imageDependencies.push_back(dep);

        // If writing, invalidate the handle and create a new version
        if (access & Access::Write)
            return m_rg->CreateRGImageHandle(handle, m_pass);

        return handle;
    }

    Handle<RGBuffer> RenderGraphBuilder::UseBufferInternal(Handle<RGBuffer> handle, HandlePool<RGBuffer>& pool, Access access, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage)
    {
        TL_ASSERT(m_rg->m_state.frameRecording);
        TL_ASSERT(m_rg->CheckHandleIsValid(handle));

        RGBuffer* resource = pool.Get(handle);

        if (resource->m_producer == nullptr)
        {
            // TL_ASSERT(access & Access::Write);
            resource->m_producer = m_pass;
        }

        // Add dependency from the resource's producer to this pass
        if (resource->m_producer && resource->m_producer != m_pass)
            m_rg->AddDependency(resource->m_producer, m_pass);

        // Add resource dependency to the pass
        m_rg->ExtendBufferUsage(resource, usage);
        RGBufferDependency dep{
            .buffer    = handle,
            .subregion = subresource,
            .state     = BufferBarrierState{usage, stage, access},
        };
        m_pass->m_bufferDependencies.push_back(dep);

        // If writing, invalidate the handle and create a new version
        // if (access & Access::Write)
        //     return m_rg->CreateRGBufferHandle(handle, m_pass);

        return handle;
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
        if (m_compileCallback)
        {
            m_compileCallback(context);
        }
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

    Handle<Image> RenderGraphResourcePool::InitTransientImage(RGImage* rgImage)
    {
        auto it = m_imageCache.find(rgImage->m_name);

        if (rgImage->m_handle)
        {
            // Imported
            return rgImage->m_handle;
        }

        ImageCreateInfo ci{
            .name        = rgImage->m_name.c_str(),
            .usageFlags  = rgImage->m_desc.usageFlags,
            .type        = rgImage->m_desc.type,
            .size        = rgImage->m_desc.size,
            .format      = rgImage->m_desc.format,
            .sampleCount = rgImage->m_desc.sampleCount,
            .mipLevels   = rgImage->m_desc.mipLevels,
            .arrayCount  = rgImage->m_desc.arrayCount,
        };

        // First try to find a in cache
        if (it != m_imageCache.end())
        {
            const auto& [cachedCI, handle] = it->second;
            if (CompareImageCreateInfo(cachedCI, ci))
                return handle;

            // Resource with same name, but different properties were found, so recreate the resource.
            TL_LOG_INFO("...Recreating resource {}", rgImage->m_name);
            m_device->DestroyImage(handle);
            m_imageCache.erase(it); // No need (I think) as
        }

        TL_LOG_INFO("Creating image {}", Debug::ToString(ci));

        auto [_, handle] = m_imageCache[rgImage->m_name] = std::pair{ci, m_device->CreateImage(ci)};
        return handle;
    }

    Handle<Buffer> RenderGraphResourcePool::InitTransientBuffer(RGBuffer* rgBuffer)
    {
        auto it = m_bufferCache.find(rgBuffer->m_name);

        if (rgBuffer->m_handle)
        {
            // Imported
            return rgBuffer->m_handle;
        }

        BufferCreateInfo ci{
            .name       = rgBuffer->m_name.c_str(),
            .hostMapped = true, // TODO: Should be I don't care?
            .usageFlags = rgBuffer->m_desc.usageFlags,
            .byteSize   = rgBuffer->m_desc.size,
        };

        if (it != m_bufferCache.end())
        {
            const auto& [cachedCI, handle] = it->second;
            if (CompareBufferCreateInfo(cachedCI, ci))
                return handle;

            // Resource with same name, but different properties were found, so recreate the resource.
            TL_LOG_INFO("...Recreating resource {}", rgBuffer->m_name);
            m_device->DestroyBuffer(handle);
            m_bufferCache.erase(it);
        }

        TL_LOG_INFO("Creating buffer {}", Debug::ToString(ci));

        auto [_, handle] = m_bufferCache[rgBuffer->m_name] = std::pair{ci, m_device->CreateBuffer(ci)};
        return handle;
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
        m_resourcePool  = TL::CreatePtr<RenderGraphResourcePool>();

        auto result = m_resourcePool->Init(device);

        return result;
    }

    void RenderGraph::Shutdown()
    {
        m_resourcePool->Shutdown();

        // Only delete m_allocator if you own it!
        // delete m_allocator;
        delete m_tempAllocator;
        // Ensure all pools free their objects
        m_passPool.clear();   // If TL::Ptr is not unique_ptr, manually delete
        m_imagePool.Clear();  // Ensure this frees all images
        m_bufferPool.Clear(); // Ensure this frees all buffers
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
                        auto img = m_imagePool.Get(dep.image);
                        output += std::format("    Image: {} | Usage: {}\n", img->m_name, Debug::ToString(dep.state.usage));
                    }
                    // Print buffer transitions
                    for (const auto& dep : pass->m_bufferDependencies)
                    {
                        auto buf = m_bufferPool.Get(dep.buffer);
                        output += std::format("    Buffer: {} | Usage: {}\n", buf->m_name, Debug::ToString(dep.state.usage));
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
            m_imagePool.Clear();
            m_bufferPool.Clear();
            m_imageList.clear();
            m_bufferList.clear();
            m_dependencyLevels.clear();
            m_dependencyLevels.clear();
            // Finally collect temp arena allocations
            m_tempAllocator->Collect();
            memset(&m_state, 0, sizeof(State));
        }

        m_frameIndex++;
    }

    Handle<RGImage> RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain, Format format)
    {
        TL_ASSERT(m_state.frameRecording == true);
        m_swapchain = &swapchain;
        auto handle = m_imagePool.Emplace(RGImage(name, swapchain.GetImage(), format));
        m_imageList.push_back(handle);
        return handle;
    }

    Handle<RGImage> RenderGraph::ImportImage(const char* name, Handle<Image> image, Format format)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto handle                           = m_imagePool.Emplace(RGImage(name, image, format));
        m_imagePool.Get(handle)->m_isImported = true;
        m_imageList.push_back(handle);
        return handle;
    }

    Handle<RGBuffer> RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto handle                            = m_bufferPool.Emplace(RGBuffer(name, buffer));
        m_bufferPool.Get(handle)->m_isImported = true;
        m_bufferList.push_back(handle);
        return handle;
    }

    Handle<RGImage> RenderGraph::CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto handle = m_imagePool.Emplace(RGImage(name, type, size, format, mipLevels, arrayCount, samples));
        m_imageList.push_back(handle);
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
        m_bufferList.push_back(handle);
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

    Handle<Image> RenderGraph::GetImageHandle(Handle<RGImage> handle) const
    {
        TL_ASSERT(m_state.compiled);
        auto image = m_imagePool.Get(handle);
        TL_ASSERT(image->m_handle);
        return image->m_handle;
    }

    Handle<Buffer> RenderGraph::GetBufferHandle(Handle<RGBuffer> handle) const
    {
        TL_ASSERT(m_state.compiled);
        auto buffer = m_bufferPool.Get(handle);
        TL_ASSERT(buffer->m_handle);
        return buffer->m_handle;
    }

    bool RenderGraph::CheckHandleIsValid(Handle<RGImage> imageHandle) const
    {
        return m_imagePool.Get(imageHandle)->m_isValid;
    }

    bool RenderGraph::CheckHandleIsValid(Handle<RGBuffer> bufferHandle) const
    {
        return m_bufferPool.Get(bufferHandle)->m_isValid;
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

        TL::Vector<TL::Vector<uint32_t>> adjacencyLists(m_passPool.size());
        BuildAdjacencyLists(adjacencyLists);

        TL::Vector<uint32_t> sortedPasses;
        TopologicalSort(adjacencyLists, sortedPasses);

        uint32_t detectedQueueCount = 0;
        BuildDependencyLevels(sortedPasses, adjacencyLists, m_dependencyLevels, detectedQueueCount);

        CreateTransientResources();

        // {
        //     ZoneScopedN("Compile Pass Resources");
        //     for (auto& pass : m_passPool)
        //     {
        //         size_t hash = 0;
        //         for (auto& dep : pass->m_imageDependencies)
        //         {
        //             auto rgImage = m_imagePool.Get(dep.image);
        //             hash         = TL::HashCombine(hash, rgImage->GetHash());
        //         }
        //         for (auto& dep : pass->m_bufferDependencies)
        //         {
        //             auto rgBuffer = m_bufferPool.Get(dep.buffer);
        //             hash          = TL::HashCombine(hash, rgBuffer->GetHash());
        //         }
        //         auto hashValue = m_passHashMap[pass->GetName()];
        //         if (hashValue != hash)
        //         {
        //             m_passHashMap[pass->GetName()] = hash;
        //             RenderGraphContext context(this, pass.get());
        //             pass->Compile(context);
        //         }
        //     }
        // }

        PassBuildBarriers();

        m_state.compiled = true;
    }

    void RenderGraph::BuildAdjacencyLists(TL::Vector<TL::Vector<uint32_t>>& adjacencyLists)
    {
        for (size_t nodeIdx = 0; nodeIdx < m_passPool.size(); ++nodeIdx)
        {
            auto  pass                = m_passPool[nodeIdx].get();
            auto& adjacentNodeIndices = adjacencyLists[nodeIdx];
            for (uint32_t otherNodeIdx = 0; otherNodeIdx < m_passPool.size(); ++otherNodeIdx)
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

    void RenderGraph::CreateTransientResources()
    {
        ZoneScoped;

        for (auto image : m_imageList)
        {
            auto rgImage = m_imagePool.Get(image);
            if (rgImage->m_isImported)
                continue;

            rgImage->m_handle = m_resourcePool->InitTransientImage(rgImage);
            for (auto before = rgImage->m_prevHandle; before != NullHandle; before = m_imagePool.Get(before)->m_prevHandle)
            {
                auto beforeImage      = m_imagePool.Get(before);
                beforeImage->m_handle = rgImage->m_handle;
            }
        }

        for (auto buffer : m_bufferList)
        {
            auto rgBuffer = m_bufferPool.Get(buffer);
            if (rgBuffer->m_isImported)
                continue;

            rgBuffer->m_handle = m_resourcePool->InitTransientBuffer(rgBuffer);
            for (auto before = rgBuffer->m_prevHandle; before != NullHandle; before = m_bufferPool.Get(before)->m_prevHandle)
            {
                auto beforeImage      = m_bufferPool.Get(before);
                beforeImage->m_handle = rgBuffer->m_handle;
            }
        }
    }

    void RenderGraph::PassBuildBarriers()
    {
        ZoneScoped;

        auto TransitionImageResource = [this](RGPass* pass, RGImageDependency dep)
        {
            auto resource = m_imagePool.Get(dep.image);

            if (resource->m_activeState == dep.state)
            {
                return;
            }

            auto& passImageBarriers = pass->m_barriers[0].imageBarriers;

            passImageBarriers.push_back({
                .image    = resource->m_handle,
                .srcState = resource->m_activeState,
                .dstState = dep.state,
            });

            resource->m_activeState = dep.state;
        };

        auto TransitionBufferResource = [this](RGPass* pass, RGBufferDependency dep)
        {
            auto resource = m_bufferPool.Get(dep.buffer);

            if (resource->m_activeState == dep.state)
            {
                return;
            }

            auto& passBufferBarriers = pass->m_barriers[0].bufferBarriers;

            passBufferBarriers.push_back({
                .buffer   = resource->m_handle,
                .srcState = resource->m_activeState,
                .dstState = dep.state,
            });

            resource->m_activeState = dep.state;
        };

        for (auto level : m_dependencyLevels)
        {
            for (auto pass : level.GetPasses())
            {
                if (pass->m_state.culled)
                    continue;

                for (auto dep : pass->m_imageDependencies)
                {
                    TransitionImageResource(pass, dep);
                }
                for (auto dep : pass->m_bufferDependencies)
                {
                    TransitionBufferResource(pass, dep);
                }
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

        if (m_swapchain)
        {
            // FIXME: This is hardcoded for now
            RHI::ImageBarrierInfo barrier{
                .image    = m_swapchain->GetImage(),
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

        QueueSubmitInfo submitInfo{
            .queueType            = QueueType::Graphics,
            .commandLists         = commandList,
            .signalStage          = PipelineStage::BottomOfPipe,
            .waitInfos            = {},
            .m_swapchainToAcquire = m_swapchain,
            .m_swapchainToSignal  = m_swapchain,
        };
        TL_MAYBE_UNUSED auto timeline = frame->QueueSubmit(submitInfo);

        m_frameIndex = frame->End();
#endif

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

        const auto& [postbarriers, postImageBarriers, postBufferBarriers] = pass->m_barriers[RGPass::Epilogue];
        commandList->AddPipelineBarrier(postbarriers, postImageBarriers, postBufferBarriers);
        commandList->PopDebugMarker();
    }

    void RenderGraph::ExtendImageUsage(RGImage* imageBefore, ImageUsage usage)
    {
        // Add the new usage flag to the image's usageFlags
        imageBefore->m_desc.usageFlags |= usage;

        for (auto before = imageBefore->m_prevHandle; before != NullHandle; before = m_imagePool.Get(before)->m_prevHandle)
        {
            auto beforeImage = m_imagePool.Get(before);
            beforeImage->m_desc.usageFlags |= usage;
        }
    }

    void RenderGraph::ExtendBufferUsage(RGBuffer* bufferBefore, BufferUsage usage)
    {
        // Add the new usage flag to the buffer's usageFlags
        bufferBefore->m_desc.usageFlags |= usage;

        for (auto before = bufferBefore->m_prevHandle; before != NullHandle; before = m_bufferPool.Get(before)->m_prevHandle)
        {
            auto beforeBuffer = m_bufferPool.Get(before);
            beforeBuffer->m_desc.usageFlags |= usage;
        }
    }

    Handle<RGImage> RenderGraph::CreateRGImageHandle(Handle<RGImage> imageBeforeHandle, RGPass* producer)
    {
        auto imageBefore = m_imagePool.Get(imageBeforeHandle);

        // Create a new RGImage as a versioned copy of imageBefore, update producer
        auto     handle           = m_imagePool.Emplace(RGImage(
            imageBefore->m_name.c_str(),
            imageBefore->m_desc.type,
            imageBefore->m_desc.size,
            imageBefore->m_desc.format,
            imageBefore->m_desc.mipLevels,
            imageBefore->m_desc.arrayCount,
            imageBefore->m_desc.sampleCount));
        RGImage* newImage         = m_imagePool.Get(handle);
        newImage->m_producer      = producer;
        newImage->m_isImported    = imageBefore->m_isImported;
        newImage->m_prevHandle    = imageBefore->m_prevHandle;
        newImage->m_nextHandle    = {};
        newImage->m_state         = imageBefore->m_state;
        newImage->m_activeState   = imageBefore->m_activeState;
        // Link version chain
        imageBefore->m_nextHandle = handle;
        newImage->m_prevHandle    = imageBeforeHandle;
        return handle;
    }

    Handle<RGBuffer> RenderGraph::CreateRGBufferHandle(Handle<RGBuffer> bufferBeforeHandle, RGPass* producer)
    {
        auto bufferBefore = m_bufferPool.Get(bufferBeforeHandle);

        // Create a new RGBuffer as a versioned copy of bufferBefore, update producer
        auto      handle           = m_bufferPool.Emplace(RGBuffer(
            bufferBefore->m_name.c_str(),
            bufferBefore->m_desc.size));
        RGBuffer* newBuffer        = m_bufferPool.Get(handle);
        newBuffer->m_producer      = producer;
        newBuffer->m_isImported    = bufferBefore->m_isImported;
        newBuffer->m_prevHandle    = bufferBefore->m_prevHandle;
        newBuffer->m_nextHandle    = {};
        newBuffer->m_state         = bufferBefore->m_state;
        newBuffer->m_activeState   = bufferBefore->m_activeState;
        // Link version chain
        bufferBefore->m_nextHandle = handle;
        newBuffer->m_prevHandle    = bufferBeforeHandle;
        return handle;
    }
} // namespace RHI