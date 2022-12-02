#include "RHI/RHI.hpp"

class DebugCallbacks final : public RHI::IDebugCallbacks
{
  public:
    virtual void Log(std::string_view message) override
    {
    }

    virtual void Warn(std::string_view message) override
    {
    }

    virtual void Error(std::string_view message) override
    {
    }

    virtual void Fatel(std::string_view message) override
    {
    }
};

int main()
{
    RHI::Unique<DebugCallbacks> debugCallbacks =
        RHI::CreateUnique<DebugCallbacks>();
    auto instance = RHI::IInstance::Create(RHI::EBackend::Vulkan,
                                           std::move(debugCallbacks));
    if (instance.has_value())
    {
        return 0;
    }
    else
    {
        return static_cast<int>(instance.error());
    }
}
