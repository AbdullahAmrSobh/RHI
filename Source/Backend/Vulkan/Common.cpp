#include "Backend/Vulkan/Common.hpp"

#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

    EResultCode ConvertResult(VkResult resultCode)
    {
        switch (resultCode)
        {
        case VK_SUCCESS: return EResultCode::Success;
        case VK_TIMEOUT: return EResultCode::Timeout;
        case VK_NOT_READY: return EResultCode::NotReady;
        case VK_ERROR_OUT_OF_HOST_MEMORY: return EResultCode::HostOutOfMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return EResultCode::DeviceOutOfMemory;
        case VK_ERROR_EXTENSION_NOT_PRESENT: return EResultCode::ExtensionNotAvailable;
        case VK_ERROR_FEATURE_NOT_PRESENT: return EResultCode::FeatureNotAvailable;
        default: return EResultCode::Fail;
        }
    }
    
    
} // namespace Vulkan
} // namespace RHI