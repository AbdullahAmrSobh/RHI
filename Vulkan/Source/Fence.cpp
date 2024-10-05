#include "Fence.hpp"

#include "Context.hpp"
#include "Common.hpp"

namespace RHI::Vulkan
{
    ///////////////////////////////////////////////////////////////////////////
    /// Fence
    ///////////////////////////////////////////////////////////////////////////

    IFence::~IFence()
    {
        // vkDestroyFence(m_context->m_device, m_fence, nullptr);
        m_context->m_deleteQueue.DestroyObject(m_fence);
    }

    ResultCode IFence::Init()
    {
        m_state = FenceState::NotSubmitted;

        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        TRY_OR_RETURN(vkCreateFence(m_context->m_device, &createInfo, nullptr, &m_fence));
        return ResultCode::Success;
    }

    void IFence::Reset()
    {
        m_state = FenceState::NotSubmitted;
        Validate(vkResetFences(m_context->m_device, 1, &m_fence));
    }

    bool IFence::WaitInternal(uint64_t timeout)
    {
        // if (m_state == FenceState::NotSubmitted)
        //     return VK_SUCCESS;

        auto result = vkWaitForFences(m_context->m_device, 1, &m_fence, VK_TRUE, timeout);
        return result == VK_SUCCESS;
    }

    FenceState IFence::GetState()
    {
        // if (m_state == FenceState::Pending)
        // {
        auto result = vkGetFenceStatus(m_context->m_device, m_fence);
        return result == VK_SUCCESS ? FenceState::Signaled : FenceState::Pending;
        // }

        // return FenceState::NotSubmitted;
    }

    VkFence IFence::UseFence()
    {
        // m_state = FenceState::Pending;
        return m_fence;
    }
} // namespace RHI::Vulkan