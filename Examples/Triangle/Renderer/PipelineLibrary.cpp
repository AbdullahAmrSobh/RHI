#include "Shaders/Public/GPU.h"

#include "PipelineLibrary.hpp"
#include "RendererImpl/DeferredRenderer.hpp"

#include <RHI/ShaderUtils.inl>

#include <TL/Defer.hpp>

#include <glm/glm.hpp>

namespace Engine
{
    static TL::Map<const char*, std::pair<std::vector<RHI::Format>, RHI::Format>> formatsLookup =
        {
            {ShaderNames::GBufferFill, {{GBufferPass::Formats[0], GBufferPass::Formats[1], GBufferPass::Formats[2]}, GBufferPass::DepthFormat}},
            {ShaderNames::Compose,     {{RHI::Format::RGBA8_UNORM}, RHI::Format::Unknown}                                                     },
    };

    inline static RHI::Handle<RHI::GraphicsPipeline>
    CreateGraphicsPipeline(RHI::Device* device, RHI::Handle<RHI::PipelineLayout> layout, const char* name)
    {
        auto vertexShaderPath   = std::format("{}.vertex.spv", name);
        auto fragmentShaderPath = std::format("{}.fragment.spv", name);

        auto vertexModule = LoadShaderModule(device, vertexShaderPath.c_str());
        auto pixelModule  = LoadShaderModule(device, fragmentShaderPath.c_str());

        TL_defer
        {
            device->DestroyShaderModule(vertexModule);
            device->DestroyShaderModule(pixelModule);
        };
        RHI::ColorAttachmentBlendStateDesc attachmentBlendDesc =
            {
                true,
                RHI::BlendEquation::Add,
                RHI::BlendFactor::SrcAlpha,
                RHI::BlendFactor::OneMinusSrcAlpha,
                RHI::BlendEquation::Add,
                RHI::BlendFactor::One,
                RHI::BlendFactor::OneMinusSrcAlpha,
                RHI::ColorWriteMask::All,
            };
        // Fetch render target layouts from the lookup table using the shader name
        auto it = formatsLookup.find(name);
        TL_ASSERT(it != formatsLookup.end(), "No render target layout found for shader");
        const auto& [colorFormats, depthFormat] = it->second;

        RHI::GraphicsPipelineCreateInfo pipelineCI{
            .name               = name,
            .vertexShaderName   = "VSMain",
            .vertexShaderModule = vertexModule,
            .pixelShaderName    = "PSMain",
            .pixelShaderModule  = pixelModule,
            .layout             = layout,
            .vertexBufferBindings =
                {
                                       {
                        .stride     = sizeof(glm::vec3),
                        .attributes = {{.format = RHI::Format::RGB32_FLOAT}},
                    },
                                       {
                        .stride     = sizeof(glm::vec3),
                        .attributes = {{.format = RHI::Format::RGB32_FLOAT}},
                    },
                                       {
                        .stride     = sizeof(glm::vec2),
                        .attributes = {{.format = RHI::Format::RG32_FLOAT}},
                    },
                                       {
                        .stride     = sizeof(glm::mat4),
                        .stepRate   = RHI::PipelineVertexInputRate::PerInstance,
                        .attributes = {{.format = RHI::Format::RGBA32_FLOAT}, {.format = RHI::Format::RGBA32_FLOAT}, {.format = RHI::Format::RGBA32_FLOAT}, {.format = RHI::Format::RGBA32_FLOAT}},
                    },
                                       },
            .renderTargetLayout =
                {
                                       .colorAttachmentsFormats = colorFormats,
                                       .depthAttachmentFormat   = depthFormat,
                                       },
            .colorBlendState = {
                                       .blendStates = {attachmentBlendDesc, attachmentBlendDesc, attachmentBlendDesc},
                                       },
            .rasterizationState = {
                                       .cullMode  = RHI::PipelineRasterizerStateCullMode::BackFace,
                                       .frontFace = RHI::PipelineRasterizerStateFrontFace::Clockwise,
                                       },
            .depthStencilState = {
                                       .depthTestEnable  = true,
                                       .depthWriteEnable = true,
                                       },
        };
        return device->CreateGraphicsPipeline(pipelineCI);
    }

    RHI::Handle<RHI::ComputePipeline> CreateComputePipeline(RHI::Device* device, RHI::Handle<RHI::PipelineLayout> layout, const char* name)
    {
        auto computeShaderPath = std::format("{}.compute.spv", name);
        auto computeModule     = LoadShaderModule(device, computeShaderPath.c_str());

        RHI::ComputePipelineCreateInfo pipelineCI{
            .name         = name,
            .shaderName   = "CSMain",
            .shaderModule = computeModule,
            .layout       = layout,
        };
        return device->CreateComputePipeline(pipelineCI);
    }

