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

    protected:
        Context*             m_context; ///< Pointer to the context associated with this shader module.
        TL::Vector<uint32_t> m_spirv;   ///< SPIR-V bytecode for the shader module. TODO: Should be `TL::Block` instead.
    };
} // namespace RHI
