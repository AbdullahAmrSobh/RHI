#include "RHI/Resources.hpp"

namespace RHI
{

    void ShaderBindGroupData::BindImages(uint32_t index, TL::Span<ImagePassAttachment*> images, uint32_t arrayOffset)
    {
    }

    void ShaderBindGroupData::BindBuffers(uint32_t index, TL::Span<BufferPassAttachment*> buffers, uint32_t arrayOffset)
    {
    }

    void ShaderBindGroupData::BindSamplers(uint32_t index, TL::Span<Handle<Sampler>> samplers, uint32_t arrayOffset)
    {
    }

} // namespace RHI