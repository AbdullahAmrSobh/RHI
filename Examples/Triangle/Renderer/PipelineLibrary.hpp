#pragma once

#include <RHI/RHI.hpp>

#include "Common.hpp"

#include <TL/FileSystem/File.hpp>
#include <TL/FileSystem/FileWatcher.hpp>

#include <slang/slang.h>
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-gfx.h>

namespace Engine
{
    inline static RHI::ShaderModule* LoadShaderModule(RHI::Device* device, const char* path)
    {
        auto file       = TL::File(path, TL::IOMode::Read);
        auto shaderBlob = TL::Vector<uint8_t>(file.size());
        auto [_, err]   = file.read(TL::Block::create(shaderBlob));
        TL_ASSERT(err == TL::IOResultCode::Success);
        auto module = device->CreateShaderModule({
            .name = path,
            .code = {(uint32_t*)shaderBlob.data(), shaderBlob.size() / size_t(4)},
        });
        return module;
    }

    namespace Bindings
    {
        inline static constexpr uint32_t SceneView           = 0;
        inline static constexpr uint32_t DrawRequests        = 1;
        inline static constexpr uint32_t IndexedMeshes       = 2;
        inline static constexpr uint32_t Transforms          = 3;
        inline static constexpr uint32_t Materials           = 4;
        inline static constexpr uint32_t DrawParametersCount = 5;
        inline static constexpr uint32_t OutDrawParameters   = 6;

        // Pass bind points
        inline static constexpr uint32_t gBuffer_wsPosition = 7;
        inline static constexpr uint32_t gBuffer_normal     = 8;
        inline static constexpr uint32_t gBuffer_material   = 9;
        inline static constexpr uint32_t gBuffer_depth      = 10;

        inline static constexpr uint32_t lighting_input = 11;
        inline static constexpr uint32_t compose_output = 12;

        // Bindless (needs to be last)
        inline static constexpr uint32_t BindlessTextures = 13;
    }; // namespace Bindings

    namespace ShaderNames
    {
        inline constexpr const char* Cull        = "./Shaders/Cull";        // compute
        inline constexpr const char* GBufferFill = "./Shaders/GBufferPass"; // graphics
        inline constexpr const char* Lighting    = "./Shaders/Lighting";    // compute
        inline constexpr const char* Compose     = "./Shaders/Compose";     // graphics
    }; // namespace ShaderNames

    /// A class which holds all the pipelines that are used by the renderer
    /// Also handles hot reloading of the pipelines when the shader files change
    class PipelineLibrary final : public Singleton<PipelineLibrary>
    {
    public:
        ResultCode Init(RHI::Device* device);
        void       Shutdown();

        RHI::GraphicsPipeline* GetGraphicsPipeline(const char* name);
        RHI::ComputePipeline*  GetComputePipeline(const char* name);

        RHI::BindGroupLayout* GetBindGroupLayout() const
        {
            return m_bindGroupLayout;
        }

    private:
        RHI::Device*    m_device;
        TL::FileWatcher m_watcher;

        RHI::BindGroupLayout* m_bindGroupLayout;
        RHI::PipelineLayout*  m_pipelineLayout;

        TL::Map<TL::String, RHI::GraphicsPipeline*> m_graphicsPipelines;
        TL::Map<TL::String, RHI::ComputePipeline*>  m_computePipelines;
    };
} // namespace Engine