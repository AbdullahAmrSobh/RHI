#include <Examples-Base/ApplicationBase.hpp>

#include <RHI/RHI.hpp>
#include <RHI/Pass.hpp>

#include <tracy/Tracy.hpp>

class BasicRenderer final : public ApplicationBase
{
public:
    BasicRenderer()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
    {
        // m_scene = RHI::CreatePtr<Scene>(m_context.get(), "asdasda");
    }

    void OnInit() override
    {
        ZoneScoped;

        auto& scheduler = m_context->GetScheduler();

        auto colorAttachment = scheduler.ImportSwapchain("color-attachment", *m_swapchain);

        RHI::ImageCreateInfo depthCreateInfo{};
        depthCreateInfo.debugName = "depth-attachment";
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
        m_renderPass->UseImageAttachment(depthAttachment, useInfo);

        scheduler.Compile();
    }

    void OnShutdown() override
    {
        ZoneScoped;
    }

    void OnUpdate(Timestep timestep) override
    {
        (void)timestep;

        ZoneScoped;

        m_camera.Update(timestep);
        // m_scene->UpdateUniformBuffers(m_camera.GetProjection() * m_camera.GetView());

        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

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

        auto commandList = m_commandPool->Allocate(RHI::QueueType::Graphics);
        commandList->Begin(*m_renderPass);
        commandList->SetViewport(viewport);
        commandList->SetSicssor(scissor);
        // m_scene->Draw(*commandList);
        m_imguiRenderer->RenderDrawData(ImGui::GetDrawData(), *commandList);

        commandList->End();
        m_renderPass->Submit(commandList);
        m_commandPool->Release(commandList);

        scheduler.End();
    }

private:
    RHI::Ptr<RHI::Pass> m_renderPass;
};

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    return ApplicationBase::Entry<BasicRenderer>();
}
