#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Memory.hpp"

#include "RHI/Device.hpp"

namespace RHI
{

class IDevice
{
public:
    virtual ~IDevice() = default;
    
    inline DeviceAddress MapResourceMemory(IBuffer& resource, size_t offset, size_t range)
	{
		return MapResourceMemory(MapableResource(resource), offset, range);
	}
    
	inline void UnmapResourceMemory(IBuffer& resource)
	{
		UnmapResourceMemory(MapableResource(resource));
	}
    
	inline DeviceAddress MapResourceMemory(IImage& resource, size_t offset, size_t range)
    {
        return MapResourceMemory(MapableResource(resource), offset, range);
    }
    
	inline void UnmapResourceMemory(IImage& resource)
	{
		UnmapResourceMemory(MapableResource(resource));
	}
    
protected:
    virtual DeviceAddress MapResourceMemory(const MapableResource& resource, size_t offset, size_t range) = 0;
    virtual void          UnmapResourceMemory(const MapableResource& resource)                            = 0;

};
using DevicePtr = Unique<IDevice>;

} // namespace RHI
