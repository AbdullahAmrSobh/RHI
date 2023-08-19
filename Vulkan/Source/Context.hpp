#pragma once

#include "RHI/RHI.hpp"

namespace Vulkan
{
    class Context final : public RHI::Context
    {
    public:
        ~Context() = default;

        std::string GetName() const override;
    };
}