    ResultCode PipelineLibrary::Init(RHI::Device* device)
    {
        m_device = device;

        // struct ShaderBindingTable
        // {
        //     ConstantBuffer<GPU::SceneView>              sceneView;                // 0
        //     StructuredBuffer<GPU::DrawRequest>          drawRequests;             // 1
        //     StructuredBuffer<GPU::StaticMeshIndexed>    indexedMeshes;            // 2
        //     StructuredBuffer<float4x4>                  transforms;               // 3
        //     StructuredBuffer<GPU::MeshMaterialBindless> materials;                // 4
        //     RWStructuredBuffer<U32>                     drawParametersCount;      // 5
        //     RWStructuredBuffer<GPU::DrawIndexedParameters> outDrawParameters;     // 6
        //     Texture2D                                   bindlessTextures[];       // 7
        // };

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI = {
            .name     = "BGL-Bindless",
            .bindings = {
                         {RHI::BindingType::UniformBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllGraphics, sizeof(GPU::SceneView)},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Write, 1, RHI::ShaderStage::AllStages},

                         // GBuffer
                {RHI::BindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},

                         // Lighting
                {RHI::BindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},

                         // compute compose
                {RHI::BindingType::StorageImage, RHI::Access::Write, 1, RHI::ShaderStage::AllStages},

                         {RHI::BindingType::SampledImage, RHI::Access::Read, RHI::BindlessArraySize, RHI::ShaderStage::AllStages},
                         },
        };
        m_bindGroupLayout = m_device->CreateBindGroupLayout(bindGroupLayoutCI);
        m_pipelineLayout  = m_device->CreatePipelineLayout({"PipelineLayout", m_bindGroupLayout});

        m_watcher.subscribe([this](const TL::FileEvent& event)
            {
                if (event.type == TL::FileEventType::Modified)
                    return false;

                TL_LOG_INFO("Reloading `{}`", event.path);

                if (auto computeIt = m_computePipelines.find(event.path); computeIt != m_computePipelines.end())
                {
                    m_device->DestroyComputePipeline(computeIt->second);
                    // m_computePipelines[event.path.data()] = CreateComputePipeline(event.path.data());

                    m_computePipelines[event.path.data()] = CreateComputePipeline(m_device, m_pipelineLayout, event.path.data());
                }
                else if (auto graphicsIt = m_graphicsPipelines.find(event.path); graphicsIt != m_graphicsPipelines.end())
                {
                    m_device->DestroyGraphicsPipeline(graphicsIt->second);
                    // m_graphicsPipelines[event.path.data()] = CreateGraphicsPipeline(event.path.data());

                    m_graphicsPipelines[event.path.data()] = CreateGraphicsPipeline(m_device, m_pipelineLayout, event.path.data());
                }
                else
                {
                    TL_UNREACHABLE();
                }

                return false;
            });

#if 0

