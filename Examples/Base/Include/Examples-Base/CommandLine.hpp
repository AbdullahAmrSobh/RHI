#pragma once

#include <filesystem>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

namespace Examples::CommandLine
{
    struct LaunchSettings
    {
        TL::String sceneFileLocation;
    };

    enum OptionType
    {
        Help,
        SceneFileLocation,
        Count,
    };

    struct Option
    {
        const char* name;
        const char* description;
        OptionType type;
        uint32_t argumentCount;
        TL::UnorderedSet<TL::String> validInputs; // Empty set means any input is valid
        bool isFilePath;
    };

    // clang-format off
    inline static Option m_optionsLut[OptionType::Count] =
    {
        { "help",  "Print this help message and exit.", Help,              0, {}, false },
        { "scene", "Scene file location.",              SceneFileLocation, 1, {}, true  },
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
            case Count:
                break;
            }
        }

        return settings;
    }
} // namespace Examples::CommandLine