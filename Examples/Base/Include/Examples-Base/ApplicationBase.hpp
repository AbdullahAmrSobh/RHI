#pragma once

#include <RHI/RHI.hpp>
#include <string>
#include <string_view>

#include <Examples-Base/Timestep.hpp>
#include <Examples-Base/Camera.hpp>
#include <Examples-Base/ImGuiRenderer.hpp>

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct ImageData
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t channels;
    uint32_t bytesPerChannel;
    std::vector<uint8_t> data;
};

class ApplicationBase
{
public:
    const uint32_t m_windowWidth;
    const uint32_t m_windowHeight;

    ApplicationBase(std::string name, uint32_t width, uint32_t height);
    virtual ~ApplicationBase() = default;

    template<typename ExampleType>
    static int Entry();

private:
    enum class State
    {
        ShouldExit,
        Running,
    };

    void Init();

    void Shutdown();

    void Run();

    void ProcessInput();

protected:
    ImageData LoadImage(std::string_view path) const;

    std::vector<uint8_t> ReadBinaryFile(std::string_view path) const;

    virtual void OnInit() = 0;

    virtual void OnShutdown() = 0;

    virtual void OnUpdate(Timestep timestep) = 0;

protected:
    Camera m_camera;

    RHI::Ptr<RHI::Context> m_context;

    RHI::Ptr<RHI::Swapchain> m_swapchain;

    RHI::Ptr<ImGuiRenderer> m_imguiRenderer;
    
    RHI::Ptr<RHI::CommandListAllocator> m_graphicsCommandsAllocator;
    RHI::Ptr<RHI::CommandListAllocator> m_computeCommandsAllocator;

    void* m_window;

    State m_state;
};

template<typename ExampleType>
inline int ApplicationBase::Entry()
{
    auto example = ExampleType();

    example.Init();

    example.Run();

    example.Shutdown();

    return 0;
}
