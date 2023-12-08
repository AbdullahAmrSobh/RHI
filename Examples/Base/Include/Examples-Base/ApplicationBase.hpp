#pragma once

#include <RHI/RHI.hpp>
#include <memory>
#include <string>
#include <string_view>

#include <Examples-Base/Timestep.hpp>
#include <Examples-Base/Log.hpp>
#include <Examples-Base/Camera.hpp>

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

protected:
    ImageData LoadImage(std::string_view path) const;

    std::vector<uint32_t> ReadBinaryFile(std::string_view path) const;

    virtual void OnInit() = 0;

    virtual void OnShutdown() = 0;

    virtual void OnUpdate(Timestep timestep) = 0;

    Camera m_camera;

    std::unique_ptr<RHI::Context> m_context;

    std::unique_ptr<RHI::Swapchain> m_swapchain;

    std::unique_ptr<RHI::FrameScheduler> m_frameScheduler;

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
