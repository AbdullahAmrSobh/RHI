#pragma once
#include "RHI/Common.hpp"

namespace RHI
{

enum class EMemoryType
{
    Host,
    Device,
};

using MappedAllocationPtr = void*;

struct AllocationMemoryRequirement
{
    size_t byteSize;
    size_t byteAlignment;
};

class IMemoryPool
{
public:
    size_t GetCapacity() const;
};

struct AllocationDesc
{
    EMemoryType                 type;
    size_t                      byteOffset;
    AllocationMemoryRequirement memoryRequirement;
    IMemoryPool*                pMemoryPool = nullptr;
};

} // namespace RHI