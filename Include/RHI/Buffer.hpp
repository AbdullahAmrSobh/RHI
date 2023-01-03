#pragma once
#include "RHI/Resource.hpp"

namespace RHI
{

enum class BufferUsageFlagBits
{
    Vertex   = 0x000001,
    Index    = 0x000002,
    Transfer = 0x000004,
};
using BufferUsageFlags = Flags<BufferUsageFlagBits>;

struct BufferDesc
{
    BufferUsageFlags usage;
    size_t           size;
};

class IBuffer : public IResource
{
public:
    IBuffer()
        : m_desc(CreateUnique<BufferDesc>())
    {
    }

    virtual ~IBuffer() = default;

    const BufferDesc& GetDescription() const
    {
        return *m_desc;
    }

protected:
    Unique<BufferDesc> m_desc;
};

struct BufferRange
{
    size_t byteOffset;
    size_t byteRange;
};

struct BufferViewDesc
{
    Format      format;
    BufferRange range;
};

class IBufferView
{
public:
    IBufferView()
        : m_desc(CreateUnique<BufferViewDesc>())
    {
    }

    virtual ~IBufferView() = default;

    const BufferViewDesc& GetDescription() const
    {
        return *m_desc;
    }

protected:
    Unique<BufferViewDesc> m_desc;
};

}  // namespace RHI