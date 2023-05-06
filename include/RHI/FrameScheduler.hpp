#pragma once

#include <memory>
#include <string>

#include "RHI/FrameGraph.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

class Pass;
class CommandList; 

class RHI_EXPORT FrameScheduler
{
public:
    virtual ~FrameScheduler() = default;

    virtual ResultCode Init() = 0;

    void FrameBegin();
    void FrameEnd();

    void Submit(Pass& pass);

protected:
    virtual CommandList& BeginPassCommandList(uint32_t dispatchIndex) = 0;
    virtual void         EndPassCommandList()                         = 0;

    virtual void PassPresent(PassState& passState) = 0;
    virtual void PassExecute(PassState& passState) = 0;

    void Compile();

protected:
    std::unique_ptr<AttachmentsRegistry> m_attachmentsRegistry;
    std::unique_ptr<FrameGraph>          m_frameGraph;
    std::vector<Pass*>                   m_passes;
};

}  // namespace RHI