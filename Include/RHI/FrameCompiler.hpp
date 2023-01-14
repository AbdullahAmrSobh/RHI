#pragma once

namespace RHI
{

class FrameCompiler
{
public:
    void Begin();
    void End();

    void CompileTransientResources();

    void CompileAttachmentsViews();

    void CompileResourceBarriers();
};

}  // namespace RHI