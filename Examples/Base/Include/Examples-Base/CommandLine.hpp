#pragma once

#include <TL/Containers/Vector.hpp>
#include <TL/Containers/Set.hpp>
#include <TL/Span.hpp>

#include <filesystem>

namespace Engine::CommandLine
{
    struct LaunchSettings
    {
        std::filesystem::path sceneFileLocation;
        enum class Backend
        {
            Vulkan,
            WebGPU,
            D3D12,
        };
        Backend backend;
    };

    enum OptionType
    {
        Help,
        SceneFileLocation,
        Backend,
        Count,
    };

    struct Option
    {
        const char*         name;
        const char*         description;
        OptionType          type;
        uint32_t            argumentCount;
        TL::Set<TL::String> validInputs; // Empty set means any input is valid
        bool                isFilePath;
    };

    // clang-format off
    inline static Option m_optionsLut[OptionType::Count] =
    {
        { "help",  "Print this help message and exit.", Help,              0, {}, false },
        { "scene", "Scene file location.",              SceneFileLocation, 1, {}, true  },
        { "backend", "RHI backend to run",              Backend,           1, { "vulkan", "webgpu", "d3d12" }, false },
    };

    // clang-format on

    inline static bool StrEql(const char* a, const char* b)
    {
        return strcmp(a, b) == 0;
    }

    inline static OptionType GetOptionType(const char* arg)
    {
        if (strncmp(arg, "--", 2) != 0)
            return Count;

        const char* optionName = arg + 2;

        for (int i = 0; i < OptionType::Count; ++i)
        {
            if (StrEql(optionName, m_optionsLut[i].name))
            {
                return static_cast<OptionType>(i);
            }
        }

        return Count;
    }

    inline static void PrintHelp()
    {
        TL_LOG_INFO("Usage: program [options]\n\nOptions:\n");
        for (const auto& option : m_optionsLut)
        {
            TL_LOG_INFO(" --{}: {}", option.name, option.description);
        }
    }

    inline static void Print(TL::String message)
    {
        TL_LOG_INFO("{}", message);
    }

    inline static void PrintErrorAndExit(TL::String message)
    {
        TL_LOG_ERROR("{}", message);
        PrintHelp();
        exit(1);
    }

    inline static bool ValidateInput(const Option& option, const TL::String& input)
    {
        if (option.validInputs.empty() && !option.isFilePath)
        {
            return true; // Any input is valid
        }

        if (!option.validInputs.empty())
        {
            return option.validInputs.find(input) != option.validInputs.end();
        }

        if (option.isFilePath)
        {
            std::filesystem::path path(input.c_str());
            return std::filesystem::exists(path);
        }

        return false;
    }

    inline static LaunchSettings Parse(TL::Span<const char*> args)
    {
        LaunchSettings settings;

        for (auto it = args.begin(); it != args.end(); ++it)
        {
            auto optionType = GetOptionType(*it);

            if (optionType == Count)
            {
                PrintErrorAndExit(TL::String("Unknown option: ") + *it);
            }

            const auto& optionData = m_optionsLut[optionType];

            if (optionData.argumentCount > 0)
            {
                ++it;
                if (it == args.end() || GetOptionType(*it) != Count)
                {
                    PrintErrorAndExit(TL::String("Missing argument for option: ") + *(it - 1));
                }

                TL::String argument(*it);
                if (!ValidateInput(optionData, argument))
                {
                    PrintErrorAndExit(TL::String("Invalid argument for option ") + optionData.name + ": " + argument);
                }
            }

            switch (optionType)
            {
            case Help:
                PrintHelp();
                exit(0);
            case SceneFileLocation:
                settings.sceneFileLocation = *it;
                break;
            case Backend:
                if (StrEql(*it, "vulkan"))
                    settings.backend = LaunchSettings::Backend::Vulkan;
                else if (StrEql(*it, "webgpu"))
                    settings.backend = LaunchSettings::Backend::WebGPU;
                else if (StrEql(*it, "d3d12"))
                    settings.backend = LaunchSettings::Backend::D3D12;
                else
                    settings.backend = LaunchSettings::Backend::Vulkan;
                break;
            case Count:

                break;
            }
        }

        return settings;
    }
} // namespace Engine::CommandLine