#include <string>

#include "RHI/RHI.hpp"

int main()
{
    RHI::ApplicationInfo appInfo {};
    appInfo.graphicsBackend = RHI::Backend::Vulkan;
    appInfo.applicationName = "RHI initialization test";
    appInfo.engineName      = "No engine";
    auto context            = RHI::Context::Create(appInfo, nullptr);

    return context != nullptr; 
}
