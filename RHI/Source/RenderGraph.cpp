#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>

#include <cstdint>
#include <tracy/Tracy.hpp>

namespace RHI
{
    inline static RHI::Access LoadStoreToAccess(LoadOperation loadOp, StoreOperation storeOp)
    {
        if (loadOp == LoadOperation::Load && storeOp == StoreOperation::Store)
        {
            return RHI::Access::ReadWrite;
        }
        else if (loadOp == LoadOperation::Load && storeOp != StoreOperation::Store)
        {
            return RHI::Access::Read;
        }
        else
        {
            return RHI::Access::Write;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Pass
    ///////////////////////////////////////////////////////////////////////////

    ResultCode Pass::Init(RenderGraph* rg, const PassCreateInfo& ci)
    {
        m_renderGraph     = rg;
        m_name            = ci.name;
        m_type            = ci.type;
        m_setupCallback   = ci.setupCallback;
        m_compileCallback = ci.compileCallback;
        m_executeCallback = ci.executeCallback;
        m_imageSize       = ci.size;
        m_isCompiled      = false;
        return ResultCode::Success;
    }

    void Pass::Shutdown()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    ResultCode RenderGraph::Init(const RenderGraphCreateInfo& ci)
    {
        m_allocator = (TL::IAllocator*)ci.allocator;
        return ResultCode::Success;
    }

    void RenderGraph::Shutdown()
    {
        delete m_allocator;
    }

    Handle<RenderGraphImage> RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain, Format format)
    {
        if (auto handle = FindImportedImage(swapchain.GetImage()))
            return handle;

        auto [imageHandle, result] = m_imageOwner.Create(name, swapchain.GetImage(), format);
        TL_ASSERT(IsSuccess(result));
        m_graphImportedSwapchainsLookup[&swapchain]       = imageHandle;
        m_graphImportedImagesLookup[swapchain.GetImage()] = imageHandle;
        return imageHandle;
    }

    Handle<RenderGraphImage> RenderGraph::ImportImage(const char* name, Handle<Image> image, Format format)
    {
        if (auto handle = FindImportedImage(image))
            return handle;

        auto [imageHandle, result] = m_imageOwner.Create(name, image, format);
        TL_ASSERT(IsSuccess(result));
        m_graphImportedImagesLookup[image] = imageHandle;
        return imageHandle;
    }

    Handle<RenderGraphBuffer> RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        if (auto handle = FindImportedBuffer(buffer))
            return handle;

        auto [bufferHandle, result] = m_bufferOwner.Create(name, buffer);
        TL_ASSERT(IsSuccess(result));
        m_graphImportedBuffersLookup[buffer] = bufferHandle;
        return bufferHandle;
    }