        {
            Slang::ComPtr<slang::IGlobalSession> globalSession;
            createGlobalSession(globalSession.writeRef());
            slang::SessionDesc sessionDesc = {};

            slang::TargetDesc targetDesc = {};
            targetDesc.format            = SLANG_SPIRV;
            targetDesc.profile           = globalSession->findProfile("spirv_1_5");

            sessionDesc.targets     = &targetDesc;
            sessionDesc.targetCount = 1;
            // slang::PreprocessorMacroDesc preprocessorMacroDesc[]{
            //     // {"BIAS_VALUE",  "1138" },
            // };
            // sessionDesc.preprocessorMacros     = preprocessorMacroDesc.data();
            // sessionDesc.preprocessorMacroCount = preprocessorMacroDesc.size();

            std::array<slang::CompilerOptionEntry, 1> options = {
                {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
            };
            sessionDesc.compilerOptionEntries    = options.data();
            sessionDesc.compilerOptionEntryCount = options.size();

            Slang::ComPtr<slang::ISession> session;
            globalSession->createSession(sessionDesc, session.writeRef());

            const char* moduleName = "VSMain";
            const char* modulePath = "I:/repos/repos3/RHI/Examples/Triangle/Renderer/Shaders/GBufferPass.slang";
            // auto        sourceCode = TL::ReadTextFile(modulePath);

            Slang::ComPtr<slang::IModule> slangModule;
            Slang::ComPtr<slang::IBlob>   diagnosticsBlob;
            slangModule = session->loadModule(modulePath, diagnosticsBlob.writeRef());

            {
    #define TRY_SLANG_RETURN_ON_FAIL(x) \
        if ((x) != SLANG_OK) return ResultCode::ErrorUnknown;

                // Next we will collect all of the entry points defined in the module,
                // to form the list of components we want to link together to form
                // a program.
                //
                TL::Vector<Slang::ComPtr<slang::IComponentType>> componentsToLink;
                int                                              definedEntryPointCount = slangModule->getDefinedEntryPointCount();
                for (int i = 0; i < definedEntryPointCount; i++)
                {
                    Slang::ComPtr<slang::IEntryPoint> entryPoint;
                    TRY_SLANG_RETURN_ON_FAIL(slangModule->getDefinedEntryPoint(i, entryPoint.writeRef()));
                    componentsToLink.emplace_back(Slang::ComPtr<slang::IComponentType>(entryPoint.get()));
                }

                // Once we've collected the list of entry points we want to compose,
                // we use the Slang compilation API to compose them.
                //
                Slang::ComPtr<slang::IComponentType> composed;
                auto                                 result = session->createCompositeComponentType(
                    (slang::IComponentType**)componentsToLink.data(),
                    componentsToLink.capacity(),
                    composed.writeRef(),
                    diagnosticsBlob.writeRef());
                // diagnoseIfNeeded(diagnosticsBlob);
                TRY_SLANG_RETURN_ON_FAIL(result);

                // As the final compilation step, we will use the compilation API
                // to link the composed code. Think of this as equivalent to
                // applying the linker to a bunch of `.o` and/or `.a` files to
                // produce a binary (executable or shared library).
                //
                Slang::ComPtr<slang::IComponentType> program;
                result = composed->link(program.writeRef(), diagnosticsBlob.writeRef());
                // diagnoseIfNeeded(diagnostics);
                TRY_SLANG_RETURN_ON_FAIL(result);

                // Once the program has been compiled succcessfully, we can
                // go ahead and grab reflection data from the program.
                //
                int                   targetIndex   = 0;
                slang::ProgramLayout* programLayout = program->getLayout(targetIndex, diagnosticsBlob.writeRef());
                // diagnoseIfNeeded(diagnostics);
                if (programLayout == nullptr)
                {
                    return ResultCode::ErrorUnknown;
                }

                int  relativeSetIndex = 0;
                auto typeLayout       = programLayout->getGlobalParamsTypeLayout();
                int  rangeCount       = typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);

                auto bindingRangeIndex        = typeLayout->getSubObjectRangeBindingRangeIndex(0);
                auto bindingType              = typeLayout->getBindingRangeType(bindingRangeIndex);
                auto parameterBlockTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                auto elementTypeLayout        = parameterBlockTypeLayout->getElementTypeLayout();
                // auto range0Type               = elementTypeLayout->getBindingRangeLeafVariable(0)();
                auto name                     = elementTypeLayout->getName();
            }
        }
#endif

        return ResultCode::Success;
    }

    void PipelineLibrary::Shutdown()
    {
        for (auto [_, pipeline] : m_graphicsPipelines)
            m_device->DestroyGraphicsPipeline(pipeline);

        for (auto [_, pipeline] : m_computePipelines)
            m_device->DestroyComputePipeline(pipeline);

        if (m_pipelineLayout != RHI::NullHandle)
            m_device->DestroyPipelineLayout(m_pipelineLayout);

        if (m_bindGroupLayout != RHI::NullHandle)
            m_device->DestroyBindGroupLayout(m_bindGroupLayout);
    }

    RHI::Handle<RHI::GraphicsPipeline> PipelineLibrary::GetGraphicsPipeline(const char* name)
    {
        auto it = m_graphicsPipelines.find(name);
        if (it == m_graphicsPipelines.end())
        {
            auto vertexPath   = std::format("{}.vertex.spv", name);
            auto fragmentPath = std::format("{}.fragment.spv", name);
            m_watcher.watch(vertexPath.data(), TL::FileEventType::Modified, false);
            m_watcher.watch(fragmentPath.data(), TL::FileEventType::Modified, false);
            m_graphicsPipelines[name] = CreateGraphicsPipeline(m_device, m_pipelineLayout, name);
            it                        = m_graphicsPipelines.find(name);
        }
        return it->second;
    }

    RHI::Handle<RHI::ComputePipeline> PipelineLibrary::GetComputePipeline(const char* name)
    {
        auto it = m_computePipelines.find(name);
        if (it == m_computePipelines.end())
        {
            auto computePath = std::format("{}.compute.spv", name);
            m_watcher.watch(computePath.data(), TL::FileEventType::Modified, false);
            m_computePipelines[name] = CreateComputePipeline(m_device, m_pipelineLayout, name);
            it                       = m_computePipelines.find(name);
        }
        return it->second;
    }

    // RHI::Handle<RHI::BindGroupLayout> LoadGroupLayout(RHI::Device& device)
    // {

    // }

    // DeserializedLayouts LoadPipelineLayout(RHI::Device& device, RHI::Shader::PipelineLayoutsReflectionBlob& layouts)
    // {
    //     for (auto group : layouts.GetGroups())
    //     {

    //     }
    // }

    // RHI::ShaderModule* LoadShaderModuleFromFile(RHI::Device& device, const RHI::Shader::Spirv& module)
    // {
    //     RHI::ShaderModuleCreateInfo ci
    //     {
    //         .code = module.GetCode(),
    //     };
    //     return device.CreateShaderModule(ci);
    // };

} // namespace Engine