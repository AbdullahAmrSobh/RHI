#include <Examples-Base/ApplicationBase.hpp>
#include <Examples-Base/SceneGraph.hpp>

#include <RHI/RHI.hpp>
#include <RHI/Pass.hpp>

#include <tracy/Tracy.hpp>

class BasicRenderer final : public ApplicationBase
{
public:
    BasicRenderer()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
    {
    }

    void OnInit() override
    {
        ZoneScoped;

        {
            // Load scene
            m_scene = RHI::CreatePtr<Scene>(m_context.get(), "I:/Main.1_Sponza/NewSponza_Main_glTF_002.gltf");
        }

        auto& scheduler = m_context->GetScheduler();

        auto colorAttachment = scheduler.ImportSwapchain("color-attachment", *m_swapchain);

        RHI::ImageCreateInfo depthCreateInfo{};
        depthCreateInfo.name = "depth-attachment";
        depthCreateInfo.format = RHI::Format::D32;
        depthCreateInfo.usageFlags = RHI::ImageUsage::DepthStencil;
        depthCreateInfo.type = RHI::ImageType::Image2D;
        depthCreateInfo.size = { m_windowWidth, m_windowHeight, 1 };
        auto depthAttachment = scheduler.CreateImage(depthCreateInfo);

        m_renderPass = scheduler.CreatePass("Render-Pass", RHI::QueueType::Graphics);
        m_renderPass->SetSize({ m_windowWidth, m_windowHeight });

        RHI::ImageAttachmentUseInfo useInfo{};
        useInfo.usage = RHI::ImageUsage::Color;
        useInfo.clearValue = { 0.0f, 0.2f, 0.3f, 1.0f };
        useInfo.loadStoreOperations.loadOperation = RHI::LoadOperation::Discard;
        useInfo.loadStoreOperations.storeOperation = RHI::StoreOperation::Discard;
        m_renderPass->UseImageAttachment(colorAttachment, useInfo);
        useInfo.usage = RHI::ImageUsage::DepthStencil;
        useInfo.clearValue.depthStencil.depthValue = 1.0f;
        m_renderPass->UseImageAttachment(depthAttachment, useInfo);

        scheduler.Compile();

        m_commandList[0] = m_commandPool->Allocate(RHI::QueueType::Graphics);
        m_commandList[1] = m_commandPool->Allocate(RHI::QueueType::Graphics);
    }

    void OnShutdown() override
    {
        ZoneScoped;

        m_commandPool->Release(m_commandList[0]);
        m_commandPool->Release(m_commandList[1]);
    }

    void OnUpdate(Timestep timestep) override
    {
        (void)timestep;

        ZoneScoped;

        m_camera.Update(timestep);
        m_scene->UpdateUniformBuffers(*m_context, m_camera.GetView(), m_camera.GetProjection());

        static float cameraSpeed = 1.0f;

        ImGui::NewFrame();
        ImGui::Text("Basic scene: ");
        ImGui::SliderFloat("camera speed", &cameraSpeed, 0.1f, 5.0f);
        ImGui::Render();

        m_camera.SetMovementSpeed(cameraSpeed);

        Render(m_context->GetScheduler());
        m_swapchain->Present();
    }

    void Render(RHI::FrameScheduler& scheduler)
    {
        scheduler.Begin();

        RHI::Viewport viewport = {};
        viewport.width = float(m_windowWidth);
        viewport.height = float(m_windowHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        RHI::Scissor scissor = {};
        scissor.width = m_windowWidth;
        scissor.height = m_windowHeight;

        static int i = 0;
        i = i & 1 ? 0 : 1;

        m_commandList[i]->Begin(*m_renderPass);
        m_commandList[i]->SetViewport(viewport);
        m_commandList[i]->SetSicssor(scissor);
        m_scene->Draw(*m_commandList[i]);
        m_imguiRenderer->RenderDrawData(ImGui::GetDrawData(), *m_commandList[i]);

        m_commandList[i]->End();
        m_renderPass->Submit(m_commandList[i]);

        scheduler.End();
    }

private:
    RHI::Ptr<Scene> m_scene;
    RHI::Ptr<RHI::Pass> m_renderPass;
    RHI::CommandList* m_commandList[2];
};

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    return ApplicationBase::Entry<BasicRenderer>();
}
