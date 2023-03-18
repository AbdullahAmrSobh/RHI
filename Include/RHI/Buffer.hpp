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
    BufferDesc() = default;

    static BufferDesc Create(BufferUsageFlags usage, size_t byteSize);

    size_t GetHash() const
    {
        return 0;
    }

    BufferUsageFlags usage;
    size_t           size;
};

class IBuffer : public IResource
{
public:
    IBuffer()
        : m_desc(std::make_unique<BufferDesc>())
    {
    }

    virtual ~IBuffer() = default;

    const BufferDesc& GetDescription() const
    {
        return *m_desc;
    }

protected:
    std::unique_ptr<BufferDesc> m_desc;
};

struct BufferRange
{
    BufferRange() = default;
    BufferRange(size_t byteSize);

    size_t byteOffset;
    size_t byteRange;
};

struct BufferViewDesc
{
    BufferViewDesc() = default;

    static BufferViewDesc Create(Format format, BufferRange range);

    size_t GetHash() const
    {
        return 0;
    }

    Format      format;
    BufferRange range;
};

class IBufferView
{
public:
    IBufferView()
        : m_desc(std::make_unique<BufferViewDesc>())
    {
    }

    virtual ~IBufferView() = default;

    const BufferViewDesc& GetDescription() const
    {
        return *m_desc;
    }

protected:
    std::unique_ptr<BufferViewDesc> m_desc;
};

}  // namespace RHI