    Handle<RenderGraphImage> RenderGraph::CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples)
    {
        auto [image, result] = m_imageOwner.Create(name, type, size, format, mipLevels, arrayCount, samples);
        TL_ASSERT(IsSuccess(result));

        // m_graphImportedImagesLookup[image] = imageHandle;

        return image;
    }

    Handle<RenderGraphBuffer> RenderGraph::CreateBuffer(const char* name, size_t size)
    {
        auto [buffer, result] = m_bufferOwner.Create(name, size);
        TL_ASSERT(IsSuccess(result));

        // m_graphImportedBuffersLookup[buffer] = bufferHandle;

        return buffer;
    }

    Handle<RenderGraphImage> RenderGraph::FindImportedImage(Handle<Image> image) const
    {
        if (auto it = m_graphImportedImagesLookup.find(image); it != m_graphImportedImagesLookup.end())
            return it->second;
        return NullHandle;
    }

    Handle<RenderGraphBuffer> RenderGraph::FindImportedBuffer(Handle<Buffer> buffer) const
    {
        if (auto it = m_graphImportedBuffersLookup.find(buffer); it != m_graphImportedBuffersLookup.end())
            return it->second;
        return NullHandle;
    }

    void RenderGraph::DestroyImage(Handle<RenderGraphImage> handle)
    {
        // To destroy an image, the image must no be used by any pass
    }

    void RenderGraph::DestroyBuffer(Handle<RenderGraphBuffer> handle)
    {
    }

    void RenderGraph::BeginFrame(ImageSize2D frameSize)
    {
        TL_ASSERT(m_isCompiled == true);
        TL_ASSERT(m_isExecuting == false);
        m_isExecuting = true;

        m_frameSize = frameSize;
    }

    void RenderGraph::EndFrame()
    {
        TL_ASSERT(m_isExecuting == true);
        m_isExecuting = false;

        if (m_isCompiled == false)
        {
            Compile();
        }
    }

    // ImageSize2D RenderGraph::GetFrameSize() const
    // {
    //     TL_ASSERT(m_isExecuting);
    //     return m_frameSize;
    // }

    // const RenderGraphImage* RenderGraph::GetImage(Handle<RenderGraphImage> handle) const
    // {
    //     TL_ASSERT(m_isCompiled == true);
    //     return m_imageOwner.Get(handle);
    // }

    // const RenderGraphBuffer* RenderGraph::GetBuffer(Handle<RenderGraphBuffer> handle) const
    // {
    //     TL_ASSERT(m_isCompiled == true);
    //     return m_bufferOwner.Get(handle);
    // }

    // Handle<Image> RenderGraph::GetImageHandle(Handle<RenderGraphImage> handle) const
    // {
    //     TL_ASSERT(m_isCompiled == true);
    //     return m_imageOwner.Get(handle)->m_handle;
    // }

    // Handle<Buffer> RenderGraph::GetBufferHandle(Handle<RenderGraphBuffer> handle) const
    // {
    //     TL_ASSERT(m_isCompiled == true);
    //     return m_bufferOwner.Get(handle)->m_handle;
    // }

    // Pass* RenderGraph::AddPass(const PassCreateInfo& createInfo)
    // {
    //     auto pass   = m_graphPasses.emplace_back(TL::CreatePtr<Pass>()).get();
    //     auto result = pass->Init(this, createInfo);
    //     pass->m_setupCallback(*pass);
    //     TL_ASSERT(result == ResultCode::Success);
    //     return pass;
    // }

    // void RenderGraph::QueueBufferRead(Handle<RenderGraphBuffer> buffer, uint32_t offset, TL::Block data)
    // {
    // }

    // void RenderGraph::QueueBufferWrite(Handle<RenderGraphBuffer> buffer, uint32_t offset, TL::Block data)
    // {
    // }

    // void RenderGraph::QueueImageRead(Handle<RenderGraphImage> image, ImageOffset3D offset, ImageSize3D size, ImageSubresourceLayers dstLayers, TL::Block block)
    // {
    // }

    // void RenderGraph::QueueImageWrite(Handle<RenderGraphImage> image, ImageOffset3D offset, ImageSize3D size, ImageSubresourceLayers dstLayers, TL::Block block)
    // {
    // }

    void RenderGraph::TransientResourcesInit(bool enableAliasing)
    {
        TransientResourcesShutdown();

        if (enableAliasing)
        {
            TL_UNREACHABLE();
        }
        else
        {
            for (auto transientImageHandle : m_graphTransientImagesLookup)
            {
                auto image            = m_imageOwner.Get(transientImageHandle);
                auto [handle, result] = m_device->CreateImage({
                    .name        = image->m_name.c_str(),
                    .usageFlags  = image->m_usageFlags,
                    .type        = image->m_type,
                    .size        = image->m_size,
                    .format      = image->m_format,
                    .sampleCount = image->m_sampleCount,
                    .mipLevels   = image->m_mipLevels,
                    .arrayCount  = image->m_arrayCount,
                });
                TL_ASSERT(result == ResultCode::Success);
                image->m_handle = handle;
            }

            for (auto transientBufferHandle : m_graphTransientBuffersLookup)
            {
                auto buffer           = m_bufferOwner.Get(transientBufferHandle);
                auto [handle, result] = m_device->CreateBuffer({
                    .name       = buffer->m_name.c_str(),
                    .hostMapped = true,
                    .usageFlags = buffer->m_usageFlags,
                    .byteSize   = buffer->m_size,
                });
                TL_ASSERT(result == ResultCode::Success);
                buffer->m_handle = handle;
            }
        }

        for (auto transientImageHandle : m_graphTransientImagesLookup)
        {
            auto image = m_imageOwner.Get(transientImageHandle);
            for (auto it = image->m_begin; it != nullptr; it = it->next)
            {
                ImageViewCreateInfo viewCI{};
                if (auto handle = image->m_views.find(viewCI); handle != image->m_views.end())
                {
                    it->view = image->m_views[viewCI] = m_device->CreateImageView(viewCI);
                }
                else
                {
                    it->view = handle->second;
                }
            }
        }
    }

    void RenderGraph::TransientResourcesShutdown()
    {
        for (auto transientImageHandle : m_graphTransientImagesLookup)
        {
            auto image = m_imageOwner.Get(transientImageHandle);
            m_device->DestroyImage(image->m_handle);
        }
        for (auto transientBufferHandle : m_graphTransientBuffersLookup)
        {
            auto buffer = m_bufferOwner.Get(transientBufferHandle);
            m_device->DestroyBuffer(buffer->m_handle);
        }
    }

    struct Node
    {
        uint32_t          mIndexInUnorderedList;
        Pass*             pass;
        TL::Vector<Pass*> producers; // list of passes that this pass will depend on for reading or writing or whatever
    };

    inline static void DepthFirstSearch(
        uint64_t                                nodeIndex,
        TL::Vector<bool>&                       visited,
        TL::Vector<bool>&                       onStack,
        bool&                                   isCyclic,
        const TL::Vector<Node*>&                passNodes,
        const TL::Vector<TL::Vector<uint64_t>>& adjacencyLists,
        TL::Vector<Node*>&                      topologicallySortedNodes)
    {
        if (isCyclic) return;
        visited[nodeIndex]          = true;
        onStack[nodeIndex]          = true;
        uint64_t adjacencyListIndex = passNodes[nodeIndex]->mIndexInUnorderedList;
        for (uint64_t neighbour : adjacencyLists[adjacencyListIndex])
        {
            if (visited[neighbour] && onStack[neighbour])
            {
                isCyclic = true;
                return;
            }
            if (!visited[neighbour])
            {
                DepthFirstSearch(neighbour, visited, onStack, isCyclic, passNodes, adjacencyLists, topologicallySortedNodes);
            }
        }
        onStack[nodeIndex] = false;
        topologicallySortedNodes.push_back(passNodes[nodeIndex]);
    }

    void RenderGraph::Compile()
    {
        ZoneScoped;

        TL::Vector<TL::Vector<uint32_t>> adjacencyLists;
        TL::Vector<Node*>                topologicallySortedNodes;

        auto BuildAdjacencyLists = [this](const TL::Vector<Node>& passNodes, TL::Vector<TL::Vector<uint64_t>>& adjacencyLists)
        {
            adjacencyLists.resize(passNodes.size());
            for (size_t nodeIdx = 0; nodeIdx < passNodes.size(); ++nodeIdx)
            {
                auto& node                = passNodes[nodeIdx];
                auto& adjacentNodeIndices = adjacencyLists[nodeIdx];

                for (size_t otherNodeIdx = 0; otherNodeIdx < passNodes.size(); ++otherNodeIdx)
                {
                    // Do not check dependencies on itself
                    if (nodeIdx == otherNodeIdx) continue;

                    auto& otherNode = passNodes[otherNodeIdx];
                    if (CheckDependency(&node, &otherNode))
                    {
                        adjacentNodeIndices.push_back(otherNodeIdx);
                    }
                }
            }
        };

        auto TopologicalSort = [this](TL::Vector<Node*>& topologicallySortedNodes)
        {
            TL::Vector<bool> visitedNodes(m_graphPasses.size(), false);
            TL::Vector<bool> onStackNodes(m_graphPasses.size(), false);
            bool             isCyclic = false;
            for (auto nodeIndex = 0; nodeIndex < m_graphPasses.size(); ++nodeIndex)
            {
                if (!visitedNodes[nodeIndex])
                {
                    DepthFirstSearch(nodeIndex, visitedNodes, onStackNodes, isCyclic, {}, {}, topologicallySortedNodes);
                    // TL_ASSERT(!isCyclic, "Detected cyclic dependency in pass: ", m_graphPasses[nodeIndex].PassMetadata().Name.ToString());
                }
            }
            std::reverse(topologicallySortedNodes.begin(), topologicallySortedNodes.end());
        };

        auto BuildDependencyLevels = [](TL::Vector<Node*>& topologicallySortedNodes)
        {
            TL::Vector<int64_t> longestDistances(topologicallySortedNodes.size(), 0);

            uint64_t dependencyLevelCount = 1;

            // Perform longest node distance search
            for (auto nodeIndex = 0; nodeIndex < topologicallySortedNodes.size(); ++nodeIndex)
            {
                uint64_t originalIndex      = topologicallySortedNodes[nodeIndex]->mIndexInUnorderedList;
                uint64_t adjacencyListIndex = originalIndex;

                for (uint64_t adjacentNodeIndex : mAdjacencyLists[adjacencyListIndex])
                {
                    if (longestDistances[adjacentNodeIndex] < longestDistances[originalIndex] + 1)
                    {
                        int64_t newLongestDistance          = longestDistances[originalIndex] + 1;
                        longestDistances[adjacentNodeIndex] = newLongestDistance;
                        dependencyLevelCount                = std::max(uint64_t(newLongestDistance + 1), dependencyLevelCount);
                    }
                }
            }

            mDependencyLevels.resize(dependencyLevelCount);
            mDetectedQueueCount = 1;

            // Dispatch nodes to corresponding dependency levels.
            // Iterate through unordered nodes because adjacency lists contain indices to
            // initial unordered list of nodes and longest distances also correspond to them.
            for (auto nodeIndex = 0; nodeIndex < mPassNodes.size(); ++nodeIndex)
            {
                Node&            node            = mPassNodes[nodeIndex];
                uint64_t         levelIndex      = longestDistances[nodeIndex];
                DependencyLevel& dependencyLevel = mDependencyLevels[levelIndex];
                dependencyLevel.mLevelIndex      = levelIndex;
                dependencyLevel.AddNode(&node);
                node.mDependencyLevelIndex = levelIndex;
                mDetectedQueueCount        = std::max(mDetectedQueueCount, node.ExecutionQueueIndex + 1);
            }
        };

#if 0
        auto CullRedundantSynchronizations = []()
        {
            // Initialize synchronization index sets
            for (Node& node : mPassNodes)
            {
                node.mSynchronizationIndexSet.resize(mDetectedQueueCount, Node::InvalidSynchronizationIndex);
            }

            for (DependencyLevel& dependencyLevel : mDependencyLevels)
            {
                // First pass: find closest nodes to sync with, compute initial SSIS (sufficient synchronization index set)
                for (Node* node : dependencyLevel.mNodes)
                {
                    // Closest node to sync with on each queue
                    TL::Vector<const Node*> closestNodesToSyncWith{mDetectedQueueCount, nullptr};

                    // Find closest dependencies from other queues for the current node
                    for (const Node* dependencyNode : node->mNodesToSyncWith)
                    {
                        const Node* closestNode = closestNodesToSyncWith[dependencyNode->ExecutionQueueIndex];

                        if (!closestNode || dependencyNode->LocalToQueueExecutionIndex() > closestNode->LocalToQueueExecutionIndex())
                        {
                            closestNodesToSyncWith[dependencyNode->ExecutionQueueIndex] = dependencyNode;
                        }
                    }

                    // Get rid of nodes to sync that may have had redundancies
                    node->mNodesToSyncWith.clear();

                    for (const Node* closestNode : closestNodesToSyncWith)
                    {
                        if (!closestNode)
                        {
                            continue;
                        }

                        // Update SSIS using closest nodes' indices
                        if (closestNode->ExecutionQueueIndex != node->ExecutionQueueIndex)
                        {
                            node->mSynchronizationIndexSet[closestNode->ExecutionQueueIndex] = closestNode->LocalToQueueExecutionIndex();
                        }

                        // Store only closest nodes to sync with
                        node->mNodesToSyncWith.push_back(closestNode);
                    }

                    // Use node's execution index as synchronization index on its own queue
                    node->mSynchronizationIndexSet[node->ExecutionQueueIndex] = node->LocalToQueueExecutionIndex();
                }

                // Second pass: cull redundant dependencies by searching for indirect synchronizations
                for (Node* node : dependencyLevel.mNodes)
                {
                    // Keep track of queues we still need to sync with
                    std::unordered_set<uint64_t> queueToSyncWithIndices;

                    // Store nodes and queue syncs they cover
                    TL::Vector<SyncCoverage> syncCoverageArray;

                    // Final optimized list of nodes without redundant dependencies
                    TL::Vector<const Node*> optimalNodesToSyncWith;

                    for (const Node* nodeToSyncWith : node->mNodesToSyncWith)
                    {
                        queueToSyncWithIndices.insert(nodeToSyncWith->ExecutionQueueIndex);
                    }

                    while (!queueToSyncWithIndices.empty())
                    {
                        uint64_t maxNumberOfSyncsCoveredBySingleNode = 0;

                        for (auto dependencyNodeIdx = 0u; dependencyNodeIdx < node->mNodesToSyncWith.size(); ++dependencyNodeIdx)
                        {
                            const Node* dependencyNode = node->mNodesToSyncWith[dependencyNodeIdx];

                            // Take a dependency node and check how many queues we would sync with
                            // if we would only sync with this one node. We very well may encounter a case
                            // where by synchronizing with just one node we will sync with more then one queue
                            // or even all of them through indirect synchronizations,
                            // which will make other synchronizations previously detected for this node redundant.

                            TL::Vector<uint64_t> syncedQueueIndices;

                            for (uint64_t queueIndex : queueToSyncWithIndices)
                            {
                                uint64_t currentNodeDesiredSyncIndex = node->mSynchronizationIndexSet[queueIndex];
                                uint64_t dependencyNodeSyncIndex     = dependencyNode->mSynchronizationIndexSet[queueIndex];

                                assert_format(currentNodeDesiredSyncIndex != Node::InvalidSynchronizationIndex,
                                    "Bug! Node that wants to sync with some queue must have a valid sync index for that queue.");

                                if (queueIndex == node->ExecutionQueueIndex)
                                {
                                    currentNodeDesiredSyncIndex -= 1;
                                }

                                if (dependencyNodeSyncIndex != Node::InvalidSynchronizationIndex &&
                                    dependencyNodeSyncIndex >= currentNodeDesiredSyncIndex)
                                {
                                    syncedQueueIndices.push_back(queueIndex);
                                }
                            }

                            syncCoverageArray.emplace_back(SyncCoverage{dependencyNode, dependencyNodeIdx, syncedQueueIndices});
                            maxNumberOfSyncsCoveredBySingleNode = std::max(maxNumberOfSyncsCoveredBySingleNode, syncedQueueIndices.size());
                        }

                        for (const SyncCoverage& syncCoverage : syncCoverageArray)
                        {
                            auto coveredSyncCount = syncCoverage.SyncedQueueIndices.size();

                            if (coveredSyncCount >= maxNumberOfSyncsCoveredBySingleNode)
                            {
                                // Optimal list of synchronizations should not contain nodes from the same queue,
                                // because work on the same queue is synchronized automatically and implicitly
                                if (syncCoverage.NodeToSyncWith->ExecutionQueueIndex != node->ExecutionQueueIndex)
                                {
                                    optimalNodesToSyncWith.push_back(syncCoverage.NodeToSyncWith);

                                    // Update SSIS
                                    auto& index = node->mSynchronizationIndexSet[syncCoverage.NodeToSyncWith->ExecutionQueueIndex];
                                    index       = std::max(index, node->mSynchronizationIndexSet[syncCoverage.NodeToSyncWith->ExecutionQueueIndex]);
                                }

                                // Remove covered queues from the list of queues we need to sync with
                                for (uint64_t syncedQueueIndex : syncCoverage.SyncedQueueIndices)
                                {
                                    queueToSyncWithIndices.erase(syncedQueueIndex);
                                }
                            }
                        }

                        // Remove nodes that we synced with from the original list. Reverse iterating to avoid index invalidation.
                        for (auto syncCoverageIt = syncCoverageArray.rbegin(); syncCoverageIt != syncCoverageArray.rend(); ++syncCoverageIt)
                        {
                            node->mNodesToSyncWith.erase(node->mNodesToSyncWith.begin() + syncCoverageIt->NodeToSyncWithIndex);
                        }
                    }

                    // Finally, assign an optimal list of nodes to sync with to the current node
                    node->mNodesToSyncWith = optimalNodesToSyncWith;
                }
            }
        };

        auto GatherResourceTransitionKnowledge = [](const RenderPassGraph::DependencyLevel& dependencyLevel)
        {
            mDependencyLevelQueuesThatRequireTransitionRerouting = dependencyLevel.QueuesInvoledInCrossQueueResourceReads();

            bool backBufferTransitioned = false;

            for (const RenderPassGraph::Node* node : dependencyLevel.Nodes())
            {
                auto requestTransition = [&](RenderPassGraph::SubresourceName subresourceName, bool isReadDependency)
                {
                    auto [resourceName, subresourceIndex]         = mRenderPassGraph->DecodeSubresourceName(subresourceName);
                    PipelineResourceStorageResource* resourceData = mResourceStorage->GetPerResourceData(resourceName);

                    HAL::ResourceState newState = isReadDependency ? resourceData->SchedulingInfo.GetSubresourceCombinedReadStates(subresourceIndex) : resourceData->SchedulingInfo.GetSubresourceWriteState(subresourceIndex);

                    std::optional<HAL::ResourceTransitionBarrier> barrier =
                        mResourceStateTracker->TransitionToStateImmediately(resourceData->GetGPUResource()->HALResource(), newState, subresourceIndex, false);

                    if (node->ExecutionQueueIndex == 0 && !backBufferTransitioned)
                    {
                        std::optional<HAL::ResourceTransitionBarrier> backBufferBarrier =
                            mResourceStateTracker->TransitionToStateImmediately(mBackBuffer->HALResource(), HAL::ResourceState::RenderTarget, 0, false);

                        if (backBufferBarrier)
                        {
                            mDependencyLevelTransitionBarriers[node->LocalToDependencyLevelExecutionIndex()].push_back({0, *backBufferBarrier, mBackBuffer->HALResource()});
                        }

                        backBufferTransitioned = true;
                    }

                    // Redundant transition
                    if (!barrier)
                    {
                        return;
                    }

                    mDependencyLevelTransitionBarriers[node->LocalToDependencyLevelExecutionIndex()].push_back({subresourceName, *barrier, resourceData->GetGPUResource()->HALResource()});

                    // Another reason to reroute resource transitions into another queue is incompatibility
                    // of resource state transitions with receiving queue
                    if (!IsStateTransitionSupportedOnQueue(node->ExecutionQueueIndex, barrier->BeforeStates(), barrier->AfterStates()))
                    {
                        mDependencyLevelQueuesThatRequireTransitionRerouting.insert(node->ExecutionQueueIndex);
                    }
                };

                for (RenderPassGraph::SubresourceName subresourceName : node->ReadSubresources())
                {
                    requestTransition(subresourceName, true);
                }

                for (RenderPassGraph::SubresourceName subresourceName : node->WrittenSubresources())
                {
                    requestTransition(subresourceName, false);
                }

                for (Foundation::Name resourceName : node->AllResources())
                {
                    const PipelineResourceStorageResource*          resourceData = mResourceStorage->GetPerResourceData(resourceName);
                    const PipelineResourceSchedulingInfo::PassInfo* passInfo     = resourceData->SchedulingInfo.GetInfoForPass(node->PassMetadata().Name);

                    if (passInfo->NeedsAliasingBarrier)
                    {
                        mPerNodeAliasingBarriers[node->GlobalExecutionIndex()].AddBarrier(HAL::ResourceAliasingBarrier{nullptr, resourceData->GetGPUResource()->HALResource()});
                    }

                    if (passInfo->NeedsUnorderedAccessBarrier)
                    {
                        mPassHelpers[node->GlobalExecutionIndex()].UAVBarriers.AddBarrier(HAL::UnorderedAccessResourceBarrier{resourceData->GetGPUResource()->HALResource()});
                    }
                }
            }
        };

        auto BatchCommandListsWithTransitionRerouting = [](const RenderPassGraph::DependencyLevel& dependencyLevel)
        {
            if (mDependencyLevelQueuesThatRequireTransitionRerouting.empty())
            {
                return;
            }

            uint64_t mostCompetentQueueIndex                               = FindMostCompetentQueueIndex(mDependencyLevelQueuesThatRequireTransitionRerouting);
            mReroutedTransitionsCommandLists[dependencyLevel.LevelIndex()] = AllocateCommandListForQueue(mostCompetentQueueIndex);
            CommandListPtrVariant&       commandListVariant                = mReroutedTransitionsCommandLists[dependencyLevel.LevelIndex()];
            HAL::ComputeCommandListBase* transitionsCommandList            = GetComputeCommandListBase(commandListVariant);

            TL::Vector<CommandListBatch>& mostCompetentQueueBatches = mCommandListBatches[mostCompetentQueueIndex];
            CommandListBatch*             reroutedTransitionsBatch  = &mostCompetentQueueBatches.emplace_back();
            reroutedTransitionsBatch->FenceToSignal                 = &FenceForQueueIndex(mostCompetentQueueIndex);
            reroutedTransitionsBatch->CommandLists.emplace_back(GetHALCommandListVariant(commandListVariant));
            uint64_t reroutedTransitionsBatchIndex = mostCompetentQueueBatches.size() - 1;

            HAL::ResourceBarrierCollection reroutedTransitionBarrires;

            TL::Vector<CommandListBatch*> dependencyLevelPerQueueBatches{mQueueCount, nullptr};

            for (RenderPassGraph::Node::QueueIndex queueIndex : mDependencyLevelQueuesThatRequireTransitionRerouting)
            {
                // Make rerouted transitions wait for fences from involved queues
                if (queueIndex != mostCompetentQueueIndex)
                {
                    mostCompetentQueueBatches[reroutedTransitionsBatchIndex].FencesToWait.insert(&FenceForQueueIndex(queueIndex));
                }

                for (const RenderPassGraph::Node* node : dependencyLevel.NodesForQueue(queueIndex))
                {
                    // A special case of waiting for BVH build fence, if of course pass is not executed on the same queue as BVH build
                    if (mRenderPassGraph->FirstNodeThatUsesRayTracing() == node && mBVHBuildsQueueIndex != queueIndex)
                    {
                        mostCompetentQueueBatches[reroutedTransitionsBatchIndex].FencesToWait.insert(&FenceForQueueIndex(mBVHBuildsQueueIndex));
                    }

                    if (!dependencyLevelPerQueueBatches[node->ExecutionQueueIndex])
                    {
                        dependencyLevelPerQueueBatches[node->ExecutionQueueIndex] = &mCommandListBatches[node->ExecutionQueueIndex].emplace_back();
                    }

                    CommandListBatch* currentBatchInCurrentDependencyLevel = dependencyLevelPerQueueBatches[node->ExecutionQueueIndex];

                    // Make command lists in a batch wait for rerouted transitions
                    currentBatchInCurrentDependencyLevel->FencesToWait.insert(mostCompetentQueueBatches[reroutedTransitionsBatchIndex].FenceToSignal);
                    currentBatchInCurrentDependencyLevel->CommandLists.push_back(GetHALCommandListVariant(mPassCommandLists[node->GlobalExecutionIndex()].WorkCommandList));

                    uint64_t currentCommandListBatchIndex = mCommandListBatches[queueIndex].size() - 1;
                    CollectNodeTransitions(node, currentCommandListBatchIndex, reroutedTransitionBarrires);

                    if (node->IsSyncSignalRequired())
                    {
                        currentBatchInCurrentDependencyLevel->FenceToSignal       = &FenceForQueueIndex(node->ExecutionQueueIndex);
                        dependencyLevelPerQueueBatches[node->ExecutionQueueIndex] = &mCommandListBatches[node->ExecutionQueueIndex].emplace_back();
                    }
                }

                // Do not leave empty batches
                if (mCommandListBatches[queueIndex].back().CommandLists.empty())
                {
                    mCommandListBatches[queueIndex].pop_back();
                }
            }

            transitionsCommandList->InsertBarriers(reroutedTransitionBarrires);
        };

        auto CollectNodeTransitions = [](const RenderPassGraph::Node* node, uint64_t currentCommandListBatchIndex, HAL::ResourceBarrierCollection& collection)
        {
            const TL::Vector<SubresourceTransitionInfo>& nodeTransitionBarriers = mDependencyLevelTransitionBarriers[node->LocalToDependencyLevelExecutionIndex()];
            const HAL::ResourceBarrierCollection&        nodeAliasingBarriers   = mPerNodeAliasingBarriers[node->GlobalExecutionIndex()];

            collection.AddBarriers(nodeAliasingBarriers);

            for (const SubresourceTransitionInfo& transitionInfo : nodeTransitionBarriers)
            {
                auto previousTransitionInfoIt           = mSubresourcesPreviousTransitionInfo.find(transitionInfo.SubresourceName);
                bool foundPreviousTransition            = previousTransitionInfoIt != mSubresourcesPreviousTransitionInfo.end();
                bool subresourceTransitionedAtLeastOnce = foundPreviousTransition && previousTransitionInfoIt->second.CommandListBatchIndex == currentCommandListBatchIndex;

                if (!subresourceTransitionedAtLeastOnce)
                {
                    bool implicitTransitionPossible = Memory::ResourceStateTracker::CanResourceBeImplicitlyTransitioned(
                        *transitionInfo.Resource, transitionInfo.TransitionBarrier.BeforeStates(), transitionInfo.TransitionBarrier.AfterStates());

                    if (implicitTransitionPossible)
                    {
                        continue;
                    }
                }

                if (foundPreviousTransition)
                {
                    const SubresourcePreviousTransitionInfo& previousTransitionInfo = previousTransitionInfoIt->second;

                    // Split barrier is only possible when transmitting queue supports transitions for both before and after states
                    bool isSplitBarrierPossible = IsStateTransitionSupportedOnQueue(
                        previousTransitionInfo.Node->ExecutionQueueIndex, transitionInfo.TransitionBarrier.BeforeStates(), transitionInfo.TransitionBarrier.AfterStates());

                    // There is no sense in splitting barriers between two adjacent render passes.
                    // That will only double the amount of barriers without any performance gain.
                    bool currentNodeIsNextToPrevious = node->LocalToQueueExecutionIndex() - previousTransitionInfo.Node->LocalToQueueExecutionIndex() <= 1;

                    if (isSplitBarrierPossible && !currentNodeIsNextToPrevious)
                    {
                        auto [beginBarrier, endBarrier] = transitionInfo.TransitionBarrier.Split();
                        collection.AddBarrier(endBarrier);
                        mPerNodeBeginBarriers[previousTransitionInfo.Node->GlobalExecutionIndex()].AddBarrier(beginBarrier);
                    }
                    else
                    {
                        collection.AddBarrier(transitionInfo.TransitionBarrier);
                    }
                }
                else
                {
                    collection.AddBarrier(transitionInfo.TransitionBarrier);
                }

                mSubresourcesPreviousTransitionInfo[transitionInfo.SubresourceName] = {node, currentCommandListBatchIndex};
            }
        };

        auto ExetuteCommandLists = []()
        {
            // Run initial upload commands
            mGraphicsQueueFence.IncrementExpectedValue();
            // Transition uploaded resources to readable states
            mPreRenderUploadsCommandList->InsertBarriers(mResourceStateTracker->ApplyRequestedTransitions());
            mPreRenderUploadsCommandList->Close();
            mGraphicsQueue.ExecuteCommandList(*mPreRenderUploadsCommandList);
            mGraphicsQueue.SignalFence(mGraphicsQueueFence);

            // Wait for uploads, run RT AS builds
            mComputeQueueFence.IncrementExpectedValue();
            mComputeQueue.WaitFence(mGraphicsQueueFence);
            mRTASBuildsCommandList->Close();
            mComputeQueue.ExecuteCommandList(*mRTASBuildsCommandList);
            mComputeQueue.SignalFence(mComputeQueueFence);

            for (auto queueIdx = 0; queueIdx < mQueueCount; ++queueIdx)
            {
                TL::Vector<CommandListBatch>& batches = mCommandListBatches[queueIdx];

                for (auto batchIdx = 0; batchIdx < batches.size(); ++batchIdx)
                {
                    CommandListBatch&  batch = batches[batchIdx];
                    HAL::CommandQueue& queue = GetCommandQueue(queueIdx);

                    for (const HAL::Fence* fenceToWait : batch.FencesToWait)
                    {
                        queue.WaitFence(*fenceToWait);
                    }

                    if (RenderPassExecutionQueue{queueIdx} == RenderPassExecutionQueue::Graphics)
                    {
                        TL::Vector<HAL::GraphicsCommandList*> graphicsCommands;
                        HAL::GraphicsCommandQueue*            graphicsQueue = dynamic_cast<HAL::GraphicsCommandQueue*>(&queue);

                        for (auto cmdListIdx = 0; cmdListIdx < batch.CommandLists.size(); ++cmdListIdx)
                        {
                            HALCommandListPtrVariant& cmdListVariant = batch.CommandLists[cmdListIdx];
                            auto                      cmdList        = std::get<HAL::GraphicsCommandList*>(cmdListVariant);

                            bool isLastGraphicsCmdList = batchIdx == batches.size() - 1 && cmdListIdx == batch.CommandLists.size() - 1;

                            if (isLastGraphicsCmdList)
                            {
                                cmdList->InsertBarriers(mResourceStateTracker->TransitionToStateImmediately(mBackBuffer->HALResource(), HAL::ResourceState::Present));
                            }

                            cmdList->Close();
                            graphicsCommands.push_back(cmdList);
                        }

                        graphicsQueue->ExecuteCommandLists(graphicsCommands.data(), graphicsCommands.size());
                    }
                    else
                    {
                        TL::Vector<HAL::ComputeCommandList*> computeCommands;
                        HAL::ComputeCommandQueue*            computeQueue = dynamic_cast<HAL::ComputeCommandQueue*>(&queue);

                        for (HALCommandListPtrVariant& cmdListVariant : batch.CommandLists)
                        {
                            auto cmdList = std::get<HAL::ComputeCommandList*>(cmdListVariant);
                            cmdList->Close();
                            computeCommands.push_back(cmdList);
                        }

                        computeQueue->ExecuteCommandLists(computeCommands.data(), computeCommands.size());
                    }

                    if (batch.FenceToSignal)
                    {
                        batch.FenceToSignal->IncrementExpectedValue();
                        queue.SignalFence(*batch.FenceToSignal);
                    }
                }
            }
        };
#endif
    }

    void RenderGraph::Execute()
    {
        constexpr ClearValue queueColor[3] =
            {
                {.f32{0.0f, 0.5f, 0.0f, 1.0f}},
                {.f32{0.0f, 0.0f, 0.5f, 1.0f}},
                {.f32{0.5f, 0.0f, 0.0f, 1.0f}},
            };

        // for (auto& pass : m_graphPasses)
        // {
        //     // Only update deps of active passes
        //     if (pass->m_isActive == false)
        //         continue;

        //     for (auto imageDep : pass->m_imageDependencies)
        //     {
        //         ImageBarrierState prevState = GetBarrierStateInfo(imageDep);

        //         ImageBarrierInfo barrier{};
        //         pass->m_barriers[Pass::BarrierSlot::Prilogue].imageBarriers.push_back(barrier);
        //     }
        //     for (auto bufferDep : pass->m_bufferDependencies)
        //     {
        //         BufferBarrierState prevState = GetBarrierStateInfo(bufferDep);

        //         BufferBarrierInfo barrier{};
        //         pass->m_barriers[Pass::BarrierSlot::Prilogue].bufferBarriers.push_back(barrier);
        //     }
        // }

        auto executePass = [=, this](CommandList& commandList, Pass* pass)
        {
            commandList.PushDebugMarker(pass->GetName(), queueColor[(uint32_t)pass->GetType()]);

            commandList.AddPipelineBarrier(
                pass->m_barriers[Pass::BarrierSlot::Prilogue].memoryBarriers,
                pass->m_barriers[Pass::BarrierSlot::Prilogue].imageBarriers,
                pass->m_barriers[Pass::BarrierSlot::Prilogue].bufferBarriers);

            if (pass->GetType() == PassType::Graphics)
            {
                TL::Vector<ColorAttachment>          colorAttachments{};
                TL::Optional<DepthStencilAttachment> depthStencilAttachment{};

                for (auto colorAttachment : pass->m_colorAttachments)
                {
                    colorAttachments.push_back(
                        ColorAttachment{
                            .view        = GetImageHandle(colorAttachment.view),
                            .loadOp      = colorAttachment.loadOp,
                            .storeOp     = colorAttachment.storeOp,
                            .clearValue  = colorAttachment.clearValue,
                            .resolveMode = colorAttachment.resolveMode,
                            .resolveView = colorAttachment.resolveView ? GetImageHandle(colorAttachment.resolveView) : NullHandle,
                        });
                }

                if (pass->m_depthStencilAttachment)
                {
                    depthStencilAttachment = DepthStencilAttachment{
                        .view           = GetImageHandle(pass->m_depthStencilAttachment->view),
                        .depthLoadOp    = pass->m_depthStencilAttachment->depthLoadOp,
                        .depthStoreOp   = pass->m_depthStencilAttachment->depthStoreOp,
                        .stencilLoadOp  = pass->m_depthStencilAttachment->stencilLoadOp,
                        .stencilStoreOp = pass->m_depthStencilAttachment->stencilStoreOp,
                        .clearValue     = pass->m_depthStencilAttachment->clearValue,
                    };
                }

                RenderPassBeginInfo beginInfo{
                    .size                   = pass->GetImageSize(),
                    .offset                 = {},
                    .colorAttachments       = colorAttachments,
                    .depthStencilAttachment = depthStencilAttachment,
                };
                commandList.BeginRenderPass(beginInfo);
            }
            else if (pass->GetType() == PassType::Compute)
            {
                ComputePassBeginInfo beginInfo{};
                commandList.BeginComputePass(beginInfo);
            }

            pass->m_executeCallback(commandList);

            if (pass->GetType() == PassType::Graphics)
            {
                commandList.EndRenderPass();
            }
            else if (pass->GetType() == PassType::Compute)
            {
                commandList.EndComputePass();
            }

            commandList.AddPipelineBarrier(
                pass->m_barriers[Pass::BarrierSlot::Epilogue].memoryBarriers,
                pass->m_barriers[Pass::BarrierSlot::Epilogue].imageBarriers,
                pass->m_barriers[Pass::BarrierSlot::Epilogue].bufferBarriers);

            commandList.PopDebugMarker();
        };

        auto commandList = m_device->CreateCommandList({.name = "Frame", .queueType = QueueType::Graphics});
        for (auto& pass : m_graphPasses)
        {
            executePass(CommandList & commandList, pass.get());
        }
    }
} // namespace RHI