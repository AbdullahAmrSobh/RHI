#pragma once
#include <vector>

#include <RHI/RHI.hpp>

typedef void* HWND;
typedef void* HINSTANCE;

class WindowInfo
{
public:
    HWND      hwnd;
    HINSTANCE hinstance;
    uint32_t  width, height;
};

struct ImageData
{
    uint32_t width, height, depth;
    uint32_t channels, bytesPerChannel;
    void*    data;

    size_t GetSize();
    void*  GetPtr();
};

class ExampleBase
{
public:
    ExampleBase();
    virtual ~ExampleBase() = default;

    std::vector<uint32_t> ReadBinaryFile(std::string_view path);

    ImageData LoadImage(std::string_view path);

    virtual void OnInit(const WindowInfo& windowInfo) = 0;

    virtual void OnShutdown() = 0;

    virtual void OnUpdate() = 0;

protected:
    std::unique_ptr<RHI::Context> m_context;
};

        // example->OnInit(RHI::WindowInfo{});                                   \

#define EXAMPLE_ENTRY_POINT(exampleClassName)                \
    int main(int argc, const char* argv[])                   \
    {                                                        \
        auto example = std::make_unique<exampleClassName>(); \
                                                             \
                                                             \
        example->OnUpdate();                                 \
                                                             \
        example->OnShutdown();                               \
    }\
