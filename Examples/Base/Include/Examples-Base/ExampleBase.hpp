#pragma once

#include <RHI/RHI.hpp>

#include <memory>
#include <string>
#include <string_view>

struct ImageData
{
    uint32_t             width;
    uint32_t             height;
    uint32_t             depth;
    uint32_t             channels;
    uint32_t             bytesPerChannel;
    std::vector<uint8_t> data;
};

struct WindowInfo
{
    void*    hwnd;
    void*    hinstance;
    uint32_t width;
    uint32_t height;
};

class ExampleBase
{
public:
    ExampleBase(std::string windowName, uint32_t width, uint32_t height);
    virtual ~ExampleBase() = default;

    /// @brief Loads an image from disk.
    ImageData             LoadImage(std::string_view path) const;

    /// @brief Reads (in binary mode) a file from disk.
    std::vector<uint32_t> ReadBinaryFile(std::string_view path) const;

    /// @brief Init.
    void                  Init();

    /// @brief Shutdown.
    void                  Shutdown();

    /// @brief Run.
    void                  Run();

    virtual void          OnInit(WindowInfo windowInfo) = 0;

    virtual void          OnShutdown()                  = 0;

    virtual void          OnUpdate()                    = 0;

protected:
    std::unique_ptr<RHI::Context>        m_context;

    std::unique_ptr<RHI::Swapchain>      m_swapchain;

    std::unique_ptr<RHI::FrameScheduler> m_frameScheduler;

    void*                                m_window;
};

#define EXAMPLE_ENTRY_POINT(exampleClassName)                \
    int main(int argc, const char* argv[])                   \
    {                                                        \
        (void)argc;                                          \
        (void)argv;                                          \
        auto example = std::make_unique<exampleClassName>(); \
        example->Init();                                     \
        example->Run();                                      \
        example->Shutdown();                                 \
    }\
\
