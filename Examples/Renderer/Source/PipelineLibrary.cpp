#include "Renderer/PipelineLibrary.hpp"
#include "Shaders/GpuCommonStructs.h"

#include <TL/Defer.hpp>

#include <glm/glm.hpp>

namespace Engine
{

    ResultCode PipelineLibrary::Init(RHI::Device* device)
    {
        m_device = device;
        return ResultCode::Success;
    }

    void PipelineLibrary::Shutdown()
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
} // namespace Engine