#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct MemoryAllocationDesc
{
    MemoryAllocationDesc() = default;
    MemoryAllocationDesc(EResourceUsage usage)
        : usage(usage)
    {
    }

    inline bool operator==(const MemoryAllocationDesc& other) const
    {
        return static_cast<uint32_t>(usage) == static_cast<uint32_t>(other.usage);
    }
    inline bool operator!=(const MemoryAllocationDesc& other) const
    {
        return !(*this == other);
    }
    
    EResourceUsage usage;
};

class IDeviceMemoryAllocation
{
public:
    virtual ~IDeviceMemoryAllocation() = default;

    virtual DeviceAddress Map(size_t offset, size_t range) = 0;
    virtual void          Unmap()                          = 0;
};
using DeviceMemoryAllocationPtr = Unique<IDeviceMemoryAllocation>;

class IMemoryPool
{
public:
    virtual ~IMemoryPool() = default;

    inline size_t GetSize() const;

    virtual DeviceMemoryAllocationPtr Allocate(const MemoryAllocationDesc& allocationDesc) = 0;
};
using MemoryPoolPtr = Unique<IMemoryPool>;

} // namespace RHI
