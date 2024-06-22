#pragma once

#include <Examples-Base/Timestep.hpp>
#include <Examples-Base/Camera.hpp>
#include <Examples-Base/ImGuiRenderer.hpp>

#include <RHI/RHI.hpp>

#include <fstream>

namespace TL = RHI::TL;

template<typename T>
using Handle = RHI::Handle<T>;

template<typename T>
using Ptr = RHI::Ptr<T>;

inline static TL::Vector<uint32_t> ReadBinaryFile(std::string_view filePath)
{
    std::ifstream file(filePath.data(), std::ios::binary | std::ios::ate);
    RHI_ASSERT(file.is_open()); // "Failed to open SPIR-V file: " + filePath

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    RHI_ASSERT(size % 4 == 0); // "Invalid SPIR-V file size: " + filePath

    TL::Vector<uint32_t> spirv(size / 4);
    RHI_ASSERT(file.read(reinterpret_cast<char*>(spirv.data()), size)); // "Failed to read SPIR-V file: " + filePath

    return spirv;
}

class ApplicationBase
{
public:
    const uint32_t m_windowWidth;
    const uint32_t m_windowHeight;

    ApplicationBase(std::string name, uint32_t width, uint32_t height);
    virtual ~ApplicationBase() = default;

    template<typename ExampleType>
    static int Entry(TL::Span<const char*> args);

private:
    void Init();

    void Shutdown();

    void Run();

    void ProcessInput();

    virtual void OnInit() = 0;

    virtual void OnShutdown() = 0;

    virtual void OnUpdate(Timestep timestep) = 0;

protected:
    Camera m_camera;

    RHI::Ptr<RHI::Context> m_context;

    RHI::Ptr<RHI::Swapchain> m_swapchain;

    RHI::Ptr<ImGuiRenderer> m_imguiRenderer;

    RHI::Ptr<RHI::CommandPool> m_commandPool[2];

    void* m_window;
};

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