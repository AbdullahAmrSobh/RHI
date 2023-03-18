#include "RHI/Pch.hpp"

#include "RHI/Common.hpp"

#include "Backend/Vulkan/Instance.hpp"

#include "Backend/Vulkan/Device.hpp"

namespace RHI
{

static std::shared_ptr<IDebugCallbacks> s_callbacks;

void Debug::Init(std::unique_ptr<IDebugCallbacks> callbacks)
{
    s_callbacks = std::move(callbacks);
}

std::shared_ptr<IDebugCallbacks>& Debug::Get()
{
    return s_callbacks;
}

Expected<std::unique_ptr<IInstance>> IInstance::Create(BackendType backend, std::unique_ptr<IDebugCallbacks> callbacks)
{
    switch (backend)
    {
        case BackendType::Vulkan: {
            std::unique_ptr<Vulkan::Instance> instance = std::make_unique<Vulkan::Instance>();

            Debug::Init(std::move(callbacks));

            VkResult result = instance->Init();

            if (result == VK_SUCCESS)
            {
                return instance;
            }

            return Unexpected(ResultCode::Fail);
        }
        default: return Unexpected(ResultCode::Fail);
    }
}

std::vector<const IPhysicalDevice*> IInstance::GetPhysicalDevices() const
{
    std::vector<const IPhysicalDevice*> result;
    for (auto& physicalDevice : m_physicalDevices)
    {
        result.push_back(physicalDevice.get());
    }
    return result;
}

}  // namespace RHI