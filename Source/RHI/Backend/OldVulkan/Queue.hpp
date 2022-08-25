#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/RenderPass.hpp"

namespace RHI
{
namespace Vulkan
{
    struct SubmitInfo
    {
        std::vector<VkSemaphore>          waitSemaphores;
        std::vector<VkPipelineStageFlags> waitStages;
        std::vector<VkCommandBuffer>      commandBuffers;
        std::vector<VkSemaphore>          signalSemaphores;
    };

    struct PresentInfo
    {
        std::vector<VkSemaphore>    waitSemaphores;
        std::vector<VkSwapchainKHR> swapchainHandles;
        std::vector<uint32_t>       imageIndices;
        std::vector<VkResult*>      results;
    };
    
    struct QueueDesc
    {
        uint32_t queueFamilyIndex;
        uint32_t queueIndex = 1;
        uint32_t count      = 1;
    };

    class Queue final : public DeviceObject<VkQueue>
    {
    public:
        Queue(Device& device, const QueueDesc& queueDesc);
        ~Queue() = default;
        
        VkResult Submit(const std::vector<SubmitInfo>& submitInfos, IFence& signalFence);
        VkResult Present(const PresentInfo& desc, std::vector<VkResult>& outPresentResults);
        
        uint32_t m_queueFamilyIndex = UINT32_MAX;
        uint32_t m_queueIndex       = UINT32_MAX;
    };
} // namespace Vulkan
} // namespace RHI
