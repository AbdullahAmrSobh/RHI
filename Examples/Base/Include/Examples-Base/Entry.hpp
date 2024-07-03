#pragma once

#include "Examples-Base/ApplicationBase.hpp"

template<typename ExampleType>
inline int ApplicationBase::Entry([[maybe_unused]] TL::Span<const char*> args)
{
    auto example = ExampleType();
    example.Init();
    example.Run();
    example.Shutdown();
    return 0;
}

#define RHI_APP_MAIN(ExampleType)                                                                \
    int main(int argc, const char* argv[])                                                       \
    {                                                                                            \
        return ApplicationBase::Entry<ExampleType>(TL::Span<const char*>{ argv, (size_t)argc }); \
    }