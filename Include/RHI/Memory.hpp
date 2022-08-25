#pragma once

namespace RHI
{

enum class EMemoryLevel
{
    Host,
    Device
};

enum class EMemoryAccess
{
    Read,
    Write, 
};

struct MemoryAllocationRequirement
{
    EMemoryLevel  level;
    EMemoryAccess access;
    size_t        byteSize;
    siez_t        byteAlignment;
};

class IAllocation
{
public:
    using Ptr = void*;

    virtual size_t GetSize() const = 0;
    
    // Only to use valid if this allocation object is a sub-allocation.
    virtual IAllocation* GetParantAllocation();

    virtual Ptr  Map(size_t byteOffset, size_t range);
    virtual void Unmap();

protected:
    Ptr m_ptr;
};

class IMemoryAllocator
{
public:
    virtual ~MemoryAllocator() = default;
    
    Unique<IAllocation> Allocate(const MemoryAllocationRequirement& requirement);
    EResultCode Free(IAllocation& allocation);
};

} // namespace RHI