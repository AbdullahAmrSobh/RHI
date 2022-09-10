#pragma once

#include <vulkan/vulkan_core.h>
namespace RHI {
namespace Vulkan {

    
    class ReleaseQueue 
    {
    public:
        
        template<typename T>
        void DestroyDeviceObject(VkDevice handle, T objectHandle, const VkAllocationCallbacks* pAllocator)
        {
            
        }
        
        template<typename T>
        void FreeObject(T object);
                
    };


}
}