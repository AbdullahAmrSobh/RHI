#include "RHI/RHI.hpp"

#include <memory>

class DebugCallbacks final : public RHI::IDebugCallbacks
{
public:
    void Log(std::string_view message) override
    {
        (void)message;
    }

    void Warn(std::string_view message) override
    {
        (void)message;
    }

    void Error(std::string_view message) override
    {
        (void)message;
    }

};

int main()
{
    std::unique_ptr<DebugCallbacks> debugCallbacks = std::make_unique<DebugCallbacks>();
    auto                        instance       = RHI::IInstance::Create(RHI::BackendType::Vulkan, std::move(debugCallbacks));
    if (instance.has_value())
    {
        return 0;
    }
    return static_cast<int>(instance.error());
}
