#include "RHI/RHI.hpp"

class HelloTriangle final : public RHI::PassCallbacks
{
public:
    void setupAttachments(FrameGraphBuilder& builder) override
    {
        builder.useImageAttachment("output", )
    }

    void bindResources(ResourcesBindingContext& context) override
    {
    }

    void dispatch(uint32_t dispatch_index, CommandContext& context) override
    {
    }
};