#include "RHI/Backend/Vulkan/Fence.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{
    
    Expected<FencePtr> Factory::CreateFence()
    {
        auto     fence  = CreateUnique<Fence>(*m_device);
        VkResult result = fence->Init();
        if (result != VK_SUCCESS)
            return fence;
        else
            return tl::unexpected(ToResultCode(result));
    }

    Fence::~Fence() { vkDestroyFence(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult Fence::Init()
    {
        VkFenceCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags             = 0;
        createInfo.pNext             = nullptr;

        return vkCreateFence(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    EResultCode Fence::Wait() const
    {
        VkResult result = vkWaitForFences(m_pDevice->GetHandle(), 1, &m_handle, VK_TRUE, UINT64_MAX);
        return ToResultCode(result);
    }

    EResultCode Fence::Reset() const
    {
        VkResult result = vkResetFences(m_pDevice->GetHandle(), 1, &m_handle);
        return ToResultCode(result);
    }

    EResultCode Fence::GetStatus() const
    {
        VkResult result = vkGetFenceStatus(m_pDevice->GetHandle(), m_handle);
        return ToResultCode(result);
    }

} // namespace Vulkan
} // namespace RHI
