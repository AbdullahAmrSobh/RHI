#pragma once

#include "RHI/Backend/Vulkan/Vulkan.hpp"
#include "RHI/FrameScheduler.hpp"

namespace RHI
{
class Context;
}

namespace Vulkan
{

class Context;
class CommandList;

class FrameScheduler final : public RHI::FrameScheduler
{
public:
    FrameScheduler(RHI::Context& context);
    ~FrameScheduler();

    RHI::ResultCode Init() override;

    RHI::CommandList& BeginPassCommandList(uint32_t dispatchIndex) override;
    void              EndPassCommandList() override;

    void PassPresent(RHI::PassState& passState) override;
    void PassExecute(RHI::PassState& passState) override;

private:
    Context*        m_context;
    vk::CommandPool m_commandPool;
    CommandList*    m_commandBuffer;
};

}  // namespace Vulkan