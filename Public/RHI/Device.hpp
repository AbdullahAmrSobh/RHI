#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

#include "RHI/Device.hpp"
#include "RHI/Queue.hpp"

namespace RHI
{

class Device
{
public:
    virtual ~Device() = default;
    
    inline DeviceAddress MapResourceMemory(IBuffer& resource, size_t offset, size_t range)
	{
		return MapResourceMemory(resource, offset, range);
	}
    
	inline void UnmapResourceMemory(IBuffer& resource)
	{
		UnmapResourceMemory(MapableResource(resource));
	}
    
	inline DeviceAddress MapResourceMemory(ITexture& resource, size_t offset, size_t range)
    {
        return MapResourceMemory(MapableResource(resource), offset, range);
    }
    
	inline void UnmapResourceMemory(ITexture& resource)
	{
		UnmapResourceMemory(MapableResource(resource));
	}
	
	virtual IQueue& GetMainQueue() = 0;
    
protected:
    virtual DeviceAddress MapResourceMemory(const MapableResource& resource, size_t offset, size_t range) = 0;
    virtual void          UnmapResourceMemory(const MapableResource& resource)                            = 0;

};

} // namespace RHI
