#pragma once

#include "RHI/Export.hpp"

#include <TL/Containers.hpp>
#include <TL/Flags.hpp>

namespace RHI
{
    class Context;

    /// @brief Represents a shader module used in pipeline creation.
    class RHI_EXPORT ShaderModule
    {
    public:
        /// @brief Default constructor.
        ShaderModule() = default;

        /// @brief Virtual destructor.
        virtual ~ShaderModule() = default;
    };
} // namespace RHI
