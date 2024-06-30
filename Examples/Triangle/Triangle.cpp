#include <Examples-Base/ApplicationBase.hpp>
#include <Examples-Base/SceneGraph.hpp>

#include <ShaderInterface/Core.slang>

#include <RHI/RHI.hpp>

#include <tracy/Tracy.hpp>

// clang-format off


// inline static Handle<RHI::ImageAttachment> CreateAttachment(RHI::RenderGraph& renderGraph, Handle<RHI::Pass> pass, const char* name, RHI::Format format, RHI::LoadStoreOperations loadStoreOperations )
// {
//     RHI::ImageCreateInfo createInfo
//     {
//         .name        = name,
//         .usageFlags  = RHI::GetFormatInfo(format).hasRed ? RHI::ImageUsage::Color : RHI::ImageUsage::Depth,
//         .type        =   RHI::ImageType::Image2D,
//         .size        =  {RHI::SizeRelative2D.width, RHI::SizeRelative2D.height, 1},
//         .format      = format,
//         .sampleCount =  RHI::SampleCount::Samples1,
//         .mipLevels   = 1,
//         .arrayCount  = 1
//     };
//     auto attachment = renderGraph.CreateImage(createInfo);
//     RHI::ImageViewInfo viewInfo {};
//     viewInfo.loadStoreOperations = loadStoreOperations;
//     viewInfo.viewType = RHI::ImageViewType::View2D;
//     renderGraph.PassUseImage(pass, attachment, viewInfo, (RHI::ImageUsage)((int)createInfo.usageFlags), RHI::ShaderStage::None, RHI::Access::None);
//     return attachment;
// }

// struct PassGBuffer
// {
//     RHI::Handle<RHI::Pass>            pass             = RHI::NullHandle;

//     // GBuffer ouptut targets
//     RHI::Handle<RHI::ImageAttachment> albedoTarget     = RHI::NullHandle;
//     RHI::Handle<RHI::ImageAttachment> roughnessTarget  = RHI::NullHandle;
//     RHI::Handle<RHI::ImageAttachment> wsPositionTarget = RHI::NullHandle;
//     RHI::Handle<RHI::ImageAttachment> normalTarget     = RHI::NullHandle;
//     RHI::Handle<RHI::ImageAttachment> depthTarget      = RHI::NullHandle;

//     // Scene Transform uniform buffer
//     RHI::Handle<RHI::BufferAttachment> sceneTransformUB   = RHI::NullHandle;
//     RHI::Handle<RHI::BufferAttachment> objectsTransformUB = RHI::NullHandle;

//     RHI::Handle<RHI::BindGroup> bindGroup = RHI::NullHandle;

//     SceneTransform sceneTransformData;
//     TL::Vector<ObjectTransform> objectTransformDatas;

//     void SetupAttachments(RHI::RenderGraph& renderGraph)
//     {
//         ZoneScoped;

//         this->pass = renderGraph.CreatePass({ .name = "GBuffer", .flags = RHI::PassFlags::Graphics });

//         this->albedoTarget     = CreateAttachment (renderGraph, pass, "albedoTarget", RHI::Format::RGBA8_UNORM, RHI::LoadStoreOperations());
//         // this->roughnessTarget  = CreateAttachment (renderGraph, pass, "roughnessTarget", RHI::Format::RGBA8_UNORM, RHI::LoadStoreOperations());
//         // this->wsPositionTarget = CreateAttachment (renderGraph, pass, "wsPositionTarget", RHI::Format::RGBA8_UNORM, RHI::LoadStoreOperations());
//         // this->normalTarget     = CreateAttachment (renderGraph, pass, "normalTarget", RHI::Format::RGBA8_UNORM, RHI::LoadStoreOperations());
//         this->depthTarget      = CreateAttachment (renderGraph, pass, "depthTarget", RHI::Format::RGBA8_UNORM, RHI::LoadStoreOperations());
//     }

//     void SetupBindGroups(RHI::Context& context, RHI::RenderGraph& renderGraph, RHI::Handle<RHI::BindGroupLayout> bindGroupLayout)
//     {
//         ZoneScoped;

//         this->bindGroup = context.CreateBindGroup(bindGroupLayout);

//         RHI::ResourceBinding bindings[]
//         {
//             RHI::ResourceBinding(0, 0, renderGraph.PassGetBuffer(this->sceneTransformUB)),
//             RHI::ResourceBinding(1, 0, renderGraph.PassGetBuffer(this->objectsTransformUB)),
//         };

//         context.UpdateBindGroup(bindGroup, bindings);
//     }
// };

// clang-format on

class BasicRenderer final : public ApplicationBase
{
public:
    RHI::Ptr<RHI::RenderGraph> m_renderGraph;

    // PassGBuffer m_passGBuffer;

    BasicRenderer()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
    {
    }

    void OnInit() override
    {
        ZoneScoped;

        m_renderGraph = m_context->CreateRenderGraph();
        [[maybe_unused]] auto pass = m_renderGraph->CreatePass({});

        // m_passGBuffer.SetupAttachments(*m_renderGraph);
    }

    void OnShutdown() override
    {
        ZoneScoped;
    }

    void OnUpdate(Timestep timestep) override
    {
        (void)timestep;

        ZoneScoped;

        static float cameraSpeed = 1.0f;

        ImGui::NewFrame();
        ImGui::Text("Basic scene: ");
        ImGui::SliderFloat("camera speed", &cameraSpeed, 0.1f, 5.0f);
        ImGui::Render();

        m_camera.SetMovementSpeed(cameraSpeed);
        m_camera.Update(timestep);

        [[maybe_unused]] auto result = m_swapchain->Present();
    }
};

RHI_APP_MAIN(BasicRenderer);