#include "Backend/Vulkan/Commands.hpp"

#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/FrameGraph.hpp"
#include "Backend/Vulkan/FrameGraphPass.hpp"
#include "Backend/Vulkan/Resource.hpp"


#include <iostream>

namespace RHI
{
namespace Vulkan
{

    Expected<Unique<IFrameGraph>> Device::CreateFrameGraph()
    {
        Unique<FrameGraph> frameGraph = CreateUnique<FrameGraph>(*this);
        VkResult           result     = frameGraph->Init();
        if (result != VK_SUCCESS)
        {
            return Unexpected(ConvertResult(result));
        }

        return std::move(frameGraph);
    }

    Expected<Unique<IPass>> FrameGraph::CreatePass(std::string name, EPassType type)
    {
        Unique<Pass> pass = CreateUnique<Pass>(*m_pDevice, *this, std::move(name), type);

        VkResult result = pass->Init();
        if (result != VK_SUCCESS)
        {
            return Unexpected(ConvertResult(result));
        }

        return std::move(pass);
    }

    EResultCode FrameGraph::Execute(const IPassProducer& passProducer)
    {
        std::cout << passProducer.GetPass().GetName() << " Executing pass" << std::endl;
        return EResultCode::Fail;
    }

    VkResult FrameGraph::Init() {
        return VK_SUCCESS;
    }

    VkResult Pass::Init()
    {
        m_commandAllocator->Init(GetType() == EPassType::Graphics ? ECommandPrimaryTask::Graphics : ECommandPrimaryTask::Compute);
        m_semaphore->Init();

        return VK_SUCCESS;
    }

} // namespace Vulkan
} // namespace RHI