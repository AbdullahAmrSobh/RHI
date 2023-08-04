#pragma once

#include <memory>
#include <string>

#include "RHI/Export.hpp"

namespace RHI
{

enum class PassQueue;

class FrameGraph;
class ShaderResourceContext;
class CommandList;

// Callbacks used to setup this Pass
// User must override this class to create a new pass
class PassInterface
{
    friend class FrameScheduler;

public:
    PassInterface(std::string name, PassQueue queueType)
        : m_pass(std::make_unique<Pass>(name, queueType))
    {
    }

    virtual ~PassInterface() = default;

    // Sets up the attachments that will be used in this pass.
    virtual void SetupAttachments(FrameGraph& frameGraph) = 0;

    // Sets up all the shader resource group used by this pass.
    virtual void BindPassResources(ShaderResourceContext& context) = 0;

    // Builds the CommandList for this pass.
    virtual void BuildCommandList(CommandList& commandList) = 0;

private:
    std::unique_ptr<Pass> m_pass;
};

}  // namespace RHI