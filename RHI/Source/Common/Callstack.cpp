#include <RHI/Common/Callstack.hpp>
#include <RHI/Context.hpp>

#include <Windows.h>
#include <DbgHelp.h>

#include <format>

namespace RHI
{
#if RHI_PLATFORM_WINDOWS
    inline static bool _InitializeSymbolHandler()
    {
        ::SymInitialize(::GetCurrentProcess(), nullptr, TRUE);
        return true;
    }
#endif

    Callstack CaptureCallstack()
    {
#if RHI_PLATFORM_WINDOWS
        Callstack callstack = {};
        _InitializeSymbolHandler();
        ::CaptureStackBackTrace(1, RHI_STACK_CALLSTACK_DEPTH, callstack.data(), nullptr);
        return callstack;
#else
    #error "This function not implemented for the target paltform"
#endif
    }

    std::string ReportCallstack(Callstack callstack)
    {
        std::string message = "Callstack Report\n";

        for (void* address : callstack)
        {
            if (address == nullptr)
            {
                break;
            }

            message.append(std::format("\t{} {} \"{}\"\n", address, GetSymbolName(address), GetSymbolFileAndLine(address)));
        }

        return message;
    }

    std::string GetSymbolName(void* address)
    {
#if RHI_PLATFORM_WINDOWS
        constexpr DWORD64 MAX_NAME_LENGTH = 256;
        char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_NAME_LENGTH - 1];
        SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_NAME_LENGTH;

        DWORD64 displacement;
        ::SymFromAddr(::GetCurrentProcess(), reinterpret_cast<DWORD64>(address), &displacement, symbol);

        return symbol->Name;
#else
    #error "This function not implemented for the target paltform"
#endif
    }

    std::string GetSymbolFileAndLine(void* address)
    {
#if RHI_PLATFORM_WINDOWS
        IMAGEHLP_LINE64 lineInfo;
        lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement;

        if (::SymGetLineFromAddr64(::GetCurrentProcess(), reinterpret_cast<DWORD64>(address), &displacement, &lineInfo))
        {
            return std::format("{}({})", lineInfo.FileName, lineInfo.LineNumber);
        }
        else
        {
            return "Unknown";
        }
#else
    #error "This function not implemented for the target paltform"
#endif
    }

} // namespace RHI