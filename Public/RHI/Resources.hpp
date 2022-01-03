#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

using DeviceAddress = void*;

enum class EResourceUsage
{
    Invalid            = 0,
    GpuOnly            = 1,
    CpuOnly            = 2,
    CpuToGpu           = 3,
    GpuToCpu           = 4,
    CpuCopy            = 5,
    GpuLazilyAllocated = 6,
    MaxEnum            = 0x7FFFFFFF
};

struct MemoryAllocationDesc
{
    EResourceUsage usage;
};

class IResource
{
public:
    virtual ~IResource() = default;

    virtual size_t                  GetSize() const                  = 0;
    virtual Expected<DeviceAddress> Map(size_t offset, size_t range) = 0;
    virtual void                    Unmap()                          = 0;
};

} // namespace RHI
