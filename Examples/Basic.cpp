#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

int main()
{
    RHI::ApplicationInfo appInfo {};
    appInfo.applicationName = "basic-example";
    appInfo.applicationVersion.major = 1;
    appInfo.applicationVersion.minor = 1;
    appInfo.applicationVersion.patch = 1;
    appInfo.engineName = "basic-engine";
    appInfo.engineVersion.minor = 1;
    appInfo.engineVersion.patch = 1;
    auto context = RHI::CreateVulkanContext(appInfo);
}