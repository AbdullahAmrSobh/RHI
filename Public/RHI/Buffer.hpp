#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

struct BufferDesc
{
    size_t           size;
    BufferUsageFlags usage;
};

class IBuffer : public IResource
{
public:
    virtual ~IBuffer() = default;
    
};
using BufferPtr = Unique<IBuffer>;

struct BufferViewDesc
{
    EBufferFormat format;
    size_t       offset;
    size_t       size;
    IBuffer*     pBuffer;
};

class IBufferView
{
public:
    virtual ~IBufferView() = default;
    
};
using BufferViewPtr = Unique<IBufferView>;

} // namespace RHI
