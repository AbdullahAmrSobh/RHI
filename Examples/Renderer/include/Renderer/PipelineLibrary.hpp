#pragma once

#include <RHI/RHI.hpp>

#include "Renderer/Common.hpp"

#include <TL/FileSystem/File.hpp>
#include <TL/FileSystem/FileWatcher.hpp>

namespace Engine
{
    using GraphicsPipelineInitCB = TL::Function<RHI::GraphicsPipeline*(const char* path)>;
    using ComputePipelineInitCB  = TL::Function<RHI::GraphicsPipeline*(const char* path)>;

    /// A class which holds all the pipelines that are used by the renderer
    /// Also handles hot reloading of the pipelines when the shader files change
    class PipelineLibrary final : public Singleton<PipelineLibrary>
    {
    public:
        ResultCode Init(RHI::Device* device);
        void       Shutdown();

        static RHI::ShaderModule* LoadShaderModule(TL::StringView path);

        // RHI::GraphicsPipeline* GetGraphicsPipeline(const char* name, GraphicsPipelineInitCB&& cb);
        // RHI::ComputePipeline*  GetComputePipeline(const char* name, ComputePipelineInitCB&& cb);

    private:
        RHI::Device*    m_device;
        TL::FileWatcher m_watcher;

        RHI::BindGroupLayout* m_bindGroupLayout;
        RHI::PipelineLayout*  m_pipelineLayout;

        TL::Map<TL::String, RHI::GraphicsPipeline*> m_graphicsPipelines;
        TL::Map<TL::String, RHI::ComputePipeline*>  m_computePipelines;
    };
} // namespace Engine