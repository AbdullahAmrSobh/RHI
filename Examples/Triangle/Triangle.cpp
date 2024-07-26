#include <Examples-Base/ApplicationBase.hpp>

#include <tracy/Tracy.hpp>

#include <imgui.h>

namespace Examples
{
    class BasicRenderer final : public ApplicationBase
    {
    public:
        BasicRenderer()
            : ApplicationBase("Hello, Triangle", 1600, 1200)
        {
        }

        void OnInit() override
        {
        }

        void OnShutdown() override
        {
        }

        void OnUpdate(Timestep timestep) override
        {
            (void)timestep;

            // scene->settransfrom from camera
        }

        void OnEvent(Event& e) override
        {
            (void)e;
        }
    };
} // namespace Examples

#include <Examples-Base/Entry.hpp>
int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{ argv, (size_t)argc };
    return Entry<BasicRenderer>(args);
}
