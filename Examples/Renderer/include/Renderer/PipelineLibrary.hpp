#pragma once

#include <RHI/RHI.hpp>

#include "Renderer/Common.hpp"

#include <TL/FileSystem/File.hpp>
#include <TL/FileSystem/FileWatcher.hpp>

namespace Engine
{
    class PipelineLibrary final : public Singleton<PipelineLibrary>
    {
    public:
        TL::Error init(RHI::Device* device);
        void      shutdown();

        static RHI::ShaderModule* LoadShaderModule(TL::StringView path);

        using AcquireGraphicsPipelineCB = std::function<void(RHI::Device* device, RHI::ShaderModule* vs, RHI::ShaderModule* ps)>;
        using AcquireComputePipelineCB  = std::function<void(RHI::Device* device, RHI::ShaderModule* cs)>;

        void acquireGraphicsPipeline(TL::StringView vertexShaderPath, TL::StringView pixelShaderPath, AcquireGraphicsPipelineCB&& cb);

        void acquireComputePipeline(TL::StringView computeShaderPath, AcquireComputePipelineCB&& cb);

        void updatePipelinesIfChanged();

    private:
        void invokeGraphicsCallback(TL::StringView path);
        void invokeComputeCallback(TL::StringView path);

    private:
        RHI::Device*    m_device;
        TL::FileWatcher m_gfxWatcher;
        TL::FileWatcher m_cmpWatcher;

        // Maps vs and ps to vs\nps
        TL::Map<TL::String, TL::String> m_gfxStageToCombinedStagePaths;

        TL::Map<TL::String, AcquireGraphicsPipelineCB> m_graphicsHandler;
        TL::Map<TL::String, AcquireComputePipelineCB>  m_computeHandler;
    };
} // namespace Engine