#include "RHI/RHI.hpp"

class DebugCallbacks final : public RHI::IDebugCallbacks
{
public:
    void Log(std::string_view message) override
    {
    }

    void Warn(std::string_view message) override
    {
    }

    void Error(std::string_view message) override
    {
    }

};

int main()
{
    RHI::Unique<DebugCallbacks> debugCallbacks = RHI::CreateUnique<DebugCallbacks>();
    auto                        instance       = RHI::IInstance::Create(RHI::BackendType::Vulkan, std::move(debugCallbacks));
    if (instance.has_value())
    {
        return 0;
    }
    return static_cast<int>(instance.error());
}
