#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

enum class EBufferUsageFlagBits
{
    Invalid            = 0x00000000,
    TransferSrc        = 0x00000001,
    TransferDst        = 0x00000002,
    UniformTexelBuffer = 0x00000004,
    StorageTexelBuffer = 0x00000008,
    UniformBuffer      = 0x00000010,
    StorageBuffer      = 0x00000020,
    IndexBuffer        = 0x00000040,
    VertexBuffer       = 0x00000080,
    IndirectBuffer     = 0x00000100,
    MaxEnum            = 0x7FFFFFFF,
};
using BufferUsageFlags = Flags<EBufferUsageFlagBits>;

struct BufferDesc
{
    size_t           size;
    BufferUsageFlags usage;
};

class IBuffer : public IResource
{
public:
    virtual ~IBuffer() = default;
    
    inline const BufferDesc& GetDesc() const { return m_desc; };

protected:
    BufferDesc m_desc;
};
using BufferPtr = Unique<IBuffer>;

struct BufferViewDesc
{
    EPixelFormat format;
    size_t       offset;
    size_t       size;
    IBuffer*     pBuffer;
};

class IBufferView
{
public:
    virtual ~IBufferView() = default;

    inline const BufferViewDesc& GetDesc() const { return m_desc; };

protected:
    BufferViewDesc m_desc;
};
using BufferViewPtr = Unique<IBufferView>;

} // namespace RHI
