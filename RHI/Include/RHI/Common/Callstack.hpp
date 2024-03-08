#pragma once

#include "RHI/Export.hpp"

#include <array>
#include <string>

#define RHI_STACK_CALLSTACK_DEPTH 20

namespace RHI
{
    class DebugCallbacks;

    using Callstack = std::array<void*, RHI_STACK_CALLSTACK_DEPTH>;

    // Capture the current callstack
    RHI_EXPORT Callstack CaptureCallstack(uint32_t skipFramesCount = 1);

    // Report the callstack to the debug callbacks
    RHI_EXPORT std::string ReportCallstack(Callstack callstack);

    // Return the name of the symbol address
    RHI_EXPORT std::string GetSymbolName(void* address);

    // return the file and line of the symbol address
    RHI_EXPORT std::string GetSymbolFileAndLine(void* address);

} // namespace RHI