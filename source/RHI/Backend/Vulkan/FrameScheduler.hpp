#pragma once

#include "RHI/FrameScheduler.hpp"

#include "RHI/Backend/Vulkan/Vulkan.hpp"

namespace Vulkan
{

class FrameScheduler final : public RHI::FrameScheduler
{
public:
    FrameScheduler(RHI::Context& context)
        : RHI::FrameScheduler(context)
    {
    }

    ~FrameScheduler() = default;

    RHI::ResultCode Init() override;

    RHI::CommandList& PassExecuteBegin(RHI::Pass& pass) override;
    void              PassExecuteEnd(RHI::Pass& pass) override;            

    void TransientAllocatorBegin() override;
    void TransientAllocatorEnd() override;

    RHI::Buffer* AllocateTransientBuffer(const RHI::BufferCreateInfo& createInfo) override;
    RHI::Image*  AllocateTransientImage(const RHI::ImageCreateInfo& createInfo) override;

private:
    vk::CommandPool m_commandPool;

    std::vector<Buffer*> m_availableBuffers;
    std::vector<Image*>  m_availableImages;
    
};

}  // namespace Vulkan 