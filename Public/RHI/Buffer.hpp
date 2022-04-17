#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct BufferDesc
{
    BufferDesc() = default;

    size_t           size;
    BufferUsageFlags usage;
};

class IBuffer
{
public:
    virtual ~IBuffer() = default;

    virtual size_t GetMemorySize() const
    {
        return 0;
    };
};
using BufferPtr = Unique<IBuffer>;

struct BufferViewDesc
{
    BufferViewDesc() = default;

    EBufferFormat format;
    size_t        offset;
    size_t        size;
    IBuffer*      pBuffer;
};

class IBufferView
{
public:
    virtual ~IBufferView() = default;
};
using BufferViewPtr = Unique<IBufferView>;

} // namespace RHI
