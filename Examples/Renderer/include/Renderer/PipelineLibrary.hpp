#pragma once

#include <RHI/RHI.hpp>

#include "Renderer/Common.hpp"

#include <TL/FileSystem/File.hpp>
#include <TL/FileSystem/FileWatcher.hpp>

namespace Engine
{
    class Shader
    {
    public:
        Shader(RHI::BindGroupLayout* layout, RHI::PipelineLayout* pipelineLayout)
            : m_bindGroupLayout(layout)
            , m_pipelineLayout(pipelineLayout)
        {
        }

        RHI::BindGroupLayout* getBindGroupLayout(uint32_t group) { return m_bindGroupLayout; }

        RHI::PipelineLayout* getPipelineLayout() { return m_pipelineLayout; }

    protected:
        RHI::BindGroupLayout* m_bindGroupLayout;
        RHI::PipelineLayout*  m_pipelineLayout;
    };

    class GraphicsShader final : public Shader
    {
    public:
        GraphicsShader(RHI::BindGroupLayout* layout, RHI::PipelineLayout* pipelineLayout, RHI::GraphicsPipeline* pipeline)
            : Shader(layout, pipelineLayout)
            , m_pipeline(pipeline)
        {
        }

        RHI::GraphicsPipeline* getPipeline() { return m_pipeline; }

    private:
        RHI::GraphicsPipeline* m_pipeline;
    };

    class ComputeShader final : public Shader
    {
    public:
        ComputeShader(RHI::BindGroupLayout* layout, RHI::PipelineLayout* pipelineLayout, RHI::ComputePipeline* pipeline)
            : Shader(layout, pipelineLayout)
            , m_pipeline(pipeline)
        {
        }

        RHI::ComputePipeline* getPipeline() { return m_pipeline; }

    private:
        RHI::ComputePipeline* m_pipeline;
    };

    class PipelineLibrary final : public Singleton<PipelineLibrary>
    {
    public:
        TL::Error init(RHI::Device* device);
        void      shutdown();

        static RHI::ShaderModule* LoadShaderModule(TL::StringView path);

        void updatePipelinesIfChanged();

        template<typename ShaderBindGroupStruct>
        TL::Ptr<GraphicsShader> acquireGraphicsPipeline(TL::StringView view, TL::Span<const RHI::PipelineVertexBindingDesc> vertexBindings, RHI::PipelineRenderTargetLayout renderPassLayout)
        {
            auto bindGroupLayout = ShaderBindGroupStruct::createBindGroupLayout(m_device);
            return acquireGraphicsPipeline(view, bindGroupLayout, vertexBindings, renderPassLayout);
        }

        template<typename ShaderBindGroupStruct>
        TL::Ptr<ComputeShader> acquireComputePipeline(TL::StringView view)
        {
            auto bindGroupLayout = ShaderBindGroupStruct::createBindGroupLayout(m_device);
            return acquireComputePipeline(view, bindGroupLayout);
        }

    private:
        TL::Ptr<GraphicsShader> acquireGraphicsPipeline(TL::StringView view, RHI::BindGroupLayout* layout, TL::Span<const RHI::PipelineVertexBindingDesc> vertexBindings, RHI::PipelineRenderTargetLayout renderPassLayout);

        TL::Ptr<ComputeShader> acquireComputePipeline(TL::StringView view, RHI::BindGroupLayout* layout);

    private:
        RHI::Device* m_device;

        TL::FileWatcher m_watcher;

        TL::Map<TL::String, GraphicsShader*> m_graphicsHandler;
        TL::Map<TL::String, ComputeShader*>  m_computeHandler;
    };
} // namespace Engine