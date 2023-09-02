#pragma once

#include <RHI/CommandList.hpp>

namespace Vulkan
{

class CommandList final : public RHI::CommandList
{
public:


    void Submit(const RHI::CommandDraw& command) override;

    void Submit(const RHI::CommandCopy& command) override;

    void Submit(const RHI::CommandCompute& command) override;
};

}  // namespace Vulkan