#pragma once
#include "RHI/CommandList.hpp"
#include "RHI/Export.hpp"
#include "RHI/Common.hpp"
#include "RHI/PipelineAccess.hpp"

#include <TL/Flags.hpp>
#include <TL/Containers.hpp>

namespace RHI
{
    /// @enum QueueType
    /// @brief Enumerates the types of queues available for rendering operations.
    enum class QueueType : uint8_t;

    class Device;
    class Pass;
    class Swapchain;

    struct AsyncQueueSyncInfo
    {
        uint64_t                 timelineValue;
        TL::Flags<PipelineStage> waitStage;
    };

    struct SwapchainSyncInfo
    {
        Swapchain*               swapchain;
        TL::Flags<PipelineStage> stage;
    };

    /// @class RenderGraphExecuteGroup
    /// @brief Interface for managing the execution of passes in a render graph.
    ///
    /// The RenderGraphExecuteGroup class handles the scheduling and synchronization of render passes
    /// and their dependencies, ensuring proper execution order and resource usage within the rendering pipeline.
    class RHI_EXPORT RenderGraphExecuteGroup
    {
    public:
        /// @brief Constructs a new render graph execution group.
        RenderGraphExecuteGroup(TL::IAllocator& allocator)
            : m_passList(allocator)
        {
        }

        /// @brief Adds a render pass to the execution group.
        ///
        /// @param pass   The render pass to be added.
        void AddPass(Pass& pass)
        {
            m_passList.push_back(&pass);
            // m_queueSignalInfo = pass
        }

        /// @brief Specifies a queue dependency that the group must wait for before proceeding.
        ///
        /// @param type          The type of queue to wait for.
        /// @param timelineValue The timeline semaphore value to wait until.
        /// @param waitStage     The pipeline stages where the wait operation should occur.
        void WaitForQueue(QueueType type, uint64_t timelineValue, TL::Flags<PipelineStage> waitStage)
        {
            m_queueWaitInfos[(uint32_t)type] = {timelineValue, waitStage};
        }

        /// @brief Specifies a swapchain dependency that the group must wait for before proceeding.
        ///
        /// @param swapchain   The swapchain to wait for.
        /// @param waitStage   The pipeline stages where the wait operation should occur.
        void WaitForSwapchain(Swapchain& swapchain, TL::Flags<PipelineStage> waitStage) { m_swapchainToWait = {&swapchain, waitStage}; }

        /// @brief Signals that the swapchain is ready for presentation.
        ///
        /// @param swapchain   The swapchain to signal for presentation.
        /// @param signalStage The pipeline stages where the signal operation should occur.
        void SignalSwapchainPresent(Swapchain& swapchain, TL::Flags<PipelineStage> signalStage)
        {
            m_swapchainToSignal = {&swapchain, signalStage};
        }

        TL::Span<Pass* const> GetPassList() const { return m_passList; }

        AsyncQueueSyncInfo    GetQueueSignalInfo() const { return m_queueSignalInfo; }

        AsyncQueueSyncInfo    GetQueueWaitInfo(QueueType type) const { return m_queueWaitInfos[(uint32_t)type]; }

        SwapchainSyncInfo     GetSwapchainToWait() const { return m_swapchainToWait; }

        SwapchainSyncInfo     GetSwapchainToSignal() const { return m_swapchainToSignal; }

    protected:
        TL::Vector<Pass*, TL::IAllocator> m_passList;
        AsyncQueueSyncInfo                m_queueSignalInfo;
        AsyncQueueSyncInfo                m_queueWaitInfos[AsyncQueuesCount];
        SwapchainSyncInfo                 m_swapchainToWait;
        SwapchainSyncInfo                 m_swapchainToSignal;
    };
} // namespace RHI
