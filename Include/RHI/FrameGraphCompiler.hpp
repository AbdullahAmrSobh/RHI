#pragma once

namespace RHI
{

class IFrameGraphCompiler
{
public:
    void CompileTransientFrameAttachments();

    void CompilePassAttachments();

    void CompilePass(); 
};

} // namespace RHI