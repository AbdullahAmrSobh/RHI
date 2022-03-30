#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct MemoryAllocationDesc
{
    EResourceUsage usage;
};

class IResource
{
public:
    virtual ~IResource() = default;
	
	virtual size_t GetSize() const = 0;
};

} // namespace RHI
