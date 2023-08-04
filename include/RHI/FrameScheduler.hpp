#pragma once

#include "RHI/FrameGraph.hpp"

namespace RHI
{

class ShaderResourceGroup;
class PassInterface;
class CommandList;

class RHI_EXPORT FrameScheduler
{
public:
    FrameScheduler(Context& context)
        : m_context(&context)
    {
    }

    virtual ~FrameScheduler() = default;

    virtual ResultCode Init() = 0;

    // Called at the begining of the render loop
    void FrameBegin();

    // Called at the end of the render loop
    void FrameEnd();

    // Submit the provided Pass to be executed
    void Submit(PassInterface& pass);

private:
    ImageView* CreateView(const ImageViewCreateInfo& viewInfo);
    BufferView* CreateView(const BufferViewCreateInfo& viewInfo);

protected:
    virtual CommandList& PassExecuteBegin(Pass& pass) = 0;
    virtual void         PassExecuteEnd(Pass& pass) = 0;

    virtual void TransientAllocatorBegin() = 0;
    virtual void TransientAllocatorEnd()   = 0;

    virtual Buffer* AllocateTransientBuffer(const BufferCreateInfo& createInfo) = 0;
    virtual Image*  AllocateTransientImage(const ImageCreateInfo& createInfo)   = 0;

protected:
    Context* m_context;

    std::unique_ptr<FrameGraph> m_frameGraph;

    std::vector<PassInterface*> m_passesList;

    std::vector<ShaderResourceGroup*> m_shaderResourceGroups;

    std::vector<ImageView*> m_usedImageViews;

    std::vector<BufferView*> m_usedBufferViews;
};

}  // namespace RHI