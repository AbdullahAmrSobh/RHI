#pragma once
#include <RHI/FrameScheduler.hpp>

namespace Vulkan
{

class FrameScheduler final : public RHI::FrameScheduler
{
public:
    void BeginInternal() override;

    void EndInternal() override;
};

}  // namespace Vulkan