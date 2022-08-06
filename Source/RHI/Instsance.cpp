#include "RHI/Instance.hpp"

namespace RHI {
    
    Result<Unique<Instance>> Instnace::createInstance(EBackend backend)
    {
        switch (backend)
        {
        case EBackend::Vulkan:
            return VulkanInstance::createInstance();
        
        case EBackend::D3D12:
        case EBackend::Metal:
        default:
        
            return ErrorNotImplemented;
        };
    }

}