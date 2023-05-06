#include <string>

#include "RHI/RHI.hpp"

int main()
{
    RHI::ApplicationInfo appInfo {};
    appInfo.applicationName = "RHI initialization test";
    appInfo.engineName      = "No engine";
    auto context            = RHI::Context::Create(appInfo, nullptr, RHI::Backend::Validation);

    return context != nullptr; 
}
