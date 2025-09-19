#include "Renderer/PipelineLibrary.hpp"
#include "Shaders/GpuCommonStructs.h"

#include <TL/Defer.hpp>

#include <glm/glm.hpp>

namespace Engine
{
    inline static std::pair<TL::String, TL::String> splitPath(TL::StringView p)
    {
        auto s1 = p.substr(0, p.find("\n"));
        auto s2 = p.substr(p.find("\n") + 1, p.size());
        return std::pair{TL::String(s1), TL::String(s2)};
    }

    TL::Error PipelineLibrary::init(RHI::Device* device)
    {
        m_device = device;

        m_gfxWatcher.subscribe([this](TL::FileEvent event)
            {
                auto combinedPath = m_gfxStageToCombinedStagePaths.find(event.path)->second;
                invokeGraphicsCallback(combinedPath);
                return true;
            });

        m_cmpWatcher.subscribe([this](TL::FileEvent event)
            {
                invokeComputeCallback(event.path);
                return true;
            });

        return TL::NoError;
    }

    void PipelineLibrary::shutdown()
    {
    }

    RHI::ShaderModule* PipelineLibrary::LoadShaderModule(TL::StringView path)
    {
        auto file       = TL::File(path, TL::IOMode::Read);
        auto shaderBlob = TL::Vector<uint8_t>(file.size());
        auto [_, err]   = file.read(TL::Block::create(shaderBlob));
        TL_ASSERT(err == TL::IOResultCode::Success);
        auto module = PipelineLibrary::ptr->m_device->CreateShaderModule({
            .name = path.data(),
            .code = {(uint32_t*)shaderBlob.data(), shaderBlob.size() / size_t(4)},
        });
        return module;
    }

    void PipelineLibrary::acquireGraphicsPipeline(
        TL::StringView              vertexShaderPath,
        TL::StringView              pixelShaderPath,
        AcquireGraphicsPipelineCB&& cb)
    {
        m_gfxWatcher.watch(vertexShaderPath, TL::FileEventType::Modified, false);
        m_gfxWatcher.watch(pixelShaderPath, TL::FileEventType::Modified, false);

        auto joinedPaths = std::format("{}\n{}", vertexShaderPath, pixelShaderPath);

        m_gfxStageToCombinedStagePaths[vertexShaderPath.data()] = joinedPaths;
        m_gfxStageToCombinedStagePaths[pixelShaderPath.data()] = joinedPaths;

        m_graphicsHandler[joinedPaths.data()] = cb;
        invokeGraphicsCallback(joinedPaths);
    }

    void PipelineLibrary::acquireComputePipeline(
        TL::StringView             computeShaderPath,
        AcquireComputePipelineCB&& cb)
    {
        m_cmpWatcher.watch(computeShaderPath, TL::FileEventType::Modified, false);
        m_computeHandler[computeShaderPath.data()] = cb;
        invokeComputeCallback(computeShaderPath);
    }

    void PipelineLibrary::updatePipelinesIfChanged()
    {
        m_gfxWatcher.poll();
        m_cmpWatcher.poll();
    }

    void PipelineLibrary::invokeGraphicsCallback(TL::StringView path)
    {
        auto [vertexModulePath, pixelModulePath] = splitPath(path);
        auto vertexModule                        = LoadShaderModule(vertexModulePath);
        auto pixelModule                         = LoadShaderModule(pixelModulePath);
        if (auto it = m_graphicsHandler.find(path.data()); it != m_graphicsHandler.end())
        {
            TL_LOG_INFO("(Re)Loading: {}", path);
            it->second(m_device, vertexModule, pixelModule);
        }
        m_device->DestroyShaderModule(vertexModule);
        m_device->DestroyShaderModule(pixelModule);
    }

    void PipelineLibrary::invokeComputeCallback(TL::StringView path)
    {
        auto computeModule = LoadShaderModule(path);
        if (auto it = m_computeHandler.find(path.data()); it != m_computeHandler.end())
        {
            TL_LOG_INFO("(Re)Loading: {}", path);
            it->second(m_device, computeModule);
        }
        m_device->DestroyShaderModule(computeModule);
    }

} // namespace Engine