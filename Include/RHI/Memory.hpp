#pragma once

namespace RHI
{

enum class MemoryUsage
{
    Stream,
    Stage,
    Local,
    Hosted,
};

enum class MemoryLevel
{
    Host,
    Device,
};

struct AllocationMemoryRequirement
{
    size_t byteSize;
    size_t byteAlignment;
};

struct AllocationDesc
{
    MemoryUsage                 usage;
    MemoryLevel                 type;
    size_t                      byteOffset;
    AllocationMemoryRequirement memoryRequirement;
};

}  // namespace RHI