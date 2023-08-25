#include <iostream>
#include <memory>

#include "Examples-Base/ExampleBase.hpp"

#include <RHI-Vulkan/Loader.hpp>

class DebugCallbacks final : public RHI::DebugCallbacks
{
public:
    void LogInfo(std::string_view message) override
    {
        std::cout << "INFO: " << message << "\n";
    }

    void LogWarnning(std::string_view message) override
    {
        std::cout << "WARNNING: " << message << "\n";
    }

    void LogError(std::string_view message) override
    {
        std::cout << "ERROR: " << message << "\n";
    }
};

ExampleBase::ExampleBase()
{
    RHI::DeviceSelectionCallback callback = [=](RHI::TL::Span<RHI::DeviceProperties> properties)
    {
        (void)properties;
        return 0u;
    };

    RHI::ApplicationInfo appInfo {};
    m_context = RHI::CreateVulkanRHI(appInfo, callback, std::make_unique<DebugCallbacks>());
}
