#pragma once

#include <RHI/Pass.hpp>

namespace Vulkan
{

class Context;

class Pass final : public RHI::Pass
{
    friend class CommandList;
public:
    static Pass* Create(Context* context, RHI::PassCreateInfo& createInfo);

    using RHI::Pass::Pass;
    virtual ~Pass() = default;

    RHI::CommandList& BeginCommandList(uint32_t commandsCount = 1) override;

    void EndCommandList() override;

    void OnBegin() override;

    void OnEnd() override;

    RHI::PassQueueState GetPassQueueStateInternal() const override;
};

};  // namespace Vulkan
